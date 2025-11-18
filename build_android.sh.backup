#!/bin/bash

################################################################################
# SWORDCOMM Android - Complete Build Script
# Builds Android APK/AAB with proper signing configuration
################################################################################

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
MAGENTA='\033[0;35m'
NC='\033[0m'

# Configuration
SWORDCOMM_ROOT="${1:-.}"
BUILD_DATE=$(date +%Y%m%d_%H%M%S)
LOG_FILE="/tmp/swordcomm_build_$BUILD_DATE.log"

# Signing configuration (set these before running or via environment)
KEYSTORE_PATH="${CI_KEYSTORE_PATH:-}"
KEYSTORE_PASSWORD="${CI_KEYSTORE_PASSWORD:-}"
KEYSTORE_ALIAS="${CI_KEYSTORE_ALIAS:-}"
MAPS_API_KEY="${CI_MAPS_API_KEY:-}"

# Functions
print_header() {
    echo -e "${MAGENTA}╔════════════════════════════════════════════════════════════════╗${NC}"
    echo -e "${MAGENTA}║${NC} $1"
    echo -e "${MAGENTA}╚════════════════════════════════════════════════════════════════╝${NC}"
}

print_step() {
    echo -e "\n${CYAN}┌────────────────────────────────────────────────────────────────┐${NC}"
    echo -e "${CYAN}│${NC} STEP $1: $2"
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
    echo ""
    echo "Log file: $LOG_FILE"
    echo "Last 50 lines:"
    tail -50 "$LOG_FILE" 2>/dev/null || echo "No log file"
    exit 1
}

get_time() {
    date +"%Y-%m-%d %H:%M:%S"
}

################################################################################
# MAIN
################################################################################

clear
print_header "SWORDCOMM Android Build System"

echo "Project root: $SWORDCOMM_ROOT"
echo "Log file: $LOG_FILE"
echo ""

# Check if SWORDCOMM exists
if [ ! -d "$SWORDCOMM_ROOT" ]; then
    fail "SWORDCOMM directory not found at: $SWORDCOMM_ROOT"
fi

if [ ! -f "$SWORDCOMM_ROOT/build.gradle.kts" ] && [ ! -f "$SWORDCOMM_ROOT/build.gradle" ]; then
    fail "Not a valid Gradle project (no build.gradle or build.gradle.kts found)"
fi

print_info "SWORDCOMM project found"
cd "$SWORDCOMM_ROOT"

echo ""
echo "Build Configuration:"
echo ""

# Check signing configuration
if [ -n "$KEYSTORE_PATH" ] && [ -n "$KEYSTORE_PASSWORD" ] && [ -n "$KEYSTORE_ALIAS" ]; then
    print_info "Release build with signing enabled"
    BUILD_TYPE="Release"

    if [ ! -f "$KEYSTORE_PATH" ]; then
        fail "Keystore file not found at: $KEYSTORE_PATH"
    fi
    print_info "Keystore found: $KEYSTORE_PATH"
else
    print_warning "Signing credentials not set - building debug APK"
    print_warning "To build release, set environment variables:"
    echo "  export CI_KEYSTORE_PATH=/path/to/keystore.jks"
    echo "  export CI_KEYSTORE_PASSWORD=password"
    echo "  export CI_KEYSTORE_ALIAS=alias_name"
    echo "  export CI_MAPS_API_KEY=your_api_key (optional)"
    BUILD_TYPE="Debug"
fi

echo ""

if [ "$BUILD_TYPE" = "Debug" ]; then
    read -p "Continue with DEBUG build? (Ctrl+C to cancel)"
fi

echo ""

