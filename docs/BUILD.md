# Building CRYPTOGRAM

## Prerequisites

### Desktop (Linux)
- CMake 3.16+
- Ninja
- Qt6 (base, wayland, svg)
- OpenSSL 3.x
- FFmpeg (libavcodec, libavformat, libavutil, libswscale, libswresample)
- libvpx, libopus, libopenal, liblz4, libxxhash
- libprotobuf, protobuf-compiler
- librlottie, libminizip, libhunspell
- pkg-config

### Android
- Android SDK (API 35)
- Android NDK 25.2.9519653
- CMake 3.22.1 (via SDK)
- JDK 21
- Kotlin 2.1.0
- Gradle 8.7+

## Desktop Build (Linux)

### Quick build
```bash
./build_linux.sh
```

### Manual build
```bash
# Initialize cmake submodule
git submodule update --init --recursive cmake

# Configure
mkdir build_release && cd build_release
cmake .. -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DENABLE_QUANTUMGUARD=ON

# Build
ninja
```

The binary will be at `build_release/bin/Telegram`.

### Debian package
```bash
# From project root
dpkg-buildpackage -us -uc -b

# Or manual packaging
dpkg-deb --build --root-owner-group debian/cryptogram-desktop
```

### AppImage
```bash
# Create AppDir structure
mkdir -p AppDir/usr/bin AppDir/usr/share/applications AppDir/usr/share/icons/hicolor/256x256/apps
cp build_release/bin/Telegram AppDir/usr/bin/cryptogram-desktop
cp debian/cryptogram-desktop.desktop AppDir/usr/share/applications/
cp Telegram/Resources/art/logo_256.png AppDir/usr/share/icons/hicolor/256x256/apps/cryptogram.png

# Create AppRun
cat > AppDir/AppRun << 'EOF'
#!/bin/bash
SELF=$(readlink -f "$0")
HERE=$(dirname "$SELF")
exec "$HERE/usr/bin/cryptogram-desktop" "$@"
EOF
chmod +x AppDir/AppRun

# Build AppImage
ARCH=x86_64 appimagetool AppDir cryptogram-desktop-x86_64.AppImage
```

## Android Build

### Quick build
```bash
./build_android.sh
```

### Manual build
```bash
cd telegram-android

# Set up local.properties
echo "sdk.dir=/path/to/android-sdk" > local.properties

# Build release APK for all ABIs
JAVA_HOME=/path/to/jdk-21 ANDROID_HOME=/path/to/android-sdk \
  ./gradlew :TMessagesProj_App:assembleRelease --no-daemon
```

The APK will be at:
```
TMessagesProj_App/build/outputs/apk/afat/release/app.apk
```

### FFmpeg rebuild (if needed)
FFmpeg must be built with `-fPIC` for all ABIs. Use the build script:
```bash
bash /tmp/build_ffmpeg_android.sh
```

For arm64-v8a specifically, use `--disable-asm` to avoid NEON relocation errors:
```bash
./configure --enable-pic --disable-asm --disable-inline-asm \
  --extra-cflags="-fPIC -O2 -DANDROID" ...
```

### Supported ABIs
- `arm64-v8a` - ARM 64-bit
- `armeabi-v7a` - ARM 32-bit
- `x86` - Intel/AMD 32-bit
- `x86_64` - Intel/AMD 64-bit

## Full Build (Desktop + Android)
```bash
./build_all.sh
```

This script auto-bootstraps all dependencies including OpenAL, LZ4, xxHash, minizip, rlottie, RNNoise, tg_owt/WebRTC, tde2e, and Protocol Buffers.
