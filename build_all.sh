#!/bin/bash

################################################################################
# TSM + CRYPTOGRAM - Ultimate Automated Build Script (ULTRA VERBOSE)
# Version: 2.0.0
# Builds everything: ada, protobuf, CMake config, and CRYPTOGRAM desktop
# FEATURES: Full logging, real-time output, error recovery, progress tracking,
#           dependency checking, system profiling, build caching, and more
################################################################################

set -Eeuo pipefail

# ──────────────────────────────────────────────────────────────────────────────
# Advanced Color Palette & Unicode Symbols
# ──────────────────────────────────────────────────────────────────────────────
readonly RED='\033[0;31m'
readonly GREEN='\033[0;32m'
readonly YELLOW='\033[1;33m'
readonly BLUE='\033[0;34m'
readonly CYAN='\033[0;36m'
readonly MAGENTA='\033[0;35m'
readonly WHITE='\033[1;37m'
readonly GRAY='\033[0;90m'
readonly BOLD='\033[1m'
readonly UNDERLINE='\033[4m'
readonly BLINK='\033[5m'
readonly REVERSE='\033[7m'
readonly NC='\033[0m'

# Unicode symbols
readonly CHECK_MARK="✓"
readonly CROSS_MARK="✗"
readonly WARNING_SIGN="⚠"
readonly INFO_SIGN="ℹ"
readonly ARROW="→"
readonly BULLET="•"
readonly HOURGLASS="⏳"
readonly CLOCK="🕐"
readonly ROCKET="🚀"
readonly GEAR="⚙"
readonly PACKAGE="📦"
readonly FOLDER="📁"
readonly FILE="📄"
readonly LINK="🔗"
readonly LOCK="🔒"
readonly KEY="🔑"
readonly SHIELD="🛡"
readonly BUG="🐛"
readonly WRENCH="🔧"
readonly HAMMER="🔨"
readonly SPARKLES="✨"
readonly FIRE="🔥"
readonly CHECKERED_FLAG="🏁"

# ──────────────────────────────────────────────────────────────────────────────
# Core Configuration
# ──────────────────────────────────────────────────────────────────────────────
readonly SCRIPT_VERSION="2.0.0"
readonly SCRIPT_NAME="$(basename "$0")"
readonly SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
readonly BUILD_DATE="$(date +%Y%m%d_%H%M%S)"
readonly BUILD_ID="$(uuidgen 2>/dev/null || echo "${BUILD_DATE}_$$")"
readonly BUILD_START_TIME="$(date +%s)"

# Logging configuration
readonly LOG_DIR="${LOG_DIR:-/tmp/cryptogram_builds}"
readonly LOG_FILE="${LOG_DIR}/build_${BUILD_DATE}.log"
readonly ERROR_LOG="${LOG_DIR}/errors_${BUILD_DATE}.log"
readonly DEBUG_LOG="${LOG_DIR}/debug_${BUILD_DATE}.log"
readonly METRICS_LOG="${LOG_DIR}/metrics_${BUILD_DATE}.json"
readonly SUMMARY_FILE="${LOG_DIR}/summary_${BUILD_DATE}.txt"

# Build paths
readonly CRYPTOGRAM_ROOT="${CRYPTOGRAM_ROOT:-$HOME/CRYPTOGRAM}"
readonly BUILD_DIR="${CRYPTOGRAM_ROOT}/build_release"
readonly CACHE_DIR="${HOME}/.cache/cryptogram_build"
readonly STATE_FILE="${CACHE_DIR}/build_state.json"
readonly DEPS_CACHE="${CACHE_DIR}/dependencies"

# Build configuration
readonly DEFAULT_INSTALL_PREFIX="/usr/local"
readonly USER_INSTALL_PREFIX="${HOME}/.local"
INSTALL_PREFIX="${DEFAULT_INSTALL_PREFIX}"

# System detection
readonly OS_TYPE="$(uname -s)"
readonly ARCH_TYPE="$(uname -m)"
readonly KERNEL_VERSION="$(uname -r)"
readonly NUM_CORES="$(nproc 2>/dev/null || getconf _NPROCESSORS_ONLN 2>/dev/null || echo 4)"
readonly TOTAL_MEMORY="$(free -b 2>/dev/null | awk '/^Mem:/{print $2}' || echo 0)"
readonly MEMORY_GB="$((TOTAL_MEMORY / 1073741824))"

# Build flags and options
VERBOSE_MODE=1
DRY_RUN=0
FORCE_REBUILD=0
SKIP_DEPENDENCIES=0
PARALLEL_JOBS="${NUM_CORES}"
USE_CCACHE=1
USE_NINJA=0
ENABLE_LTO=0
ENABLE_NATIVE_ARCH=1
SANITIZERS=""

# Component versions (can be overridden)
ADA_VERSION="${ADA_VERSION:-main}"
PROTOBUF_VERSION="${PROTOBUF_VERSION:-main}"

# ──────────────────────────────────────────────────────────────────────────────
# Build State Management
# ──────────────────────────────────────────────────────────────────────────────
declare -A BUILD_STATE
BUILD_STATE["ada_cloned"]=0
BUILD_STATE["ada_built"]=0
BUILD_STATE["ada_installed"]=0
BUILD_STATE["protobuf_cloned"]=0
BUILD_STATE["protobuf_built"]=0
BUILD_STATE["protobuf_installed"]=0
BUILD_STATE["cryptogram_configured"]=0
BUILD_STATE["cryptogram_built"]=0

# ──────────────────────────────────────────────────────────────────────────────
# Progress Tracking
# ──────────────────────────────────────────────────────────────────────────────
declare -A STEP_TIMES
declare -A STEP_STATUS
CURRENT_STEP=""
TOTAL_STEPS=8

# ──────────────────────────────────────────────────────────────────────────────
# Initialize Directories and Logs
# ──────────────────────────────────────────────────────────────────────────────
initialize_environment() {
    # Create necessary directories
    mkdir -p "$LOG_DIR" "$CACHE_DIR" "$DEPS_CACHE"
    
    # Initialize log files
    : > "$LOG_FILE"
    : > "$ERROR_LOG"
    : > "$DEBUG_LOG"
    : > "$METRICS_LOG"
    
    # Set up file descriptors for logging
    exec 3>&1 4>&2
    exec 1> >(tee -a "$LOG_FILE")
    exec 2> >(tee -a "$LOG_FILE" "$ERROR_LOG" >&2)
    
    # Enable debug logging if requested
    if [[ "${DEBUG:-0}" == "1" ]]; then
        set -x
        exec 5> >(tee -a "$DEBUG_LOG")
        BASH_XTRACEFD=5
    fi
}

# ──────────────────────────────────────────────────────────────────────────────
# Utility Functions
# ──────────────────────────────────────────────────────────────────────────────
get_timestamp() {
    date +"%Y-%m-%d %H:%M:%S.%3N"
}

get_elapsed_time() {
    local start="$1"
    local end="${2:-$(date +%s)}"
    local elapsed=$((end - start))
    printf "%02d:%02d:%02d" $((elapsed/3600)) $((elapsed%3600/60)) $((elapsed%60))
}

format_size() {
    local size="$1"
    if [[ $size -ge 1073741824 ]]; then
        echo "$(echo "scale=2; $size/1073741824" | bc) GB"
    elif [[ $size -ge 1048576 ]]; then
        echo "$(echo "scale=2; $size/1048576" | bc) MB"
    elif [[ $size -ge 1024 ]]; then
        echo "$(echo "scale=2; $size/1024" | bc) KB"
    else
        echo "$size B"
    fi
}

