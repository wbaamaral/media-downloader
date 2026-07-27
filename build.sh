#!/usr/bin/env bash
# ==============================================================================
# Nome:        build.sh
# Descrição:   Build unificado + empacotamento (tarball/DEB/RPM) para Linux
# Autor:       wba-skill-sysadm
# Data:        $(date +%F)
# Uso:         ./build.sh [--qt5|--qt6] [--clean] [--install-deps] [--package FORMAT] [--jobs N] [--run] [--dry-run]
# Dependências: bash 4+, cmake, gcc/g++, qt5-base-dev ou qt6-base-dev
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
BUILD_DIR="$SCRIPT_DIR/build"
DIST_DIR="$SCRIPT_DIR/dist"
LOG_DIR="$SCRIPT_DIR/logs"
mkdir -p "$LOG_DIR" "$DIST_DIR"

LOG_FILE="$LOG_DIR/build-$(date +%Y%m%d-%H%M%S).log"
exec > >(tee -a "$LOG_FILE") 2>&1
info "Log: $LOG_FILE"

# ── Argumentos ────────────────────────────────────────────────────────────────
DRY_RUN=false
CLEAN=false
INSTALL_DEPS=false
RUN_AFTER=false
PACKAGE_FORMAT=""
QT_VERSION=""
JOBS="$(nproc)"

usage() {
    cat <<EOF
Uso: $0 [OPÇÕES]

Opções:
  --qt5              Forçar build com Qt5
  --qt6              Forçar build com Qt6
  --clean            Limpar build/ antes de configurar
  --install-deps     Instalar dependências do sistema (requer root)
  --package FORMAT   Empacotar: tarball, deb, rpm, all
  --jobs N           Paralelismo (padrão: $(nproc))
  --run              Executar após build
  --dry-run          Mostrar comandos sem executar
  -h, --help         Mostrar esta ajuda

Exemplos:
  $0                                 # Build simples (auto-detect Qt)
  $0 --qt6 --clean                   # Build limpo com Qt6
  $0 --install-deps --package all    # Instalar deps + gerar todos os pacotes
  $0 --package deb --run             # Gerar .deb e executar
EOF
    exit 0
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --qt5)           QT_VERSION="5" ;;
        --qt6)           QT_VERSION="6" ;;
        --clean)         CLEAN=true ;;
        --install-deps)  INSTALL_DEPS=true ;;
        --package)       shift; PACKAGE_FORMAT="${1:-}" ;;
        --jobs)          shift; JOBS="${1:-$(nproc)}" ;;
        --run)           RUN_AFTER=true ;;
        --dry-run)       DRY_RUN=true ;;
        -h|--help)       usage ;;
        *) die "Argumento desconhecido: $1 (use --help)" ;;
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

# ── Verificação de dependências ────────────────────────────────────────────────
check_deps() {
    local missing=()
    for dep in "$@"; do
        command -v "$dep" &>/dev/null || missing+=("$dep")
    done
    if [[ ${#missing[@]} -gt 0 ]]; then
        die "Dependências não encontradas: ${missing[*]} (use --install-deps)"
    fi
}

# ── Detecção de distro ────────────────────────────────────────────────────────
detect_distro() {
    if [[ ! -f /etc/os-release ]]; then
        die "/etc/os-release não encontrado. Distro não suportada."
    fi

    # shellcheck disable=SC1091
    source /etc/os-release

    DISTRO_ID="${ID:-unknown}"
    DISTRO_LIKE="${ID_LIKE:-$ID}"

    info "Distro detectada: ${PRETTY_NAME:-$DISTRO_ID}"
}

# ── Instalação de dependências ────────────────────────────────────────────────
install_deps_debian() {
    title "Instalando dependências (Debian/Ubuntu)"
    run "sudo apt update"
    run "sudo apt install -y build-essential cmake git \
        qtbase5-dev qtdeclarative5-dev \
        qml-module-qtquick-controls2 qml-module-qtwebsockets \
        qml-module-qtwebchannel qttools5-dev qttools5-dev-tools"
}

install_deps_fedora() {
    title "Instalando dependências (Fedora/RHEL)"
    run "sudo dnf -y install gcc gcc-c++ make cmake git \
        qt5-qtbase-devel qt5-qtdeclarative-devel \
        qt5-qtquickcontrols2-devel qt5-qtwebsockets-devel \
        qt5-qtwebchannel-devel qt5-qttools-devel"
}

install_deps_arch() {
    title "Instalando dependências (Arch/Manjaro)"
    run "sudo pacman -Syu --needed --noconfirm --noprogressbar \
        base-devel cmake git \
        qt5-base qt5-declarative qt5-quickcontrols2 \
        qt5-websockets qt5-webchannel qt5-tools"
}

install_deps() {
    detect_distro
    case "$DISTRO_ID" in
        debian|ubuntu|linuxmint|pop)    install_deps_debian ;;
        fedora|rhel|centos|rocky|alma)  install_deps_fedora ;;
        arch|manjaro|biglinux|endeavouros) install_deps_arch ;;
        *) die "Distro '$DISTRO_ID' não suportada para instalação de deps" ;;
    esac
    ok "Dependências instaladas"
}

