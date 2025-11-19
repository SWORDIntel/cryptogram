# CRYPTOGRAM Build Instructions

## Quick Start

```bash
git clone --recursive https://github.com/SWORDOps/CRYPTOGRAM.git
cd CRYPTOGRAM
git checkout claude/build-ada-protobuf-01Tpe3rvKAkdWnRVGPd3uLET

# Apply required local modifications (see below)
# Then run the build
./build_all.sh
```

## Required Local Modifications

The following modifications must be applied locally after cloning:

### 1. cmake/external/tde2e/CMakeLists.txt - Make tde2e optional

Change lines 10-14 to:
```cmake
if (DESKTOP_APP_USE_PACKAGED)
    find_package(tde2e QUIET)
    if (tde2e_FOUND)
        target_link_libraries(external_tde2e INTERFACE tde2e::tde2e)
    endif()
    return()
endif()
```

### 2. cmake/external/webrtc/CMakeLists.txt - Make tg_owt optional

Change lines 10-14 to:
```cmake
if (DESKTOP_APP_USE_PACKAGED)
    find_package(tg_owt QUIET)
    if (tg_owt_FOUND)
        target_link_libraries(external_webrtc INTERFACE tg_owt::tg_owt)
    endif()
    return()
endif()
```

### 3. Telegram/lib_tsm/CMakeLists.txt - Create this file

```cmake
# TSM (Telegram Security Module) - Python-based module
add_library(lib_tsm INTERFACE)
add_library(tdesktop::lib_tsm ALIAS lib_tsm)
add_library(desktop-app::lib_tsm ALIAS lib_tsm)

target_include_directories(lib_tsm
INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
```

## Building tg_owt Library (Required)

```bash
sudo apt-get install -y libpipewire-0.3-dev libopenh264-dev

mkdir -p ~/Libraries && cd ~/Libraries
git clone https://github.com/desktop-app/tg_owt.git
cd tg_owt
git submodule update --init --recursive
mkdir -p out/Release && cd out/Release
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF ../..
ninja
```

Creates: `~/Libraries/tg_owt/out/Release/libtg_owt.a` (~32MB)

## System Dependencies

```bash
sudo apt-get install -y qt6-base-dev qt6-wayland-dev libqt6svg6-dev \
  libxkbcommon-dev libboost-regex-dev libavcodec-dev libavformat-dev \
  libavutil-dev libavfilter-dev libswscale-dev libswresample-dev \
  libhunspell-dev libpipewire-0.3-dev libopenh264-dev \
  gobject-introspection libgirepository1.0-dev libxcb-record0-dev

# Install all XCB libraries
apt-cache search libxcb | grep -- '-dev' | awk '{print $1}' | xargs sudo apt-get install -y
```

## Build CRYPTOGRAM

```bash
./build_all.sh
```

Build time: 20-40 minutes. Binary will be in `build_release/bin/`
