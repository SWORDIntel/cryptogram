# CRYPTOGRAM Desktop Features

This page summarizes the desktop feature surface that exists in the repository. Several CRYPTOGRAM modules are compiled into the desktop target today, while others remain source-only, optional, or dependent on additional runtime validation.

## Core Messaging

### 1. Signal Protocol / Double Ratchet
**Status**: Present in source and desktop build

Desktop source and build files include a Signal/Double Ratchet messaging path for encrypted chat flows.

### 2. MLS Protocol
**Status**: Present in build, still partial at the feature level

MLS-related code is compiled into the desktop build, but the repository still contains partial cryptographic plumbing and limited end-to-end validation.

### 3. Post-Quantum Cryptography
**Status**: Documented

The desktop docs reference ML-KEM / ML-DSA style post-quantum naming and hybrid crypto claims. Treat those as documented intent unless you have validated the runtime path on a target build.

## Security Modules

### 4. Audio Steganography
**Status**: Present as a security module

The repository contains acoustic/steganography-oriented security code and settings references. The docs should present this as a module in the tree, not as a blanket guarantee of undetectable covert messaging.

### 5. Location Randomization
**Status**: Documented

Location/privacy wording appears in docs and settings-related code paths, but this should be treated as a privacy feature description rather than a fully verified anti-tracking proof.

### 6. Surveillance Detection
**Status**: Present as a security module

The desktop tree includes surveillance-detection and hardware-profile code paths, including OpenVINO-aware and device-aware helpers.

### 7. Covert Channels
**Status**: Present as a security module

The repository contains covert-channel-related code paths and settings references. The backend module is present, but the desktop settings surface still describes it as not yet wired end-to-end.

### 8. Enhanced Privacy
**Status**: Present in source and desktop build

Privacy helpers and metadata-related code are present in the tree and included in the desktop build, but behavior still depends on the specific runtime path being exercised.

## Network Privacy

### 9. Tor Integration
**Status**: Documented in settings/code paths

Tor-related options are referenced in settings and docs. The repo snapshot does not provide a full runtime verification story for every platform, and no dedicated desktop Tor module is surfaced in the current build scan.

### 10. I2P Integration
**Status**: Documented in settings/code paths

I2P-related options are present in docs and settings code, but should be treated as an integration path rather than a completed assurance claim.

## Optional Features

### 11. Monero Mining
**Status**: Present in source and desktop build, still optional

The desktop build includes mining-related code and settings, but the feature should still be treated as optional and runtime-dependent rather than a default production requirement.

### 12. Premium Features Override
**Status**: Present as a testing hook

The desktop tree includes premium-related settings references. As with Android, this should be described as a test/configuration hook only.

## Settings and UI

The desktop settings tree includes CRYPTOGRAM-related entries for:

- Privacy toggles
- Covert-channel related options
- Translation / hardware-aware options
- Mining-related options

Not every settings surface is fully wired. In particular, covert-channel transport, bridge support, and some hardware-dependent features still require follow-up work or platform validation.

## Source Code

- `Telegram/SourceFiles/security/` - security, steganography, and hardware-aware modules
- `Telegram/SourceFiles/settings/settings_cryptogram.*` - CRYPTOGRAM settings surface
- `Telegram/SourceFiles/api/` - privacy and message-flow hooks

## Known Gaps

- Some features are stronger at the source/build level than at the end-user runtime level.
- Optional hardware/library support can change what actually builds or runs.
- End-to-end validation across desktop targets is still limited in the repository itself.
- Location randomization and OpenVINO translation remain more limited than the core compiled CRYPTOGRAM modules.
