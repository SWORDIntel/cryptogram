# CRYPTOGRAM Patch Plan

Prepared: 2026-04-03
Repo root: `/media/john/NVME_STORAGE10/CRYPTOGRAM`

## Goal

Close the highest-value gaps between the repository and its documentation without making false readiness claims.

## Workstream 1: Documentation Truth Pass

Priority: immediate

Files to patch first:

- `docs/status/TEST_RESULTS.md`
- `docs/implementation/CRYPTOGRAM_ANDROID_COMPLETE.md`
- `docs/dev/PULL_REQUEST.md`
- `docs/status/TEST_HARNESS_SCOPE.md`
- `docs/README.md`
- `README.md`

Changes:

- Remove or downgrade "ready for deployment", "production-quality", and similar completion language.
- Make it explicit when a check is static-only.
- Fix stale path references to archived docs.
- Align top-level README wording with the more careful feature pages under `docs/features/`.

## Workstream 2: Desktop Truth vs Build Reality

Priority: immediate

Files to inspect and patch:

- `Telegram/CMakeLists.txt`
- `Telegram/SourceFiles/settings/settings_cryptogram.cpp`
- `docs/features/desktop-features.md`

Decision required:

- Either re-enable the commented-out CRYPTOGRAM desktop data modules and fix resulting build issues,
- or keep them disabled and rewrite settings/docs so they are clearly marked as partial or source-present only.

Recommended first step:

- Patch docs and settings copy first.
- Re-enable modules only after the build and dependency story is understood.

## Workstream 3: Android Crypto Truth Boundary

Priority: near-term

Files to inspect and patch:

- `TMessagesProj/jni/cryptogram/CryptogramWrapper.cpp`
- `TMessagesProj/jni/cryptogram/data_mls_protocol.cpp`
- `docs/features/android-features.md`
- `docs/features/security-overview.md`
- `docs/dev/SECURITY_STANDARDS.md`

Decision required:

- Either finish the remaining partial crypto paths and produce runtime evidence,
- or narrow the docs so they describe the current wrapper and MLS behavior accurately.

Specific targets:

- Replace placeholder MLS sign/verify and X448 notes if those ciphersuites are intended to ship.
- Clarify whether Android post-quantum and hardware-backed-key claims are implemented, planned, or desktop-only/source-present.

## Workstream 4: Verification Tiering

Priority: near-term

Files to patch:

- `run_tests.sh`
- `docs/status/TEST_RESULTS.md`
- `.github/workflows/build-android.yml`

Recommended verification tiers:

1. Static structure checks
2. Desktop unit-test execution
3. Android build verification
4. Android runtime/manual validation checklist

Recommended outcome:

- Keep `run_tests.sh` as the static harness.
- Stop treating it as proof of runtime quality.
- Add separate commands or CI targets for build and runtime validation status.

## Suggested Execution Order

1. Documentation truth pass
2. Desktop settings/docs reconciliation
3. Android crypto truth boundary decision
4. Verification tiering improvements

## Safe First Patch

If the next step is a narrow patch instead of a larger feature sprint, start with:

- `docs/status/TEST_RESULTS.md`
- `docs/status/TEST_HARNESS_SCOPE.md`
- `docs/dev/PULL_REQUEST.md`
- `docs/README.md`

That will remove the highest-risk documentation inaccuracies before touching product code.