spinner() {
    local pid="$1"
    local msg="${2:-Processing...}"
    local spinstr='⠋⠙⠹⠸⠼⠴⠦⠧⠇⠏'
    local i=0
    
    while kill -0 "$pid" 2>/dev/null; do
        local temp="${spinstr:i++%${#spinstr}:1}"
        printf "\r${CYAN}%s${NC} %s" "$temp" "$msg"
        sleep 0.1
    done
    printf "\r%*s\r" $((${#msg} + 2)) ""
}

progress_bar() {
    local current="$1"
    local total="$2"
    local width=50
    local percentage=$((current * 100 / total))
    local filled=$((current * width / total))
    local empty=$((width - filled))
    
    printf "\r["
    printf "%${filled}s" | tr ' ' '█'
    printf "%${empty}s" | tr ' ' '░'
    printf "] %3d%% (%d/%d)" "$percentage" "$current" "$total"
}

# ──────────────────────────────────────────────────────────────────────────────
# Logging Functions (Enhanced)
# ──────────────────────────────────────────────────────────────────────────────
log() {
    local level="$1"
    shift
    local message="$*"
    local timestamp="$(get_timestamp)"
    
    echo "[${timestamp}] [${level}] ${message}" >> "$LOG_FILE"
    
    case "$level" in
        ERROR)
            echo "[${timestamp}] ${message}" >> "$ERROR_LOG"
            ;;
        DEBUG)
            echo "[${timestamp}] ${message}" >> "$DEBUG_LOG"
            ;;
    esac
    
    # Also log to metrics in JSON format
    if [[ -n "$CURRENT_STEP" ]]; then
        echo "{\"timestamp\":\"${timestamp}\",\"level\":\"${level}\",\"step\":\"${CURRENT_STEP}\",\"message\":\"${message}\"}" >> "$METRICS_LOG"
    fi
}

print_header() {
    local text="$1"
    local width=80
    local padding=$(( (width - ${#text} - 2) / 2 ))
    
    echo ""
    echo -e "${MAGENTA}╔${"═"*$width}╗${NC}"
    printf "${MAGENTA}║%*s%s%*s║${NC}\n" $padding "" "$text" $((width - padding - ${#text})) ""
    echo -e "${MAGENTA}╚${"═"*$width}╝${NC}"
    echo ""
    
    log "INFO" "=== $text ==="
}

print_section() {
    local step_num="$1"
    local step_name="$2"
    CURRENT_STEP="${step_num}_${step_name// /_}"
    STEP_TIMES["$CURRENT_STEP"]="$(date +%s)"
    
    echo ""
    echo -e "${CYAN}┌────────────────────────────────────────────────────────────────────────────┐${NC}"
    echo -e "${CYAN}│${NC} ${BOLD}STEP ${step_num}/${TOTAL_STEPS}:${NC} ${step_name}"
    echo -e "${CYAN}│${NC} ${GRAY}Started at: $(get_timestamp)${NC}"
    echo -e "${CYAN}└────────────────────────────────────────────────────────────────────────────┘${NC}"
    echo ""
    
    log "INFO" "Starting Step ${step_num}/${TOTAL_STEPS}: ${step_name}"
}

print_info() {
    echo -e "${GREEN}${CHECK_MARK}${NC} $1"
    log "INFO" "$1"
}

print_success() {
    echo -e "${GREEN}${SPARKLES} ${BOLD}SUCCESS:${NC} $1"
    log "SUCCESS" "$1"
}

print_warning() {
    echo -e "${YELLOW}${WARNING_SIGN}${NC} ${YELLOW}WARNING:${NC} $1"
    log "WARNING" "$1"
}

print_error() {
    echo -e "${RED}${CROSS_MARK}${NC} ${RED}ERROR:${NC} $1" >&2
    log "ERROR" "$1"
}

print_debug() {
    if [[ $VERBOSE_MODE -eq 1 ]]; then
        echo -e "${GRAY}${BUG} DEBUG: $1${NC}"
    fi
    log "DEBUG" "$1"
}

print_progress() {
    echo -e "${BLUE}${ARROW}${NC} $1"
    log "PROGRESS" "$1"
}

print_metric() {
    local name="$1"
    local value="$2"
    echo -e "${GRAY}  ${BULLET} ${name}: ${WHITE}${value}${NC}"
    log "METRIC" "${name}=${value}"
}

# ──────────────────────────────────────────────────────────────────────────────
# Command Execution with Enhanced Error Handling
# ──────────────────────────────────────────────────────────────────────────────
execute_command() {
    local cmd="$1"
    local description="${2:-Executing command}"
    local allow_failure="${3:-0}"
    local output_file="${4:-}"
    
    print_progress "$description"
    print_debug "Command: $cmd"
    
    local start_time="$(date +%s)"
    local exit_code=0
    
    if [[ $DRY_RUN -eq 1 ]]; then
        print_info "[DRY RUN] Would execute: $cmd"
        return 0
    fi
    
    # Execute with optional output redirection
    if [[ -n "$output_file" ]]; then
        if eval "$cmd" > "$output_file" 2>&1; then
            exit_code=0
        else
            exit_code=$?
        fi
    else
        if eval "$cmd"; then
            exit_code=0
        else
            exit_code=$?
        fi
    fi
    
    local end_time="$(date +%s)"
    local elapsed=$((end_time - start_time))
    
    if [[ $exit_code -eq 0 ]]; then
        print_info "Completed in ${elapsed}s"
    else
        if [[ $allow_failure -eq 0 ]]; then
            print_error "Command failed with exit code $exit_code after ${elapsed}s"
            print_error "Command was: $cmd"
            fail "$description failed"
        else
            print_warning "Command returned non-zero exit code $exit_code (ignored)"
        fi
    fi
    
    return $exit_code
}

run_with_spinner() {
    local cmd="$1"
    local msg="$2"
    
    eval "$cmd" &
    local pid=$!
    spinner $pid "$msg"
    wait $pid
    return $?
}

# ──────────────────────────────────────────────────────────────────────────────
# Error Handling and Recovery
# ──────────────────────────────────────────────────────────────────────────────
save_state() {
    local state_json="{"
    for key in "${!BUILD_STATE[@]}"; do
        state_json="${state_json}\"${key}\":${BUILD_STATE[$key]},"
    done
    state_json="${state_json%,}}"
    echo "$state_json" > "$STATE_FILE"
    print_debug "Build state saved to $STATE_FILE"
}

load_state() {
    if [[ -f "$STATE_FILE" ]]; then
        print_info "Found previous build state, loading..."
        # Simple JSON parsing (would need jq for proper parsing)
        while IFS='=' read -r key value; do
            BUILD_STATE["$key"]="$value"
        done < <(grep -oP '"\K[^"]+(?=":)|(?<=":)\d+' "$STATE_FILE" | paste -d= - -)
        return 0
    fi
    return 1
}

cleanup() {
    local exit_code=$?
    
    if [[ $exit_code -eq 0 ]]; then
        print_success "Build completed successfully!"
    else
        print_error "Build failed with exit code $exit_code"
        save_state
        print_info "Build state saved. You can resume from where it failed."
    fi
    
    # Restore file descriptors
    exec 1>&3 2>&4
    
    # Generate summary
    generate_summary
    
    exit $exit_code
}

fail() {
    local msg="${1:-Build failed}"
    local line="${2:-${LINENO}}"
    
    print_error "$msg (at line $line)"
    
    # Save diagnostic information
    echo "=== FAILURE DIAGNOSTICS ===" >> "$ERROR_LOG"
    echo "Message: $msg" >> "$ERROR_LOG"
    echo "Line: $line" >> "$ERROR_LOG"
    echo "Last command: ${BASH_COMMAND}" >> "$ERROR_LOG"
    echo "Call stack:" >> "$ERROR_LOG"
    local frame=0
    while caller $frame >> "$ERROR_LOG"; do
        ((frame++))
    done
    
    # Show last 50 lines of log
    echo ""
    print_error "Build failed! Check logs for details:"
    echo "  Main log: $LOG_FILE"
    echo "  Error log: $ERROR_LOG"
    echo ""
    echo "Last 50 lines of main log:"
    echo "════════════════════════════════════════════════════════════════════════════"
    tail -n 50 "$LOG_FILE" 2>/dev/null || echo "(log file not readable)"
    echo "════════════════════════════════════════════════════════════════════════════"
    
    exit 1
}

# Set up error traps
trap cleanup EXIT
trap 'fail "Unexpected error at line $LINENO"' ERR
trap 'print_warning "Interrupted by user"; exit 130' INT TERM

# ──────────────────────────────────────────────────────────────────────────────
# System Requirements Checking
# ──────────────────────────────────────────────────────────────────────────────
check_system_requirements() {
    print_section 1 "System Requirements Check"
    
    local errors=0
    local warnings=0
    
    # Check OS
    print_progress "Checking operating system..."
    case "$OS_TYPE" in
        Linux)
            print_info "Operating System: Linux (${OS_TYPE} ${KERNEL_VERSION})"
            if [[ -f /etc/os-release ]]; then
                . /etc/os-release
                print_metric "Distribution" "${NAME} ${VERSION}"
            fi
            ;;
        Darwin)
            print_info "Operating System: macOS"
            print_metric "Version" "$(sw_vers -productVersion)"
            ;;
        *)
            print_error "Unsupported operating system: $OS_TYPE"
            ((errors++))
            ;;
    esac
    
    # Check architecture
    print_progress "Checking system architecture..."
    print_metric "Architecture" "$ARCH_TYPE"
    case "$ARCH_TYPE" in
        x86_64|amd64)
            print_info "64-bit x86 architecture detected"
            ;;
        aarch64|arm64)
            print_info "ARM64 architecture detected"
            ;;
        *)
            print_warning "Untested architecture: $ARCH_TYPE"
            ((warnings++))
            ;;
    esac
    
    # Check memory
    print_progress "Checking system memory..."
    print_metric "Total Memory" "$(format_size $TOTAL_MEMORY)"
    if [[ $MEMORY_GB -lt 4 ]]; then
        print_error "Insufficient memory: ${MEMORY_GB}GB (minimum 4GB required)"
        ((errors++))
    elif [[ $MEMORY_GB -lt 8 ]]; then
        print_warning "Low memory: ${MEMORY_GB}GB (8GB+ recommended)"
        ((warnings++))
    else
        print_info "Memory check passed: ${MEMORY_GB}GB"
    fi
    
    # Check disk space
    print_progress "Checking disk space..."
    local available_space=$(df "$HOME" | awk 'NR==2 {print $4}')
    local available_gb=$((available_space / 1048576))
    print_metric "Available Space" "${available_gb}GB"
    if [[ $available_gb -lt 10 ]]; then
        print_error "Insufficient disk space: ${available_gb}GB (minimum 10GB required)"
        ((errors++))
    elif [[ $available_gb -lt 20 ]]; then
        print_warning "Low disk space: ${available_gb}GB (20GB+ recommended)"
        ((warnings++))
    else
        print_info "Disk space check passed: ${available_gb}GB"
    fi
    
    # Check CPU cores
    print_progress "Checking CPU information..."
    print_metric "CPU Cores" "$NUM_CORES"
    if [[ -f /proc/cpuinfo ]]; then
        local cpu_model=$(grep "model name" /proc/cpuinfo | head -1 | cut -d: -f2 | xargs)
        print_metric "CPU Model" "$cpu_model"
    fi
    
    if [[ $NUM_CORES -lt 2 ]]; then
        print_warning "Low CPU core count: $NUM_CORES (build will be slow)"
        ((warnings++))
    else
        print_info "CPU check passed: $NUM_CORES cores"
    fi
    
    # Summary
    echo ""
    if [[ $errors -gt 0 ]]; then
        print_error "System requirements check failed with $errors error(s)"
        return 1
    elif [[ $warnings -gt 0 ]]; then
        print_warning "System requirements check passed with $warnings warning(s)"
    else
        print_success "All system requirements met!"
    fi
    
    return 0
}

# ──────────────────────────────────────────────────────────────────────────────
# Dependency Checking
# ──────────────────────────────────────────────────────────────────────────────
check_dependencies() {
    print_section 2 "Dependency Check"
    
    local missing_deps=()
    local optional_deps=()
    
    # Required dependencies
    local required_tools=(
        "git:Git version control"
        "cmake:CMake build system (3.16+)"
        "make:GNU Make"
        "gcc:GNU C Compiler"
        "g++:GNU C++ Compiler"
        "pkg-config:Package configuration tool"
        "wget:File downloader"
        "curl:URL transfer tool"
        "tar:Archive tool"
        "unzip:ZIP extractor"
    )
    
    # Optional but recommended
    local optional_tools=(
        "ccache:Compiler cache"
        "ninja:Ninja build system"
        "clang:Clang compiler"
        "clang++:Clang++ compiler"
        "lld:LLVM linker"
        "mold:Modern linker"
        "sccache:Shared compilation cache"
    )
    
    # Check required tools
    print_progress "Checking required dependencies..."
    for tool_desc in "${required_tools[@]}"; do
        local tool="${tool_desc%%:*}"
        local desc="${tool_desc#*:}"
        
        if command -v "$tool" &>/dev/null; then
            local version="$(get_tool_version "$tool")"
            print_info "${CHECK_MARK} $desc: $version"
        else
            print_error "${CROSS_MARK} $desc: NOT FOUND"
            missing_deps+=("$tool")
        fi
    done
    
    # Check optional tools
    echo ""
    print_progress "Checking optional dependencies..."
    for tool_desc in "${optional_tools[@]}"; do
        local tool="${tool_desc%%:*}"
        local desc="${tool_desc#*:}"
        
        if command -v "$tool" &>/dev/null; then
            local version="$(get_tool_version "$tool")"
            print_info "${CHECK_MARK} $desc: $version"
            
            # Enable features based on available tools
            case "$tool" in
                ccache)
                    USE_CCACHE=1
                    print_info "  ${ARROW} ccache will be used to speed up rebuilds"
                    ;;
                ninja)
                    if [[ $USE_NINJA -eq 1 ]]; then
                        print_info "  ${ARROW} Ninja will be used instead of Make"
                    fi
                    ;;
            esac
        else
            print_debug "Optional tool not found: $tool"
            optional_deps+=("$tool")
        fi
    done
    
    # Check libraries
    echo ""
    print_progress "Checking system libraries..."
    
    local required_libs=(
        "libssl:OpenSSL"
        "libz:zlib compression"
        "libpthread:POSIX threads"
    )
    
    for lib_desc in "${required_libs[@]}"; do
        local lib="${lib_desc%%:*}"
        local desc="${lib_desc#*:}"
        
        if pkg-config --exists "${lib#lib}" 2>/dev/null || \
           ldconfig -p 2>/dev/null | grep -q "$lib"; then
            print_info "${CHECK_MARK} $desc found"
        else
            print_warning "${WARNING_SIGN} $desc might be missing"
        fi
    done
    
    # Summary
    echo ""
    if [[ ${#missing_deps[@]} -gt 0 ]]; then
        print_error "Missing required dependencies: ${missing_deps[*]}"
        echo ""
        print_info "Install missing dependencies with:"
        suggest_install_command "${missing_deps[@]}"
        return 1
    else
        print_success "All required dependencies are installed!"
    fi
    
    if [[ ${#optional_deps[@]} -gt 0 ]]; then
        print_info "Optional tools not found: ${optional_deps[*]}"
        print_info "Build will proceed without these optimizations"
    fi
    
    return 0
}

get_tool_version() {
    local tool="$1"
    case "$tool" in
        git)
            git --version 2>/dev/null | awk '{print $3}'
            ;;
        cmake)
            cmake --version 2>/dev/null | head -n1 | awk '{print $3}'
            ;;
        gcc|g++)
            $tool --version 2>/dev/null | head -n1 | awk '{print $NF}'
            ;;
        clang|clang++)
            $tool --version 2>/dev/null | head -n1 | awk '{print $3}'
            ;;
        make)
            make --version 2>/dev/null | head -n1 | awk '{print $3}'
            ;;
        ninja)
            ninja --version 2>/dev/null
            ;;
        *)
            command -v "$tool" &>/dev/null && echo "installed" || echo "not found"
            ;;
    esac
}

