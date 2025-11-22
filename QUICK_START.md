# CRYPTOGRAM Quick Start

## 30-Second Setup

```bash
cd /home/user/CRYPTOGRAM
./setup.sh --gcc14 --build
./build_release/bin/Telegram
```

## Common Commands

### Setup & Build

```bash
# Step 1: One-time setup (30 minutes)
./setup.sh

# Step 2: Build project (15-45 minutes depending on hardware)
./build.sh

# Step 3: Run
./build_release/bin/Telegram
```

### Building

```bash
# Release build (optimized, default)
./build.sh

# Debug build (with debug symbols)
./build.sh --debug

# Clean rebuild
./build.sh --clean

# Verbose output
./build.sh --verbose

# Custom parallel jobs
./build.sh --jobs 8
```

### GCC Selection

```bash
# Use GCC 14 (default)
./setup.sh --gcc14 --build

# Use GCC 15
./setup.sh --gcc15 --build

# Manual build with specific GCC
CC=gcc-15 CXX=g++-15 ./build.sh
```

### Manual Build

```bash
cd build_release
make -j$(nproc)      # Build with all CPU cores
make -j4             # Build with 4 jobs
make clean           # Clean build artifacts
make install         # Install (if configured)
```

## File Locations

| File | Purpose |
|------|---------|
| `./setup.sh` | Configure build environment |
| `./build.sh` | Compile project |
| `.env.sh` | Environment variables (created by setup) |
| `build_release/` | Build artifacts |
| `build_release/bin/Telegram` | Compiled executable |

## Troubleshooting

| Problem | Solution |
|---------|----------|
| GCC not found | Run `./setup.sh --gcc14` (auto-installs) |
| Qt6 not found | Run `./setup.sh` (auto-installs) |
| Out of memory | Use `./build.sh --jobs 2` |
| Build fails | Check `build_release/cmake_build.log` |
| Permission denied | Run `chmod +x *.sh` then try again |

## Environment Variables

```bash
# Set for this session
export CC=gcc-14
export CXX=g++-14
export CMAKE_BUILD_TYPE=Release

# Or load saved environment
source .env.sh
```

## Performance Tips

- **Fast build**: Use more jobs - `./build.sh --jobs 16`
- **Faster rebuilds**: Install ccache - `sudo apt-get install ccache`
- **Less memory**: Reduce jobs - `./build.sh --jobs 2`

## Getting Help

```bash
# Show script help
./setup.sh --help
./build.sh --help

# Full documentation
cat SETUP_GUIDE.md
```

## Development Build

```bash
# Setup for debugging
./setup.sh --debug

# Build with debug symbols
./build.sh --debug --verbose

# Run with gdb
gdb ./build_release/bin/Telegram
```

---

See **SETUP_GUIDE.md** for detailed instructions.
