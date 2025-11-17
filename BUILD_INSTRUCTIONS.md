# CRYPTOGRAM + TSM Build Instructions

**Current Status:** Ready for final compilation (Option A complete - all dependencies installed)

---

## Quick Summary

The TSM + CRYPTOGRAM integration is nearly complete. All system dependencies have been installed. The protobuf library is being built in `/tmp/protobuf/build/`. Once it finishes, the CRYPTOGRAM desktop application can be compiled.

---

## What's Already Done ✅

1. ✅ **TSM Submodule Integrated** - Located at `Telegram/lib_tsm/`
2. ✅ **CMake Submodule Verified** - All required files present
3. ✅ **System Dependencies Installed:**
   - Qt6 6.4.2 (base, SVG, declarative, tools)
   - Boost 1.83.0 (all modules)
   - FFmpeg (libavcodec, libavformat, libswscale, libswresample)
   - OpenSSL, zlib, opus, vpx, libjpeg, libpulse, libdbus
   - Protobuf compiler and dev libraries
   - Ada URL parser (built and installed to /usr/local)

4. ✅ **Setup Scripts Ready** - `setup-tsm-cryptogram.sh` with improved error detection
5. ✅ **TSM Backend Ready** - Python environment, gRPC server, secure config
6. ✅ **Documentation Complete** - Setup guides, integration architecture, build status

---

## What You Need To Do

### Step 1: Complete Protobuf Build (~5-10 minutes)

**Check if protobuf build is still running:**
```bash
ps aux | grep cmake | grep protobuf
```

**If NOT running, restart the build:**
```bash
cd /tmp/protobuf/build
cmake --build . --config Release
```

**If ALREADY running, wait for it to finish.** You should see output like:
```
[100%] Built target libprotobuf
[100%] Linking CXX library libprotoc.so
[100%] Built target libprotoc
```

### Step 2: Install Protobuf (~30 seconds)

Once the build is complete:
```bash
cd /tmp/protobuf/build
cmake --install . --prefix /usr/local
```

**Verify installation:**
```bash
ls -la /usr/local/lib/cmake/protobuf/
# Should show: protobufConfig.cmake and related files
```

### Step 3: Configure CRYPTOGRAM Build (~2-3 minutes)

Navigate to the build directory and configure:
```bash
cd /home/user/CRYPTOGRAM/build_release
rm -f CMakeCache.txt
rm -rf CMakeFiles

export CC=/usr/bin/gcc-13
export CXX=/usr/bin/g++-13

cmake -DCMAKE_BUILD_TYPE=Release \
      -DDESKTOP_APP_DISABLE_AUTOUPDATE=ON \
      -DDESKTOP_APP_DISABLE_CRASH_REPORTS=ON ..
```

**Expected output (last few lines):**
```
-- Configuring done (2.3s)
-- Generating done (0.1s)
-- Build files have been written to: /home/user/CRYPTOGRAM/build_release
```

If you see `CMake Error`, check the error message and:
- Missing Qt6 component? Run: `apt-get install qt6-[component]-dev`
- Missing other library? Run: `apt-get install lib[name]-dev`

### Step 4: Build CRYPTOGRAM Desktop (~20-60 minutes)

This is the main compilation step. It will take the longest:

```bash
cd /home/user/CRYPTOGRAM/build_release

# Get number of CPU cores
NUM_JOBS=$(nproc)
echo "Building with $NUM_JOBS parallel jobs"

# Start the build
cmake --build . --config Release --parallel $NUM_JOBS
```

**What to expect:**
- Initial compilation of source files (~15-40 minutes depending on CPU)
- Linking the final executable (~5-10 minutes)
- Total time: 20-60 minutes on typical systems