suggest_install_command() {
    local deps="$*"
    
    if [[ -f /etc/debian_version ]]; then
        echo "  sudo apt-get update && sudo apt-get install -y $deps"
    elif [[ -f /etc/redhat-release ]]; then
        echo "  sudo yum install -y $deps"
    elif [[ -f /etc/arch-release ]]; then
        echo "  sudo pacman -S $deps"
    elif [[ "$OS_TYPE" == "Darwin" ]]; then
        echo "  brew install $deps"
    else
        echo "  Please install: $deps"
    fi
}

# ──────────────────────────────────────────────────────────────────────────────
# Compiler Detection and Configuration
# ──────────────────────────────────────────────────────────────────────────────
configure_compiler() {
    print_section 3 "Compiler Configuration"
    
    # Detect and select C compiler
    print_progress "Detecting C compiler..."
    local cc_candidates=("gcc-13" "gcc-12" "gcc-11" "gcc" "clang")
    for cc in "${cc_candidates[@]}"; do
        if command -v "$cc" &>/dev/null; then
            export CC="$(command -v "$cc")"
            break
        fi
    done
    
    if [[ -z "$CC" ]]; then
        print_error "No C compiler found!"
        return 1
    fi
    
    local cc_version="$($CC --version 2>/dev/null | head -n1)"
    print_info "C Compiler: $CC"
    print_metric "Version" "$cc_version"
    
    # Detect and select C++ compiler
    print_progress "Detecting C++ compiler..."
    local cxx_candidates=("g++-13" "g++-12" "g++-11" "g++" "clang++")
    for cxx in "${cxx_candidates[@]}"; do
        if command -v "$cxx" &>/dev/null; then
            export CXX="$(command -v "$cxx")"
            break
        fi
    done
    
    if [[ -z "$CXX" ]]; then
        print_error "No C++ compiler found!"
        return 1
    fi
    
    local cxx_version="$($CXX --version 2>/dev/null | head -n1)"
    print_info "C++ Compiler: $CXX"
    print_metric "Version" "$cxx_version"
    
    # Configure compiler flags
    print_progress "Configuring compiler flags..."
    
    export CFLAGS="${CFLAGS:--O2 -pipe}"
    export CXXFLAGS="${CXXFLAGS:--O2 -pipe}"
    export LDFLAGS="${LDFLAGS:-}"
    
    if [[ $ENABLE_NATIVE_ARCH -eq 1 ]]; then
        CFLAGS="$CFLAGS -march=native"
        CXXFLAGS="$CXXFLAGS -march=native"
        print_info "Native architecture optimizations enabled"
    fi
    
    if [[ $ENABLE_LTO -eq 1 ]]; then
        CFLAGS="$CFLAGS -flto"
        CXXFLAGS="$CXXFLAGS -flto"
        LDFLAGS="$LDFLAGS -flto"
        print_info "Link-time optimization (LTO) enabled"
    fi
    
    if [[ -n "$SANITIZERS" ]]; then
        CFLAGS="$CFLAGS -fsanitize=$SANITIZERS"
        CXXFLAGS="$CXXFLAGS -fsanitize=$SANITIZERS"
        LDFLAGS="$LDFLAGS -fsanitize=$SANITIZERS"
        print_info "Sanitizers enabled: $SANITIZERS"
    fi
    
    print_metric "CFLAGS" "$CFLAGS"
    print_metric "CXXFLAGS" "$CXXFLAGS"
    print_metric "LDFLAGS" "$LDFLAGS"
    
    # Configure build tools
    if [[ $USE_CCACHE -eq 1 ]] && command -v ccache &>/dev/null; then
        export CC="ccache $CC"
        export CXX="ccache $CXX"
        print_info "ccache enabled for faster rebuilds"
        
        # Show ccache stats
        if ccache -s &>/dev/null; then
            local cache_size=$(ccache -s | grep "cache size" | awk '{print $3, $4}')
            print_metric "ccache size" "$cache_size"
        fi
    fi
    
    # Select build system
    if [[ $USE_NINJA -eq 1 ]] && command -v ninja &>/dev/null; then
        export CMAKE_GENERATOR="Ninja"
        print_info "Using Ninja build system"
    else
        export CMAKE_GENERATOR="Unix Makefiles"
        print_info "Using Make build system"
    fi
    
    # Test compiler
    print_progress "Testing compiler..."
    local test_file="/tmp/compiler_test_$$.c"
    cat > "$test_file" <<EOF
#include <stdio.h>
int main() {
    printf("Compiler test successful\\n");
    return 0;
}
EOF
    
    if $CC -o "/tmp/test_$$" "$test_file" 2>/dev/null && /tmp/test_$$ &>/dev/null; then
        print_info "Compiler test passed"
        rm -f "/tmp/test_$$" "$test_file"
    else
        print_error "Compiler test failed"
        rm -f "/tmp/test_$$" "$test_file"
        return 1
    fi
    
    return 0
}

