/**
 * CRYPTOGRAM JNI Wrapper
 * Bridges native CRYPTOGRAM functionality to Android Java/Kotlin.
 */

#include "cryptogram/data_mls_protocol.h"
#include "cryptogram/data_enhanced_privacy.h"

#include <jni.h>
#include <android/log.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <mutex>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#define LOG_TAG "CryptogramNative"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

using namespace Data;

namespace {

constexpr uint8_t kEnvelopeVersion = 1;
constexpr size_t kAes256KeySize = 32;
constexpr size_t kGcmIvSize = 12;
constexpr size_t kGcmTagSize = 16;
constexpr size_t kEnvelopeHeaderSize = 1 + sizeof(uint64_t) + kGcmIvSize + kGcmTagSize;

struct RatchetSession {
    std::vector<uint8_t> key;
    uint64_t sendCounter = 0;
    uint64_t recvCounter = 0;
};

std::mutex gRatchetMutex;
std::unordered_map<int64_t, RatchetSession> gRatchetSessions;

std::mutex gMlsMutex;
std::unordered_map<int64_t, MLSGroupId> gMlsGroups;

RatchetSession &ensureRatchetSession(int64_t userId) {
    auto &session = gRatchetSessions[userId];
    if (session.key.empty()) {
        session.key.resize(kAes256KeySize);
        if (RAND_bytes(session.key.data(), static_cast<int>(session.key.size())) != 1) {
            session.key.clear();
        }
    }
    return session;
}

std::vector<uint8_t> encryptAesGcm(
    const std::vector<uint8_t> &key,
    uint64_t counter,
    const uint8_t *plaintext,
    size_t plaintextLen) {
    if (key.size() != kAes256KeySize) {
        return {};
    }

    std::vector<uint8_t> iv(kGcmIvSize);
    if (RAND_bytes(iv.data(), static_cast<int>(iv.size())) != 1) {
        return {};
    }

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        return {};
    }

    std::vector<uint8_t> ciphertext(plaintextLen + kGcmTagSize);
    std::vector<uint8_t> tag(kGcmTagSize);
    int outLen = 0;
    int totalLen = 0;
    const auto *counterBytes = reinterpret_cast<const unsigned char*>(&counter);

    bool ok =
        EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) == 1 &&
        EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, static_cast<int>(iv.size()), nullptr) == 1 &&
        EVP_EncryptInit_ex(ctx, nullptr, nullptr, key.data(), iv.data()) == 1 &&
        EVP_EncryptUpdate(ctx, nullptr, &outLen, counterBytes, static_cast<int>(sizeof(counter))) == 1 &&
        EVP_EncryptUpdate(ctx, ciphertext.data(), &outLen, plaintext, static_cast<int>(plaintextLen)) == 1;

    totalLen = outLen;
    if (ok) {
        ok = EVP_EncryptFinal_ex(ctx, ciphertext.data() + totalLen, &outLen) == 1;
        totalLen += outLen;
    }
    if (ok) {
        ok = EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, static_cast<int>(tag.size()), tag.data()) == 1;
    }

    EVP_CIPHER_CTX_free(ctx);
    if (!ok) {
        return {};
    }

    ciphertext.resize(totalLen);

    std::vector<uint8_t> envelope;
    envelope.reserve(kEnvelopeHeaderSize + ciphertext.size());
    envelope.push_back(kEnvelopeVersion);
    envelope.insert(envelope.end(), counterBytes, counterBytes + sizeof(counter));
    envelope.insert(envelope.end(), iv.begin(), iv.end());
    envelope.insert(envelope.end(), tag.begin(), tag.end());
    envelope.insert(envelope.end(), ciphertext.begin(), ciphertext.end());
    return envelope;
}

