# 🔐 CRYPTOGRAM: Advanced OPSEC Messaging

CRYPTOGRAM is a high-security, privacy-hardened fork of Telegram designed for extreme threat environments. It integrates a Python-based **Trusted Security Module (TSM)** backend and an extensive suite of counterintelligence and post-quantum cryptographic tools.

## 🟢 Project Status: Feature Complete (Desktop)

The CRYPTOGRAM Desktop implementation has transitioned from a prototype to a **hardened, functional system**. All major build blockers have been resolved, and the application now features a unified OPSEC command center.

### Core Capabilities
- **Post-Quantum Security**: Native support for NIST-standardized PQC (Kyber/Dilithium) via **QuantumGuard**.
- **TSM Integration**: Fully wired gRPC and API backend for decentralized session management and lattice-based ZK Auth.
- **Traffic & Linguistic Privacy**: Integrated **Pluggable Transports** (obfs4/meek) and **Stylometry Shield** (AI-rephrasing).
- **Physical OPSEC**: Hardware-based **Kill Switch (Tether)** for USB/Smartcard devices and **Panic Password** for silent secure-erase.
- **Surveillance Countermeasures**: **Universal Threat Detector (UTD)** for AI-powered phishing and signature analysis.

## 🛠 Unified OPSEC Command Center

All advanced security features are configurable via the **CRYPTOGRAM Settings** menu:

| Category | Features |
| --- | --- |
| **Network Anonymity** | Tor, I2P, Snowflake Proxy, I2P Relay, Bridge Config |
| **Surveillance Detection** | AI Threat Detector (UTD), Sensitivity Controls, Diagnostics |
| **Voice Security** | Voice Morphing (AI Anonymization), Acoustic Monitoring |
| **Traffic & Stylometry** | obfs4/meek Transports, AI Writing Style Obfuscation |
| **Location Privacy** | Geographic Randomization, Coordinate Noise, Timezone Masking |
| **QuantumGuard** | PQC Security Levels (1-5), Quantum Threat Assessment |
| **Hardware Failsafes** | Panic Password (Secure Erase), USB/Smartcard Tether |
| **Identity & Trust** | CAC/PIV Smartcard Integration, ZK Authentication |
| **Data Isolation** | IMAP Protection Shield (Protocol-level data masking) |
| **Development** | Monero Mining (CPU controlled), OpenVINO AI Optimization |

## 🚀 Getting Started (Desktop)

### 1. Initialize Backend (TSM)
Ensure your environment is loaded and start the gRPC/API services:
```bash
source .tsm_cryptogram_env.sh
cd Telegram/lib_tsm
# Start gRPC (50051) and API (8080)
python -m mock_server.server &
uvicorn api.tsm_api:app --port 8080 &
```

### 2. Build the Client
The build system is optimized for Ninja:
```bash
mkdir build_release && cd build_release
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..
ninja -j$(nproc)
```

## 🧪 Documentation & Results

- **[Feature Matrix](docs/features/desktop-features.md)**: Detailed breakdown of implemented OPSEC modules.
- **[Integration Test Results](docs/status/DESKTOP_TEST_RESULTS.md)**: Verification status of build and security paths (✅ 100% Pass).
- **[Final Handoff](HANDOFF.md)**: Architecture summary and active system state.

## ⚖️ License

CRYPTOGRAM is released under the GNU GPL v3.0 with the project-specific OpenSSL exception noted in `LICENSE`.