# ──────────────────────────────────────────────────────────────────────────────
# Permission Checking
# ──────────────────────────────────────────────────────────────────────────────
check_permissions() {
    print_section 4 "Permission Check"
    
    print_progress "Checking installation directory permissions..."
    
    # Check if we can write to the install prefix
    if [[ -w "$INSTALL_PREFIX" ]]; then
        print_info "${CHECK_MARK} Write access to $INSTALL_PREFIX"
    else
        print_warning "${WARNING_SIGN} No write access to $INSTALL_PREFIX"
        
        echo ""
        print_info "Options to fix this:"
        echo ""
        echo "1. ${BOLD}Run with sudo (RECOMMENDED for system-wide installation):${NC}"
        echo "   sudo $0"
        echo ""
        echo "2. ${BOLD}Use user-local installation:${NC}"
        echo "   $0 --prefix=$USER_INSTALL_PREFIX"
        echo ""
        echo "3. ${BOLD}Grant yourself write access:${NC}"
        echo "   sudo chown -R \$USER $INSTALL_PREFIX"
        echo ""
        
        read -rp "Choose option (1/2/3) or press Enter to continue anyway: " choice
        
        case "$choice" in
            1)
                print_info "Please run: sudo $0"
                exit 0
                ;;
            2)
                INSTALL_PREFIX="$USER_INSTALL_PREFIX"
                print_info "Switching to user-local installation: $INSTALL_PREFIX"
                mkdir -p "$INSTALL_PREFIX"
                ;;
            3)
                print_info "Please run: sudo chown -R \$USER $INSTALL_PREFIX"
                exit 0
                ;;
            *)
                print_warning "Continuing with potentially limited permissions..."
                ;;
        esac
    fi
    
    # Check CRYPTOGRAM directory
    print_progress "Checking CRYPTOGRAM directory..."
    if [[ ! -d "$CRYPTOGRAM_ROOT" ]]; then
        print_error "CRYPTOGRAM directory not found at: $CRYPTOGRAM_ROOT"
        return 1
    fi
    
    if [[ -w "$CRYPTOGRAM_ROOT" ]]; then
        print_info "${CHECK_MARK} Write access to CRYPTOGRAM directory"
    else
        print_error "${CROSS_MARK} No write access to CRYPTOGRAM directory"
        return 1
    fi
    
    # Check for required CRYPTOGRAM files
    print_progress "Verifying CRYPTOGRAM structure..."
    local required_files=(
        "CMakeLists.txt"
        "Telegram/CMakeLists.txt"
    )
    
    for file in "${required_files[@]}"; do
        if [[ -f "$CRYPTOGRAM_ROOT/$file" ]]; then
            print_info "${CHECK_MARK} Found: $file"
        else
            print_error "${CROSS_MARK} Missing: $file"
            return 1
        fi
    done
    
    return 0
}

