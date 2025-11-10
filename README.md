Here’s a **clean, de-conflicted README.md**—no merge markers, consistent tone, and both branches’ useful content preserved. I removed jokes/insults and kept it professional while retaining your WIP caveats.

---

# 🔐 CRYPTOGRAM (WIP)

**Use at your own risk — this is ~90–95% complete and may remain in flux for weeks.**

**Military-grade secure messenger combining Telegram’s infrastructure with advanced cryptography**

[![License](https://img.shields.io/badge/license-GPLv3-blue.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS%20%7C%20Android-lightgrey.svg)](#-platforms)
[![Security](https://img.shields.io/badge/security-military--grade-green.svg)](#-security)
[![Build Android](https://github.com/SWORDOps/CRYPTOGRAM/workflows/Build%20CRYPTOGRAM%20Android%20APK/badge.svg)](https://github.com/SWORDOps/CRYPTOGRAM/actions)

CRYPTOGRAM provides advanced end-to-end encryption and privacy features for journalists, activists, whistleblowers, and anyone who values digital privacy.

---

## 🌟 Key Features

### Capability Matrix

| Feature                        | Description                                  | Platforms |
| ------------------------------ | -------------------------------------------- | --------- |
| **End-to-End Encryption**      | Signal Protocol + MLS                        | 🖥️ 📱    |
| **Post-Quantum Security**      | ML-KEM-1024 + ML-DSA-87                      | 🖥️ 📱    |
| **Audio Steganography**        | Hide messages in voice notes                 | 🖥️       |
| **Location Privacy**           | Randomize GPS coordinates                    | 🖥️       |
| **Surveillance Detection**     | Detect monitoring/VM/debuggers               | 🖥️       |
| **Tor/I2P Integration**        | Anonymous routing                            | 🖥️       |
| **Hardware Security**          | TPM/KeyStore integration                     | 🖥️ 📱    |
| **Premium Features (Testing)** | Access Telegram Premium features for testing | 🖥️ 📱    |

🖥️ = Desktop (Windows/Linux/macOS) | 📱 = Android

### Comparison (at a glance)

| Feature                  | Signal | WhatsApp | Telegram             | **CRYPTOGRAM** |
| ------------------------ | ------ | -------- | -------------------- | -------------- |
| End-to-End Encryption    | ✅      | ✅        | 🟡 Secret Chats only | ✅              |
| Post-Quantum Crypto      | ❌      | ❌        | ❌                    | ✅ **Unique**   |
| AES-256-GCM              | ✅      | ✅        | 🟡                   | ✅              |
| Signal Protocol          | ✅      | ✅        | ❌                    | ✅              |
| MLS (Groups)             | ❌      | ❌        | ❌                    | ✅              |
| Hide in Audio            | ❌      | ❌        | ❌                    | ✅ (Desktop)    |
| Location Randomization   | ❌      | ❌        | ❌                    | ✅ (Desktop)    |
| Surveillance Detection   | ❌      | ❌        | ❌                    | ✅ (Desktop)    |
| Hardware KeyStore/TPM    | ❌      | ❌        | ❌                    | ✅              |
| Metadata Stripping       | 🟡     | ❌        | ❌                    | ✅              |
| Hide Online/Typing/Reads | ❌      | ❌        | ❌                    | ✅ **Unique**   |

---

## 🚀 Quick Start

### 📱 Android

```bash
# 1) Download latest APK (GitHub Actions artifact)
xdg-open https://github.com/SWORDOps/CRYPTOGRAM/actions/workflows/build-android.yml

# 2) Install (example with adb)
adb install cryptogram-debug.apk
```

Then open **Telegram → Settings → 🔐 CRYPTOGRAM** and enable features.

### 🖥️ Desktop

```bash
# Download binaries
xdg-open https://github.com/SWORDOps/CRYPTOGRAM/releases

# Or build from source
git clone https://github.com/SWORDOps/CRYPTOGRAM
cd CRYPTOGRAM
# See docs/setup/ for platform build instructions
```

---

## 📱 Platforms

### 🖥️ CRYPTOGRAM Desktop (Windows 10/11, Linux, macOS 10.15+)

**Core Encryption**

* Signal Protocol (Double Ratchet)
* MLS (RFC 9420)
* Post-Quantum: ML-KEM-1024, ML-DSA-87

**Advanced Security**

* Audio steganography (multiple methods)
* Location randomization (realistic GPS fuzzing)
* Surveillance detection (debugger/VM/monitoring)
* Covert channels (DPI-resistant)
* Enhanced privacy (metadata stripping)

**Network Privacy**

* Tor (SOCKS5, Snowflake bridges)
* I2P (Garlic routing)

**Optional**

* OpenVINO translation (offline, 100+ languages)
* *Monero mining* (opt-in, configurable)

**Premium Override (Testing)**

* Enable Telegram Premium features for testing only

[📥 Download Desktop](https://github.com/SWORDOps/CRYPTOGRAM/releases) · [📖 Desktop Features](docs/features/desktop-features.md)

---

### 📱 CRYPTOGRAM Android (API 21+)

**Core Encryption**

* Signal Protocol (X25519, Ed25519, AES-256-GCM)
* MLS (TreeKEM groups)
* Post-Quantum: ML-KEM-1024, ML-DSA-87

**Privacy**

* Hardware KeyStore integration
* Metadata stripping (EXIF/GPS)
* Visual lock indicators

**Premium Override (Testing)**

* Enable Telegram Premium features for testing only

**Architectures**

* armeabi-v7a, arm64-v8a, x86, x86_64

[📥 Android APK](https://github.com/SWORDOps/CRYPTOGRAM/actions/workflows/build-android.yml) · [📖 Android Guide](docs/features/android-features.md)

---

## 🎯 Who Is CRYPTOGRAM For?

* **Journalists** — covert channels, surveillance detection, location privacy
* **Activists** — censorship bypass (Tor/I2P), MLS secure groups
* **Whistleblowers** — audio stego, forward secrecy, anonymous routing
* **Privacy Advocates** — hardware-backed keys, metadata protection
* **Users in Restrictive Countries** — DPI evasion, protocol mimicry

---

## 🔒 Security

### Cryptographic Primitives

**Symmetric:** AES-256-GCM, ChaCha20-Poly1305
**Asymmetric:** X25519 (KX), Ed25519 (sig)
**Post-Quantum:** ML-KEM-1024 (Kyber), ML-DSA-87 (Dilithium)
**Hash/KDF:** SHA-256/512, BLAKE2b, HKDF-SHA256

**Security Properties**

* ✅ Forward Secrecy
* ✅ Post-Compromise Security
* ✅ Deniability
* ✅ Quantum-resistant key exchange & signatures
* ✅ Hardware-backed key storage (TPM/KeyStore)
* ✅ Metadata minimization (GPS/EXIF/timestamps)

See **[Technical Specifications](docs/implementation/CRYPTOGRAM_ANDROID_COMPLETE.md#cryptographic-algorithms)** and **[Security Overview](docs/features/security-overview.md)**.

---

## 📖 Documentation

### Getting Started

* [Android Features](docs/features/android-features.md)
* [Desktop Features](docs/features/desktop-features.md)
* [Security Overview](docs/features/security-overview.md)

### Building from Source

* [Windows](docs/setup/building-win-x64.md)
* [Linux](docs/setup/building-linux.md)
* [macOS](docs/setup/building-mac.md)
* [API Credentials](docs/setup/api_credentials.md)
* [Docker Build](DOCKER_BUILD.md)
* [CI/CD (GitHub Actions)](GITHUB_ACTIONS_BUILD.md) or [docs/archive/GITHUB_ACTIONS_BUILD.md](docs/archive/GITHUB_ACTIONS_BUILD.md)

### Technical Details

* [Signal (Double Ratchet)](docs/implementation/DOUBLE_RATCHET_PORT.md)
* [MLS Protocol](docs/implementation/CRYPTOGRAM_ANDROID_COMPLETE.md#mls-protocol)
* [Message Flow](docs/implementation/MESSAGE_FLOW_COMPLETE.md)
* [Test Results](docs/status/TEST_RESULTS.md)
* [Implementation Status](docs/status/FINAL_STATUS.md)

---

## 🤝 Contributing

We welcome contributions in:

**Android**

* UI/UX improvements, performance, features, bug fixes

**Desktop**

* Platform features, security hardening, performance

**Both**

* Docs, localization, testing, bug reports

**How to Contribute**

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit (`git commit -m "Add amazing feature"`)
4. Push (`git push origin feature/amazing-feature`)
5. Open a Pull Request

[📖 Contributing Guidelines](CONTRIBUTING.md) *(coming soon)*

---

## 📜 License

CRYPTOGRAM is released under **GNU GPL v3.0 with OpenSSL exception**.

* ✅ Free to use, modify, distribute
* ✅ Share source for modifications
* ✅ Copyleft (GPL-compatible)
* ⚠️ No warranty

See [LICENSE](LICENSE) for details.

---

## 🙏 Credits

**Based On**

* [Telegram Desktop](https://github.com/telegramdesktop/tdesktop)
* [Telegram Android](https://github.com/DrKLO/Telegram)
* [64Gram](https://github.com/TDesktop-x64/tdesktop)

**Cryptography**

* [Signal Protocol](https://signal.org/docs/)
* [MLS (RFC 9420)](https://messaginglayersecurity.rocks/)
* [BoringSSL](https://boringssl.googlesource.com/boringssl/)
* [OpenSSL](https://www.openssl.org/)

**Post-Quantum**

* [NIST PQC](https://csrc.nist.gov/projects/post-quantum-cryptography)
* [Literally the NSA for...encryption in general]
**Development**

* CRYPTOGRAM Team & Open-source community

---

## 🔗 Links

* **GitHub:** [https://github.com/SWORDOps/CRYPTOGRAM](https://github.com/SWORDOps/CRYPTOGRAM)
* **Releases:** [https://github.com/SWORDOps/CRYPTOGRAM/releases](https://github.com/SWORDOps/CRYPTOGRAM/releases)
* **Android Builds:** [https://github.com/SWORDOps/CRYPTOGRAM/actions/workflows/build-android.yml](https://github.com/SWORDOps/CRYPTOGRAM/actions/workflows/build-android.yml)
* **Issues:** [https://github.com/SWORDOps/CRYPTOGRAM/issues](https://github.com/SWORDOps/CRYPTOGRAM/issues)
* **Discussions:** [https://github.com/SWORDOps/CRYPTOGRAM/discussions](https://github.com/SWORDOps/CRYPTOGRAM/discussions)
* **Security Contact:** [intel@swordintelligence.airforce](mailto:intel@swordintelligence.airforce)

---

## ⚠️ Disclaimer

CRYPTOGRAM is intended for legitimate privacy/security use. Ensure compliance with local laws.
Status: **not externally audited**; use at your own risk.

* Good for: journalists, activists, privacy advocates
* Not recommended for: classified/national-security secrets (until audited)

---

## ⭐ Star This Project

If CRYPTOGRAM helps protect your privacy, please consider starring the repo.

[![GitHub stars](https://img.shields.io/github/stars/SWORDOps/CRYPTOGRAM?style=social)](https://github.com/SWORDOps/CRYPTOGRAM/stargazers)

---

## 📊 Project Status (Alpha/WIP)

| Component              | Status        | Notes                   |
| ---------------------- | ------------- | ----------------------- |
| Desktop (Windows)      | 🟡 Alpha      | Core features usable    |
| Desktop (Linux)        | 🟡 Alpha      | Core features usable    |
| Desktop (macOS)        | 🟡 Alpha      | Core features usable    |
| Android                | 🟡 Alpha      | Core features usable    |
| Signal Protocol        | ✅ Implemented | 1:1 chats               |
| MLS Protocol           | ✅ Implemented | Groups                  |
| Post-Quantum           | ✅ Implemented | ML-KEM-1024 / ML-DSA-87 |
| Audio Steganography    | ✅ Desktop     | Multiple methods        |
| Location Privacy       | ✅ Desktop     | GPS fuzzing             |
| Surveillance Detection | ✅ Desktop     | VM/debugger checks      |
| Tor/I2P                | ✅ Desktop     | Tor + I2P               |
| Premium Override       | ✅ Testing     | For test only           |
| External Audit         | ⏳ Planned     | Pending                 |
| Bug Bounty             | ⏳ Coming Soon | Pending                 |

---

**Built with 🔐 for a more private world.**
**Available on Desktop and Android** 🖥️ 📱
