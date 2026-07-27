#!/usr/bin/env bash
# ==============================================================================
# Nome:        release.sh
# Descrição:   Build Linux + tag git + draft release no GitHub com binários
# Autor:       wba-skill-sysadm
# Data:        2026-07-27
# Uso:         ./release.sh [--version X.Y.Z] [--skip-build] [--dry-run]
# Dependências: bash 4+, gh (github-cli), git, build.sh
# ==============================================================================

set -euo pipefail
IFS=$'\n\t'

# ── Cores e helpers ────────────────────────────────────────────────────────────
readonly RED='\033[0;31m' GREEN='\033[0;32m' YELLOW='\033[1;33m'
readonly CYAN='\033[0;36m' BOLD='\033[1m' RESET='\033[0m'

info()  { echo -e "${CYAN}[INFO]${RESET}  $*"; }
ok()    { echo -e "${GREEN}[OK]${RESET}    $*"; }
warn()  { echo -e "${YELLOW}[AVISO]${RESET} $*"; }
fail()  { echo -e "${RED}[FALHA]${RESET} $*" >&2; }
title() {
    echo -e "\n${BOLD}${CYAN}══════════════════════════════════════════════${RESET}"
    echo -e "${BOLD}${CYAN}  $*${RESET}"
    echo -e "${BOLD}${CYAN}══════════════════════════════════════════════${RESET}"
}
die()   { fail "$*"; exit 1; }

# ── Diretórios ────────────────────────────────────────────────────────────────
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DIST_DIR="$SCRIPT_DIR/dist"
LOG_DIR="$SCRIPT_DIR/logs"
mkdir -p "$LOG_DIR"

LOG_FILE="$LOG_DIR/release-$(date +%Y%m%d-%H%M%S).log"
exec > >(tee -a "$LOG_FILE") 2>&1
info "Log: $LOG_FILE"

# ── Argumentos ────────────────────────────────────────────────────────────────
DRY_RUN=false
SKIP_BUILD=false
VERSION=""

while [[ $# -gt 0 ]]; do
    case "$1" in
        --version)      shift; VERSION="${1:-}" ;;
        --skip-build)   SKIP_BUILD=true ;;
        --dry-run)      DRY_RUN=true ;;
        -h|--help)
            echo "Uso: $0 [--version X.Y.Z] [--skip-build] [--dry-run]"
            exit 0
            ;;
        *) die "Argumento desconhecido: $1" ;;
    esac
    shift
done

# ── Execução com suporte a dry-run ────────────────────────────────────────────
run() {
    if $DRY_RUN; then
        echo -e "${YELLOW}[DRY]${RESET}   $*"
    else
        info "Executando: $*"
        eval "$@"
    fi
}

# ── Validações ────────────────────────────────────────────────────────────────
title "Validações"

# gh instalado
if ! command -v gh &>/dev/null; then
    die "github-cli (gh) não encontrado. Instale: pacman -S github-cli"
fi
ok "github-cli: $(gh --version | head -1)"

# gh autenticado
if ! gh auth status &>/dev/null; then
    die "gh não autenticado. Execute: gh auth login"
fi
ok "gh autenticado"

# git repo
if ! git rev-parse --is-inside-work-tree &>/dev/null; then
    die "Não é um repositório git"
fi

# branch main
CURRENT_BRANCH="$(git branch --show-current)"
if [[ "$CURRENT_BRANCH" != "main" ]]; then
    warn "Branch atual: $CURRENT_BRANCH (não é main)"
    read -rp "$(echo -e "${YELLOW}Continuar mesmo assim? [s/N]${RESET} ")" confirm
    [[ "$confirm" =~ ^[SsYy]$ ]] || die "Abortado pelo usuário"
fi
ok "Branch: $CURRENT_BRANCH"

# Versão
if [[ -z "$VERSION" ]]; then
    VERSION="$(grep 'set( PGR_VERSION' "$SCRIPT_DIR/CMakeLists.txt" | sed 's/.*"\(.*\)".*/\1/')"
    [[ -z "$VERSION" ]] && die "Não foi possível extrair versão do CMakeLists.txt"
