================================================================================
CRYPTOGRAM BUILD SCRIPT - WebRTC (tg_owt) Build Support Integration
Version: 3.2.0
Updated: 2025-11-19
================================================================================

SUMMARY OF CHANGES
==================

The build_all.sh script has been enhanced to automatically handle WebRTC (tg_owt)
library building with full dependency management.

KEY ADDITIONS
=============

1. WebRTC BUILD DEPENDENCIES (ensure_system_dependencies)
   - Added installation of critical packages:
     * libgbm-dev (GPU memory buffer support)
     * libegl-dev (OpenGL ES support)
     * libxcomposite-dev, libxdamage-dev (X11 composite/damage extensions)
     * libxtst-dev (X11 test server extension)
     * libsrtp2-dev (Secure RTP for audio/video)
     * libxxhash-dev (Hash library)

2. IMPROVED tg_owt BUILD FUNCTION (ensure_tg_owt_from_source)
   - Automatic detection of cached build (skips rebuild if libtg_owt.a exists)
   - Git submodule initialization for all tg_owt dependencies:
     * abseil-cpp (Google's C++ library)
     * crc32c (CRC32 checksum library)
     * libsrtp (Secure RTP implementation)
     * libyuv (Video processing library)
   - Robust CMake configuration with TG_OWT_PACKAGED_BUILD=OFF
   - Parallel build using Ninja with automatic job detection
   - Comprehensive error checking and reporting

3. UPDATED BUILD PIPELINE
   Previous Order:
     1. System Requirements Check
     2. Tool Dependencies Check
     3. Permissions Check
     4. Submodules Initialization
     5. Compiler Configuration
     6-8. Build Ada, Protobuf, Cryptogram

   New Order:
     1. System Requirements Check
     2. Tool Dependencies Check
     3. System Dependencies (INSTALLS WebRTC PACKAGES)
     4. Permissions Check
     5. Submodules Initialization
     6. tg_owt Build (BUILDS WebRTC LIBRARY)
     7. Compiler Configuration
     8-10. Build Ada, Protobuf, Cryptogram

BUILD PROCESS DETAILS
====================

The tg_owt build now includes:

1. Detection Phase:
   - Checks if libtg_owt.a already exists
   - If cached build found, skips entire process (fast resume)
   - Uses local source from CRYPTOGRAM_ROOT/Telegram/tg_owt if available

2. Preparation Phase:
   - Clones tg_owt from GitHub if not present locally
   - Initializes all git submodules recursively
   - Ensures Ninja build tool is available

3. Configuration Phase:
   - Runs CMake with Release build type
   - Uses TG_OWT_PACKAGED_BUILD=OFF for internal dependency management
   - Configures for optimal build performance

4. Build Phase:
   - Compiles ~1300+ source files
   - Uses parallel jobs matching system CPU count
   - Produces 30MB+ static library (libtg_owt.a)
   - Estimated runtime: 5-10 minutes on modern hardware

5. Validation Phase:
   - Verifies libtg_owt.a was created successfully
   - Displays build artifact size and location
   - Logs all operations for troubleshooting

USAGE
=====

Standard Build:
  ./build_all.sh

Force Rebuild of tg_owt:
  rm Telegram/tg_owt/out/Release/libtg_owt.a
  ./build_all.sh

Custom Parallel Jobs:
  ./build_all.sh --jobs=8

Verbose Output:
  ./build_all.sh --verbose

TROUBLESHOOTING
===============

If tg_owt build fails:

1. Missing Dependencies:
   - Check output for missing package messages
   - Install missing packages: sudo apt-get install <package-name>

2. Submodule Issues:
   - Manually initialize: cd Telegram/tg_owt && git submodule update --init --recursive

3. CMake Configuration Issues:
   - Check CMake version: cmake --version (requires 3.25+)
   - Clear build cache: rm -rf Telegram/tg_owt/out/Release
   - Reconfigure: ./build_all.sh

4. Build Failures:
   - Increase parallel jobs if low memory: ./build_all.sh --jobs=2
   - Check disk space: du -sh Telegram/tg_owt/out/Release/

PERFORMANCE NOTES
=================

- First build: 5-10 minutes (full compilation)
- Cached rebuild: <1 second (detects existing library)
- Parallel compilation: Scales with CPU cores (default: auto-detect)
- Memory usage: ~2GB for optimal performance
- Disk usage: ~3GB for source + build artifacts

TESTING
=======

After build completes, the following should exist:

1. Library:
   - CRYPTOGRAM_ROOT/Telegram/tg_owt/out/Release/libtg_owt.a

2. Headers:
   - CRYPTOGRAM_ROOT/Telegram/tg_owt/src/* (includes directory)

3. CMake Configuration Ready:
   - Main build can now successfully configure with tg_owt found

================================================================================
