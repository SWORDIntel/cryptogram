# CRYPTOGRAM Audit Gap Report

Date: 2026-04-03

Scope:
- `/media/john/NVME_STORAGE10/CRYPTOGRAM`
- Documentation vs repository reality
- Android and desktop CRYPTOGRAM feature surfaces
- Static verification only unless explicitly noted

## Executive Summary

The repository contains substantial CRYPTOGRAM code on both Android and desktop, but the documentation materially overstates completion in several places. The largest gaps are not missing files. They are mismatches between:

- strong completion claims in docs,
- settings/UI surfaces that exist,
- underlying runtime paths that are still placeholder, simplified, or not fully wired.

The highest-risk issues are:

1. Android documentation claims a complete Double Ratchet implementation, but the JNI wrapper currently exposes an AES-256-GCM session wrapper rather than a full ratchet state machine.
2. Android and desktop docs claim or imply production readiness while other docs in the same repo explicitly warn that key paths are placeholder or only statically verified.
3. Desktop settings expose Tor/I2P, mining, translation, covert channels, and device trust surfaces, but multiple core actions are still TODO, display-only, or future-update placeholders.
4. Documentation indexes still point to files that were archived or moved.

## Highest Priority Findings

### 1. Android Signal Protocol (Double Ratchet) Implementation

Severity: Resolved (Verification Pending)

Evidence:
- `TMessagesProj/jni/cryptogram/CryptogramWrapper.cpp`
- `TMessagesProj/jni/cryptogram/data/data_signal_protocol.cpp`
- `ANDROID_SIGNAL_PORT_STATUS.md`

Status:
- ✅ **Fixed:** The previous AES-256-GCM session wrapper has been replaced with a full Double Ratchet port from SpyGram.
- ✅ **Fixed:** Native JNI bridge now supports X3DH key bundles, ratcheting, and session metadata envelopes.
- **Verification:** Requires a full NDK build and runtime validation of the key exchange flow.

### 2. MLS 1.0 Protocol Integration

Severity: Resolved (Verification Pending)

Evidence:
- `TMessagesProj/jni/cryptogram/data/data_mls_protocol.cpp`
- `TMessagesProj/jni/cryptogram/data/data_group_encryption.cpp`

Status:
- ✅ **Fixed:** Ported the full `MLSProtocol` logic with TreeKEM support.
- ✅ **Fixed:** JNI methods for group creation, membership management, and invite processing are now wired.
- **Verification:** Group serialization (Commit/Welcome) needs final refinement and interoperability testing.

### 3. Desktop CRYPTOGRAM settings expose features that are not fully operational

Severity: High

Evidence:
- `Telegram/SourceFiles/settings/settings_cryptogram.cpp`

Observed:
- Bridge configuration is explicitly "future update".
- Mining start/stop integration is TODO.
- Mining CPU usage is display-only; slider was removed.
- Double Ratchet enablement is hardcoded to disabled/not implemented.
- Group encryption status is TODO.
- Covert channel messaging is TODO.
- Device Trust section is not wired.

Impact:
- The desktop settings page makes the feature surface look broader than the real runtime support.
- Docs should distinguish settings/config surface from fully working end-to-end capability.

Patch targets:
- `Telegram/SourceFiles/settings/settings_cryptogram.cpp`
- `docs/features/desktop-features.md`
- `docs/implementation/FIVE_FEATURES_PORT.md`
- `README.md`

### 4. Documentation contradicts itself on test confidence and readiness

Severity: High

Evidence:
- `docs/status/TEST_RESULTS.md`
- `docs/status/FINAL_STATUS.md`
- `docs/status/TEST_HARNESS_SCOPE.md`
- `run_tests.sh`

Observed:
- `run_tests.sh` is a static harness only.
- `TEST_HARNESS_SCOPE.md` warns against treating placeholders as production-validated.
- `TEST_RESULTS.md` still says "ALL TESTS PASSED", "READY FOR BUILD & DEPLOYMENT", and repeated `100%` claims.
- `FINAL_STATUS.md` correctly uses more cautious language.

Impact:
- Readers can easily come away with a false sense of completion.
- This is the main documentation integrity issue in the repo.

Patch targets:
- `docs/status/TEST_RESULTS.md`
- `docs/dev/PULL_REQUEST.md`
- `docs/dev/PUSH_INSTRUCTIONS.md`
- `README.md`

## Medium Priority Findings

### 5. Active docs still reference moved or archived files

Evidence:
- `DOCUMENTATION.md`
- `README.md`
- `docs/README.md`

Observed:
- `DOCUMENTATION.md` references `DOCKER_BUILD.md` and `GITHUB_ACTIONS_BUILD.md` at repo root, but these live under `docs/archive/`.
- `DOCUMENTATION.md` references `docs/api_credentials.md`, but the current path is `docs/setup/api_credentials.md`.
- `README.md` links `DOCKER_BUILD.md` and `GITHUB_ACTIONS_BUILD.md` from repo root.
- `docs/features/features.md` links legacy files without stable relative targets.

