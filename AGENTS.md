# Repository Guidelines

This file applies to the entire `CRYPTOGRAM` repository. Follow these conventions when changing code, docs, or build scripts.

## Project Structure & Modules

- Desktop client: `Telegram/` (C++/Qt), with platform build files in `CMakeLists.txt`, `cmake/`, and `build_release/`.
- Android client: `telegram-android/` (Gradle/Android Studio).
- Shared tooling and libs: `lib/`, `TMessagesProj/`, `Telegram/lib_tsm/`.
- Documentation and status: `docs/`, top-level `README*.md`, `BUILD_*.md`, and `SETUP_*.md`.
- Tests and experiments: `tests/`, `run_tests.sh`, `test_build_fixes.sh`.
- Docker and CI helpers: `docker/`, `.github/workflows/`.

## Build, Test & Development Commands

- Initial setup: `./setup-all.sh` (installs toolchains, submodules, and dependencies where possible).
- Desktop builds: `./build_all.sh` or `./build_everything.sh` (full multi-platform build); prefer platform-specific docs under `docs/building-*.md`.
- Android builds: `./build_android.sh` (CI-style build) or use Android Studio in `telegram-android/`.
- Tests: `./run_tests.sh` for core checks; `./test_build_fixes.sh` for regression build tests.

## Coding Style & Naming

- Match the surrounding style in each subproject (Qt/C++ in `Telegram/`, Java/Kotlin in `telegram-android/`, Python in `Telegram/lib_tsm/`).
- Use 2 or 4 spaces consistently with the local file; do not change indentation style globally.
- Prefer descriptive identifiers (`SecureChannelConfig`, `androidSecureClient`) over abbreviations.
- Keep filenames and directories lowercase with hyphens or underscores (e.g., `secure_channel_utils.cpp`, `android_secure_client.kt`).

## Testing Guidelines

- Add or update tests under `tests/` or the relevant platform test directory (e.g., Android unit/instrumentation tests).
- Name tests after the behavior under test (e.g., `EncryptMessage_SuccessWhenKeysValid`).
- Run `./run_tests.sh` (and platform-specific tests where applicable) before pushing.

## Commit & Pull Request Practices

- Commit messages: short, imperative, and scoped (e.g., `Fix submodule detection`, `Add desktop build guide`). Avoid long multi-topic commits.
- Group related changes per commit (build, docs, or feature) and keep diffs focused.
- Pull requests: include a one-paragraph summary, key technical notes, testing performed (`./run_tests.sh`, manual steps), and links to any relevant docs or issues. Add screenshots or logs for UI and build-related changes where useful.

