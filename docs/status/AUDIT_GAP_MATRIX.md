# CRYPTOGRAM Audit Gap Matrix

Date: 2026-04-03
Repo: `/media/john/NVME_STORAGE10/CRYPTOGRAM`

This audit compares the current repository snapshot against the active documentation surface and identifies the highest-value patch targets.

## Severity 1

### 1. Android cryptography docs still overstate runtime-backed security

Evidence:
- `TMessagesProj/src/main/java/org/telegram/ui/CryptogramSettingsActivity.java` describes the feature set as "Military-grade encryption" and claims all features are end-to-end encrypted with forward secrecy.
- `TMessagesProj/jni/cryptogram/data_mls_protocol.cpp` still contains placeholder X448, signature, and verification logic.
- `TMessagesProj/jni/cryptogram/CryptogramWrapper.cpp` exposes JNI hooks, but the implementation is a simplified AES-GCM session path rather than a complete Double Ratchet proof.

Impact:
- The UI and docs currently make stronger claims than the native implementation can justify.

Patch direction:
- Reduce claim language in Android UI and docs from "complete/active" to "present", "partial", or "requires validation".
- Replace placeholder MLS crypto paths before restoring stronger claims.

### 2. Desktop settings advertise features that are explicitly not implemented

Evidence:
- `Telegram/SourceFiles/settings/settings_cryptogram.cpp` marks Monero mining as not implemented.
- `Telegram/SourceFiles/settings/settings_cryptogram.cpp` marks covert channel messaging as not implemented.
- `Telegram/SourceFiles/settings/settings_cryptogram.cpp` disables encryption status because the required EnhancedPrivacy calls are not implemented.

Impact:
- The desktop settings surface presents capabilities that are currently configuration/UI shells rather than working end-to-end features.

Patch direction:
- Gate or relabel unfinished settings in the UI.
- Patch the missing implementation, or downgrade the docs to reflect current status.

### 3. Android build documentation implies an in-repo Gradle project that is not present

Evidence:
- `BUILD_GUIDE.md` says Android output is under `app/build/outputs/apk/...` and that a Gradle wrapper is included in the project.
- The repository snapshot does not contain `gradlew`, `build.gradle`, `build.gradle.kts`, or `settings.gradle*`.
- `build_apk.sh` is written to locate an external Android Gradle project and fails if it cannot find one.

Impact:
- The Android build path in current docs is not reproducible from this repo alone.

Patch direction:
- Update docs to state clearly that Android packaging currently depends on an external Android project.
- Or vendor the missing Gradle project into the repo and align `build_apk.sh` with it.

## Severity 2

### 4. The verification story is mostly static wiring checks, not feature validation

Evidence:
- `run_tests.sh` explicitly says it is a static verification harness for source wiring, API surface, and test assets.
- The same script treats placeholder detection as warnings, not failures.
- `tests/unit/test_cryptogram_features.cpp` only lightly exercises settings/storage/miner interfaces and includes caveats about missing real libraries or hardware.

Impact:
- Docs can be misread as if the repo has strong crypto/runtime coverage when the included harness mainly proves file presence and symbol wiring.

Patch direction:
- Tighten docs around test scope.
- Split "static wiring checks" from "runtime/crypto/device validation".
- Add focused runtime and integration tests before claiming broader coverage.

### 5. Legacy feature docs still contain broken local references and outdated certainty

Evidence:
- `docs/features/features.md` links to `FIVE_FEATURES_PORT.md`, `DOUBLE_RATCHET_PORT.md`, and `README.md` relative to `docs/features/`, but those targets are not present there.
- The same file still says CRYPTOGRAM is the only messenger with certain capabilities and describes several features as unique, complete, or automatic.

Impact:
- Readers can land on stale documentation that both breaks navigation and overstates completion.

Patch direction:
- Either archive `docs/features/features.md` or rewrite its links to the current docs tree.
- Remove or soften certainty language unless backed by implementation and tests.

### 6. Documentation status is internally inconsistent across active pages

Evidence:
- `README.md` and `docs/README.md` use cautious "snapshot/partial" language.
- `CryptogramSettingsActivity.java` still says "Encryption Active" and "Military-grade encryption".
- Some build and feature docs still read as if all platform flows are ready to run.

Impact:
- A user can read one page and get a conservative status, then open another and see production-grade language.

Patch direction:
- Standardize on one status vocabulary: `present`, `partial`, `placeholder`, `requires validation`, `externally dependent`.

## Severity 3

### 7. Quick-start paths are host-specific and can mislead first-time builders

Evidence:
- `QUICK_START.md` and `SETUP_GUIDE.md` hardcode `/home/user/CRYPTOGRAM`.
- The actual audited repo path is `/media/john/NVME_STORAGE10/CRYPTOGRAM`.

Impact:
- Low technical severity, but it adds friction and makes the docs look less maintained.

Patch direction:
- Use `$REPO_ROOT` style examples or relative paths.

## Patch Batches

### Batch A: Documentation truthfulness
- Rewrite Android and desktop feature pages to match current implementation state.
- Fix broken links in `docs/features/features.md`, or archive that file.
- Update build docs to explain the missing in-repo Android Gradle project.

### Batch B: Desktop settings cleanup
- Hide or mark unfinished controls for mining, covert channels, and encryption status.
- Replace absolute claims in the settings copy with capability/status wording.

### Batch C: Android crypto/UI honesty
- Downgrade UI status strings and explanatory copy until placeholder MLS and simplified JNI paths are replaced.
- Add explicit "experimental" or "validation required" text where appropriate.

### Batch D: Verification hardening
- Add real runtime tests for JNI bridge behavior and message round-trips.
- Add desktop unit/integration coverage for settings features that are currently only asserted by grep.

## Recommended Next Step

Patch Batch A first. It has the highest trust payoff and lowest implementation risk, and it reduces the gap between what the repo says and what the code can currently prove.
