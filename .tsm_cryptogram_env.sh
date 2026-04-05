#!/bin/bash
# TSM + CRYPTOGRAM Integration Environment

export CC="/usr/bin/gcc-14"
export CXX="/usr/bin/g++-14"

# Paths
export CRYPTOGRAM_ROOT="/media/john/NVME_STORAGE4/CRYPTOGRAM"
export TSM_ROOT="$CRYPTOGRAM_ROOT/Telegram/lib_tsm"
export TSM_VENV="$TSM_ROOT/venv"
export TSM_CONFIG="$TSM_ROOT/config"
export BUILD_DIR="$CRYPTOGRAM_ROOT/build_release"

# TSM Services (Secure Configuration)
export TSM_GRPC_PORT=50051
export TSM_API_PORT=6060
export TSM_GRPC_HOST="localhost:50051"
export TSM_API_URL="http://127.0.0.1:6060"
export TSM_REQUIRE_YUBIKEY=true

# Python path for TSM
export PYTHONPATH="$TSM_ROOT:$PYTHONPATH"

# Activate TSM venv if available
if [ -f "$TSM_VENV/bin/activate" ]; then
    source "$TSM_VENV/bin/activate"
fi

echo "TSM + CRYPTOGRAM Environment Loaded"
echo "  CRYPTOGRAM: $CRYPTOGRAM_ROOT"
echo "  TSM: $TSM_ROOT"
echo "  Compiler: /usr/bin/gcc-14"