# ──────────────────────────────────────────────────────────────────────────────
# Build Ada URL Parser
# ──────────────────────────────────────────────────────────────────────────────
build_ada() {
    print_section 5 "Building Ada URL Parser"
    
    local ada_dir="/tmp/ada_build_$$"
    local ada_repo="https://github.com/ada-url/ada.git"
    
    # Check if already built
    if [[ ${BUILD_STATE["ada_installed"]} -eq 1 ]] && [[ $FORCE_REBUILD -eq 0 ]]; then
        print_info "Ada already installed, skipping..."
        return 0
    fi
    
    # Clean previous build
    print_progress "Cleaning previous Ada build directory..."
    execute_command "rm -rf '$ada_dir'" "Removing old Ada directory" 1
    
    # Clone repository
    print_progress "Cloning Ada repository..."
    execute_command "git clone --depth 1 --branch '$ADA_VERSION' '$ada_repo' '$ada_dir'" \
                   "Cloning Ada from GitHub"
    
    BUILD_STATE["ada_cloned"]=1
    save_state
    
    # Show repository info
    cd "$ada_dir"
    local commit_hash=$(git rev-parse --short HEAD)
    print_metric "Repository" "$ada_repo"
    print_metric "Branch/Tag" "$ADA_VERSION"
    print_metric "Commit" "$commit_hash"
    
    # Configure build
    print_progress "Configuring Ada build..."
    mkdir -p build && cd build
    
    local cmake_args=(
        "-DCMAKE_BUILD_TYPE=Release"
        "-DCMAKE_INSTALL_PREFIX=$INSTALL_PREFIX"
        "-DADA_BENCHMARKS=OFF"
        "-DBUILD_TESTING=OFF"
    )
    
    execute_command "cmake .. ${cmake_args[*]}" "Running CMake for Ada"
    
    BUILD_STATE["ada_built"]=1
    save_state
    
    # Build
    print_progress "Compiling Ada..."
    local build_start=$(date +%s)
    
    if [[ "$CMAKE_GENERATOR" == "Ninja" ]]; then
        execute_command "ninja -j$PARALLEL_JOBS" "Building Ada with Ninja"
    else
        execute_command "make -j$PARALLEL_JOBS" "Building Ada with Make"
    fi
    
    local build_end=$(date +%s)
    local build_time=$((build_end - build_start))
    print_metric "Build time" "$(get_elapsed_time $build_start $build_end)"
    
    # Install
    print_progress "Installing Ada to $INSTALL_PREFIX..."
    if [[ "$CMAKE_GENERATOR" == "Ninja" ]]; then
        execute_command "ninja install" "Installing Ada"
    else
        execute_command "make install" "Installing Ada"
    fi
    
    BUILD_STATE["ada_installed"]=1
    save_state
    
    # Verify installation
    print_progress "Verifying Ada installation..."
    verify_ada_installation
    
    # Clean up
    cd /
    rm -rf "$ada_dir"
    
    print_success "Ada URL Parser built and installed successfully!"
    return 0
}

verify_ada_installation() {
    local checks_passed=0
    local checks_failed=0
    
    # Check library files
    if ls $INSTALL_PREFIX/lib/libada.* &>/dev/null; then
        print_info "${CHECK_MARK} Ada library found in $INSTALL_PREFIX/lib/"
        ((checks_passed++))
    else
        print_error "${CROSS_MARK} Ada library NOT found"
        ((checks_failed++))
    fi
    
    # Check CMake config
    if [[ -f "$INSTALL_PREFIX/lib/cmake/ada/ada-config.cmake" ]]; then
        print_info "${CHECK_MARK} Ada CMake config found"
        ((checks_passed++))
    else
        print_error "${CROSS_MARK} Ada CMake config NOT found"
        ((checks_failed++))
    fi
    
    # Check headers
    if [[ -d "$INSTALL_PREFIX/include/ada" ]]; then
        local header_count=$(find "$INSTALL_PREFIX/include/ada" -name "*.h" | wc -l)
        print_info "${CHECK_MARK} Ada headers found ($header_count files)"
        ((checks_passed++))
    else
        print_error "${CROSS_MARK} Ada headers NOT found"
        ((checks_failed++))
    fi
    
    if [[ $checks_failed -gt 0 ]]; then
        fail "Ada installation verification failed"
    fi
}

# ──────────────────────────────────────────────────────────────────────────────
# Build Protobuf
# ──────────────────────────────────────────────────────────────────────────────
build_protobuf() {
    print_section 6 "Building Protocol Buffers"
    
    local protobuf_dir="/tmp/protobuf_build_$$"
    local protobuf_repo="https://github.com/protocolbuffers/protobuf.git"
    
    # Check if already built
    if [[ ${BUILD_STATE["protobuf_installed"]} -eq 1 ]] && [[ $FORCE_REBUILD -eq 0 ]]; then
        print_info "Protobuf already installed, skipping..."
        return 0
    fi
    
    # Clean previous build
    print_progress "Cleaning previous Protobuf build directory..."
    execute_command "rm -rf '$protobuf_dir'" "Removing old Protobuf directory" 1
    
    # Clone repository
    print_progress "Cloning Protobuf repository..."
    print_warning "This is a large repository, cloning may take a few minutes..."
    execute_command "git clone --depth 1 --branch '$PROTOBUF_VERSION' '$protobuf_repo' '$protobuf_dir'" \
                   "Cloning Protobuf from GitHub"
    
    BUILD_STATE["protobuf_cloned"]=1
    save_state
    
    # Show repository info
    cd "$protobuf_dir"
    local commit_hash=$(git rev-parse --short HEAD)
    print_metric "Repository" "$protobuf_repo"
    print_metric "Branch/Tag" "$PROTOBUF_VERSION"
    print_metric "Commit" "$commit_hash"
    
    # Configure build
    print_progress "Configuring Protobuf build..."
    mkdir -p build && cd build
    
    local cmake_args=(
        "-DCMAKE_BUILD_TYPE=Release"
        "-DCMAKE_INSTALL_PREFIX=$INSTALL_PREFIX"
        "-Dprotobuf_BUILD_TESTS=OFF"
        "-Dprotobuf_BUILD_EXAMPLES=OFF"
        "-Dprotobuf_BUILD_PROTOC_BINARIES=ON"
        "-Dprotobuf_BUILD_SHARED_LIBS=ON"
        "-DCMAKE_CXX_STANDARD=14"
    )
    
    execute_command "cmake .. ${cmake_args[*]}" "Running CMake for Protobuf"
    
    BUILD_STATE["protobuf_built"]=1
    save_state
    
    # Build
    print_progress "Compiling Protobuf (this will take 10-20 minutes)..."
    print_info "Building with $PARALLEL_JOBS parallel jobs"
    local build_start=$(date +%s)
    
    # Show progress during build
    if [[ "$CMAKE_GENERATOR" == "Ninja" ]]; then
        execute_command "ninja -j$PARALLEL_JOBS" "Building Protobuf with Ninja"
    else
        execute_command "make -j$PARALLEL_JOBS VERBOSE=1" "Building Protobuf with Make"
    fi
    
    local build_end=$(date +%s)
    print_metric "Build time" "$(get_elapsed_time $build_start $build_end)"
    
    # Install
    print_progress "Installing Protobuf to $INSTALL_PREFIX..."
    if [[ "$CMAKE_GENERATOR" == "Ninja" ]]; then
        execute_command "ninja install" "Installing Protobuf"
    else
        execute_command "make install" "Installing Protobuf"
    fi
    
    BUILD_STATE["protobuf_installed"]=1
    save_state
    
    # Update library cache
    if [[ "$OS_TYPE" == "Linux" ]] && command -v ldconfig &>/dev/null; then
        print_progress "Updating library cache..."
        execute_command "ldconfig" "Running ldconfig" 1
    fi
    
    # Verify installation
    print_progress "Verifying Protobuf installation..."
    verify_protobuf_installation
    
    # Clean up
    cd /
    rm -rf "$protobuf_dir"
    
    print_success "Protocol Buffers built and installed successfully!"
    return 0
}

