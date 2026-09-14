# shellcheck shell=bash
# ---------------------------------------------------------------------------
# common.sh – shared helpers for the ENIGMA use cases
#
# Every use case `run.sh` does:  source "$(dirname ...)/../common.sh"
# It gives you:
#   ENIGMA_ROOT   absolute path to the repository root
#   BUILD_DIR     $ENIGMA_ROOT/build
#   ensure_built <target> [<target> ...]   configure + compile if missing
#   gen_platform <platform_generator args...>
#   run_enigma   <binary> [args...]        run a built binary from $ENIGMA_ROOT
# SimGrid is auto-detected and added to LD_LIBRARY_PATH.
# ---------------------------------------------------------------------------
set -euo pipefail

_UC_COMMON_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ENIGMA_ROOT="$( cd "$_UC_COMMON_DIR/.." && pwd )"
BUILD_DIR="$ENIGMA_ROOT/build"

# --- Locate a SimGrid 4.1+ installation ------------------------------------
_detect_simgrid() {
    local c
    for c in "${SIMGRID_ROOT:-}" /opt/simgrid-4.1 /usr/local /usr /opt; do
        [ -n "$c" ] || continue
        if [ -f "$c/lib/libsimgrid.so" ] || [ -f "$c/lib64/libsimgrid.so" ]; then
            SIMGRID_PREFIX="$c"; return 0
        fi
    done
    if command -v pkg-config >/dev/null 2>&1 && pkg-config --exists simgrid 2>/dev/null; then
        SIMGRID_PREFIX="$(pkg-config --variable=prefix simgrid)"; return 0
    fi
    SIMGRID_PREFIX=""
}
_detect_simgrid
if [ -n "${SIMGRID_PREFIX:-}" ]; then
    export LD_LIBRARY_PATH="${SIMGRID_PREFIX}/lib:${SIMGRID_PREFIX}/lib64:${LD_LIBRARY_PATH:-}"
    echo ">> SimGrid: ${SIMGRID_PREFIX}"
else
    echo ">> SimGrid: not auto-detected – relying on the default linker path" >&2
fi

# --- Configure + build only what is needed -------------------------------
ensure_built() {
    local need_build=0 t
    for t in "$@"; do
        [ -x "$BUILD_DIR/$t" ] || need_build=1
    done
    [ "$need_build" -eq 0 ] && return 0

    echo ">> Building ENIGMA targets: $*"
    mkdir -p "$BUILD_DIR"
    ( cd "$BUILD_DIR" \
        && cmake .. -DCMAKE_BUILD_TYPE=Release \
             ${SIMGRID_PREFIX:+-DCMAKE_PREFIX_PATH="$SIMGRID_PREFIX"} >/dev/null \
        && make -j"$(nproc)" "$@" )
}

gen_platform() {
    ensure_built platform_generator
    ( cd "$ENIGMA_ROOT" && "$BUILD_DIR/platform_generator" "$@" )
}

run_enigma() {
    local bin="$1"; shift
    echo ">> running: $bin $*"
    echo "------------------------------------------------------------------"
    ( cd "$ENIGMA_ROOT" && "$BUILD_DIR/$bin" "$@" )
}
