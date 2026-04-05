# Reality vs Roadmap (2026-04-03)

This note distills what the repository actually contains today, why the optimistically worded documentation/roadmap is inaccurate in several places, and where readers can still trust the published descriptions. The three parallel perspectives we gathered cover (1) documentation integrity, (2) Android cryptography, and (3) desktop feature wiring.

## Documentation claims that need to be rephrased
- `docs/implementation/CRYPTOGRAM_ANDROID_COMPLETE.md`, `docs/status/TEST_RESULTS.md`, and inherited “ready/build/deploy” statements keep announcing Signal Double Ratchet, MLS, mining, and other surfaces as production-ready even though `docs/status/AUDIT_GAP_REPORT_2026-04-03.md` calls those claims contradictions and the roadmap explicitly flags the same language as misinformation (`ROADMAP_2026-04-03.md`, §“Contradictions to Resolve”). Reword these documents to acknowledge partiality or gate them behind explicit liftoff criteria.
- `DOCUMENTATION.md` and `README.md` continue to point at archived files such as `DOCKER_BUILD.md` and `GITHUB_ACTIONS_BUILD.md` at the repo root, creating broken expectations. Update those links or consolidate the docs so they match the actual file layout.
- `docs/status/FINAL_STATUS.md` already uses guarded language (no “production”) and aligns with the audit evidence. Use it as the canonical tone that other docs should mimic once each surface is clarified.

## Android crypto reality
- The JNI wrapper (`TMessagesProj/jni/cryptogram/CryptogramWrapper.cpp`, §`gRatchetSessions`) is a stateful AES-256-GCM envelope with counters, not a Signal-style ratchet. `buildRatchetStateJson` even advertises an “AES-256-GCM session,” there is no DH handshake or session storage, and the self-test simply encrypts/decrypts a string with the same key (`#L31-229`). The “Double Ratchet complete” language should be replaced with “AES-GCM session placeholder” until the ratchet primitives and state machine are implemented.
- MLS support currently lives in the `gMlsGroups` map and self-tests only a single hard-coded ciphersuite (`CryptogramWrapper.cpp#L48-264`). There is no persistence of transcripts, membership changes, or full RFC 9420 lifecycle, so the docs that trumpet “MLS complete” are overclaiming. Keep the MLS section explicit about placeholder status until durability and interoperability testing exist.
- The roadmap’s warning (phase 1/2) is accurate: the UI/docs were previously overselling Double Ratchet/MLS, and until the native layer is rewritten we should keep these paths marked experimental or hidden to avoid signalling false readiness.

## Desktop wiring reality
- Bridge support remains a placeholder: `Cryptogram::createBridgeSettings` only renders a divider with text that bridge configuration “will be available in a future update” and no controls (`Telegram/SourceFiles/settings/settings_cryptogram.cpp#L212-223`), even though `docs/implementation/SECOND_WAVE_FEATURES_PORT.md` describes a full obfs4/meek/snowflake/Tor bridge subsystem.
- The covert-channel section explicitly states “desktop wiring pending,” echoing that the backend transport is not connected despite the doc claiming the functionality exists (`settings_cryptogram.cpp#L650-678` vs. `docs/features/desktop-features.md`). Keep the UI statement as the truth engine and adjust the docs to note that the transport is not live yet.
- The documented mining slider (0–100% CPU, idle/charging toggles) no longer exists: `createMiningConfiguration` now just shows a static percentage label with a TODO comment about the removed slider (`settings_cryptogram.cpp#L342-360`). Update `docs/implementation/MONERO_MINING_INTEGRATION.md` to describe the current display-only view or reintroduce the slider logic.

## Testing/readiness messaging
- `docs/status/TEST_RESULTS.md` still says “READY FOR BUILD & DEPLOYMENT” with “100% coverage,” but `docs/status/TEST_HARNESS_SCOPE.md` and the audit report are explicit that the harness performs structural/static checks only. Readers who skip the scope doc will believe the builds/tests are complete, so this headline should be softened or accompanied by a clear “static harness only” qualifier.

## Next steps
1. Align each optimistic claim with the current status (or gate it) before publishing new docs.
2. Keep `docs/status/FINAL_STATUS.md` as the reference tone while the other docs are corrected.
3. Once Android/desktop primitives are implemented and tests prove them, revisit this reality doc and remove the caveats.