verify_protobuf_installation() {
    local checks_passed=0
    local checks_failed=0
    
    # Check library files
    if ls $INSTALL_PREFIX/lib/libprotobuf.* &>/dev/null; then
        local lib_size=$(du -h $INSTALL_PREFIX/lib/libprotobuf.* | head -1 | cut -f1)
        print_info "${CHECK_MARK} Protobuf library found (size: $lib_size)"
        ((checks_passed++))
    else
        print_error "${CROSS_MARK} Protobuf library NOT found"
        ((checks_failed++))
    fi
    
    # Check protoc compiler
    if [[ -x "$INSTALL_PREFIX/bin/protoc" ]]; then
        local protoc_version=$($INSTALL_PREFIX/bin/protoc --version 2>/dev/null | awk '{print $2}')
        print_info "${CHECK_MARK} protoc compiler found (version: $protoc_version)"
        ((checks_passed++))
    else
        print_error "${CROSS_MARK} protoc compiler NOT found"
        ((checks_failed++))
    fi
    
    # Check CMake config
    local cmake_config=$(find $INSTALL_PREFIX -name "protobuf-config.cmake" -o -name "ProtobufConfig.cmake" 2>/dev/null | head -1)
    if [[ -n "$cmake_config" ]]; then
        print_info "${CHECK_MARK} Protobuf CMake config found"
        ((checks_passed++))
    else
        print_warning "${WARNING_SIGN} Protobuf CMake config not found (may need manual configuration)"
    fi
    
    # Check headers
    if [[ -d "$INSTALL_PREFIX/include/google/protobuf" ]]; then
        local header_count=$(find "$INSTALL_PREFIX/include/google/protobuf" -name "*.h" | wc -l)
        print_info "${CHECK_MARK} Protobuf headers found ($header_count files)"
        ((checks_passed++))
    else
        print_error "${CROSS_MARK} Protobuf headers NOT found"
        ((checks_failed++))
    fi
    
    if [[ $checks_failed -gt 0 ]]; then
        fail "Protobuf installation verification failed"
    fi
}

# ──────────────────────────────────────────────────────────────────────────────
# Configure CRYPTOGRAM
# ──────────────────────────────────────────────────────────────────────────────
configure_cryptogram() {
    print_section 7 "Configuring CRYPTOGRAM"
    
    # Create and enter build directory
    print_progress "Creating build directory: $BUILD_DIR"
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    
    # Clean previous configuration if requested
    if [[ $FORCE_REBUILD -eq 1 ]] || [[ ! -f "CMakeCache.txt" ]]; then
        print_progress "Cleaning previous build configuration..."
        execute_command "rm -rf CMakeCache.txt CMakeFiles" "Cleaning CMake cache" 1
    fi
    
    # Prepare CMake arguments
    print_progress "Preparing CMake configuration..."
    local cmake_args=(
        "-DCMAKE_BUILD_TYPE=Release"
        "-DCMAKE_PREFIX_PATH=$INSTALL_PREFIX"
        "-DDESKTOP_APP_DISABLE_AUTOUPDATE=ON"
        "-DDESKTOP_APP_DISABLE_CRASH_REPORTS=ON"
        "-DCMAKE_INSTALL_PREFIX=$BUILD_DIR/install"
    )
    
    # Add compiler specifications
    cmake_args+=(
        "-DCMAKE_C_COMPILER=$CC"
        "-DCMAKE_CXX_COMPILER=$CXX"
    )
    
    # Add optimization flags if enabled
    if [[ $ENABLE_LTO -eq 1 ]]; then
        cmake_args+=("-DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON")
    fi
    
    # Show configuration
    echo ""
    print_info "CMake configuration:"
    for arg in "${cmake_args[@]}"; do
        print_metric "  " "$arg"
    done
    echo ""
    
    # Run CMake
    print_progress "Running CMake configuration..."
    local config_start=$(date +%s)
    
    execute_command "cmake ${cmake_args[*]} '$CRYPTOGRAM_ROOT'" \
                   "Configuring CRYPTOGRAM with CMake"
    
    local config_end=$(date +%s)
    print_metric "Configuration time" "$(get_elapsed_time $config_start $config_end)"
    
    BUILD_STATE["cryptogram_configured"]=1
    save_state
    
    # Verify configuration
    print_progress "Verifying CMake configuration..."
    if [[ -f "CMakeCache.txt" ]]; then
        print_info "${CHECK_MARK} CMake cache generated"
        
        # Extract some useful information
        local build_type=$(grep CMAKE_BUILD_TYPE CMakeCache.txt | cut -d= -f2)
        local compiler=$(grep CMAKE_CXX_COMPILER: CMakeCache.txt | cut -d= -f2)
        print_metric "Build type" "$build_type"
        print_metric "Compiler" "$compiler"
    else
        fail "CMake configuration failed - no cache file generated"
    fi
    
    print_success "CRYPTOGRAM configured successfully!"
    return 0
}