{

echo "════════════════════════════════════════════════════════════════"
echo "SWORDCOMM Android Build Log"
echo "Started: $(get_time)"
echo "Log file: $LOG_FILE"
echo "════════════════════════════════════════════════════════════════"
echo ""

print_step "1" "Verifying Build Environment"

# Check for Docker
if command -v docker &> /dev/null; then
    print_info "Docker found: $(docker --version)"
else
    print_warning "Docker not found (Docker Compose builds will fail)"
fi

# Check for Docker Compose
if command -v docker-compose &> /dev/null; then
    print_info "Docker Compose found: $(docker-compose --version)"
elif docker compose version &> /dev/null 2>&1; then
    print_info "Docker Compose (integrated) found"
else
    print_warning "Docker Compose not found"
fi

# Check for Java/Gradle
if command -v java &> /dev/null; then
    print_info "Java found: $(java -version 2>&1 | head -1)"
else
    print_warning "Java not found (Gradle may not work)"
fi

# Check system resources
DISK_AVAILABLE=$(df -h . | awk 'NR==2 {print $4}')
MEMORY_AVAILABLE=$(free -h | grep Mem | awk '{print $7}')
print_info "Disk space available: $DISK_AVAILABLE"
print_info "Memory available: $MEMORY_AVAILABLE"

echo ""

print_step "2" "Preparing Build"

print_progress "Cleaning previous builds..."
rm -rf build/ > /dev/null 2>&1 || true
print_info "Build directory cleaned"

print_progress "Setting build properties..."
mkdir -p .gradle

# Create gradle.properties if needed
if [ ! -f "gradle.properties" ]; then
    print_info "Creating gradle.properties"
    cat > gradle.properties << 'EOF'
org.gradle.jvmargs=-Xmx4096m
org.gradle.parallel=true
org.gradle.caching=true
android.useAndroidX=true
EOF
fi

echo ""

print_step "3" "Configuring Build"

BUILD_ARGS=""

# Add signing configuration for release builds
if [ "$BUILD_TYPE" = "Release" ]; then
    print_progress "Configuring release signing..."

    BUILD_ARGS="$BUILD_ARGS -PkeystorePath=$KEYSTORE_PATH"
    BUILD_ARGS="$BUILD_ARGS -PkeystorePassword=$KEYSTORE_PASSWORD"
    BUILD_ARGS="$BUILD_ARGS -PkeyAlias=$KEYSTORE_ALIAS"

    if [ -n "$MAPS_API_KEY" ]; then
        BUILD_ARGS="$BUILD_ARGS -PmapsApiKey=$MAPS_API_KEY"
    fi

    print_info "Release configuration ready"
else
    print_progress "Configuring debug build..."
    print_info "Debug configuration ready"
fi

echo ""

print_step "4" "Building Android Application"

# Determine build task
if [ "$BUILD_TYPE" = "Release" ]; then
    BUILD_TASK="bundleRelease"
    OUTPUT_TYPE="AAB (Android App Bundle)"
else
    BUILD_TASK="assembleDebug"
    OUTPUT_TYPE="APK (Debug)"
fi

print_progress "Task: $BUILD_TASK"
print_progress "Output type: $OUTPUT_TYPE"
print_warning "This may take 5-20 minutes..."
echo ""

echo "Build output:"
echo "════════════════════════════════════════════════════════════════"

START_BUILD=$(date +%s)

# Export environment variables for Gradle
export CI_KEYSTORE_PATH="$KEYSTORE_PATH"
export CI_KEYSTORE_PASSWORD="$KEYSTORE_PASSWORD"
export CI_KEYSTORE_ALIAS="$KEYSTORE_ALIAS"
export CI_MAPS_API_KEY="$MAPS_API_KEY"

# Run build
if ./gradlew $BUILD_TASK $BUILD_ARGS 2>&1; then
    END_BUILD=$(date +%s)
    ELAPSED=$((END_BUILD - START_BUILD))
    MINUTES=$((ELAPSED / 60))
    SECONDS=$((ELAPSED % 60))

    echo "════════════════════════════════════════════════════════════════"
    echo ""
    print_info "Build completed in ${MINUTES}m ${SECONDS}s"
else
    echo "════════════════════════════════════════════════════════════════"
    echo ""
    fail "Build failed"
fi

echo ""

print_step "5" "Locating Build Artifacts"

print_progress "Searching for build outputs..."

if [ "$BUILD_TYPE" = "Release" ]; then
    # Look for AAB (App Bundle)
    AAB_PATH=$(find build -name "*.aab" -type f 2>/dev/null | head -1)
    if [ -n "$AAB_PATH" ]; then
        SIZE=$(du -h "$AAB_PATH" | cut -f1)
        print_info "App Bundle found: $AAB_PATH ($SIZE)"
        ARTIFACT="$AAB_PATH"
    else
        print_warning "AAB not found, looking for APK..."
        APK_PATH=$(find build -name "*-release.apk" -type f 2>/dev/null | head -1)
        if [ -n "$APK_PATH" ]; then
            SIZE=$(du -h "$APK_PATH" | cut -f1)
            print_info "Release APK found: $APK_PATH ($SIZE)"
            ARTIFACT="$APK_PATH"
        fi
    fi
else
    # Look for Debug APK
    APK_PATH=$(find build -name "*-debug.apk" -type f 2>/dev/null | head -1)
    if [ -n "$APK_PATH" ]; then
        SIZE=$(du -h "$APK_PATH" | cut -f1)
        print_info "Debug APK found: $APK_PATH ($SIZE)"
        ARTIFACT="$APK_PATH"
    fi
fi

if [ -z "$ARTIFACT" ]; then
    fail "Build artifacts not found"
fi

echo ""

################################################################################
# SUMMARY
################################################################################

print_header "✓ BUILD COMPLETE!"

echo ""
echo "═══════════════════════════════════════════════════════════════════"
echo "BUILD SUMMARY"
echo "═══════════════════════════════════════════════════════════════════"
echo ""
echo "Build type:       $BUILD_TYPE"
echo "Output type:      $OUTPUT_TYPE"
echo "Build time:       ${MINUTES}m ${SECONDS}s"
echo ""
echo "Artifact:"
echo "  Path:  $ARTIFACT"
echo "  Size:  $(du -h "$ARTIFACT" | cut -f1)"
echo ""
echo "Log file:"
echo "  $LOG_FILE"
echo ""

echo "═══════════════════════════════════════════════════════════════════"
echo "NEXT STEPS"
echo "═══════════════════════════════════════════════════════════════════"
echo ""

if [ "$BUILD_TYPE" = "Release" ]; then
    echo "Release build complete!"
    echo ""
    echo "To upload to Play Store:"
    echo "  1. Open Google Play Console"
    echo "  2. Go to your app's 'Release' section"
    echo "  3. Upload the AAB from: $ARTIFACT"
    echo ""
else
    echo "Debug APK built successfully!"
    echo ""
    echo "To install on device/emulator:"
    echo "  adb install \"$ARTIFACT\""
    echo ""
    echo "Or install via Android Studio"
fi

echo ""
echo "═══════════════════════════════════════════════════════════════════"
echo "End time: $(get_time)"
echo "═══════════════════════════════════════════════════════════════════"
echo ""

} 2>&1 | tee "$LOG_FILE"

echo ""
echo "✓ Log saved to: $LOG_FILE"
echo ""
