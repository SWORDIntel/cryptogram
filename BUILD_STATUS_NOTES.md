## CRYPTOGRAM Progress & Outstanding Work

This document captures the current state of the ongoing effort to re-enable the data modules and make the desktop build succeed.

### Work Completed
- Re-enabled previously disabled data modules across `Telegram/SourceFiles/data/*.cpp`.
- Integrated submodules and build rules for `libsignal`, `taglib`, and `openmls`.
- Implemented real TSM/quantum helpers:
  - Software TSM fallback (`data_tsm_interface.cpp`).
  - Signal HKDF helper using libsignal (`data_signal_hkdf.cpp`).
  - Quantum helpers (KDF/HMAC, NSA/QuantumGuard hooks) and quantum storage plumbing.
- Quantum storage:
  - Added Qt signals: `tierUpgraded`, `dataStored`, `dataRetrieved`.
  - Fixed tier detection (uses hardware probe helper).
  - Converted `QByteArray` blobs to `bytes::span` before sealing; removed private-helper misuse in the factory.
  - Added `QuantumGuard::setProtected` so the factory can enable protection via public API.
- SignalProtocol partial cleanup:
  - Added `asConstUChar/asUChar` helpers and applied them in HKDF/HMAC paths.
  - AES encrypt/decrypt now use internal AES-CBC helpers with `bytes::vector` keys/IVs.
  - Replaced one `_session->data()` call with `history(peerId)`; removed `bytes::vector::subspan` in some spots.

### Build Status - Phase 1 & 2 Complete ✅

**Data Module Compilation Status: SUCCESS**
- ✅ `data_signal_protocol.cpp` - All errors fixed and compiling
- ✅ `data_quantum_signal_impl.cpp` - All errors fixed and compiling
- ✅ `data_signal_quantum.h` - All errors fixed and compiling
- ✅ `data_signal_hkdf.cpp` - Type conversion fixed and compiling

**Phase 1 Fixes Applied (4 critical fixes):**
1. Fixed HKDF constant name typo: `EVP_PKEY_HKDEF_MODE_EXPAND_ONLY` → `EVP_PKEY_KDF_HKDF_MODE_EXPAND_ONLY`
2. Added missing `performClassicalX3DH()` function declaration and implementation
3. Fixed JSON object reference in backup restore: `backup["peers"]` → `backupObj["peers"]`
4. Fixed namespace scoping and variable references

**Phase 2 Fixes Applied (8 additional compatibility fixes):**
1. Fixed OpenSSL pointer type mismatches (std::byte → unsigned char):
   - `EVP_PKEY_new_raw_private_key()` calls (line 1655)
   - `EVP_DigestSign()` calls (line 1676)
   - `EVP_PKEY_new_raw_public_key()` calls (line 1721)
   - `EVP_DigestVerify()` calls (line 1741)
   - `EVP_EncryptUpdate()` calls (line 1809)
   - `EVP_DecryptUpdate()` calls (line 1898)
   - `EVP_DecryptFinal_ex()` calls (line 1908)
   - `EVP_PKEY_get_raw_*_key()` calls (lines 1949, 1959)
2. Removed duplicate `generateLocalKeyBundle()` definition
3. Fixed incomplete History type usage (line 843)
4. Fixed quantumKDF forward declarations and namespace scoping
5. Added missing `#include "data/data_tsm_quantum.h"` for QuantumTSMInterface
6. Fixed invalid C++ `const auto` member variable → `static constexpr`
7. Fixed bytes::vector/QByteArray type conversions in quantum signing
8. Fixed QByteArray/bytes::span conversions in attestation detection
9. Fixed Qt signals macro: `signals:` → `Q_SIGNALS:` in data_tsm_quantum.h
10. Fixed uint8_t to std::byte vector construction in HKDF (data_signal_hkdf.cpp)

### Remaining Issues (Not Related to Signal/Quantum Modules)
- UI layout API mismatches in:
  - `info_channel_earn_list.cpp` (DividerLabel constructor signature change)
  - `create_giveaway_box.cpp` (BoxContentDivider constructor signature change)
  - `info_profile_cover.cpp` (UI widget constructor calls)

These are pre-existing codebase issues unrelated to cryptographic protocol work.

### Next Steps
1. Address UI widget constructor mismatches in unrelated UI modules (Phase 3)
2. Or proceed with quantum protocol functionality testing once UI issues are resolved
3. See `/tmp/cryptogram_builds_john/build_20251121_141336.log` for full build details

