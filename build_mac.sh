#!/bin/bash

################################################################################
# CRYPTOGRAM Desktop Build Script (macOS)
# Builds CRYPTOGRAM Qt/C++ Desktop application for macOS
################################################################################

set -Eeuo pipefail

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CRYPTOGRAM_ROOT="${CRYPTOGRAM_ROOT:-$SCRIPT_DIR}"
BUILD_TYPE="${BUILD_TYPE:-Release}"
BUILD_TYPE_LOWER=$(echo "$BUILD_TYPE" | tr '[:upper:]' '[:lower:]')
BUILD_DIR="${BUILD_DIR:-$CRYPTOGRAM_ROOT/build_${BUILD_TYPE_LOWER}}"
JOBS="${JOBS:-$(sysctl -n hw.ncpu)}"
LOG_DIR="/tmp/cryptogram_builds_root"
BUILD_DATE=$(date +%Y%m%d_%H%M%S)
BUILD_LOG="$LOG_DIR/mac_build_$BUILD_DATE.log"

# Colors
if [ -t 1 ] && [ "${TERM:-}" != "dumb" ] && [ -z "${NO_COLOR:-}" ]; then
    RED='\033[0;31m'
    GREEN='\033[0;32m'
    YELLOW='\033[1;33m'
    BLUE='\033[0;34m'
    CYAN='\033[0;36m'
    MAGENTA='\033[0;35m'
    NC='\033[0m'
else
    RED=''
    GREEN=''
    YELLOW=''
    BLUE=''
    CYAN=''
    MAGENTA=''
    NC=''
fi

mkdir -p "$LOG_DIR"

print_header() {
    echo -e "${MAGENTA}╔════════════════════════════════════════════════════════════════╗${NC}"
    echo -e "${MAGENTA}║${NC} $1"
    echo -e "${MAGENTA}╚════════════════════════════════════════════════════════════════╝${NC}"
}

print_section() {
    echo -e "\n${CYAN}┌────────────────────────────────────────────────────────────────┐${NC}"
    echo -e "${CYAN}│${NC} $1"
    echo -e "${CYAN}└────────────────────────────────────────────────────────────────┘${NC}"
}

print_info() {
    echo -e "${GREEN}✓${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}⚠${NC} $1"
}

print_error() {
    echo -e "${RED}✗${NC} $1"
}

print_progress() {
    echo -e "${BLUE}→${NC} $1"
}

fail() {
    print_error "$1"
    exit 1
}

START_TIME=$(date +%s)