# ──────────────────────────────────────────────────────────────────────────────
# Build CRYPTOGRAM
# ──────────────────────────────────────────────────────────────────────────────
build_cryptogram() {
    print_section 8 "Building CRYPTOGRAM Desktop"
    
    cd "$BUILD_DIR"
    
    # Check if configuration exists
    if [[ ! -f "CMakeCache.txt" ]]; then
        print_error "CMake configuration not found. Running configuration first..."
        configure_cryptogram || return 1
    fi
    
    # Display build information
    print_info "Build configuration:"
    print_metric "Build directory" "$BUILD_DIR"
    print_metric "Parallel jobs" "$PARALLEL_JOBS"
    print_metric "Available memory" "$(format_size $TOTAL_MEMORY)"
    
    # Estimate build time
    local estimated_time=$((20 + (60 - NUM_CORES * 5)))
    [[ $estimated_time -lt 20 ]] && estimated_time=20
    print_info "Estimated build time: ${estimated_time}-$((estimated_time * 2)) minutes"
    
    # Start build
    echo ""
    print_progress "Starting CRYPTOGRAM build..."
    print_warning "This is the longest step. Output will appear below:"
    echo ""
    echo "════════════════════════════════════════════════════════════════════════════"
    
    local build_start=$(date +%s)
    
    # Build with progress
    if [[ "$CMAKE_GENERATOR" == "Ninja" ]]; then
        execute_command "ninja -j$PARALLEL_JOBS" "Building CRYPTOGRAM with Ninja"
    else
        # Use make with verbose output for progress tracking
        execute_command "make -j$PARALLEL_JOBS VERBOSE=1" "Building CRYPTOGRAM with Make"
    fi
    
    local build_end=$(date +%s)
    echo "════════════════════════════════════════════════════════════════════════════"
    echo ""
    
    print_metric "Total build time" "$(get_elapsed_time $build_start $build_end)"
    
    BUILD_STATE["cryptogram_built"]=1
    save_state
    
    # Verify build
    print_progress "Verifying CRYPTOGRAM build..."
    verify_cryptogram_build
    
    print_success "CRYPTOGRAM built successfully!"
    return 0
}

verify_cryptogram_build() {
    print_progress "Looking for CRYPTOGRAM executable..."
    
    # Common locations for the executable
    local possible_paths=(
        "$BUILD_DIR/bin/Telegram"
        "$BUILD_DIR/Telegram"
        "$BUILD_DIR/bin/telegram-desktop"
        "$BUILD_DIR/telegram-desktop"
    )
    
    local exec_found=""
    for path in "${possible_paths[@]}"; do
        if [[ -f "$path" ]]; then
            exec_found="$path"
            break
        fi
    done
    
    # If not found in common locations, search recursively
    if [[ -z "$exec_found" ]]; then
        print_warning "Executable not in expected location, searching..."
        exec_found=$(find "$BUILD_DIR" -name "Telegram" -type f -executable 2>/dev/null | head -1)
    fi
    
    if [[ -n "$exec_found" ]] && [[ -x "$exec_found" ]]; then
        local file_size=$(stat -c%s "$exec_found" 2>/dev/null || stat -f%z "$exec_found" 2>/dev/null)
        print_success "Executable found: $exec_found"
        print_metric "File size" "$(format_size $file_size)"
        
        # Check if it's properly linked
        if command -v ldd &>/dev/null; then
            print_progress "Checking executable dependencies..."
            local missing_libs=$(ldd "$exec_found" 2>/dev/null | grep "not found" || true)
            if [[ -z "$missing_libs" ]]; then
                print_info "${CHECK_MARK} All dependencies resolved"
            else
                print_warning "${WARNING_SIGN} Missing libraries detected:"
                echo "$missing_libs"
            fi
        fi
        
        # Store executable path for summary
        CRYPTOGRAM_EXEC="$exec_found"
    else
        fail "CRYPTOGRAM executable not found!"
    fi
}

