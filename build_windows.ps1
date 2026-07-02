# CRYPTOGRAM Desktop Build Script (Windows)
# Builds CRYPTOGRAM Qt/C++ Desktop application for Windows x64
# Requires: Visual Studio 2022, Python 3.10+, Git

param(
    [string]$BuildType = "Release",
    [int]$Jobs = 0,
    [string]$ApiId = "",
    [string]$ApiHash = "",
    [switch]$Package = $true
)

$ErrorActionPreference = "Stop"

# Configuration
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$CryptogramRoot = $ScriptDir
$BuildDir = Join-Path $CryptogramRoot "out"
$BuildOutDir = Join-Path $BuildDir "Release"

if ($Jobs -eq 0) {
    $Jobs = $env:NUMBER_OF_PROCESSORS
    if (-not $Jobs) { $Jobs = 4 }
}

if (-not $ApiId)   { $ApiId = "17349" }
if (-not $ApiHash) { $ApiHash = "344583e45741c457fe1862106095a5eb" }

function Write-Info($msg)  { Write-Host "[INFO]  $msg" -ForegroundColor Green }
function Write-Warn($msg)  { Write-Host "[WARN]  $msg" -ForegroundColor Yellow }
function Write-Err($msg)   { Write-Host "[ERROR] $msg" -ForegroundColor Red }
function Write-Step($msg)  { Write-Host "`n=== $msg ===" -ForegroundColor Cyan }

Write-Host ""
Write-Host "========================================" -ForegroundColor Magenta
Write-Host "  CRYPTOGRAM Desktop - Windows Build" -ForegroundColor Magenta
Write-Host "========================================" -ForegroundColor Magenta
Write-Host ""

Write-Info "Build type: $BuildType"
Write-Info "Jobs:       $Jobs"
Write-Info "Root:       $CryptogramRoot"
Write-Info "Build dir:  $BuildDir"
Write-Info "API ID:     $ApiId"

# ── Step 1: Check prerequisites ──
Write-Step "Checking prerequisites"

# Check Python
$python = Get-Command python -ErrorAction SilentlyContinue
if (-not $python) {
    $python = Get-Command python3 -ErrorAction SilentlyContinue
}
if (-not $python) {
    Write-Err "Python not found in PATH. Please install Python 3.10+."
    exit 1
}
Write-Info "Python found: $($python.Source)"

# Check CMake
$cmake = Get-Command cmake -ErrorAction SilentlyContinue
if (-not $cmake) {
    Write-Err "CMake not found in PATH. Please install CMake."
    exit 1
}
Write-Info "CMake found: $($cmake.Source)"

# Check for Visual Studio
$vsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (Test-Path $vsWhere) {
    $vsPath = & $vsWhere -latest -property installationPath 2>$null
    if ($vsPath) {
        Write-Info "Visual Studio found: $vsPath"
    }
} else {
    Write-Warn "vswhere.exe not found. Assuming MSVC is in PATH."
}

# Check for Qt6
$qtDir = $env:QT_DIR
if (-not $qtDir) {
    # Try common locations
    $qtCandidates = @(
        "C:\Qt\6.11.1\msvc2022_64",
        "C:\Qt\6.9.0\msvc2022_64",
        "C:\Qt\6.8.0\msvc2022_64",
        "C:\Qt\6.7.0\msvc2022_64",
        "C:\Qt\6.6.0\msvc2022_64"
    )
    foreach ($candidate in $qtCandidates) {
        if (Test-Path $candidate) {
            $qtDir = $candidate
            break
        }
    }
}

if ($qtDir) {
    Write-Info "Qt6 found: $qtDir"
    $env:CMAKE_PREFIX_PATH = $qtDir
} else {
    Write-Warn "Qt6 not found. Will try to use DESKTOP_APP_USE_PACKAGED=ON"
    Write-Warn "Install Qt6 or set QT_DIR environment variable."
}

# ── Step 2: Configure with CMake ──
Write-Step "CMake Configuration"

# Set up MSVC environment if needed
$vcvars = "${env:ProgramFiles}\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
if (-not (Test-Path $vcvars)) {
    $vcvars = "${env:ProgramFiles}\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
}
if (-not (Test-Path $vcvars)) {
    $vcvars = "${env:ProgramFiles}\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat"
}

# Create build directory
if (-not (Test-Path $BuildDir)) {
    New-Item -ItemType Directory -Path $BuildDir -Force | Out-Null
}

$configureArgs = @(
    "configure.bat",
    "x64",
    "-D", "TDESKTOP_API_ID=$ApiId",
    "-D", "TDESKTOP_API_HASH=$ApiHash",
    "-D", "DESKTOP_APP_DISABLE_AUTOUPDATE=ON",
    "-D", "DESKTOP_APP_DISABLE_CRASH_REPORTS=ON"
)

# Use packaged libraries if no local Libraries folder
$libsDir = Join-Path $CryptogramRoot "Libraries"
if (-not (Test-Path $libsDir)) {
    Write-Info "No Libraries folder found, using DESKTOP_APP_USE_PACKAGED=ON"
    $configureArgs += @("-D", "DESKTOP_APP_USE_PACKAGED=ON")
}

Write-Info "Running: $($configureArgs -join ' ')"

$configureProcess = Start-Process -FilePath "cmd.exe" -ArgumentList "/c", ($configureArgs -join ' ') -WorkingDirectory (Join-Path $CryptogramRoot "Telegram") -NoNewWindow -Wait -PassThru
if ($configureProcess.ExitCode -ne 0) {
    Write-Err "CMake configuration failed (exit code: $($configureProcess.ExitCode))"
    exit 1
}
Write-Info "CMake configuration complete"

# ── Step 3: Build ──
Write-Step "Building"

