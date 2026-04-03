# CRYPTOGRAM Android Features

This page describes the Android feature surface that is present in the repository today. Some items are wired into the UI and native layer, while others are still partial cryptographic implementations or runtime paths that need broader validation.

## Core Messaging

### 1. Signal Protocol / Double Ratchet
**Status**: Present in code, UI, and native self-tests

The repo contains Double Ratchet classes, native bridge code, settings toggles, and local native self-checks for 1-on-1 encrypted messaging.

- X25519 key exchange
- Ed25519-related protocol code
- AES-256-GCM-related helpers
- Forward-secrecy oriented message flow
- Settings toggle in the CRYPTOGRAM screen
- Native self-test hook exposed through `CryptogramNative`

**Note**: The native layer now exposes local self-tests, but the runtime path should still be treated as an active implementation rather than a finished cryptographic audit result.

### 2. MLS Protocol
**Status**: Partial, with native self-test coverage

MLS-related classes and JNI entry points are present for group encryption flows.

- Group messaging scaffolding
- TreeKEM-oriented data structures
- Settings toggle in the CRYPTOGRAM screen
- Feature status reporting in the UI
- Native self-test hook exposed through `CryptogramNative`

**Note**: Some MLS cryptographic paths still use placeholder logic, so the docs should not claim full end-to-end completion.

### 3. Post-Quantum Cryptography
**Status**: Documented, not fully validated here

The codebase and docs reference ML-KEM / ML-DSA naming and post-quantum oriented messaging claims.

- ML-KEM / Kyber references
- ML-DSA / Dilithium references
- HKDF-based key derivation references

**Note**: Treat this as documented capability and design intent unless you have validated the relevant runtime path on a target device.

## Privacy Features

### 4. Enhanced Privacy
**Status**: Present in settings and helper code

The Android tree exposes privacy toggles and supporting helper classes for:

- Hide online status
- Hide typing indicator
- Hide read receipts
- Metadata/privacy helper flows

Hardware-backed storage is referenced in the docs and helper code, but the repository snapshot does not provide a complete verification story for every device or build.

## UI and Settings

### 5. CRYPTOGRAM Settings Screen
**Status**: Present

The settings activity exists and exposes the currently documented toggles and a native-backed status view:

- Double Ratchet toggle
- MLS toggle
- Privacy toggles
- Curated stickers toggle
- Feature status dialog
- Native version/status display

### 6. Visual Indicators
**Status**: Present

The UI includes indicator helpers for encrypted message presentation:

- Lock icon helpers
- Badge helpers for encrypted messages
- Chat status color helpers

## Premium Override

### 7. Premium Features Override
**Status**: Present as a testing hook

The repository includes premium-override toggles and related settings/core references. This should be described as a testing or configuration hook, not as a claim that server-side premium features are available without the upstream service.

## Installation

Android build and installation steps are documented in the platform guides. Use the current build docs and the status page together, because the docs do not guarantee a clean build or fully tested device runtime.

## Source Code

- `TMessagesProj/jni/cryptogram/` - Native bridge and protocol code
- `TMessagesProj/src/main/java/org/telegram/messenger/cryptogram/` - Java/Kotlin API
- `TMessagesProj/src/main/java/org/telegram/ui/CryptogramSettingsActivity.java` - Settings screen
- `TMessagesProj/src/main/java/org/telegram/ui/Components/CryptogramIndicator.java` - UI indicators

## Known Gaps

- Some JNI-backed features still depend on partial or placeholder cryptographic paths.
- Some docs describe more complete cryptography than the current runtime paths prove.
- Device-level and end-to-end testing is still limited in this repository snapshot.
