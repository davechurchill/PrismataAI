#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SYSTEM_NAME="$(uname -s | tr '[:upper:]' '[:lower:]')"

CONFIG="Release"
BUILD_DIR="${SCRIPT_DIR}/build/cmake-${SYSTEM_NAME}"
BUILD_GUI="ON"
TARGET=""
GENERATOR=""
JOBS=""
CMAKE_ARGS=()

usage() {
    cat <<'EOF'
Usage: ./build.sh [Debug|Release|RelWithDebInfo|MinSizeRel] [options] [-- <cmake args>]

Options:
  --debug                 Build Debug configuration.
  --release               Build Release configuration.
  --relwithdebinfo        Build RelWithDebInfo configuration.
  --minsizerel            Build MinSizeRel configuration.
  --build-dir DIR         CMake build directory. Default: build/cmake-<platform>
  --gui                   Build Prismata_GUI. This requires SFML 3. Default.
  --no-gui                Skip Prismata_GUI and build only non-GUI targets.
  --target NAME           Build one CMake target, such as Prismata_Testing.
  --generator NAME        Pass a CMake generator, such as Ninja.
  -j, --parallel N        Number of parallel build jobs.
  -h, --help              Show this help.

Examples:
  ./build.sh
  ./build.sh Debug --no-gui
  ./build.sh Release -- -DSFML_DIR=/opt/homebrew/lib/cmake/SFML
  ./build.sh --generator Ninja --target Prismata_Testing
EOF
}

default_jobs() {
    if command -v nproc >/dev/null 2>&1; then
        nproc
    elif command -v sysctl >/dev/null 2>&1; then
        sysctl -n hw.ncpu
    else
        echo 4
    fi
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        Debug|Release|RelWithDebInfo|MinSizeRel)
            CONFIG="$1"
            shift
            ;;
        --debug)
            CONFIG="Debug"
            shift
            ;;
        --release)
            CONFIG="Release"
            shift
            ;;
        --relwithdebinfo)
            CONFIG="RelWithDebInfo"
            shift
            ;;
        --minsizerel)
            CONFIG="MinSizeRel"
            shift
            ;;
        --build-dir)
            BUILD_DIR="$2"
            shift 2
            ;;
        --gui)
            BUILD_GUI="ON"
            shift
            ;;
        --no-gui)
            BUILD_GUI="OFF"
            shift
            ;;
        --target)
            TARGET="$2"
            shift 2
            ;;
        --generator)
            GENERATOR="$2"
            shift 2
            ;;
        -j|--parallel)
            JOBS="$2"
            shift 2
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        --)
            shift
            CMAKE_ARGS+=("$@")
            break
            ;;
        *)
            echo "Unknown argument: $1" >&2
            usage >&2
            exit 2
            ;;
    esac
done

if [[ -z "${JOBS}" ]]; then
    JOBS="$(default_jobs)"
fi

CONFIGURE_CMD=(
    cmake
    -S "${SCRIPT_DIR}"
    -B "${BUILD_DIR}"
    -DCMAKE_BUILD_TYPE="${CONFIG}"
    -DPRISMATA_BUILD_GUI="${BUILD_GUI}"
)

if [[ -n "${GENERATOR}" ]]; then
    CONFIGURE_CMD+=(-G "${GENERATOR}")
elif command -v ninja >/dev/null 2>&1; then
    CONFIGURE_CMD+=(-G Ninja)
fi

if [[ ${#CMAKE_ARGS[@]} -gt 0 ]]; then
    CONFIGURE_CMD+=("${CMAKE_ARGS[@]}")
fi

BUILD_CMD=(
    cmake
    --build "${BUILD_DIR}"
    --config "${CONFIG}"
    --parallel "${JOBS}"
)

if [[ -n "${TARGET}" ]]; then
    BUILD_CMD+=(--target "${TARGET}")
fi

echo "Configuring ${CONFIG} build in ${BUILD_DIR}"
"${CONFIGURE_CMD[@]}"

echo "Building with ${JOBS} parallel job(s)"
"${BUILD_CMD[@]}"