std::optional<std::string> decryptAesGcm(
    const std::vector<uint8_t> &key,
    const uint8_t *ciphertext,
    size_t ciphertextLen,
    uint64_t &counterOut) {
    if (key.size() != kAes256KeySize || ciphertextLen < kEnvelopeHeaderSize) {
        return std::nullopt;
    }
    if (ciphertext[0] != kEnvelopeVersion) {
        return std::nullopt;
    }

    std::memcpy(&counterOut, ciphertext + 1, sizeof(counterOut));
    const auto *iv = ciphertext + 1 + sizeof(counterOut);
    const auto *tag = iv + kGcmIvSize;
    const auto *payload = tag + kGcmTagSize;
    const auto payloadLen = ciphertextLen - kEnvelopeHeaderSize;

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        return std::nullopt;
    }

    std::vector<uint8_t> plaintext(payloadLen + kGcmTagSize);
    int outLen = 0;
    int totalLen = 0;
    const auto *counterBytes = reinterpret_cast<const unsigned char*>(&counterOut);

    bool ok =
        EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) == 1 &&
        EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, static_cast<int>(kGcmIvSize), nullptr) == 1 &&
        EVP_DecryptInit_ex(ctx, nullptr, nullptr, key.data(), iv) == 1 &&
        EVP_DecryptUpdate(ctx, nullptr, &outLen, counterBytes, static_cast<int>(sizeof(counterOut))) == 1 &&
        EVP_DecryptUpdate(ctx, plaintext.data(), &outLen, payload, static_cast<int>(payloadLen)) == 1;

    totalLen = outLen;
    if (ok) {
        ok = EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, static_cast<int>(kGcmTagSize), const_cast<uint8_t*>(tag)) == 1;
    }
    if (ok) {
        ok = EVP_DecryptFinal_ex(ctx, plaintext.data() + totalLen, &outLen) == 1;
        totalLen += outLen;
    }

    EVP_CIPHER_CTX_free(ctx);
    if (!ok) {
        return std::nullopt;
    }

    plaintext.resize(totalLen);
    return std::string(reinterpret_cast<const char*>(plaintext.data()), plaintext.size());
}

bool ensureMlsProtocol() {
    auto *protocol = GetMLSProtocol();
    if (!protocol) {
        InitializeMLSProtocol();
        protocol = GetMLSProtocol();
    }
    return protocol != nullptr;
}

std::string buildRatchetStateJson(int64_t userId, const RatchetSession &session) {
    std::ostringstream out;
    out << "{\"userId\": " << userId
        << ", \"initialized\": true"
        << ", \"protocol\": \"Double Ratchet compatible AES-256-GCM session\""
        << ", \"sendCounter\": " << session.sendCounter
        << ", \"recvCounter\": " << session.recvCounter
        << ", \"keySize\": " << session.key.size()
        << "}";
    return out.str();
}

bool runDoubleRatchetSelfTest() {
    constexpr int64_t kSelfTestUserId = -424242;
    constexpr auto kPlaintext = "cryptogram-selftest";

    std::lock_guard<std::mutex> lock(gRatchetMutex);
    auto &session = ensureRatchetSession(kSelfTestUserId);
    if (session.key.empty()) {
        return false;
    }

    session.sendCounter = 0;
    session.recvCounter = 0;

    const auto ciphertext = encryptAesGcm(
        session.key,
        session.sendCounter++,
        reinterpret_cast<const uint8_t*>(kPlaintext),
        std::strlen(kPlaintext));
    if (ciphertext.empty()) {
        return false;
    }

    uint64_t counter = 0;
    const auto plaintext = decryptAesGcm(
        session.key,
        ciphertext.data(),
        ciphertext.size(),
        counter);
    if (!plaintext.has_value() || plaintext.value() != kPlaintext) {
        return false;
    }

    session.recvCounter = std::max(session.recvCounter, counter + 1);
    EnhancedPrivacy::RegisterCryptogramUser(UserId(static_cast<uint64>(kSelfTestUserId)));
    return true;
}

bool runMlsSelfTest() {
    if (!ensureMlsProtocol()) {
        return false;
    }

    constexpr auto kMessage = "cryptogram-mls-selftest";
    QVector<UserId> members = {
        UserId(static_cast<uint64>(101)),
        UserId(static_cast<uint64>(202)),
    };

    std::lock_guard<std::mutex> lock(gMlsMutex);
    auto *protocol = GetMLSProtocol();
    if (!protocol) {
        return false;
    }

    const auto groupId = protocol->createGroup(
        members,
        MLSCiphersuite::MLS_128_DHKEMX25519_AES128GCM_SHA256_Ed25519);
    if (groupId.empty() || !protocol->hasGroup(groupId)) {
        return false;
    }

    bytes::vector plaintext(
        reinterpret_cast<const std::byte*>(kMessage),
        reinterpret_cast<const std::byte*>(kMessage + std::strlen(kMessage)));
    const auto ciphertext = protocol->encryptMessage(groupId, plaintext);
    if (ciphertext.empty()) {
        return false;
    }

    const auto decrypted = protocol->decryptMessage(groupId, ciphertext);
    if (!decrypted.has_value() || decrypted.value() != plaintext) {
        return false;
    }

    return protocol->addMember(groupId, UserId(static_cast<uint64>(303)))
        && protocol->removeMember(groupId, UserId(static_cast<uint64>(303)));
}

} // namespace

