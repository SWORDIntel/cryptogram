# CRYPTOGRAM Desktop Features

This page summarizes the desktop feature surface. Several CRYPTOGRAM modules are compiled into the desktop target, providing advanced privacy and security controls.

## Core Messaging

### 1. Signal Protocol / Double Ratchet
**Status**: ✅ **Implemented**
Desktop source and build files include a Signal/Double Ratchet messaging path for encrypted chat flows.

### 2. MLS Protocol
**Status**: 🟡 **Partial**
MLS-related code is compiled into the desktop build. End-to-end validation is ongoing.

### 3. Post-Quantum Cryptography
**Status**: ✅ **Integrated (Hybrid)**
Supports ML-KEM and ML-DSA hybrid modes for future-proof security.

## Security Modules

### 4. Audio Steganography & GNA
**Status**: ✅ **Module Present**
Includes `GNAAcousticSecurity` for acoustic surveillance detection and voice morphing. Supports Intel GNA hardware acceleration.

### 5. Surveillance Detection
**Status**: ✅ **Implemented**
`UniversalThreatDetector` provides AI-powered analysis of incoming text, files, and links to detect surveillance or phishing attempts.

### 6. Covert Channels
**Status**: 🧪 **Experimental**
Backend module is functional. Desktop transport layer is currently being wired for typing-indicator based covert messaging.

### 7. Zero-Knowledge Authentication
**Status**: ✅ **Implemented**
Lattice-based ZK proofs allow identity verification without password disclosure. Integrated with TSM backend.

## Advanced OPSEC

### 8. Location Privacy & Randomization
**Status**: ✅ **Implemented**
Sophisticated geographic privacy via `LocationRandomizationManager`. Includes coordinate noise, timezone anonymization, and realistic location pools.

### 9. Counterintelligence (Canary & Honeypot)
**Status**: ✅ **UI Integrated**
Context-menu actions to deploy Canary Links and create Chat Honeypots. Integrated with `CounterIntelligenceManager` for trigger tracking.

### 10. NSA-Grade Security Architecture
**Status**: ✅ **Implemented**
High-security classification levels (Secret/Top Secret) with corresponding countermeasures: traffic obfuscation, anti-forensics, and emergency data destruction.

### 11. Dead Man's Switch
**Status**: ✅ **Functional**
Failsafe protocol requiring periodic activity proof. Automatically triggers emergency protocols upon timeout.

## Network Privacy

### 12. Tor Integration (Snowflake)
**Status**: ✅ **Functional**
Includes a configurable Tor Snowflake proxy with CPU usage controls to assist the Tor network.

### 13. I2P Integration (Relay)
**Status**: ✅ **Functional**
Supports internal I2P relay operations with configurable hardware resource limits.

### 14. Bridge Support
**Status**: ✅ **UI Integrated**
Dedicated configuration surface for Tor/I2P bridges to bypass network censorship.

## Hardware & Optimization

### 15. Monero Mining
**Status**: ✅ **Functional (Optional)**
Optional background mining to support project development with user-defined CPU limits.

### 16. OpenVINO Translation
**Status**: ✅ **Functional**
Local, AI-powered translation for Russian and Chinese using Intel OpenVINO.

## Settings and UI

The CRYPTOGRAM settings menu provides a unified interface for all advanced features:
- **Network Anonymity**: Tor, I2P, and Bridge controls.
- **Encryption**: Algorithm selection and ZK Auth.
- **Advanced OPSEC**: Location, Counterintelligence, and NSA-grade protocols.
- **Privacy Controls**: Online status, typing indicators, and read receipts.
- **Device Trust**: CAC/PIV smart card integration.
- **TSM Integration**: Optional backend session management.
- **Development Support**: Mining and diagnostics.
