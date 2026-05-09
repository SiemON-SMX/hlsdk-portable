#!/usr/bin/env bash
# build-android.sh — Build WantedHL .so files for Android (32-bit and 64-bit)
# Usage:
#   ./build-android.sh                        # uses $ANDROID_NDK or $ANDROID_NDK_HOME
#   ./build-android.sh /path/to/ndk           # explicit NDK path
#   ./build-android.sh /path/to/ndk Release   # explicit NDK + build type (Release/Debug)
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

# --- NDK resolution ---
NDK_PATH="${1:-${ANDROID_NDK_HOME:-${ANDROID_NDK:-}}}"
BUILD_TYPE="${2:-Release}"

if [ -z "$NDK_PATH" ]; then
  echo ""
  echo "ERROR: Android NDK not found."
  echo "  Set ANDROID_NDK_HOME or ANDROID_NDK env var, or pass the path as the first argument:"
  echo "    ./build-android.sh /path/to/android-ndk-r25c"
  echo ""
  exit 1
fi

if [ ! -f "$NDK_PATH/build/cmake/android.toolchain.cmake" ]; then
  echo "ERROR: '$NDK_PATH' does not look like a valid NDK directory."
  echo "  Expected to find: $NDK_PATH/build/cmake/android.toolchain.cmake"
  exit 1
fi

echo "Using NDK:   $NDK_PATH"
echo "Build type:  $BUILD_TYPE"
echo ""

# --- Dependency check ---
for cmd in cmake ninja; do
  if ! command -v "$cmd" &>/dev/null; then
    echo "ERROR: '$cmd' is not installed or not in PATH."
    echo "  On Ubuntu/Debian:  sudo apt install cmake ninja-build"
    echo "  On macOS:          brew install cmake ninja"
    exit 1
  fi
done

JOBS=$(nproc 2>/dev/null || sysctl -n hw.logicalcpu 2>/dev/null || echo 4)

build_abi() {
  local ABI="$1"
  local TOOLCHAIN="$SCRIPT_DIR/cmake/android-${ABI}.cmake"
  local BUILD_DIR="$SCRIPT_DIR/build-${ABI}"
  local DIST_DIR="$SCRIPT_DIR/dist/${ABI}"

  echo "=========================================="
  echo "  Configuring $ABI"
  echo "=========================================="

  cmake -G Ninja -S "$SCRIPT_DIR" -B "$BUILD_DIR" \
    -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN" \
    -DANDROID_NDK="$NDK_PATH" \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DANDROID_APK=ON \
    -DCMAKE_INSTALL_PREFIX="$DIST_DIR"

  echo ""
  echo "  Building $ABI (using $JOBS jobs)..."
  cmake --build "$BUILD_DIR" --parallel "$JOBS"

  echo ""
  echo "  Installing $ABI..."
  cmake --install "$BUILD_DIR"

  echo ""
  echo "  Done: $ABI  →  $DIST_DIR"
  echo ""
}

build_abi armeabi-v7a
build_abi arm64-v8a

echo "=========================================="
echo "  All builds complete!"
echo "=========================================="
echo ""
echo "Output:"
find "$SCRIPT_DIR/dist" -name "*.so" | sort | while read -r f; do
  size=$(du -sh "$f" 2>/dev/null | cut -f1)
  echo "  [$size]  $f"
done
echo ""
