# Desktop Build Alignment Status

Audit date: 2026-04-03  
Repo root: `/media/john/NVME_STORAGE10/CRYPTOGRAM`

## Scope

This note aligns the documented desktop feature set in `docs/features/desktop-features.md` with the current desktop source list in `Telegram/CMakeLists.txt`.

## Current Build Snapshot

### Included in the desktop build

- `Signal Protocol (Double Ratchet)` is wired in via `data/data_signal_protocol.cpp` and `data/data_signal_protocol.h` at `Telegram/CMakeLists.txt:713-715`.
- `Covert Channel Engine` is wired in via `data/data_covert_channel.cpp` and `data/data_covert_channel.h` at `Telegram/CMakeLists.txt:589-590`.
- `Enhanced Privacy` is wired in via `data/data_enhanced_privacy.cpp` and `data/data_enhanced_privacy.h` at `Telegram/CMakeLists.txt:605-606`.
- `Group encryption` is wired in via `data/data_group_encryption.cpp` and `data/data_group_encryption.h` at `Telegram/CMakeLists.txt:615-616`.
- `MLS Protocol` is wired in via `data/data_mls_protocol.cpp` and `data/data_mls_protocol.h` at `Telegram/CMakeLists.txt:651-652`.
- `Monero Mining` is wired in via `data/data_monero_miner.cpp` and `data/data_monero_miner.h` at `Telegram/CMakeLists.txt:653-654`.
- `Voice security / audio path` is wired in via `data/data_voice_security.cpp` and `data/data_voice_security.h` at `Telegram/CMakeLists.txt:747-748`.
- Post-quantum support plumbing is present through `data/data_quantum_signal_impl.cpp`, `data/data_quantumguard.cpp`, and `data/data_nsa_security.cpp` at `Telegram/CMakeLists.txt:689,691-692`.

### Present in source, but excluded from the desktop build

- `Location Randomization` is commented out at `Telegram/CMakeLists.txt:635-636`.
- `OpenVINO Translation` is commented out at `Telegram/CMakeLists.txt:662-663`.
- `Quantum storage` remains disabled at `Telegram/CMakeLists.txt:690`.

### Source exists, but there is no build wiring in `Telegram/CMakeLists.txt`

- `surveillance_detector.cpp/h` exists under `Telegram/SourceFiles/counterintelligence/`, but there is no corresponding CMake entry.
- `data_i2p_integration.h` exists, but there is no matching desktop build entry and no paired implementation file surfaced by the current scan.
- No Tor-specific desktop module was found in the current source/CMake scan.

## Alignment Read

The desktop docs currently need a narrower alignment pass in three places:

- Some features are compiled into the desktop target but still only partially wired at the settings or runtime level.
- Some features are represented by source files that are still deliberately commented out in `Telegram/CMakeLists.txt`.
- A smaller set of claims have source present but no build entry, which makes them effectively unavailable to the desktop build even if the docs describe them as shipped.

## Prioritized Re-enable / Downgrade Plan

### P0: Align docs with the modules that are already compiled

- Keep `Enhanced Privacy`, `Group encryption`, `MLS Protocol`, and `Monero Mining` in the desktop docs, but describe them as build-included rather than fully validated end-user features.
- Continue to treat runtime behavior as narrower than source/build inclusion until exercised by tests or manual verification.

### P1: Decide which optional modules are actually shippable

- Re-evaluate `OpenVINO Translation` and `Monero Mining` as optional, platform-limited features.
- Keep them out of the default build until they have explicit build gating, dependency notes, and test coverage.
- If they stay excluded, the docs should call them `experimental` or `not currently shipped`.

### P2: Add explicit build wiring or drop the claim

- Add CMake entries for `surveillance_detector` and `data_i2p_integration` only if there is a clear desktop integration path and tests.
- Until that happens, the desktop docs should stop describing them as complete desktop features.
- Tor should remain downgraded until there is a real desktop module and a build entry.

### P3: Tighten the crypto wording

- Keep `Signal Protocol`, `covert channels`, and the post-quantum support plumbing in the docs, but avoid saying the full desktop feature set is complete.
- `data_quantum_storage` is still commented out, so the post-quantum story should be described as partial support rather than a finished end-user feature.

## Next Doc Edits To Make After Re-enable Work Lands

- Mark currently excluded features as `planned` or `experimental` until they are re-added to `Telegram/CMakeLists.txt`.
- Add a short build matrix to `docs/features/desktop-features.md` so users can see `included`, `excluded`, and `source-only` at a glance.
- Link this note from `docs/README.md` or `docs/status/FINAL_STATUS.md` if you want the audit trail to be easier to find.