{

print_header "CRYPTOGRAM Desktop — macOS Build"

print_info "Build type: $BUILD_TYPE"
print_info "Build dir:  $BUILD_DIR"
print_info "Jobs:       $JOBS"
print_info "Root:       $CRYPTOGRAM_ROOT"

# Check for Qt6
print_section "Checking Dependencies"

if ! command -v qmake6 &>/dev/null && ! command -v qmake &>/dev/null; then
    print_warning "Qt6 not found in PATH. Attempting brew install..."
    brew install qt6 2>/dev/null || true
    export CMAKE_PREFIX_PATH="$(brew --prefix qt6):${CMAKE_PREFIX_PATH:-}"
fi

if ! command -v cmake &>/dev/null; then
    print_warning "CMake not found. Attempting brew install..."
    brew install cmake 2>/dev/null || true
fi

if ! command -v ninja &>/dev/null; then
    print_warning "Ninja not found. Attempting brew install..."
    brew install ninja 2>/dev/null || true
fi

# Set up environment
export CMAKE_PREFIX_PATH="$(brew --prefix qt6 2>/dev/null || echo /opt/homebrew):${CMAKE_PREFIX_PATH:-}"

print_section "CMake Configuration"

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

print_progress "Configuring CMake..."
if ! cmake \
    -G Ninja \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DDESKTOP_APP_DISABLE_AUTOUPDATE=ON \
    -DDESKTOP_APP_DISABLE_CRASH_REPORTS=ON \
    -DDESKTOP_APP_USE_PACKAGED=ON \
    -DTDESKTOP_API_ID="${TDESKTOP_API_ID:-17349}" \
    -DTDESKTOP_API_HASH="${TDESKTOP_API_HASH:-344583e45741c457fe1862106095a5eb}" \
    "$CRYPTOGRAM_ROOT/Telegram"; then
    fail "CMake configuration failed"
fi

print_section "Building"

print_progress "Building with $JOBS jobs..."
if ! cmake --build . --parallel "$JOBS"; then
    fail "Build failed"
fi

print_info "Build complete"

# Locate the .app bundle
APP_BUNDLE=""
for candidate in \
    "$BUILD_DIR/Debug/Cryptogram.app" \
    "$BUILD_DIR/Release/Cryptogram.app" \
    "$BUILD_DIR/Cryptogram.app" \
    "$BUILD_DIR/bin/Cryptogram.app"; do
    if [ -d "$candidate" ]; then
        APP_BUNDLE="$candidate"
        break
    fi
done

if [ -z "$APP_BUNDLE" ]; then
    APP_BUNDLE=$(find "$BUILD_DIR" -name "Cryptogram.app" -type d 2>/dev/null | head -1)
fi

if [ -z "$APP_BUNDLE" ]; then
    # Look for Telegram.app (in case output_name wasn't changed)
    APP_BUNDLE=$(find "$BUILD_DIR" -name "Telegram.app" -type d 2>/dev/null | head -1)
fi

if [ -n "$APP_BUNDLE" ]; then
    print_info "Found app bundle: $APP_BUNDLE"

    # Create DMG
    print_section "Packaging DMG"

    DMG_NAME="cryptogram-desktop_${BUILD_TYPE}.dmg"
    DMG_PATH="$BUILD_DIR/$DMG_NAME"

    # Remove existing DMG
    rm -f "$DMG_PATH"

    # Create a temporary DMG folder
    DMG_STAGING="$BUILD_DIR/dmg_staging"
    rm -rf "$DMG_STAGING"
    mkdir -p "$DMG_STAGING"
    cp -R "$APP_BUNDLE" "$DMG_STAGING/"
    ln -s /Applications "$DMG_STAGING/Applications"

    # Create DMG
    hdiutil create -volname "CRYPTOGRAM" -srcfolder "$DMG_STAGING" -ov -format UDZO "$DMG_PATH" 2>/dev/null && {
        print_info "DMG created: $DMG_PATH"
        DMG_SIZE=$(du -h "$DMG_PATH" | cut -f1)
        print_info "DMG size: $DMG_SIZE"
    } || print_warning "DMG creation failed — app bundle is still available at $APP_BUNDLE"

    rm -rf "$DMG_STAGING"
else
    print_warning "No .app bundle found. Check $BUILD_DIR for output."
    # List what we got
    find "$BUILD_DIR" -maxdepth 3 -name "*.app" -o -name "cryptogram*" -o -name "Telegram" 2>/dev/null | head -10
fi

END_TIME=$(date +%s)
DURATION=$((END_TIME - START_TIME))

print_header "✓ BUILD SUCCESSFUL"

echo ""
echo "═══════════════════════════════════════════════════════════════════"
echo "BUILD SUMMARY"
echo "═══════════════════════════════════════════════════════════════════"
echo ""
echo "Build time: $((DURATION / 60))m $((DURATION % 60))s"
echo "Build type: $BUILD_TYPE"
echo ""
if [ -n "${APP_BUNDLE:-}" ]; then
    echo "App bundle: $APP_BUNDLE"
fi
if [ -f "${DMG_PATH:-}" ]; then
    echo "DMG:        $DMG_PATH"
fi
echo ""
echo "═══════════════════════════════════════════════════════════════════"
echo "NEXT STEPS"
echo "═══════════════════════════════════════════════════════════════════"
echo ""
if [ -n "${APP_BUNDLE:-}" ]; then
    echo "Open CRYPTOGRAM:"
    echo "  open $APP_BUNDLE"
else
    echo "  Check $BUILD_DIR for binaries"
fi
echo ""
echo "═══════════════════════════════════════════════════════════════════"
echo ""

} 2>&1 | tee "$BUILD_LOG"

print_info "Build log saved to: $BUILD_LOG"
echo ""
