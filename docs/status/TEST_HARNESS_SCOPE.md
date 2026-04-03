# CRYPTOGRAM Test Harness Scope

`run_tests.sh` is a static verification harness. It checks that the documented CRYPTOGRAM surface is present in the repository and wired together at the source level.

## What It Validates

- Required JNI, Kotlin, Java, and native source files exist.
- JNI entry points for Double Ratchet, MLS, and Enhanced Privacy are declared.
- Message-flow hooks exist in `SendMessagesHelper` and `MessageObject`.
- Settings toggles exist in `SharedConfig` and the CRYPTOGRAM settings screen is reachable from the profile UI.
- The JNI library is declared in `TMessagesProj/jni/CMakeLists.txt`.
- The unit test targets are listed in `tests/unit/CMakeLists.txt`.

## What It Does Not Validate

- It does not compile the Android app or native library.
- It does not install the APK or exercise a device or emulator.
- It does not prove encryption, decryption, or key derivation correctness at runtime.
- It does not verify UI rendering, tap behavior, or persistence on a live device.
- It does not validate network behavior, account state, or actual cryptographic interoperability.

## Manual Or Runtime Checks Still Needed

- Build the Android targets with the supported SDK/NDK toolchain.
- Install on a device or emulator and open `Settings -> CRYPTOGRAM`.
- Toggle each privacy option and confirm it persists across relaunches.
- Send and receive test messages to confirm the runtime encryption and decryption path.
- Exercise 1-on-1 Double Ratchet and group MLS flows on a live account.
- Review the native wrapper placeholders in `CryptogramWrapper.cpp` and the remaining MLS placeholder logic in `data_mls_protocol.cpp`, including the dormant UpdatePath path-secret helper, before treating the crypto path as production-validated.

## Interpreting The Script

- `PASS` means the repository is structurally aligned with the documentation and test assets.
- `WARN` means the harness found code that still looks scaffolded or placeholder-driven.
- `FAIL` means the documented source surface is missing or the wiring is inconsistent.
