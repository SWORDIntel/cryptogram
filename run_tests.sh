#!/usr/bin/env bash

set -u
set -o pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

pass_count=0
fail_count=0
warn_count=0

log_pass() {
    printf '[PASS] %s\n' "$1"
    ((pass_count++))
}

log_fail() {
    printf '[FAIL] %s\n' "$1"
    ((fail_count++))
}

log_warn() {
    printf '[WARN] %s\n' "$1"
    ((warn_count++))
}

require_file() {
    local rel_path="$1"
    local label="${2:-$1}"
    if [ -f "$ROOT_DIR/$rel_path" ]; then
        log_pass "$label"
    else
        log_fail "$label (missing: $rel_path)"
    fi
}

require_grep() {
    local pattern="$1"
    local rel_path="$2"
    local label="$3"
    if grep -Eq "$pattern" "$ROOT_DIR/$rel_path"; then
        log_pass "$label"
    else
        log_fail "$label (missing pattern: $pattern)"
    fi
}

warn_grep() {
    local pattern="$1"
    local rel_path="$2"
    local label="$3"
    if grep -Eq "$pattern" "$ROOT_DIR/$rel_path"; then
        log_warn "$label"
    else
        log_pass "$label"
    fi
}

echo "======================================"
echo "CRYPTOGRAM Verification Harness"
echo "Static checks only: source wiring, API surface, and test assets"
echo "See: docs/status/TEST_HARNESS_SCOPE.md"
echo "======================================"
echo

echo "TEST 1: Required files"
echo "-------------------------------------"
required_files=(
    "TMessagesProj/jni/cryptogram/CryptogramWrapper.cpp"
    "TMessagesProj/jni/cryptogram/data_signal_protocol.cpp"
    "TMessagesProj/jni/cryptogram/data_signal_protocol.h"
    "TMessagesProj/jni/cryptogram/data_mls_protocol.cpp"
    "TMessagesProj/jni/cryptogram/data_mls_protocol.h"
    "TMessagesProj/jni/cryptogram/data_group_encryption.cpp"
    "TMessagesProj/jni/cryptogram/data_group_encryption.h"
    "TMessagesProj/jni/cryptogram/data_enhanced_privacy.cpp"
    "TMessagesProj/jni/cryptogram/data_enhanced_privacy.h"
    "TMessagesProj/src/main/java/org/telegram/messenger/cryptogram/CryptogramNative.kt"
    "TMessagesProj/src/main/java/org/telegram/messenger/cryptogram/DoubleRatchet.kt"
    "TMessagesProj/src/main/java/org/telegram/messenger/cryptogram/MLSProtocol.kt"
    "TMessagesProj/src/main/java/org/telegram/messenger/cryptogram/EnhancedPrivacy.kt"
    "TMessagesProj/src/main/java/org/telegram/messenger/cryptogram/CryptogramMessageHelper.java"
    "TMessagesProj/src/main/java/org/telegram/ui/CryptogramSettingsActivity.java"
    "TMessagesProj/src/main/java/org/telegram/ui/Components/CryptogramIndicator.java"
    "tests/unit/test_cryptogram_features.cpp"
    "tests/unit/test_double_ratchet.cpp"
    "tests/unit/test_mls_protocol.cpp"
    "tests/unit/CMakeLists.txt"
    "TMessagesProj/jni/CMakeLists.txt"
    "docs/status/TEST_HARNESS_SCOPE.md"
)

for rel_path in "${required_files[@]}"; do
    require_file "$rel_path"
done

echo
echo "TEST 2: JNI and API surface"
echo "-------------------------------------"
require_grep 'Java_org_telegram_messenger_cryptogram_DoubleRatchet_nativeInitializeSession' \
    "TMessagesProj/jni/cryptogram/CryptogramWrapper.cpp" \
    "JNI DoubleRatchet initializeSession exists"
require_grep 'Java_org_telegram_messenger_cryptogram_DoubleRatchet_nativeEncrypt' \
    "TMessagesProj/jni/cryptogram/CryptogramWrapper.cpp" \
    "JNI DoubleRatchet encrypt exists"