$buildDir = Join-Path $CryptogramRoot "out"
# Find the actual build output directory
$possibleBuildDirs = @(
    (Join-Path $buildDir "Release"),
    (Join-Path $buildDir "Debug"),
    $buildDir
)

$actualBuildDir = $null
foreach ($dir in $possibleBuildDirs) {
    $hasCache = Test-Path (Join-Path $dir "CMakeCache.txt")
    $hasSln = Test-Path (Join-Path $dir "Telegram.sln")
    $hasNinja = Test-Path (Join-Path $dir "build.ninja")
    if ($hasCache -or $hasSln -or $hasNinja) {
        $actualBuildDir = $dir
        break
    }
}

if (-not $actualBuildDir) {
    # Search for CMakeCache.txt
    $cacheFiles = Get-ChildItem -Path $buildDir -Recurse -Filter "CMakeCache.txt" -ErrorAction SilentlyContinue
    if ($cacheFiles) {
        $actualBuildDir = Split-Path -Parent $cacheFiles[0].FullName
    }
}

if (-not $actualBuildDir) {
    Write-Err "Could not find CMake build directory under $buildDir"
    Write-Warn "Contents of build dir:"
    Get-ChildItem $buildDir -Recurse -Depth 2 | Select-Object FullName | Format-Table -AutoSize
    exit 1
}

Write-Info "Build directory: $actualBuildDir"

# Build with cmake --build
$buildArgs = @("--build", $actualBuildDir, "--parallel", $Jobs, "--config", $BuildType)
Write-Info "Running: cmake $($buildArgs -join ' ')"

$buildProcess = Start-Process -FilePath "cmake" -ArgumentList $buildArgs -NoNewWindow -Wait -PassThru
if ($buildProcess.ExitCode -ne 0) {
    Write-Err "Build failed (exit code: $($buildProcess.ExitCode))"
    exit 1
}
Write-Info "Build complete"

# ── Step 4: Locate binary ──
Write-Step "Locating binary"

$binary = $null
$binaryCandidates = @(
    (Join-Path $actualBuildDir "Release\Cryptogram.exe"),
    (Join-Path $actualBuildDir "Debug\Cryptogram.exe"),
    (Join-Path $actualBuildDir "Release\Telegram.exe"),
    (Join-Path $actualBuildDir "Debug\Telegram.exe"),
    (Join-Path $actualBuildDir "Cryptogram.exe"),
    (Join-Path $actualBuildDir "Telegram.exe")
)

foreach ($candidate in $binaryCandidates) {
    if (Test-Path $candidate) {
        $binary = $candidate
        break
    }
}

if (-not $binary) {
    # Search for it
    $exes = Get-ChildItem -Path $buildDir -Recurse -Filter "*.exe" -ErrorAction SilentlyContinue | Where-Object { $_.Name -match "Cryptogram|Telegram" }
    if ($exes) {
        $binary = $exes[0].FullName
    }
}

if (-not $binary) {
    Write-Err "No Cryptogram.exe or Telegram.exe found"
    Write-Warn "Searching for any .exe in build dir:"
    Get-ChildItem $buildDir -Recurse -Filter "*.exe" | Select-Object FullName | Format-Table -AutoSize
    exit 1
}

Write-Info "Found binary: $binary"
$binarySize = (Get-Item $binary).Length / 1MB
Write-Info ("Binary size: {0:N1} MB" -f $binarySize)

# ── Step 5: Package with windeployqt ──
if ($Package) {
    Write-Step "Packaging"

    $artifactsDir = Join-Path $CryptogramRoot "artifacts\windows"
    if (-not (Test-Path $artifactsDir)) {
        New-Item -ItemType Directory -Path $artifactsDir -Force | Out-Null
    }

    # Copy binary to artifacts
    $destExe = Join-Path $artifactsDir "Cryptogram.exe"
    Copy-Item $binary -Destination $destExe -Force
    Write-Info "Copied binary to $destExe"

    # Run windeployqt if available
    $windeployqt = $null
    if ($qtDir) {
        $windeployqt = Join-Path $qtDir "bin\windeployqt.exe"
    }
    if (-not (Test-Path $windeployqt)) {
        $windeployqt = Get-Command windeployqt -ErrorAction SilentlyContinue
        if ($windeployqt) { $windeployqt = $windeployqt.Source }
    }

    if ($windeployqt -and (Test-Path $windeployqt)) {
        Write-Info "Running windeployqt to bundle Qt DLLs..."
        & $windeployqt --release --no-translations --no-system-d3d-compiler --no-opengl-sw --skip-plugin-types "qmltooling,quick3dassetimport,quick3dparticles,scenegraph" $destExe
        Write-Info "windeployqt complete"
    } else {
        Write-Warn "windeployqt not found. Binary may not run without Qt DLLs."
        Write-Warn "Install Qt6 or set QT_DIR to enable windeployqt packaging."
    }

    # Create ZIP
    $zipPath = Join-Path $artifactsDir "cryptogram-windows-x64.zip"
    if (Test-Path $zipPath) { Remove-Item $zipPath -Force }
    Compress-Archive -Path "$artifactsDir\*" -DestinationPath $zipPath
    $zipSize = (Get-Item $zipPath).Length / 1MB
    Write-Info ("Created ZIP: $zipPath ({0:N1} MB)" -f $zipSize)
}

# ── Summary ──
Write-Host ""
Write-Host "========================================" -ForegroundColor Magenta
Write-Host "  BUILD SUCCESSFUL" -ForegroundColor Magenta
Write-Host "========================================" -ForegroundColor Magenta
Write-Host ""
Write-Host "Binary: $binary"
if ($Package) {
    Write-Host "Artifacts: $artifactsDir"
}
Write-Host ""