extern "C" {

JNIEXPORT jboolean JNICALL
Java_org_telegram_messenger_cryptogram_DoubleRatchet_nativeInitializeSession(
    JNIEnv *, jobject, jlong userId) {
    LOGD("Initializing Double Ratchet session for user %lld", (long long)userId);

    try {
        std::lock_guard<std::mutex> lock(gRatchetMutex);
        auto &session = ensureRatchetSession(userId);
        if (session.key.empty()) {
            LOGE("Failed to generate session key");
            return JNI_FALSE;
        }
        session.sendCounter = 0;
        session.recvCounter = 0;
        EnhancedPrivacy::RegisterCryptogramUser(UserId(static_cast<uint64>(userId)));
        return JNI_TRUE;
    } catch (const std::exception &e) {
        LOGE("Failed to initialize session: %s", e.what());
        return JNI_FALSE;
    }
}

JNIEXPORT jbyteArray JNICALL
Java_org_telegram_messenger_cryptogram_DoubleRatchet_nativeEncrypt(
    JNIEnv *env, jobject, jlong userId, jstring plaintext) {
    if (!plaintext) {
        LOGE("Null plaintext provided");
        return nullptr;
    }

    const char *messageText = env->GetStringUTFChars(plaintext, nullptr);
    if (!messageText) {
        return nullptr;
    }

    try {
        std::lock_guard<std::mutex> lock(gRatchetMutex);
        auto &session = ensureRatchetSession(userId);
        if (session.key.empty()) {
            env->ReleaseStringUTFChars(plaintext, messageText);
            return nullptr;
        }

        auto ciphertext = encryptAesGcm(
            session.key,
            session.sendCounter++,
            reinterpret_cast<const uint8_t*>(messageText),
            std::strlen(messageText));
        env->ReleaseStringUTFChars(plaintext, messageText);
        if (ciphertext.empty()) {
            LOGE("Encryption failed");
            return nullptr;
        }

        jbyteArray result = env->NewByteArray(static_cast<jsize>(ciphertext.size()));
        env->SetByteArrayRegion(
            result,
            0,
            static_cast<jsize>(ciphertext.size()),
            reinterpret_cast<const jbyte*>(ciphertext.data()));
        return result;
    } catch (const std::exception &e) {
        env->ReleaseStringUTFChars(plaintext, messageText);
        LOGE("Encryption failed: %s", e.what());
        return nullptr;
    }
}

JNIEXPORT jstring JNICALL
Java_org_telegram_messenger_cryptogram_DoubleRatchet_nativeDecrypt(
    JNIEnv *env, jobject, jlong userId, jbyteArray ciphertext) {
    if (!ciphertext) {
        LOGE("Null ciphertext provided");
        return nullptr;
    }

    const auto len = env->GetArrayLength(ciphertext);
    auto *bytes = env->GetByteArrayElements(ciphertext, nullptr);
    if (!bytes) {
        return nullptr;
    }

    try {
        std::lock_guard<std::mutex> lock(gRatchetMutex);
        const auto it = gRatchetSessions.find(userId);
        if (it == gRatchetSessions.end()) {
            env->ReleaseByteArrayElements(ciphertext, bytes, JNI_ABORT);
            LOGE("No session found for user %lld", (long long)userId);
            return nullptr;
        }

        uint64_t counter = 0;
        auto plaintext = decryptAesGcm(
            it->second.key,
            reinterpret_cast<const uint8_t*>(bytes),
            static_cast<size_t>(len),
            counter);
        env->ReleaseByteArrayElements(ciphertext, bytes, JNI_ABORT);
        if (!plaintext.has_value()) {
            LOGE("Authenticated decryption failed");
            return nullptr;
        }

        auto &session = gRatchetSessions[userId];
        session.recvCounter = std::max(session.recvCounter, counter + 1);
        return env->NewStringUTF(plaintext->c_str());
    } catch (const std::exception &e) {
        env->ReleaseByteArrayElements(ciphertext, bytes, JNI_ABORT);
        LOGE("Decryption failed: %s", e.what());
        return nullptr;
    }
}

JNIEXPORT jstring JNICALL
Java_org_telegram_messenger_cryptogram_DoubleRatchet_nativeGetState(
    JNIEnv *env, jobject, jlong userId) {
    try {
        std::lock_guard<std::mutex> lock(gRatchetMutex);
        const auto it = gRatchetSessions.find(userId);
        if (it == gRatchetSessions.end()) {
            return env->NewStringUTF("{\"initialized\": false}");
        }
        const auto stateJson = buildRatchetStateJson(userId, it->second);
        return env->NewStringUTF(stateJson.c_str());
    } catch (const std::exception &e) {
        LOGE("Failed to get state: %s", e.what());
        return nullptr;
    }
}

JNIEXPORT jboolean JNICALL
Java_org_telegram_messenger_cryptogram_MLSProtocol_nativeCreateGroup(
    JNIEnv *env, jobject, jlong groupId, jlongArray memberIds) {
    if (!memberIds) {
        LOGE("Null memberIds provided");
        return JNI_FALSE;
    }
    if (!ensureMlsProtocol()) {
        LOGE("MLS protocol unavailable");
        return JNI_FALSE;
    }

    const auto memberCount = env->GetArrayLength(memberIds);
    auto *members = env->GetLongArrayElements(memberIds, nullptr);
    if (!members) {
        return JNI_FALSE;
    }

    try {
        QVector<UserId> initialMembers;
        initialMembers.reserve(memberCount);
        for (jsize i = 0; i < memberCount; ++i) {
            initialMembers.push_back(UserId(static_cast<uint64>(members[i])));
        }

        std::lock_guard<std::mutex> lock(gMlsMutex);
        gMlsGroups[groupId] = GetMLSProtocol()->createGroup(initialMembers);
        env->ReleaseLongArrayElements(memberIds, members, JNI_ABORT);
        return JNI_TRUE;
    } catch (const std::exception &e) {
        env->ReleaseLongArrayElements(memberIds, members, JNI_ABORT);
        LOGE("Failed to create MLS group: %s", e.what());
        return JNI_FALSE;
    }
}

JNIEXPORT jbyteArray JNICALL
Java_org_telegram_messenger_cryptogram_MLSProtocol_nativeEncryptGroupMessage(
    JNIEnv *env, jobject, jlong groupId, jstring plaintext) {
    if (!plaintext) {
        LOGE("Null plaintext provided");
        return nullptr;
    }
    if (!ensureMlsProtocol()) {
        LOGE("MLS protocol unavailable");
        return nullptr;
    }

    const char *messageText = env->GetStringUTFChars(plaintext, nullptr);
    if (!messageText) {
        return nullptr;
    }

    try {
        std::lock_guard<std::mutex> lock(gMlsMutex);
        const auto it = gMlsGroups.find(groupId);
        if (it == gMlsGroups.end() || !GetMLSProtocol()->hasGroup(it->second)) {
            env->ReleaseStringUTFChars(plaintext, messageText);
            LOGE("MLS group %lld not initialized", (long long)groupId);
            return nullptr;
        }

        bytes::vector plaintextBytes(
            reinterpret_cast<const uint8_t*>(messageText),
            reinterpret_cast<const uint8_t*>(messageText) + std::strlen(messageText));
        auto ciphertext = GetMLSProtocol()->encryptMessage(it->second, plaintextBytes);
        env->ReleaseStringUTFChars(plaintext, messageText);
        if (ciphertext.empty()) {
            return nullptr;
        }

        jbyteArray result = env->NewByteArray(static_cast<jsize>(ciphertext.size()));
        env->SetByteArrayRegion(
            result,
            0,
            static_cast<jsize>(ciphertext.size()),
            reinterpret_cast<const jbyte*>(ciphertext.data()));
        return result;
    } catch (const std::exception &e) {
        env->ReleaseStringUTFChars(plaintext, messageText);
        LOGE("Group encryption failed: %s", e.what());
        return nullptr;
    }
}

JNIEXPORT jstring JNICALL
Java_org_telegram_messenger_cryptogram_MLSProtocol_nativeDecryptGroupMessage(
    JNIEnv *env, jobject, jlong groupId, jbyteArray ciphertext) {
    if (!ciphertext) {
        LOGE("Null ciphertext provided");
        return nullptr;
    }
    if (!ensureMlsProtocol()) {
        LOGE("MLS protocol unavailable");
        return nullptr;
    }

    const auto len = env->GetArrayLength(ciphertext);
    auto *bytes = env->GetByteArrayElements(ciphertext, nullptr);
    if (!bytes) {
        return nullptr;
    }

    try {
        std::lock_guard<std::mutex> lock(gMlsMutex);
        const auto it = gMlsGroups.find(groupId);
        if (it == gMlsGroups.end() || !GetMLSProtocol()->hasGroup(it->second)) {
            env->ReleaseByteArrayElements(ciphertext, bytes, JNI_ABORT);
            LOGE("MLS group %lld not initialized", (long long)groupId);
            return nullptr;
        }

        bytes::vector ciphertextVec(
            reinterpret_cast<const uint8_t*>(bytes),
            reinterpret_cast<const uint8_t*>(bytes) + len);
        auto plaintext = GetMLSProtocol()->decryptMessage(it->second, ciphertextVec);
        env->ReleaseByteArrayElements(ciphertext, bytes, JNI_ABORT);
        if (!plaintext.has_value()) {
            return nullptr;
        }

        std::string plaintextString(
            reinterpret_cast<const char*>(plaintext->data()),
            plaintext->size());
        return env->NewStringUTF(plaintextString.c_str());
    } catch (const std::exception &e) {
        env->ReleaseByteArrayElements(ciphertext, bytes, JNI_ABORT);
        LOGE("Group decryption failed: %s", e.what());
        return nullptr;
    }
}

JNIEXPORT jboolean JNICALL
Java_org_telegram_messenger_cryptogram_MLSProtocol_nativeAddMember(
    JNIEnv *, jobject, jlong groupId, jlong userId) {
    if (!ensureMlsProtocol()) {
        return JNI_FALSE;
    }

    try {
        std::lock_guard<std::mutex> lock(gMlsMutex);
        const auto it = gMlsGroups.find(groupId);
        if (it == gMlsGroups.end() || !GetMLSProtocol()->hasGroup(it->second)) {
            return JNI_FALSE;
        }
        return GetMLSProtocol()->addMember(it->second, UserId(static_cast<uint64>(userId)))
            ? JNI_TRUE
            : JNI_FALSE;
    } catch (const std::exception &e) {
        LOGE("Failed to add member: %s", e.what());
        return JNI_FALSE;
    }
}

JNIEXPORT jboolean JNICALL
Java_org_telegram_messenger_cryptogram_MLSProtocol_nativeRemoveMember(
    JNIEnv *, jobject, jlong groupId, jlong userId) {
    if (!ensureMlsProtocol()) {
        return JNI_FALSE;
    }

    try {
        std::lock_guard<std::mutex> lock(gMlsMutex);
        const auto it = gMlsGroups.find(groupId);
        if (it == gMlsGroups.end() || !GetMLSProtocol()->hasGroup(it->second)) {
            return JNI_FALSE;
        }
        return GetMLSProtocol()->removeMember(it->second, UserId(static_cast<uint64>(userId)))
            ? JNI_TRUE
            : JNI_FALSE;
    } catch (const std::exception &e) {
        LOGE("Failed to remove member: %s", e.what());
        return JNI_FALSE;
    }
}

JNIEXPORT jboolean JNICALL
Java_org_telegram_messenger_cryptogram_EnhancedPrivacy_nativeIsCryptogramUser(
    JNIEnv *, jobject, jlong userId) {
    return EnhancedPrivacy::IsCryptogramUser(UserId(static_cast<uint64>(userId)))
        ? JNI_TRUE
        : JNI_FALSE;
}

JNIEXPORT jstring JNICALL
Java_org_telegram_messenger_cryptogram_CryptogramNative_nativeGetVersion(
    JNIEnv *env, jobject) {
    return env->NewStringUTF("CRYPTOGRAM Android 1.1.0-dev");
}

JNIEXPORT jboolean JNICALL
Java_org_telegram_messenger_cryptogram_CryptogramNative_nativeCheckDoubleRatchet(
    JNIEnv *, jobject) {
    return runDoubleRatchetSelfTest() ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jboolean JNICALL
Java_org_telegram_messenger_cryptogram_CryptogramNative_nativeCheckMLS(
    JNIEnv *, jobject) {
    return runMlsSelfTest() ? JNI_TRUE : JNI_FALSE;
}

} // extern "C"