require_grep 'Java_org_telegram_messenger_cryptogram_DoubleRatchet_nativeDecrypt' \
    "TMessagesProj/jni/cryptogram/CryptogramWrapper.cpp" \
    "JNI DoubleRatchet decrypt exists"
require_grep 'Java_org_telegram_messenger_cryptogram_DoubleRatchet_nativeGetState' \
    "TMessagesProj/jni/cryptogram/CryptogramWrapper.cpp" \
    "JNI DoubleRatchet getState exists"
require_grep 'Java_org_telegram_messenger_cryptogram_MLSProtocol_nativeCreateGroup' \
    "TMessagesProj/jni/cryptogram/CryptogramWrapper.cpp" \
    "JNI MLS createGroup exists"
require_grep 'Java_org_telegram_messenger_cryptogram_MLSProtocol_nativeEncryptGroupMessage' \
    "TMessagesProj/jni/cryptogram/CryptogramWrapper.cpp" \
    "JNI MLS encryptGroupMessage exists"
require_grep 'Java_org_telegram_messenger_cryptogram_MLSProtocol_nativeDecryptGroupMessage' \
    "TMessagesProj/jni/cryptogram/CryptogramWrapper.cpp" \
    "JNI MLS decryptGroupMessage exists"
require_grep 'Java_org_telegram_messenger_cryptogram_MLSProtocol_nativeAddMember' \
    "TMessagesProj/jni/cryptogram/CryptogramWrapper.cpp" \
    "JNI MLS addMember exists"
require_grep 'Java_org_telegram_messenger_cryptogram_MLSProtocol_nativeRemoveMember' \
    "TMessagesProj/jni/cryptogram/CryptogramWrapper.cpp" \
    "JNI MLS removeMember exists"
require_grep 'Java_org_telegram_messenger_cryptogram_EnhancedPrivacy_nativeIsCryptogramUser' \
    "TMessagesProj/jni/cryptogram/CryptogramWrapper.cpp" \
    "JNI EnhancedPrivacy isCryptogramUser exists"
require_grep 'Java_org_telegram_messenger_cryptogram_CryptogramNative_nativeCheckDoubleRatchet' \
    "TMessagesProj/jni/cryptogram/CryptogramWrapper.cpp" \
    "JNI native Double Ratchet self-check exists"
require_grep 'Java_org_telegram_messenger_cryptogram_CryptogramNative_nativeCheckMLS' \
    "TMessagesProj/jni/cryptogram/CryptogramWrapper.cpp" \
    "JNI native MLS self-check exists"
require_grep 'System\.loadLibrary\("cryptogram"\)' \
    "TMessagesProj/src/main/java/org/telegram/messenger/cryptogram/CryptogramNative.kt" \
    "Kotlin native library load present"
require_grep 'nativeCheckDoubleRatchet' \
    "TMessagesProj/src/main/java/org/telegram/messenger/cryptogram/CryptogramNative.kt" \
    "Kotlin Double Ratchet self-check binding present"
require_grep 'nativeCheckMLS' \
    "TMessagesProj/src/main/java/org/telegram/messenger/cryptogram/CryptogramNative.kt" \
    "Kotlin MLS self-check binding present"
require_grep 'System\.loadLibrary\("cryptogram"\)' \
    "TMessagesProj/src/main/java/org/telegram/messenger/cryptogram/DoubleRatchet.kt" \
    "DoubleRatchet native coupling present"
require_grep 'System\.loadLibrary\("cryptogram"\)' \
    "TMessagesProj/src/main/java/org/telegram/messenger/cryptogram/MLSProtocol.kt" \
    "MLS native coupling present"
require_grep 'nativeIsCryptogramUser' \
    "TMessagesProj/src/main/java/org/telegram/messenger/cryptogram/EnhancedPrivacy.kt" \
    "EnhancedPrivacy native binding present"

echo
echo "TEST 3: Integration hooks"
echo "-------------------------------------"
require_grep 'encryptOutgoingMessage\(' \
    "TMessagesProj/src/main/java/org/telegram/messenger/SendMessagesHelper.java" \
    "Outgoing encryption hook present"