# ──────────────────────────────────────────────────────────────────────────────
# Generate Summary
# ──────────────────────────────────────────────────────────────────────────────
generate_summary() {
    local build_end_time=$(date +%s)
    local total_time=$((build_end_time - BUILD_START_TIME))
    
    {
        echo "════════════════════════════════════════════════════════════════════════════"
        echo "                         BUILD SUMMARY REPORT"
        echo "════════════════════════════════════════════════════════════════════════════"
        echo ""
        echo "Build ID: $BUILD_ID"
        echo "Date: $(date)"
        echo "Total Time: $(get_elapsed_time $BUILD_START_TIME $build_end_time)"
        echo ""
        echo "System Information:"
        echo "  OS: $OS_TYPE ($KERNEL_VERSION)"
        echo "  Architecture: $ARCH_TYPE"
        echo "  CPU Cores: $NUM_CORES"
        echo "  Memory: $(format_size $TOTAL_MEMORY)"
        echo ""
        echo "Compiler Configuration:"
        echo "  CC: ${CC:-not set}"
        echo "  CXX: ${CXX:-not set}"
        echo "  CFLAGS: ${CFLAGS:-default}"
        echo "  CXXFLAGS: ${CXXFLAGS:-default}"
        echo ""
        echo "Build Configuration:"
        echo "  Install Prefix: $INSTALL_PREFIX"
        echo "  Build Directory: $BUILD_DIR"
        echo "  Parallel Jobs: $PARALLEL_JOBS"
        echo "  Generator: ${CMAKE_GENERATOR:-Make}"
        echo "  ccache: $([ $USE_CCACHE -eq 1 ] && echo "enabled" || echo "disabled")"
        echo ""
        echo "Component Status:"
        for component in "${!BUILD_STATE[@]}"; do
            local status="$([ ${BUILD_STATE[$component]} -eq 1 ] && echo "✓ Complete" || echo "✗ Incomplete")"
            printf "  %-30s %s\n" "$component:" "$status"
        done
        echo ""
        
        if [[ -n "${CRYPTOGRAM_EXEC:-}" ]]; then
            echo "Build Artifacts:"
            echo "  Executable: $CRYPTOGRAM_EXEC"
            echo ""
        fi
        
        echo "Logs:"
        echo "  Main Log: $LOG_FILE"
        echo "  Error Log: $ERROR_LOG"
        echo "  Debug Log: $DEBUG_LOG"
        echo "  Metrics: $METRICS_LOG"
        echo ""
        
        # Add step timings
        if [[ ${#STEP_TIMES[@]} -gt 0 ]]; then
            echo "Step Timings:"
            for step in "${!STEP_TIMES[@]}"; do
                local step_start="${STEP_TIMES[$step]}"
                local step_end=$(date +%s)
                local step_time=$((step_end - step_start))
                printf "  %-30s %s\n" "${step//_/ }:" "$(get_elapsed_time $step_start $step_end)"
            done
            echo ""
        fi
        
        echo "════════════════════════════════════════════════════════════════════════════"
    } | tee "$SUMMARY_FILE"
    
    # Also save JSON metrics
    {
        echo "{"
        echo "  \"build_id\": \"$BUILD_ID\","
        echo "  \"date\": \"$(date -Iseconds)\","
        echo "  \"total_time_seconds\": $total_time,"
        echo "  \"system\": {"
        echo "    \"os\": \"$OS_TYPE\","
        echo "    \"arch\": \"$ARCH_TYPE\","
        echo "    \"cores\": $NUM_CORES,"
        echo "    \"memory_bytes\": $TOTAL_MEMORY"
        echo "  },"
        echo "  \"status\": {"
        for component in "${!BUILD_STATE[@]}"; do
            echo "    \"$component\": ${BUILD_STATE[$component]},"
        done | sed '$ s/,$//'
        echo "  }"
        echo "}"
    } > "${METRICS_LOG%.json}_final.json"
}

# ──────────────────────────────────────────────────────────────────────────────
# Show Final Instructions
# ──────────────────────────────────────────────────────────────────────────────
show_instructions() {
    print_header "${SPARKLES} BUILD COMPLETE! ${SPARKLES}"
    
    echo -e "${GREEN}${CHECKERED_FLAG} Congratulations! CRYPTOGRAM has been built successfully!${NC}"
    echo ""
    
    if [[ -n "${CRYPTOGRAM_EXEC:-}" ]]; then
        echo -e "${CYAN}${ROCKET} EXECUTABLE LOCATION:${NC}"
        echo -e "  ${WHITE}${CRYPTOGRAM_EXEC}${NC}"
        echo ""
        
        echo -e "${CYAN}${GEAR} NEXT STEPS:${NC}"
        echo ""
        echo "1. ${BOLD}Run CRYPTOGRAM standalone:${NC}"
        echo "   $CRYPTOGRAM_EXEC"
        echo ""
        echo "2. ${BOLD}Run with TSM integration:${NC}"
        echo "   cd $CRYPTOGRAM_ROOT"
        echo "   source .tsm_cryptogram_env.sh"
        echo "   python -m Telegram.lib_tsm.mock_server.server &"
        echo "   $CRYPTOGRAM_EXEC"
        echo ""
        echo "3. ${BOLD}Create desktop shortcut:${NC}"
        echo "   ln -s $CRYPTOGRAM_EXEC ~/Desktop/CRYPTOGRAM"
        echo ""
        echo "4. ${BOLD}Install system-wide (optional):${NC}"
        echo "   sudo cp $CRYPTOGRAM_EXEC /usr/local/bin/cryptogram"
        echo ""
    fi
    
    echo -e "${CYAN}${FILE} BUILD LOGS:${NC}"
    echo "  Main log:    $LOG_FILE"
    echo "  Summary:     $SUMMARY_FILE"
    echo "  Metrics:     ${METRICS_LOG%.json}_final.json"
    echo ""
    
    echo -e "${CYAN}${INFO_SIGN} TIPS:${NC}"
    echo "  • To rebuild from scratch: $0 --force"
    echo "  • To resume interrupted build: $0 --resume"
    echo "  • For faster rebuilds, ccache is $([ $USE_CCACHE -eq 1 ] && echo "enabled" || echo "disabled")"
    echo "  • View detailed metrics: cat ${METRICS_LOG%.json}_final.json | jq ."
    echo ""
    
    local total_time=$(get_elapsed_time $BUILD_START_TIME)
    echo -e "${GREEN}${CLOCK} Total build time: ${WHITE}${total_time}${NC}"
    echo ""
    echo -e "${MAGENTA}${FIRE} Happy messaging with CRYPTOGRAM! ${FIRE}${NC}"
    echo ""
}

# ──────────────────────────────────────────────────────────────────────────────
# Main Function
# ──────────────────────────────────────────────────────────────────────────────
main() {
    # Parse command line arguments
    while [[ $# -gt 0 ]]; do
        case "$1" in
            --help|-h)
                show_help
                exit 0
                ;;
            --version|-v)
                echo "CRYPTOGRAM Build Script v$SCRIPT_VERSION"
                exit 0
                ;;
            --verbose)
                VERBOSE_MODE=1
                ;;
            --quiet|-q)
                VERBOSE_MODE=0
                ;;
            --dry-run)
                DRY_RUN=1
                print_warning "DRY RUN MODE - No actual changes will be made"
                ;;
            --force)
                FORCE_REBUILD=1
                print_info "Force rebuild enabled"
                ;;
            --resume)
                load_state
                print_info "Resuming from previous build state"
                ;;
            --prefix=*)
                INSTALL_PREFIX="${1#*=}"
                print_info "Install prefix set to: $INSTALL_PREFIX"
                ;;
            --jobs=*|-j*)
                PARALLEL_JOBS="${1#*=}"
                PARALLEL_JOBS="${PARALLEL_JOBS#-j}"
                print_info "Parallel jobs set to: $PARALLEL_JOBS"
                ;;
            --no-ccache)
                USE_CCACHE=0
                ;;
            --ninja)
                USE_NINJA=1
                ;;
            --lto)
                ENABLE_LTO=1
                ;;
            --no-native)
                ENABLE_NATIVE_ARCH=0
                ;;
            --sanitize=*)
                SANITIZERS="${1#*=}"
                ;;
            --skip-deps)
                SKIP_DEPENDENCIES=1
                ;;
            *)
                print_error "Unknown option: $1"
                show_help
                exit 1
                ;;
        esac
        shift
    done
    
    # Initialize environment
    initialize_environment
    
    # Show header
    clear
    print_header "TSM + CRYPTOGRAM Ultimate Build System v$SCRIPT_VERSION"
    
    echo -e "${CYAN}Build Configuration:${NC}"
    print_metric "Build ID" "$BUILD_ID"
    print_metric "Date" "$(date)"
    print_metric "Root Directory" "$CRYPTOGRAM_ROOT"
    print_metric "Build Directory" "$BUILD_DIR"
    print_metric "Install Prefix" "$INSTALL_PREFIX"
    print_metric "Parallel Jobs" "$PARALLEL_JOBS"
    echo ""
    
    # Run build steps
    check_system_requirements || fail "System requirements not met"
    
    if [[ $SKIP_DEPENDENCIES -eq 0 ]]; then
        check_dependencies || fail "Missing dependencies"
    fi
    
    configure_compiler || fail "Compiler configuration failed"
    check_permissions || fail "Permission check failed"
    
    # Build components
    build_ada || fail "Ada build failed"
    build_protobuf || fail "Protobuf build failed"
    configure_cryptogram || fail "CRYPTOGRAM configuration failed"
    build_cryptogram || fail "CRYPTOGRAM build failed"
    
    # Show completion
    show_instructions
    
    # Clean exit
    exit 0
}

# ──────────────────────────────────────────────────────────────────────────────
# Help Function
# ──────────────────────────────────────────────────────────────────────────────
show_help() {
    cat <<EOF
${BOLD}TSM + CRYPTOGRAM Ultimate Build Script${NC}
Version $SCRIPT_VERSION

${BOLD}USAGE:${NC}
    $0 [OPTIONS]

${BOLD}OPTIONS:${NC}
    -h, --help              Show this help message
    -v, --version           Show version information
    --verbose               Enable verbose output (default)
    -q, --quiet             Reduce output verbosity
    --dry-run               Show what would be done without doing it
    --force                 Force rebuild all components
    --resume                Resume from previous build state
    --prefix=PATH           Set installation prefix (default: /usr/local)
    -j, --jobs=N            Number of parallel build jobs (default: auto)
    --no-ccache             Disable ccache even if available
    --ninja                 Use Ninja build system if available
    --lto                   Enable link-time optimization
    --no-native             Disable native architecture optimizations
    --sanitize=LIST         Enable sanitizers (address,undefined,thread)
    --skip-deps             Skip dependency checking

${BOLD}ENVIRONMENT VARIABLES:${NC}
    CRYPTOGRAM_ROOT         CRYPTOGRAM source directory (default: ~/CRYPTOGRAM)
    CC                      C compiler (auto-detected)
    CXX                     C++ compiler (auto-detected)
    CFLAGS                  Additional C compiler flags
    CXXFLAGS                Additional C++ compiler flags
    LDFLAGS                 Additional linker flags
    LOG_DIR                 Directory for build logs (default: /tmp/cryptogram_builds)
    DEBUG                   Enable debug logging (set to 1)

${BOLD}EXAMPLES:${NC}
    # Standard build
    $0

    # Force rebuild with 8 parallel jobs
    $0 --force -j8

    # Install to user directory
    $0 --prefix=\$HOME/.local

    # Resume interrupted build
    $0 --resume

    # Dry run to see what would be done
    $0 --dry-run

${BOLD}NOTES:${NC}
    • This script builds Ada URL parser, Protocol Buffers, and CRYPTOGRAM
    • Total build time is typically 30-90 minutes depending on hardware
    • At least 4GB RAM and 10GB disk space recommended
    • Requires sudo for system-wide installation to /usr/local

For more information, see the build logs in: $LOG_DIR

EOF
}

# ──────────────────────────────────────────────────────────────────────────────
# Script Entry Point
# ──────────────────────────────────────────────────────────────────────────────

# Only run main if not being sourced
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    main "$@"
fi
