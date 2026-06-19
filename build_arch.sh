#!/usr/bin/env bash
set -Eeuo pipefail

BUILD_TESTING="${BUILD_TESTING:-ON}"
RUN_AFTER_BUILD="${RUN_AFTER_BUILD:-0}"
INSTALL_DEPS="${INSTALL_DEPS:-0}"

deps=(
    base-devel
    cmake
    git
    qt5-base
    qt5-declarative
    qt5-quickcontrols2
    qt5-tools
)

log() {
    printf '[build] %s\n' "$*"
}

die() {
    printf 'ERRO: %s\n' "$*" >&2
    exit 1
}

realpath_dir() {
    local dir="$1"
    cd "$dir" 2>/dev/null && pwd -P
}

get_script_dir() {
    local src="${BASH_SOURCE[0]}"

    while [[ -L "$src" ]]; do
        local dir
        dir="$(cd -P "$(dirname "$src")" && pwd)"
        src="$(readlink "$src")"
        [[ "$src" != /* ]] && src="$dir/$src"
    done

    cd -P "$(dirname "$src")" && pwd
}

is_cmake_project() {
    local dir="$1"
    [[ -d "$dir" && -f "$dir/CMakeLists.txt" ]]
}

find_project_dir() {
    local call_dir script_dir candidate
    call_dir="$(pwd -P)"
    script_dir="$(get_script_dir)"

    local candidates=()

    [[ -n "${PROJECT_DIR:-}" ]] && candidates+=("$PROJECT_DIR")

    candidates+=(
        "$call_dir"
        "$call_dir/media-downloader"
        "$script_dir"
        "$script_dir/media-downloader"
        "$script_dir/.."
        "$script_dir/../media-downloader"
        "$HOME/Desenvolvimento/media-downloader.dev/media-downloader"
        "$HOME/media-downloader"
    )

    local checked=()

    for candidate in "${candidates[@]}"; do
        [[ -n "$candidate" ]] || continue

        candidate="$(realpath_dir "$candidate" || true)"
        [[ -n "$candidate" ]] || continue

        # Evita repetir o mesmo caminho.
        if printf '%s\n' "${checked[@]}" | grep -Fxq "$candidate"; then
            continue
        fi

        checked+=("$candidate")

        if is_cmake_project "$candidate"; then
            printf '%s\n' "$candidate"
            return 0
        fi
    done

    {
        printf 'Não encontrei o diretório do projeto.\n'
        printf '\nLocais verificados:\n'
        printf '  - %s\n' "${checked[@]}"
        printf '\nInforme manualmente com:\n'
        printf '  PROJECT_DIR=/caminho/do/projeto %s\n' "$0"
    } >&2

    return 1
}

PROJECT_DIR="$(find_project_dir)" || exit 1
BUILD_DIR="${BUILD_DIR:-$PROJECT_DIR/build}"

log "Projeto detectado: $PROJECT_DIR"
log "Build dir: $BUILD_DIR"

if [[ "$INSTALL_DEPS" == "1" ]]; then
    log "Instalando dependências via pacman..."
    sudo pacman -Syu --needed --noconfirm --noprogressbar "${deps[@]}"
else
    log "Pulando instalação de dependências. Use INSTALL_DEPS=1 para instalar."
fi

log "Limpando build anterior..."
rm -rf "$BUILD_DIR"

log "Configurando projeto..."
cmake -S "$PROJECT_DIR" -B "$BUILD_DIR" \
    -DBUILD_TESTING="$BUILD_TESTING"

log "Compilando..."
cmake --build "$BUILD_DIR" -j"$(nproc)"

log "Executando testes..."
ctest --test-dir "$BUILD_DIR" --output-on-failure

if [[ "$RUN_AFTER_BUILD" == "1" ]]; then
    APP="$BUILD_DIR/media-downloader"

    [[ -f "$APP" ]] || die "Binário não encontrado: $APP"
    [[ -x "$APP" ]] || chmod +x "$APP"

    log "Executando aplicação..."
    "$APP"
fi