# ── Configuração do cmake ─────────────────────────────────────────────────────
configure_cmake() {
    title "Configurando CMake"

    local cmake_args=("-DCMAKE_BUILD_TYPE=Release")

    if [[ -n "$QT_VERSION" ]]; then
        if [[ "$QT_VERSION" == "6" ]]; then
            cmake_args+=("-DBUILD_WITH_QT6=ON")
        else
            cmake_args+=("-DBUILD_WITH_QT6=OFF")
        fi
        info "Qt forçado: Qt${QT_VERSION}"
    fi

    run "cmake ${cmake_args[*]} -B '$BUILD_DIR' -S '$SCRIPT_DIR'"
}

# ── Build ─────────────────────────────────────────────────────────────────────
build_project() {
    title "Compilando (${JOBS} jobs)"
    run "cmake --build '$BUILD_DIR' -j '${JOBS}'"
    ok "Build concluído"
}

# ── Empacotamento ─────────────────────────────────────────────────────────────
package_tarball() {
    title "Gerando tarball"
    local ver
    ver="$(< "$BUILD_DIR/VERSION_INFO")"
    local name="media-downloader-${ver}"
    local staging="$BUILD_DIR/_pkg_staging"

    run "rm -rf '$staging'"
    run "cmake --install '$BUILD_DIR' --prefix '$staging/usr'"
    run "tar -C '$staging' -cJf '$DIST_DIR/${name}.tar.xz' ."
    ok "Tarball: $DIST_DIR/${name}.tar.xz"
}

package_deb() {
    title "Gerando pacote DEB"
    run "cd '$BUILD_DIR' && cpack -G DEB"
    run "mv '$BUILD_DIR'/*.deb '$DIST_DIR/'"
    ok "DEB gerado em $DIST_DIR/"
}

package_rpm() {
    title "Gerando pacote RPM"
    run "cd '$BUILD_DIR' && cpack -G RPM"
    run "mv '$BUILD_DIR'/*.rpm '$DIST_DIR/'"
    ok "RPM gerado em $DIST_DIR/"
}

package_all() {
    package_tarball
    package_deb
    package_rpm
}

do_package() {
    case "$PACKAGE_FORMAT" in
        tarball)  package_tarball ;;
        deb)      package_deb ;;
        rpm)      package_rpm ;;
        all)      package_all ;;
        "")       ;;  # sem empacotamento
        *) die "Formato desconhecido: $PACKAGE_FORMAT (use: tarball, deb, rpm, all)" ;;
    esac
}

# ── Trap de erro ──────────────────────────────────────────────────────────────
trap 'fail "Erro na linha $LINENO. Build interrompido."' ERR

# ── Execução ──────────────────────────────────────────────────────────────────
if $DRY_RUN; then
    warn "MODO DRY-RUN: nenhuma alteração será aplicada."
fi

# Verificar deps básicas
check_deps cmake gcc g++

# Instalar deps (se solicitado)
if $INSTALL_DEPS; then
    install_deps
fi

# Limpar build dir (se solicitado)
if $CLEAN; then
    title "Limpando build/"
    run "rm -rf '$BUILD_DIR'"
fi

# Pipeline principal
configure_cmake
build_project
do_package

# Executar (se solicitado)
if $RUN_AFTER; then
    title "Executando media-downloader"
    run "'$BUILD_DIR/media-downloader'" &
fi

# ── Sucesso ───────────────────────────────────────────────────────────────────
trap - ERR
echo ""
title "BUILD CONCLUÍDO"
ok "Binário: $BUILD_DIR/media-downloader"
[[ -n "$PACKAGE_FORMAT" ]] && ok "Pacotes em: $DIST_DIR/"
ok "Log completo: $LOG_FILE"
$DRY_RUN && warn "Dry-run: reexecute sem --dry-run para aplicar."