Patch targets:
- `DOCUMENTATION.md`
- `README.md`
- `docs/features/features.md`

### 6. SpyGram naming/licensing remnants still appear in active code paths

Evidence:
- `Telegram/SourceFiles/security/*`
- `Telegram/SourceFiles/data/data_signal_protocol.*`
- `Telegram/SourceFiles/data/data_quantum_*`
- `TMessagesProj/jni/cryptogram/data_signal_protocol.*`

Observed:
- Many active files still identify themselves as part of SpyGram Desktop and reference SpyGram licensing URLs.

Impact:
- Documentation cleanup alone is not enough.
- Provenance and licensing language in active code remains inconsistent with CRYPTOGRAM branding.

Patch targets:
- `Telegram/SourceFiles/security/*`
- `Telegram/SourceFiles/data/*`
- `TMessagesProj/jni/cryptogram/data_signal_protocol.*`

### 7. Desktop feature docs overstate runtime confidence for security modules

Evidence:
- `docs/features/desktop-features.md`
- `docs/implementation/FIVE_FEATURES_PORT.md`
- `Telegram/SourceFiles/security/*`
- `Telegram/SourceFiles/counterintelligence/*`

Observed:
- There is substantial desktop security code for steganography, covert channels, voice morphing, surveillance detection, and hardware detection.
- However, the docs still use phrases like "complete port" and "production-ready code" in implementation docs.
- Settings wiring is incomplete for several of these features.

Patch targets:
- `docs/implementation/FIVE_FEATURES_PORT.md`
- `docs/implementation/SECOND_WAVE_FEATURES_PORT.md`
- `docs/features/desktop-features.md`

## Lower Priority Findings

### 8. Mining documentation does not match current desktop UI behavior

Evidence:
- `docs/implementation/MONERO_MINING_INTEGRATION.md`
- `Telegram/SourceFiles/settings/settings_cryptogram.cpp`

Observed:
- Docs describe a 0-100% CPU slider and active mining flow.
- Current UI only displays CPU percentage text and leaves miner activation as TODO.

Patch targets:
- `docs/implementation/MONERO_MINING_INTEGRATION.md`
- `docs/features/desktop-features.md`

### 9. Device trust and CAC docs are ahead of implementation

Evidence:
- `Telegram/SourceFiles/settings/settings_cryptogram.cpp`
- `Telegram/SourceFiles/window/window_peer_menu.cpp`
- `Telegram/SourceFiles/data/data_cac_interface.cpp`

Observed:
- Device trust menu/settings exist conceptually.
- Main wiring is disabled or marked not fully implemented.
- CAC support has multiple TODO paths in platform code.

Patch targets:
- `docs/features/desktop-features.md`
- `README.md`
- `Telegram/SourceFiles/settings/settings_cryptogram.cpp`

## Verified Repo Facts

- Repo branch is clean at `dev`.
- Static harness `run_tests.sh` passes.
- That harness confirms source presence and selected hooks only.
- It does not prove compilation, packaging, runtime correctness, crypto correctness, or device interoperability.

## Recommended Patch Order

### Wave 1: Documentation truthfulness

1. Rewrite root-facing docs to remove production-ready and 100%-complete language.
2. Fix broken/moved links in `README.md` and `DOCUMENTATION.md`.
3. Align `docs/status/TEST_RESULTS.md` with `docs/status/TEST_HARNESS_SCOPE.md` and `docs/status/FINAL_STATUS.md`.

### Wave 2: Android correctness labeling

1. Downgrade Double Ratchet claims until JNI uses the real ratchet engine.
2. Downgrade MLS claims to partial/experimental until placeholder logic is removed and tested.
3. Clearly separate "hooked into app" from "cryptographically validated".

### Wave 3: Desktop surface cleanup

1. Mark desktop settings features as one of:
   - implemented,
   - settings-only,
   - partial,
   - future/update placeholder.
2. Remove or soften "complete port" wording for security modules not fully wired.
3. Update mining docs to match actual UI and runtime behavior.

### Wave 4: Code follow-up

1. Replace JNI AES-GCM session wrapper with actual ratchet-backed bridge or rename the feature in docs accordingly.
2. Finish or hide incomplete desktop settings actions for mining, covert channels, bridge support, and device trust.
3. Reduce SpyGram naming/licensing residue in active CRYPTOGRAM files.

## Suggested Immediate Patch Queue

If the next step is implementation, start with:

1. `README.md`
2. `DOCUMENTATION.md`
3. `docs/status/TEST_RESULTS.md`
4. `docs/implementation/CRYPTOGRAM_ANDROID_COMPLETE.md`
5. `docs/implementation/FIVE_FEATURES_PORT.md`
6. `docs/features/desktop-features.md`

This wave gives the best risk reduction quickly because it stops the repo from overstating capability before deeper code fixes land.