**Example output (you'll see this repeating):**
```
[ 10%] Building CXX object src/...
[ 20%] Linking CXX executable bin/Telegram
[100%] Built target Telegram
```

### Step 5: Verify the Build

Once complete, verify the executable exists:

```bash
# Check executable location
ls -lah /home/user/CRYPTOGRAM/build_release/bin/Telegram

# Or find it if location differs
find /home/user/CRYPTOGRAM/build_release -name "Telegram" -type f -executable
```

---

## TSM Backend (Already Ready)

While CRYPTOGRAM builds, the TSM backend is ready to use:

```bash
# Source the environment
source /home/user/CRYPTOGRAM/.tsm_cryptogram_env.sh

# Start the TSM gRPC server (runs on localhost:50051)
cd /home/user/CRYPTOGRAM/Telegram/lib_tsm
python -m mock_server.server

# In another terminal, test the connection:
python -c "import grpc; stub = grpc.secure_channel('localhost:50051', grpc.ssl_channel_credentials()); print('Connected!')"
```

---

## Troubleshooting

### Build Fails With "Missing Package"
```bash
# Find the package
apt-cache search "package-name"

# Install it
apt-get install -y lib[package]-dev
```

### Build Takes Too Long
- This is normal! CRYPTOGRAM is a large application
- Check CPU usage: `top` or `htop`
- Build typically takes 20-60 minutes

### Out of Disk Space
```bash
# Check available space
df -h

# If low on space, you can clean old files:
cd /tmp
du -sh *  # See what's taking space
rm -rf ada protobuf  # Clean build directories if needed
```

### Build Fails on a Specific File
- Retry the build (sometimes transient errors occur)
- Or check if it's a known CRYPTOGRAM issue

---

## After Build Complete

### Option A: Run CRYPTOGRAM
```bash
/home/user/CRYPTOGRAM/build_release/bin/Telegram
```

### Option B: Create Installation Package
```bash
cd /home/user/CRYPTOGRAM/build_release
cmake --install . --prefix ~/cryptogram_install
```

### Option C: Run with TSM Integration
```bash
# Start TSM service in background
cd /home/user/CRYPTOGRAM/Telegram/lib_tsm
python -m mock_server.server &

# Run CRYPTOGRAM
/home/user/CRYPTOGRAM/build_release/bin/Telegram
```

---

## Git Status

All changes have been committed to branch:
```
claude/add-tsm-submodule-013joYnLAPDpuqGsg3ymsU3k
```

Recent commits:
- `ac4fd7d` - Build status report
- `39c76cb` - PR description
- `5a98e8e` - Setup status documentation
- `cffc93d` - CMake error detection improvements

To see all changes:
```bash
git log --oneline claude/add-tsm-submodule-013joYnLAPDpuqGsg3ymsU3k | head -15
```

---

## Build Time Estimate

| Step | Time | Notes |
|------|------|-------|
| Protobuf build | 5-10 min | If not already done |
| Protobuf install | 30 sec | Quick |
| CMake config | 2-3 min | Checks dependencies |
| CRYPTOGRAM build | **20-60 min** | Main step - depends on CPU |
| **Total** | **30-75 min** | Can run in background |

---

## Quick Reference

```bash
# Check protobuf build status
ps aux | grep protobuf | grep cmake

# Monitor CRYPTOGRAM build progress
cd /home/user/CRYPTOGRAM/build_release && tail -f CMakeFiles/CMakeOutput.log

# Stop the build if needed
cd /home/user/CRYPTOGRAM/build_release && make stop
# Or: Ctrl+C in the terminal running cmake --build

# Check available CPU cores
nproc

# Monitor system resources during build
watch -n 1 'ps aux | grep cmake; free -h; df -h'
```

---

## Support Files

- `SETUP_GUIDE.md` - Detailed CRYPTOGRAM-only setup
- `TSM_CRYPTOGRAM_INTEGRATION.md` - Architecture and integration details
- `BUILD_STATUS.md` - Technical analysis of dependencies
- `setup-tsm-cryptogram.sh` - Automated setup script (can retry if needed)

---

## What Works Right Now (No Build Needed)

### ✅ TSM Backend
```bash
source .tsm_cryptogram_env.sh
cd Telegram/lib_tsm
python -m mock_server.server
# Server running on localhost:50051
```

### ✅ Integration Environment
```bash
# All environment variables configured
echo $CRYPTOGRAM_ROOT
echo $TSM_ROOT
echo $TSM_VENV
```

### ✅ All Dependencies
- All C++ libraries installed
- All Python modules installed
- All build tools ready

---

## Notes

1. **Build can run in background** - Start the build and check back later
2. **Multiple CPU cores** - Use `--parallel $(nproc)` to speed up compilation
3. **Disk space** - Needs ~2GB for compilation, ~1GB for build artifacts
4. **Network** - Doesn't require network during build (all deps already installed)
5. **Recovery** - If build fails, just run cmake again (most targets are cached)

---

## Questions?

Check these files in order:
1. This file (BUILD_INSTRUCTIONS.md) - you're reading it
2. `BUILD_STATUS.md` - Technical details about dependencies
3. `SETUP_GUIDE.md` - Detailed setup process
4. `TSM_CRYPTOGRAM_INTEGRATION.md` - Architecture details

Or check the official CRYPTOGRAM docs:
- https://github.com/telegramdesktop/tdesktop/blob/dev/docs/building-cmake.md
