#!/bin/bash
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TSM_ROOT="$PROJECT_ROOT/Telegram/lib_tsm"
TSM_VENV="$TSM_ROOT/venv"
BUILD_DIR="$PROJECT_ROOT/build_release"

echo "TSM + CRYPTOGRAM Verification Checklist"
echo "========================================"
echo ""

PASSED=0
FAILED=0

check() {
    if [ $1 -eq 0 ]; then
        echo "  ✓ $2"
        PASSED=$((PASSED + 1))
    else
        echo "  ✗ $2"
        FAILED=$((FAILED + 1))
    fi
}

# TSM checks
echo "TSM:"
[ -d "$TSM_VENV" ] && check 0 "Virtual environment exists" || check 1 "Virtual environment missing"
[ -f "$TSM_ROOT/requirements.txt" ] && check 0 "Requirements file found" || check 1 "Requirements file missing"
[ -d "$TSM_ROOT/config" ] && check 0 "Config directory exists" || check 1 "Config directory missing"
[ -f "$TSM_ROOT/config/tsm.yaml" ] && check 0 "TSM configuration created" || check 1 "TSM configuration missing"

# CRYPTOGRAM checks
echo ""
echo "CRYPTOGRAM:"
[ -d "$BUILD_DIR" ] && check 0 "Build directory exists" || check 1 "Build directory missing"
[ -f "$BUILD_DIR/CMakeCache.txt" ] && check 0 "CMake configured" || check 1 "CMake not configured"

if [ -f "$BUILD_DIR/bin/Telegram" ]; then
    check 0 "Executable found at bin/Telegram"
elif [ -f "$BUILD_DIR/Telegram/Telegram" ]; then
    check 0 "Executable found at Telegram/Telegram"
else
    check 1 "Executable not found"
fi

# Integration checks
echo ""
echo "Integration:"
[ -f "$PROJECT_ROOT/.tsm_cryptogram_env.sh" ] && check 0 "Environment script exists" || check 1 "Environment script missing"
[ -f "$PROJECT_ROOT/start-integrated-system.sh" ] && check 0 "Startup script exists" || check 1 "Startup script missing"

echo ""
echo "Summary: $PASSED passed, $FAILED failed"

if [ $FAILED -eq 0 ]; then
    echo ""
    echo "✓ System is ready!"
    echo ""
    echo "Next steps:"
    echo "  1. Load environment: source $PROJECT_ROOT/.tsm_cryptogram_env.sh"
    echo "  2. Start system: $PROJECT_ROOT/start-integrated-system.sh"
else
    echo ""
    echo "⚠ Some checks failed. Run setup again: $PROJECT_ROOT/setup-tsm-cryptogram.sh"
fi
