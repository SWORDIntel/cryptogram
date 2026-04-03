# 🔐 CRYPTOGRAM

CRYPTOGRAM is a Telegram-derived messenger branch with Android and desktop privacy-related work in progress. The repository contains real feature code, but several docs and wrappers still describe partial wiring, fallback paths, or feature surfaces that have not been proven end-to-end.

## Current Snapshot

| Area | Current state |
| --- | --- |
| Android settings UI | Present |
| Double Ratchet / Signal path | Present in code and UI hooks |
| MLS group path | Present, with placeholder paths in some native wrappers |
| Privacy toggles | Present for online status, typing indicator, and read receipts |
| Desktop security modules | Present as documented code paths and helper modules |
| Audio steganography / covert channel docs | Present, but should be treated as partially validated |
| Post-quantum claims | Documented in code and docs, not fully validated here |
| Device/runtime testing | Incomplete in the repository snapshot |

## Platforms

### Android

The Android branch exposes a CRYPTOGRAM settings screen and native bridge classes for:

- Double Ratchet / Signal-style 1-on-1 encryption
- MLS-related group messaging code paths
- Privacy toggles for online status, typing indicator, and read receipts
- Message indicators and status display

The docs describe these features as user-facing, but the implementation still includes placeholder or evolving paths in the native layer.

### Desktop

The desktop tree includes documented security modules for:

- Audio steganography
- Location randomization
- Surveillance detection
- Covert-channel support
- Hardware-aware privacy and translation paths

These modules should be read as current code and documentation work, not as a blanket guarantee that every advertised behavior is complete or production-verified.

## Documentation

- [Documentation index](docs/README.md)
- [Android features](docs/features/android-features.md)
- [Desktop features](docs/features/desktop-features.md)
- [Implementation status](docs/status/FINAL_STATUS.md)

## Build Notes

Platform build instructions live under `docs/`. The current tree includes build scripts and platform-specific guides, but the docs should be read as project guidance rather than proof that every platform build is already green.

## Limitations

- Some native wrappers still contain placeholder calls.
- Some documented features are exposed through settings or helper classes but not fully exercised by automated tests.
- The repository does not provide a complete runtime verification story for all desktop and Android claims.

## License

CRYPTOGRAM is released under the GNU GPL v3.0 with the project-specific OpenSSL exception noted in `LICENSE`.