require_grep 'decryptIncomingMessage\(' \
    "TMessagesProj/src/main/java/org/telegram/messenger/MessageObject.java" \
    "Incoming decryption hook present"
require_grep 'CryptogramSettingsActivity' \
    "TMessagesProj/src/main/java/org/telegram/ui/ProfileActivity.java" \
    "Settings entry point present"
require_grep 'toggleCryptogramDoubleRatchet' \
    "TMessagesProj/src/main/java/org/telegram/messenger/SharedConfig.java" \
    "Double Ratchet toggle present"
require_grep 'toggleCryptogramMLS' \
    "TMessagesProj/src/main/java/org/telegram/messenger/SharedConfig.java" \
    "MLS toggle present"
require_grep 'toggleCryptogramHideOnlineStatus' \
    "TMessagesProj/src/main/java/org/telegram/messenger/SharedConfig.java" \
    "Hide online status toggle present"
require_grep 'toggleCryptogramHideTypingIndicator' \
    "TMessagesProj/src/main/java/org/telegram/messenger/SharedConfig.java" \
    "Hide typing indicator toggle present"
require_grep 'toggleCryptogramHideReadReceipts' \
    "TMessagesProj/src/main/java/org/telegram/messenger/SharedConfig.java" \
    "Hide read receipts toggle present"
require_grep 'toggleCryptogramCuratedStickers' \
    "TMessagesProj/src/main/java/org/telegram/messenger/SharedConfig.java" \
    "Curated stickers toggle present"

echo
echo "TEST 4: Build declarations and test wiring"
echo "-------------------------------------"
require_grep 'add_library\(cryptogram STATIC' \
    "TMessagesProj/jni/CMakeLists.txt" \
    "cryptogram static library declared"
require_grep 'target_link_libraries\(cryptogram' \
    "TMessagesProj/jni/CMakeLists.txt" \
    "cryptogram linked into JNI target"
require_grep 'test_cryptogram_features\.cpp' \
    "tests/unit/CMakeLists.txt" \
    "Feature unit test target wired"
require_grep 'test_double_ratchet\.cpp' \
    "tests/unit/CMakeLists.txt" \
    "Double Ratchet unit test target wired"
require_grep 'test_mls_protocol\.cpp' \
    "tests/unit/CMakeLists.txt" \
    "MLS protocol unit test target wired"
require_grep 'TEST_CASE\("MLS key packages are signed and verifiable"' \
    "tests/unit/test_mls_protocol.cpp" \
    "MLS key package test present"
require_grep 'TEST_CASE\("MLS basic group messaging works on supported ciphersuite"' \
    "tests/unit/test_mls_protocol.cpp" \
    "MLS group messaging test present"

echo
echo "TEST 5: Runtime gaps to review manually"
echo "-------------------------------------"
warn_grep 'Will call:' \
    "TMessagesProj/jni/cryptogram/CryptogramWrapper.cpp" \
    "JNI wrapper still contains placeholder call paths"
warn_grep 'placeholder implementation|placeholder packages|placeholder group ID' \
    "TMessagesProj/jni/cryptogram/data_mls_protocol.cpp" \
    "MLS implementation still contains placeholder logic"
warn_grep 'HPKE encryption would be used here|Current implementation passes the secret through until HPKE path encryption lands|return the secret \(placeholder\)' \
    "TMessagesProj/jni/cryptogram/data_mls_protocol.cpp" \
    "MLS dormant UpdatePath helper still contains non-HPKE placeholder path-secret handling"

echo
echo "Summary"
echo "-------------------------------------"
echo "Passed: $pass_count"
echo "Warnings: $warn_count"
echo "Failed: $fail_count"
echo
echo "Static harness verdict:"
if [ "$fail_count" -eq 0 ]; then
    echo "PASS"
    echo "This confirms the documented CRYPTOGRAM surface is wired into source and test assets."
    echo "It does not prove compilation, packaging, device runtime, or crypto correctness."
    exit 0
fi

echo "FAIL"
echo "Fix the failed static checks before relying on the runtime features."
exit 1
