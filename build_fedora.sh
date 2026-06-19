#!/usr/bin/env bash
set -Eeuo pipefail

APP_NAME="${APP_NAME:-media-downloader}"
BUILD_TESTING="${BUILD_TESTING:-ON}"
RUN_AFTER_BUILD="${RUN_AFTER_BUILD:-0}"
INSTALL_DEPS="${INSTALL_DEPS:-0}"
UPDATE_SYSTEM="${UPDATE_SYSTEM:-0}"

deps=(
    gcc
    gcc-c++
    make
    cmake
    git
    qt5-qtbase-devel
    qt5-qtdeclarative-devel
    qt5-qtquickcontrols2-devel
    qt5-qttools-devel
)

log() {
    printf '[build] %s\n' "$*"
}

die() {
    printf 'ERRO: %s\n' "$*" >&2
    exit 1
}

detect_dnf() {
    if [[ -n "${DNF_BIN:-}" ]]; then
        command -v "$DNF_BIN" >/dev/null 2>&1 || die "DNF_BIN inválido: $DNF_BIN"
        printf '%s\n' "$DNF_BIN"
        return 0
    fi

    if command -v dnf >/dev/null 2>&1; then
        printf 'dnf\n'
        return 0
    fi

    if command -v dnf5 >/dev/null 2>&1; then
        printf 'dnf5\n'
        return 0
    fi

    die "dnf/dnf5 não encontrado. Este script é destinado a Fedora."
}

realpath_dir() {
    local dir="$1"

    [[ -d "$dir" ]] || return 1
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

already_checked() {
    local needle="$1"
    shift || true

    local item
    for item in "$@"; do
        [[ "$item" == "$needle" ]] && return 0
    done

    return 1
}

find_project_dir() {
    local call_dir script_dir candidate resolved
    local checked=()
    local candidates=()

    call_dir="$(pwd -P)"
    script_dir="$(get_script_dir)"

    [[ -n "${PROJECT_DIR:-}" ]] && candidates+=("$PROJECT_DIR")

    candidates+=(
        "$call_dir"
        "$call_dir/$APP_NAME"
        "$script_dir"
        "$script_dir/$APP_NAME"
        "$script_dir/.."
        "$script_dir/../$APP_NAME"
        "$script_dir/../.."
        "$script_dir/../../$APP_NAME"
        "$HOME/Desenvolvimento/media-downloader.dev/$APP_NAME"
        "$HOME/$APP_NAME"
    )

    for candidate in "${candidates[@]}"; do
        [[ -n "$candidate" ]] || continue

        resolved="$(realpath_dir "$candidate" || true)"
        [[ -n "$resolved" ]] || continue

        if already_checked "$resolved" "${checked[@]}"; then
            continue
        fi

        checked+=("$resolved")

        if is_cmake_project "$resolved"; then
            printf '%s\n' "$resolved"
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

find_app_binary() {
    local candidate found

    for candidate in \
        "$BUILD_DIR/$APP_NAME" \
        "$BUILD_DIR/bin/$APP_NAME" \
        "$BUILD_DIR/src/$APP_NAME"
    do
        [[ -f "$candidate" ]] && {
            printf '%s\n' "$candidate"
            return 0
        }
    done

    found="$(
        find "$BUILD_DIR" \
            -maxdepth 3 \
            -type f \
            -name "$APP_NAME" \
            -print \
            2>/dev/null |
        head -n 1
    )"

    [[ -n "$found" ]] || return 1
    printf '%s\n' "$found"
}

PROJECT_DIR="$(find_project_dir)" || exit 1
BUILD_DIR="${BUILD_DIR:-$PROJECT_DIR/build}"

log "Projeto detectado: $PROJECT_DIR"
log "Build dir: $BUILD_DIR"

if [[ "$INSTALL_DEPS" == "1" ]]; then
    DNF="$(detect_dnf)"

    if [[ "$UPDATE_SYSTEM" == "1" ]]; then
        log "Atualizando sistema via $DNF..."
        sudo "$DNF" -y upgrade
    else
        log "Pulando atualização do sistema. Use UPDATE_SYSTEM=1 para atualizar."
    fi

    log "Instalando dependências via $DNF..."
    sudo "$DNF" -y install "${deps[@]}"
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

if [[ "$BUILD_TESTING" == "ON" || "$BUILD_TESTING" == "1" || "$BUILD_TESTING" == "TRUE" ]]; then
    log "Executando testes..."
    ctest --test-dir "$BUILD_DIR" --output-on-failure
else
    log "Testes desativados."
fi

if [[ "$RUN_AFTER_BUILD" == "1" ]]; then
    APP_BIN="$(find_app_binary)" || die "Binário não encontrado após o build: $APP_NAME"

    [[ -x "$APP_BIN" ]] || chmod +x "$APP_BIN"

    log "Executando aplicação: $APP_BIN"
    "$APP_BIN"
fi