fi
ok "Versão: $VERSION"

# Tag não existe
if git rev-parse "v$VERSION" &>/dev/null; then
    die "Tag v$VERSION já existe no repositório"
fi
ok "Tag v$VERSION disponível"

# ── Build ─────────────────────────────────────────────────────────────────────
if ! $SKIP_BUILD; then
    title "Build Linux"
    run "'$SCRIPT_DIR/build.sh' --clean --package all"
    ok "Build concluído"
else
    warn "Build pulado (--skip-build)"
fi

# ── Verificar pacotes ─────────────────────────────────────────────────────────
title "Verificando pacotes em dist/"

if [[ ! -d "$DIST_DIR" ]]; then
    die "Diretório dist/ não encontrado"
fi

PACKAGES=()
while IFS= read -r -d '' f; do
    PACKAGES+=("$f")
    info "  $(basename "$f") ($(du -h "$f" | cut -f1))"
done < <(find "$DIST_DIR" -maxdepth 1 -type f \( -name "*.tar.xz" -o -name "*.deb" -o -name "*.rpm" -o -name "*.zip" -o -name "*.exe" \) -print0)

if [[ ${#PACKAGES[@]} -eq 0 ]]; then
    die "Nenhum pacote encontrado em dist/"
fi
ok "${#PACKAGES[@]} pacote(s) encontrado(s)"

# ── Tag git ───────────────────────────────────────────────────────────────────
title "Criando tag v$VERSION"

run "git tag -a 'v$VERSION' -m 'Release v$VERSION'"
run "git push origin 'v$VERSION'"
ok "Tag v$VERSION criada e enviada"

# ── Criar release ─────────────────────────────────────────────────────────────
title "Criando draft release no GitHub"

RELEASE_ARGS=(
    "v$VERSION"
    --title "Media Downloader v$VERSION"
    --notes "## Media Downloader v$VERSION

### Binários Linux
- \`media-downloader-$VERSION.tar.xz\` — tarball
- \`media-downloader-$VERSION-Linux.deb\` — pacote Debian/Ubuntu
- \`media-downloader-$VERSION-Linux.rpm\` — pacote Fedora/RHEL

### Binários Windows (adicionar manualmente)
- \`media-downloader-$VERSION-win-x64.zip\` — portátil
- \`MediaDownloader-$VERSION.setup.exe\` — instalador

### Instalação
**Debian/Ubuntu:** \`sudo dpkg -i media-downloader-$VERSION-Linux.deb\`
**Fedora/RHEL:** \`sudo rpm -i media-downloader-$VERSION-Linux.rpm\`
**Portátil:** descompactar e executar \`media-downloader\`"
    --draft
)

# Adicionar pacotes como assets
for pkg in "${PACKAGES[@]}"; do
    RELEASE_ARGS+=("$pkg")
done

if $DRY_RUN; then
    echo -e "${YELLOW}[DRY]${RESET}   gh release create ${RELEASE_ARGS[*]}"
else
    gh release create "${RELEASE_ARGS[@]}"
fi

ok "Draft release criada"

# ── Link da release ──────────────────────────────────────────────────────────
RELEASE_URL="$(gh release view "v$VERSION" --json url -q .url 2>/dev/null || echo "https://github.com/wbaamaral/media-downloader/releases/tag/v$VERSION")"

# ── Sucesso ───────────────────────────────────────────────────────────────────
trap - ERR
echo ""
title "RELEASE CRIADA"
ok "Versão: v$VERSION"
ok "Pacotes: ${#PACKAGES[@]} arquivo(s)"
ok "Release: $RELEASE_URL"
echo ""
info "Próximos passos:"
info "  1. Build Windows: .\\build_windows.ps1 -Clean -Package"
info "  2. Upload dos bins Windows na draft release"
info "  3. Clique 'Publish release'"
ok "Log: $LOG_FILE"
