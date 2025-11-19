#!/bin/bash
# Apply local modifications required for building CRYPTOGRAM
# These changes are kept as local modifications to avoid submodule commit issues

set -e

echo "Applying CRYPTOGRAM build patches..."

# Apply cmake submodule patches
echo "1. Patching cmake submodule (make tde2e and tg_owt optional)..."
cd cmake
git apply ../patches/cmake-optional-libs.patch
cd ..

# Create lib_tsm CMakeLists.txt
echo "2. Creating Telegram/lib_tsm/CMakeLists.txt..."
cp patches/lib_tsm-cmakelists.patch Telegram/lib_tsm/CMakeLists.txt

echo "✅ All patches applied successfully!"
echo ""
echo "You can now run: ./build_all.sh"
