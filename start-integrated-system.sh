#!/bin/bash
# Start TSM + CRYPTOGRAM Integrated System

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TSM_ROOT="$PROJECT_ROOT/Telegram/lib_tsm"
TSM_VENV="$TSM_ROOT/venv"
BUILD_DIR="$PROJECT_ROOT/build_release"

# Load environment
source "$PROJECT_ROOT/.tsm_cryptogram_env.sh" > /dev/null

echo "Starting TSM + CRYPTOGRAM Integrated System..."
echo ""

# Start TSM gRPC server in background
echo "1. Starting TSM gRPC Server..."
cd "$TSM_ROOT"
python -m mock_server.server > /tmp/tsm_grpc.log 2>&1 &
TSM_PID=$!
echo "   TSM Server PID: $TSM_PID"
sleep 2

# Start CRYPTOGRAM
echo "2. Starting CRYPTOGRAM..."
if [ -f "$BUILD_DIR/bin/Telegram" ]; then
    "$BUILD_DIR/bin/Telegram"
elif [ -f "$BUILD_DIR/Telegram/Telegram" ]; then
    "$BUILD_DIR/Telegram/Telegram"
else
    echo "CRYPTOGRAM executable not found at:"
    echo "  $BUILD_DIR/bin/Telegram"
    echo "  $BUILD_DIR/Telegram/Telegram"
    echo ""
    echo "Try rebuilding with: $PROJECT_ROOT/setup-tsm-cryptogram.sh"
    exit 1
fi

# Cleanup on exit
trap "kill $TSM_PID 2>/dev/null; echo 'Services stopped'" EXIT
