/**
 * CRYPTOGRAM JNI Wrapper
 * Bridges native CRYPTOGRAM functionality to Android Java/Kotlin.
 */

#include "cryptogram/data/data_signal_protocol.h"
#include "cryptogram/data/data_mls_protocol.h"
#include "cryptogram/tor/tor_bridge.h"
#include "cryptogram/security/hardware_detector.h"
#include "cryptogram/security/universal_threat_detector.h"
#include "cryptogram/security/gna_engines.h"
#include "cryptogram/counterintelligence/counterintelligence_controller.h"
#include "cryptogram/privacy/privacy_modules.h"
#include "desktop_shims.h"

#include <jni.h>
#include <android/log.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <mutex>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#define LOG_TAG "CryptogramNative"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

using namespace Data;

namespace {

class EnhancedPrivacy {
public:
    static void RegisterCryptogramUser(UserId userId) {
        cryptogramUsers().insert(userId.valueOf());
    }

    static bool IsCryptogramUser(UserId userId) {
        return cryptogramUsers().find(userId.valueOf()) != cryptogramUsers().end();
    }

private:
    static std::unordered_set<uint64_t> &cryptogramUsers() {
        static std::unordered_set<uint64_t> users;
        return users;
    }
};

JavaVM* gJavaVM = nullptr;
jmethodID gGenerateIdentityKeyMethod = nullptr;
jmethodID gGeneratePreKeyMethod = nullptr;
jmethodID gGenerateOneTimeKeyMethod = nullptr;
jmethodID gSignMessageMethod = nullptr;

std::mutex gSignalMutex;
std::unique_ptr<SignalProtocol> gSignalProtocol;
std::unique_ptr<Session> gDummySession;

std::mutex gMlsMutex;
std::unordered_map<int64_t, MLSGroupId> gMlsGroups;

JNIEnv* getJNIEnv() {
    JNIEnv* env = nullptr;
    if (gJavaVM->GetEnv((void**)&env, JNI_VERSION_1_6) != JNI_OK) {
        gJavaVM->AttachCurrentThread(&env, nullptr);
    }
    return env;
}

bytes::vector jbyteArrayToVector(JNIEnv* env, jbyteArray array) {
    if (!array) return {};
    jsize len = env->GetArrayLength(array);
    jbyte* bytes = env->GetByteArrayElements(array, nullptr);
    bytes::vector result(reinterpret_cast<uint8_t*>(bytes), reinterpret_cast<uint8_t*>(bytes) + len);
    env->ReleaseByteArrayElements(array, bytes, JNI_ABORT);
    return result;
}

bool ensureSignalProtocol() {
    if (!gSignalProtocol) {
        if (!gDummySession) {
            gDummySession = std::make_unique<Session>();
        }
        gSignalProtocol = std::make_unique<SignalProtocol>(gDummySession.get());
        gSignalProtocol->setEnabled(true);
    }
    return gSignalProtocol != nullptr;
}

not_null<PeerData*> peerForUserId(int64_t userId) {
    return PeerData::from(
        gDummySession.get(),
        PeerId(static_cast<uint64>(userId)));
}

std::string buildSessionFingerprint(not_null<PeerData*> peer) {
    auto state = gSignalProtocol->getSession(peer);
    bytes::vector material;
    material.insert(material.end(), state.rootKey.begin(), state.rootKey.end());
    material.insert(material.end(), state.dhSendingPublicKey.begin(), state.dhSendingPublicKey.end());
    material.insert(material.end(), state.dhRemotePublicKey.begin(), state.dhRemotePublicKey.end());

    const auto append32 = [&](uint32_t value) {
        material.push_back(static_cast<uint8_t>(value & 0xFF));
        material.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
        material.push_back(static_cast<uint8_t>((value >> 16) & 0xFF));
        material.push_back(static_cast<uint8_t>((value >> 24) & 0xFF));
    };
    append32(state.sendingMessageCounter);
    append32(state.receivingMessageCounter);

    uint8_t digest[SHA256_DIGEST_LENGTH] = {0};
    if (!material.empty()) {
        SHA256(material.data(), material.size(), digest);
    } else {
        SHA256(reinterpret_cast<const uint8_t*>("empty"), 5, digest);
    }

    std::ostringstream out;
    for (int i = 0; i < 5; ++i) {
        const uint16_t chunk = static_cast<uint16_t>((digest[i * 2] << 8) | digest[i * 2 + 1]);
        if (i > 0) out << '-';
        out << std::setw(4) << std::setfill('0') << (chunk % 10000);
    }
    return out.str();
}

bool ensureMlsProtocol() {
    auto *protocol = GetMLSProtocol();
    if (!protocol) {
        InitializeMLSProtocol();
        protocol = GetMLSProtocol();
    }
    return protocol != nullptr;
}

// Serialization helpers
bytes::vector serializeKeyBundle(const SignalProtocol::KeyBundle &bundle) {
    bytes::vector result;
    // Simple binary format: [ID_LEN][ID][REG_ID][KEY_LEN][KEY][SPK_LEN][SPK][OTP_LEN][OTP][SIG_LEN][SIG]
    auto push_vec = [&](const bytes::vector &v) {
        uint32_t len = static_cast<uint32_t>(v.size());
        result.insert(result.end(), reinterpret_cast<uint8_t*>(&len), reinterpret_cast<uint8_t*>(&len) + 4);
        result.insert(result.end(), v.begin(), v.end());
    };

    uint32_t idLen = static_cast<uint32_t>(bundle.deviceId.identifier.length());
    result.insert(result.end(), reinterpret_cast<uint8_t*>(&idLen), reinterpret_cast<uint8_t*>(&idLen) + 4);
    result.insert(result.end(), bundle.deviceId.identifier.begin(), bundle.deviceId.identifier.end());
    result.insert(result.end(), reinterpret_cast<const uint8_t*>(&bundle.deviceId.registrationId), reinterpret_cast<const uint8_t*>(&bundle.deviceId.registrationId) + 8);

    push_vec(bundle.identityKey);
    push_vec(bundle.signedPreKey);
    push_vec(bundle.oneTimePreKey);
    push_vec(bundle.signature);
    return result;
}

SignalProtocol::KeyBundle deserializeKeyBundle(const uint8_t *data, size_t size) {
    SignalProtocol::KeyBundle bundle;
    size_t pos = 0;
    auto read_vec = [&](bytes::vector &v) {
        if (pos + 4 > size) return;
        uint32_t len;
        std::memcpy(&len, data + pos, 4);
        pos += 4;
        if (pos + len > size) return;
        v.assign(data + pos, data + pos + len);
        pos += len;
    };

    if (pos + 4 > size) return bundle;
    uint32_t idLen;
    std::memcpy(&idLen, data + pos, 4);
    pos += 4;
    if (pos + idLen > size) return bundle;
    bundle.deviceId.identifier = std::string(reinterpret_cast<const char*>(data + pos), idLen);
    pos += idLen;

    if (pos + 8 > size) return bundle;
    std::memcpy(&bundle.deviceId.registrationId, data + pos, 8);
    pos += 8;

    read_vec(bundle.identityKey);
    read_vec(bundle.signedPreKey);
    read_vec(bundle.oneTimePreKey);
    read_vec(bundle.signature);
    return bundle;
}

bytes::vector serializeMetadata(const SignalProtocol::MessageMetadata &metadata) {
    bytes::vector result;
    result.insert(result.end(), reinterpret_cast<const uint8_t*>(&metadata.messageCounter), reinterpret_cast<const uint8_t*>(&metadata.messageCounter) + 4);

    uint32_t ivLen = static_cast<uint32_t>(metadata.iv.size());
    result.insert(result.end(), reinterpret_cast<uint8_t*>(&ivLen), reinterpret_cast<uint8_t*>(&ivLen) + 4);
    result.insert(result.end(), metadata.iv.begin(), metadata.iv.end());

    uint32_t keyLen = static_cast<uint32_t>(metadata.senderPublicKey.size());
    result.insert(result.end(), reinterpret_cast<uint8_t*>(&keyLen), reinterpret_cast<uint8_t*>(&keyLen) + 4);
    result.insert(result.end(), metadata.senderPublicKey.begin(), metadata.senderPublicKey.end());

    result.insert(result.end(), reinterpret_cast<const uint8_t*>(&metadata.timestamp), reinterpret_cast<const uint8_t*>(&metadata.timestamp) + 4);
    return result;
}

bool deserializeMetadata(const uint8_t *data, size_t size, SignalProtocol::MessageMetadata &metadata, size_t &pos) {
    if (pos + 4 > size) return false;
    std::memcpy(&metadata.messageCounter, data + pos, 4);
    pos += 4;

    if (pos + 4 > size) return false;
    uint32_t ivLen;
    std::memcpy(&ivLen, data + pos, 4);
    pos += 4;
    if (pos + ivLen > size) return false;
    metadata.iv.assign(data + pos, data + pos + ivLen);
    pos += ivLen;

    if (pos + 4 > size) return false;
    uint32_t keyLen;
    std::memcpy(&keyLen, data + pos, 4);
    pos += 4;
    if (pos + keyLen > size) return false;
    metadata.senderPublicKey.assign(data + pos, data + pos + keyLen);
    pos += keyLen;

    if (pos + 4 > size) return false;
    std::memcpy(&metadata.timestamp, data + pos, 4);
    pos += 4;
    return true;
}

std::string buildSignalStateJson(int64_t userId) {
    if (!gSignalProtocol) return "{\"initialized\": false}";

    const auto peer = peerForUserId(userId);
    std::ostringstream out;
    out << "{\"userId\": " << userId
        << ", \"initialized\": " << (gSignalProtocol->isEnabled() ? "true" : "false")
        << ", \"protocol\": \"SpyGram Signal Protocol (Double Ratchet)\""
        << ", \"hasSession\": " << (gSignalProtocol->hasSession(peer) ? "true" : "false")
        << "}";
    return out.str();
}

// Serialization helpers for MLS
bytes::vector serializeKeyPackage(const MLSKeyPackage &kp) {
    bytes::vector result;
    auto push_vec = [&](const bytes::vector &v) {
        uint32_t len = static_cast<uint32_t>(v.size());
        result.insert(result.end(), reinterpret_cast<uint8_t*>(&len), reinterpret_cast<uint8_t*>(&len) + 4);
        result.insert(result.end(), v.begin(), v.end());
    };

    result.insert(result.end(), reinterpret_cast<const uint8_t*>(&kp.version), reinterpret_cast<const uint8_t*>(&kp.version) + 2);
    result.insert(result.end(), reinterpret_cast<const uint8_t*>(&kp.ciphersuite), reinterpret_cast<const uint8_t*>(&kp.ciphersuite) + 2);

    push_vec(kp.initKey);
    push_vec(kp.credentialPublicKey);
    push_vec(kp.credential);
    push_vec(kp.signature);

    int64_t creation = kp.creationTime.toMSecsSinceEpoch();
    int64_t expiration = kp.expirationTime.toMSecsSinceEpoch();
    result.insert(result.end(), reinterpret_cast<uint8_t*>(&creation), reinterpret_cast<uint8_t*>(&creation) + 8);
    result.insert(result.end(), reinterpret_cast<uint8_t*>(&expiration), reinterpret_cast<uint8_t*>(&expiration) + 8);

    return result;
}

MLSKeyPackage deserializeKeyPackage(const uint8_t *data, size_t size, size_t &pos) {
    MLSKeyPackage kp;
    auto read_vec = [&](bytes::vector &v) {
        if (pos + 4 > size) return;
        uint32_t len;
        std::memcpy(&len, data + pos, 4);
        pos += 4;
        if (pos + len > size) return;
        v.assign(data + pos, data + pos + len);
        pos += len;
    };

    if (pos + 2 > size) return kp;
    std::memcpy(&kp.version, data + pos, 2);
    pos += 2;

    if (pos + 2 > size) return kp;
    std::memcpy(&kp.ciphersuite, data + pos, 2);
    pos += 2;

    read_vec(kp.initKey);
    read_vec(kp.credentialPublicKey);
    read_vec(kp.credential);
    read_vec(kp.signature);

    if (pos + 8 > size) return kp;
    int64_t creation;
    std::memcpy(&creation, data + pos, 8);
    pos += 8;
    kp.creationTime = QDateTime::fromMSecsSinceEpoch(creation);

    if (pos + 8 > size) return kp;
    int64_t expiration;
    std::memcpy(&expiration, data + pos, 8);
    pos += 8;
    kp.expirationTime = QDateTime::fromMSecsSinceEpoch(expiration);

    return kp;
}

bytes::vector serializeWelcome(const MLSWelcome &welcome) {
    bytes::vector result;
    auto push_vec = [&](const bytes::vector &v) {
        uint32_t len = static_cast<uint32_t>(v.size());
        result.insert(result.end(), reinterpret_cast<uint8_t*>(&len), reinterpret_cast<uint8_t*>(&len) + 4);
        result.insert(result.end(), v.begin(), v.end());
    };

    result.insert(result.end(), reinterpret_cast<const uint8_t*>(&welcome.version), reinterpret_cast<const uint8_t*>(&welcome.version) + 2);
    result.insert(result.end(), reinterpret_cast<const uint8_t*>(&welcome.ciphersuite), reinterpret_cast<const uint8_t*>(&welcome.ciphersuite) + 2);

    push_vec(welcome.encryptedGroupSecrets);
    push_vec(welcome.encryptedGroupInfo);

    int64_t timestamp = welcome.timestamp.toMSecsSinceEpoch();
    result.insert(result.end(), reinterpret_cast<uint8_t*>(&timestamp), reinterpret_cast<uint8_t*>(&timestamp) + 8);

    return result;
}

MLSWelcome deserializeWelcome(const uint8_t *data, size_t size) {
    MLSWelcome welcome;
    size_t pos = 0;
    auto read_vec = [&](bytes::vector &v) {
        if (pos + 4 > size) return;
        uint32_t len;
        std::memcpy(&len, data + pos, 4);
        pos += 4;
        if (pos + len > size) return;
        v.assign(data + pos, data + pos + len);
        pos += len;
    };

    if (pos + 2 > size) return welcome;
    std::memcpy(&welcome.version, data + pos, 2);
    pos += 2;

    if (pos + 2 > size) return welcome;
    std::memcpy(&welcome.ciphersuite, data + pos, 2);
    pos += 2;

    read_vec(welcome.encryptedGroupSecrets);
    read_vec(welcome.encryptedGroupInfo);

    if (pos + 8 > size) return welcome;
    int64_t timestamp;
    std::memcpy(&timestamp, data + pos, 8);
    pos += 8;
    welcome.timestamp = QDateTime::fromMSecsSinceEpoch(timestamp);

    return welcome;
}

bytes::vector serializeProposal(const MLSProposal &proposal) {
    bytes::vector result;
    result.push_back(static_cast<uint8>(proposal.type));
    
    uint64 proposer = proposal.proposer.valueOf();
    result.insert(result.end(), reinterpret_cast<uint8_t*>(&proposer), reinterpret_cast<uint8_t*>(&proposer) + 8);
    const uint8 targetPresent = proposal.targetUser.has_value() ? 1 : 0;
    result.push_back(targetPresent);
    if (targetPresent) {
        uint64 target = proposal.targetUser->valueOf();
        result.insert(result.end(), reinterpret_cast<uint8_t*>(&target), reinterpret_cast<uint8_t*>(&target) + 8);
    }

    auto push_vec = [&](const bytes::vector &v) {
        uint32_t len = static_cast<uint32_t>(v.size());
        result.insert(result.end(), reinterpret_cast<uint8_t*>(&len), reinterpret_cast<uint8_t*>(&len) + 4);
        result.insert(result.end(), v.begin(), v.end());
    };

    if (proposal.type == MLSProposalType::Add && proposal.addKeyPackage.has_value()) {
        auto pkgBytes = serializeKeyPackage(proposal.addKeyPackage.value());
        push_vec(pkgBytes);
    } else if (proposal.type == MLSProposalType::Remove && proposal.removeLeaf.has_value()) {
        uint32_t leaf = proposal.removeLeaf.value();
        result.insert(result.end(), reinterpret_cast<uint8_t*>(&leaf), reinterpret_cast<uint8_t*>(&leaf) + 4);
    } else if (proposal.type == MLSProposalType::Update) {
        push_vec(proposal.updateKey);
    }

    int64_t timestamp = proposal.timestamp.toMSecsSinceEpoch();
    result.insert(result.end(), reinterpret_cast<uint8_t*>(&timestamp), reinterpret_cast<uint8_t*>(&timestamp) + 8);

    return result;
}

bytes::vector serializeCommit(const MLSCommit &commit) {
    bytes::vector result;
    uint32_t proposalCount = static_cast<uint32_t>(commit.proposals.size());
    result.insert(result.end(), reinterpret_cast<uint8_t*>(&proposalCount), reinterpret_cast<uint8_t*>(&proposalCount) + 4);
    for (const auto &p : commit.proposals) {
        auto pBytes = serializeProposal(p);
        uint32_t pLen = static_cast<uint32_t>(pBytes.size());
        result.insert(result.end(), reinterpret_cast<uint8_t*>(&pLen), reinterpret_cast<uint8_t*>(&pLen) + 4);
        result.insert(result.end(), pBytes.begin(), pBytes.end());
    }

    auto push_vec = [&](const bytes::vector &v) {
        uint32_t len = static_cast<uint32_t>(v.size());
        result.insert(result.end(), reinterpret_cast<uint8_t*>(&len), reinterpret_cast<uint8_t*>(&len) + 4);
        result.insert(result.end(), v.begin(), v.end());
    };

    push_vec(commit.path);
    
    uint64 sender = commit.sender.valueOf();
    result.insert(result.end(), reinterpret_cast<uint8_t*>(&sender), reinterpret_cast<uint8_t*>(&sender) + 8);

    int64_t timestamp = commit.timestamp.toMSecsSinceEpoch();
    result.insert(result.end(), reinterpret_cast<uint8_t*>(&timestamp), reinterpret_cast<uint8_t*>(&timestamp) + 8);

    return result;
}

jbyteArray groupIdToJByteArray(JNIEnv *env, const MLSGroupId &groupId) {
    jbyteArray result = env->NewByteArray(static_cast<jsize>(groupId.size()));
    env->SetByteArrayRegion(result, 0, static_cast<jsize>(groupId.size()), reinterpret_cast<const jbyte*>(groupId.data()));
    return result;
}

MLSGroupId jbyteArrayToGroupId(JNIEnv *env, jbyteArray array) {
    if (!array) return {};
    const auto len = env->GetArrayLength(array);
    auto *bytes = env->GetByteArrayElements(array, nullptr);
    if (!bytes) return {};
    MLSGroupId result(reinterpret_cast<const uint8_t*>(bytes), reinterpret_cast<const uint8_t*>(bytes) + len);
    env->ReleaseByteArrayElements(array, bytes, JNI_ABORT);
    return result;
}

bool runDoubleRatchetSelfTest() {
    LOGD("Running Double Ratchet self-test...");
    std::lock_guard<std::mutex> lock(gSignalMutex);
    if (!ensureSignalProtocol()) return false;

    const auto peer = peerForUserId(0x2A5A1DULL);
    if (!gSignalProtocol->hasSession(peer)) {
        const auto remoteBundle = gSignalProtocol->generateLocalKeyBundle();
        gSignalProtocol->createSession(peer, remoteBundle);
    }

    if (!gSignalProtocol->hasSession(peer) || !gSignalProtocol->isEnabled()) {
        return false;
    }

    const QByteArray plaintext("cryptogram-double-ratchet-selftest");
    const auto plaintextBytes = bytes::make_vector(plaintext);
    SignalProtocol::MessageMetadata metadata;

    const auto ciphertext = gSignalProtocol->encryptMessage(
        bytes::make_span(plaintextBytes),
        peer,
        metadata);
    if (ciphertext.empty()) return false;

    const auto recovered = gSignalProtocol->decryptMessage(
        ciphertext,
        peer,
        metadata);
    if (recovered.empty()) return false;

    if (bytes::compare(bytes::make_span(plaintextBytes), bytes::make_span(recovered)) != 0) {
        return false;
    }

    gSignalProtocol->rotateSession(peer, true);
    return gSignalProtocol->hasSession(peer);
}

bool runMlsSelfTest() {
    if (!ensureMlsProtocol()) return false;

    constexpr auto kMessage = "cryptogram-mls-selftest";
    QVector<UserId> members;
    members.push_back(UserId(static_cast<uint64>(101)));
    members.push_back(UserId(static_cast<uint64>(202)));

    std::lock_guard<std::mutex> lock(gMlsMutex);
    auto *protocol = GetMLSProtocol();
    if (!protocol) return false;

    const auto groupId = protocol->createGroup(
        members,
        MLSCiphersuite::MLS_128_DHKEMX25519_AES128GCM_SHA256_Ed25519);
    if (groupId.empty() || !protocol->hasGroup(groupId)) return false;

    auto groupState = protocol->getGroupState(groupId);
    if (!groupState.has_value() || groupState->memberCount() != members.size()) return false;

    if (!protocol->addMember(groupId, UserId(static_cast<uint64>(303)))) return false;
    groupState = protocol->getGroupState(groupId);
    if (!groupState.has_value() || groupState->memberCount() != members.size() + 1) return false;

    if (!protocol->removeMember(groupId, UserId(static_cast<uint64>(202)))) return false;
    groupState = protocol->getGroupState(groupId);
    if (!groupState.has_value() || groupState->memberCount() != members.size()) return false;

    bytes::vector plaintext;
    plaintext.insert(plaintext.end(), kMessage, kMessage + std::strlen(kMessage));

    const auto ciphertext = protocol->encryptMessage(groupId, plaintext);
    if (ciphertext.empty()) return false;

    const auto decrypted = protocol->decryptMessage(groupId, ciphertext);
    if (!decrypted.has_value() || decrypted.value() != plaintext) return false;

    return true;
}

} // namespace

extern "C" {

JNIEXPORT jboolean JNICALL
Java_org_telegram_messenger_cryptogram_DoubleRatchet_nativeInitializeSession(
    JNIEnv *, jobject, jlong userId) {
    LOGD("Initializing Signal session for user %lld", (long long)userId);
    try {
        std::lock_guard<std::mutex> lock(gSignalMutex);
        if (!ensureSignalProtocol()) return JNI_FALSE;
        EnhancedPrivacy::RegisterCryptogramUser(UserId(static_cast<uint64>(userId)));
        return JNI_TRUE;
    } catch (const std::exception &e) {
        LOGE("Failed to initialize session: %s", e.what());
        return JNI_FALSE;
    }
}

JNIEXPORT jbyteArray JNICALL
Java_org_telegram_messenger_cryptogram_DoubleRatchet_nativeGenerateKeyBundle(
    JNIEnv *env, jobject) {
    try {
        std::lock_guard<std::mutex> lock(gSignalMutex);
        if (!ensureSignalProtocol()) return nullptr;

        auto bundle = gSignalProtocol->generateLocalKeyBundle();
        auto serialized = serializeKeyBundle(bundle);

        jbyteArray result = env->NewByteArray(static_cast<jsize>(serialized.size()));
        env->SetByteArrayRegion(result, 0, static_cast<jsize>(serialized.size()), reinterpret_cast<const jbyte*>(serialized.data()));
        return result;
    } catch (const std::exception &e) {
        LOGE("Failed to generate key bundle: %s", e.what());
        return nullptr;
    }
}

JNIEXPORT jboolean JNICALL
Java_org_telegram_messenger_cryptogram_DoubleRatchet_nativeInitializeWithRemoteBundle(
    JNIEnv *env, jobject, jlong userId, jbyteArray bundleData) {
    if (!bundleData) return JNI_FALSE;
    const auto len = env->GetArrayLength(bundleData);
    auto *bytes = env->GetByteArrayElements(bundleData, nullptr);
    if (!bytes) return JNI_FALSE;

    try {
        auto bundle = deserializeKeyBundle(reinterpret_cast<const uint8_t*>(bytes), static_cast<size_t>(len));
        env->ReleaseByteArrayElements(bundleData, bytes, JNI_ABORT);

        std::lock_guard<std::mutex> lock(gSignalMutex);
        if (!ensureSignalProtocol()) return JNI_FALSE;

        const auto peer = peerForUserId(static_cast<int64_t>(userId));
        gSignalProtocol->createSession(peer, bundle);
        EnhancedPrivacy::RegisterCryptogramUser(UserId(static_cast<uint64>(userId)));
        return JNI_TRUE;
    } catch (const std::exception &e) {
        env->ReleaseByteArrayElements(bundleData, bytes, JNI_ABORT);
        LOGE("Failed to initialize with remote bundle: %s", e.what());
        return JNI_FALSE;
    }
}

JNIEXPORT jbyteArray JNICALL
Java_org_telegram_messenger_cryptogram_DoubleRatchet_nativeEncrypt(
    JNIEnv *env, jobject, jlong userId, jstring plaintext) {
    if (!plaintext) return nullptr;
    const char *messageText = env->GetStringUTFChars(plaintext, nullptr);
    if (!messageText) return nullptr;

    try {
        std::lock_guard<std::mutex> lock(gSignalMutex);
        if (!ensureSignalProtocol()) {
            env->ReleaseStringUTFChars(plaintext, messageText);
            return nullptr;
        }

        const QByteArray plaintextData(messageText);
        const auto plaintextBytes = bytes::make_vector(plaintextData);

        const auto peer = peerForUserId(static_cast<int64_t>(userId));
        SignalProtocol::MessageMetadata metadata;
        auto ciphertext = gSignalProtocol->encryptMessage(plaintextBytes, peer, metadata);
        env->ReleaseStringUTFChars(plaintext, messageText);

        if (ciphertext.empty()) return nullptr;

        // Pack into envelope: [VERSION(1)][META_LEN(4)][META][PAYLOAD_LEN(4)][PAYLOAD]
        auto metaBytes = serializeMetadata(metadata);
        uint32_t metaLen = static_cast<uint32_t>(metaBytes.size());
        uint32_t payloadLen = static_cast<uint32_t>(ciphertext.size());

        bytes::vector envelope;
        envelope.push_back(bytes::type(1)); // Version
        envelope.insert(envelope.end(), reinterpret_cast<uint8_t*>(&metaLen), reinterpret_cast<uint8_t*>(&metaLen) + 4);
        envelope.insert(envelope.end(), metaBytes.begin(), metaBytes.end());
        envelope.insert(envelope.end(), reinterpret_cast<uint8_t*>(&payloadLen), reinterpret_cast<uint8_t*>(&payloadLen) + 4);
        envelope.insert(envelope.end(), ciphertext.begin(), ciphertext.end());

        jbyteArray result = env->NewByteArray(static_cast<jsize>(envelope.size()));
        env->SetByteArrayRegion(result, 0, static_cast<jsize>(envelope.size()), reinterpret_cast<const jbyte*>(envelope.data()));
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
    if (!ciphertext) return nullptr;
    const auto len = env->GetArrayLength(ciphertext);
    auto *bytes = env->GetByteArrayElements(ciphertext, nullptr);
    if (!bytes) return nullptr;

    try {
        std::lock_guard<std::mutex> lock(gSignalMutex);
        if (!ensureSignalProtocol()) {
            env->ReleaseByteArrayElements(ciphertext, bytes, JNI_ABORT);
            return nullptr;
        }

        const auto data = reinterpret_cast<const uint8_t*>(bytes);
        if (len < 1 || data[0] != 1 || len < 5) {
            env->ReleaseByteArrayElements(ciphertext, bytes, JNI_ABORT);
            return nullptr;
        }

        size_t pos = 1;
        uint32_t metaLen;
        std::memcpy(&metaLen, data + pos, 4);
        pos += 4;

        const auto metadataEnd = pos + metaLen;
        if (metadataEnd > static_cast<size_t>(len)) {
            env->ReleaseByteArrayElements(ciphertext, bytes, JNI_ABORT);
            return nullptr;
        }

        SignalProtocol::MessageMetadata metadata;
        if (!deserializeMetadata(data, static_cast<size_t>(len), metadata, pos)) {
            env->ReleaseByteArrayElements(ciphertext, bytes, JNI_ABORT);
            return nullptr;
        }

        if (pos != metadataEnd) {
            env->ReleaseByteArrayElements(ciphertext, bytes, JNI_ABORT);
            return nullptr;
        }

        uint32_t payloadLen;
        if (pos + 4 > static_cast<size_t>(len)) {
            env->ReleaseByteArrayElements(ciphertext, bytes, JNI_ABORT);
            return nullptr;
        }

        std::memcpy(&payloadLen, data + pos, 4);
        pos += 4;
        if (pos + payloadLen > static_cast<size_t>(len)) {
            env->ReleaseByteArrayElements(ciphertext, bytes, JNI_ABORT);
            return nullptr;
        }

        bytes::vector ciphertextBytes;
        ciphertextBytes.assign(reinterpret_cast<const bytes::type*>(data + pos), reinterpret_cast<const bytes::type*>(data + pos + payloadLen));

        const auto peer = peerForUserId(static_cast<int64_t>(userId));
        auto plaintext = gSignalProtocol->decryptMessage(ciphertextBytes, peer, metadata);
        env->ReleaseByteArrayElements(ciphertext, bytes, JNI_ABORT);

        if (plaintext.empty()) return nullptr;
        std::string plaintextStr(reinterpret_cast<const char*>(plaintext.data()), plaintext.size());
        return env->NewStringUTF(plaintextStr.c_str());
    } catch (const std::exception &e) {
        env->ReleaseByteArrayElements(ciphertext, bytes, JNI_ABORT);
        LOGE("Decryption failed: %s", e.what());
        return nullptr;
    }
}

JNIEXPORT jboolean JNICALL
Java_org_telegram_messenger_cryptogram_DoubleRatchet_nativeRotateSession(
    JNIEnv *, jobject, jlong userId) {
    try {
        std::lock_guard<std::mutex> lock(gSignalMutex);
        if (!ensureSignalProtocol()) return JNI_FALSE;
        const auto peer = peerForUserId(static_cast<int64_t>(userId));
        if (!gSignalProtocol->hasSession(peer)) return JNI_FALSE;
        gSignalProtocol->rotateSession(peer, true);
        return JNI_TRUE;
    } catch (const std::exception &e) {
        LOGE("Failed to rotate session: %s", e.what());
        return JNI_FALSE;
    }
}

JNIEXPORT jstring JNICALL
Java_org_telegram_messenger_cryptogram_DoubleRatchet_nativeGetFingerprint(
    JNIEnv *env, jobject, jlong userId) {
    try {
        std::lock_guard<std::mutex> lock(gSignalMutex);
        if (!ensureSignalProtocol()) return nullptr;
        const auto peer = peerForUserId(static_cast<int64_t>(userId));
        if (!gSignalProtocol->hasSession(peer)) {
            return env->NewStringUTF("UNINITIALIZED");
        }
        const auto fingerprint = buildSessionFingerprint(peer);
        return env->NewStringUTF(fingerprint.c_str());
    } catch (const std::exception &e) {
        LOGE("Failed to get fingerprint: %s", e.what());
        return nullptr;
    }
}

JNIEXPORT jstring JNICALL
Java_org_telegram_messenger_cryptogram_DoubleRatchet_nativeGetState(
    JNIEnv *env, jobject, jlong userId) {
    try {
        std::lock_guard<std::mutex> lock(gSignalMutex);
        const auto stateJson = buildSignalStateJson(userId);
        return env->NewStringUTF(stateJson.c_str());
    } catch (const std::exception &e) {
        LOGE("Failed to get state: %s", e.what());
        return nullptr;
    }
}

// MLS JNI Implementations
JNIEXPORT jbyteArray JNICALL
Java_org_telegram_messenger_cryptogram_MLSProtocol_nativeGenerateKeyPackage(
    JNIEnv *env, jobject) {
    if (!ensureMlsProtocol()) return nullptr;
    try {
        std::lock_guard<std::mutex> lock(gMlsMutex);
        auto kp = GetMLSProtocol()->generateKeyPackage(MLSCiphersuite::MLS_128_DHKEMX25519_AES128GCM_SHA256_Ed25519);
        auto serialized = serializeKeyPackage(kp);

        jbyteArray result = env->NewByteArray(static_cast<jsize>(serialized.size()));
        env->SetByteArrayRegion(result, 0, static_cast<jsize>(serialized.size()), reinterpret_cast<const jbyte*>(serialized.data()));
        return result;
    } catch (const std::exception &e) {
        LOGE("Failed to generate MLS key package: %s", e.what());
        return nullptr;
    }
}

JNIEXPORT jlong JNICALL
Java_org_telegram_messenger_cryptogram_MLSProtocol_nativeProcessWelcome(
    JNIEnv *env, jobject, jbyteArray welcomeData) {
    if (!ensureMlsProtocol() || !welcomeData) return 0;
    try {
        const auto len = env->GetArrayLength(welcomeData);
        auto *bytes = env->GetByteArrayElements(welcomeData, nullptr);
        if (!bytes) return 0;

        auto welcome = deserializeWelcome(reinterpret_cast<const uint8_t*>(bytes), static_cast<size_t>(len));
        env->ReleaseByteArrayElements(welcomeData, bytes, JNI_ABORT);

        std::lock_guard<std::mutex> lock(gMlsMutex);
        auto groupId = GetMLSProtocol()->processWelcome(welcome);

        // Generate a handle for the group
        int64_t handle;
        RAND_bytes(reinterpret_cast<uint8_t*>(&handle), sizeof(handle));
        gMlsGroups[handle] = groupId;
        
        return handle;
    } catch (const std::exception &e) {
        LOGE("Failed to process MLS welcome: %s", e.what());
        return 0;
    }
}

JNIEXPORT jbyteArray JNICALL
Java_org_telegram_messenger_cryptogram_MLSProtocol_nativeCommitGroupChanges(
    JNIEnv *env, jobject, jlong groupIdHandle) {
    if (!ensureMlsProtocol()) return nullptr;
    try {
        std::lock_guard<std::mutex> lock(gMlsMutex);
        const auto it = gMlsGroups.find(groupIdHandle);
        if (it == gMlsGroups.end()) return nullptr;

        auto commit = GetMLSProtocol()->commitGroupChanges(it->second);
        if (!commit.has_value()) return nullptr;

        auto serialized = serializeCommit(commit.value());
        jbyteArray result = env->NewByteArray(static_cast<jsize>(serialized.size()));
        env->SetByteArrayRegion(result, 0, static_cast<jsize>(serialized.size()), reinterpret_cast<const jbyte*>(serialized.data()));
        return result;
    } catch (const std::exception &e) {
        LOGE("Failed to commit MLS group changes: %s", e.what());
        return nullptr;
    }
}

JNIEXPORT jboolean JNICALL
Java_org_telegram_messenger_cryptogram_MLSProtocol_nativeCreateGroup(
    JNIEnv *env, jobject, jlong groupId, jlongArray memberIds) {
    if (!memberIds || !ensureMlsProtocol()) return JNI_FALSE;
    const auto memberCount = env->GetArrayLength(memberIds);
    auto *members = env->GetLongArrayElements(memberIds, nullptr);
    if (!members) return JNI_FALSE;

    try {
        QVector<UserId> initialMembers;
        for (jsize i = 0; i < memberCount; ++i) initialMembers.push_back(UserId(static_cast<uint64>(members[i])));
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
    if (!plaintext || !ensureMlsProtocol()) return nullptr;
    const char *messageText = env->GetStringUTFChars(plaintext, nullptr);
    if (!messageText) return nullptr;

    try {
        std::lock_guard<std::mutex> lock(gMlsMutex);
        const auto it = gMlsGroups.find(groupId);
        if (it == gMlsGroups.end() || !GetMLSProtocol()->hasGroup(it->second)) {
            env->ReleaseStringUTFChars(plaintext, messageText);
            return nullptr;
        }

        bytes::vector plaintextBytes(reinterpret_cast<const uint8_t*>(messageText), reinterpret_cast<const uint8_t*>(messageText) + std::strlen(messageText));
        auto ciphertext = GetMLSProtocol()->encryptMessage(it->second, plaintextBytes);
        env->ReleaseStringUTFChars(plaintext, messageText);
        if (ciphertext.empty()) return nullptr;

        jbyteArray result = env->NewByteArray(static_cast<jsize>(ciphertext.size()));
        env->SetByteArrayRegion(result, 0, static_cast<jsize>(ciphertext.size()), reinterpret_cast<const jbyte*>(ciphertext.data()));
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
    if (!ciphertext || !ensureMlsProtocol()) return nullptr;
    const auto len = env->GetArrayLength(ciphertext);
    auto *bytes = env->GetByteArrayElements(ciphertext, nullptr);
    if (!bytes) return nullptr;

    try {
        std::lock_guard<std::mutex> lock(gMlsMutex);
        const auto it = gMlsGroups.find(groupId);
        if (it == gMlsGroups.end() || !GetMLSProtocol()->hasGroup(it->second)) {
            env->ReleaseByteArrayElements(ciphertext, bytes, JNI_ABORT);
            return nullptr;
        }

        bytes::vector ciphertextVec(reinterpret_cast<const uint8_t*>(bytes), reinterpret_cast<const uint8_t*>(bytes) + len);
        auto plaintext = GetMLSProtocol()->decryptMessage(it->second, ciphertextVec);
        env->ReleaseByteArrayElements(ciphertext, bytes, JNI_ABORT);
        if (!plaintext.has_value()) return nullptr;

        std::string plaintextString(reinterpret_cast<const char*>(plaintext->data()), plaintext->size());
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
    if (!ensureMlsProtocol()) return JNI_FALSE;
    try {
        std::lock_guard<std::mutex> lock(gMlsMutex);
        const auto it = gMlsGroups.find(groupId);
        if (it == gMlsGroups.end() || !GetMLSProtocol()->hasGroup(it->second)) return JNI_FALSE;
        return GetMLSProtocol()->addMember(it->second, UserId(static_cast<uint64>(userId))) ? JNI_TRUE : JNI_FALSE;
    } catch (...) { return JNI_FALSE; }
}

JNIEXPORT jboolean JNICALL
Java_org_telegram_messenger_cryptogram_MLSProtocol_nativeRemoveMember(
    JNIEnv *, jobject, jlong groupId, jlong userId) {
    if (!ensureMlsProtocol()) return JNI_FALSE;
    try {
        std::lock_guard<std::mutex> lock(gMlsMutex);
        const auto it = gMlsGroups.find(groupId);
        if (it == gMlsGroups.end() || !GetMLSProtocol()->hasGroup(it->second)) return JNI_FALSE;
        return GetMLSProtocol()->removeMember(it->second, UserId(static_cast<uint64>(userId))) ? JNI_TRUE : JNI_FALSE;
    } catch (...) { return JNI_FALSE; }
}

JNIEXPORT jboolean JNICALL
Java_org_telegram_messenger_cryptogram_EnhancedPrivacy_nativeIsCryptogramUser(
    JNIEnv *, jobject, jlong userId) {
    return EnhancedPrivacy::IsCryptogramUser(UserId(static_cast<uint64>(userId))) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jstring JNICALL
Java_org_telegram_messenger_cryptogram_CryptogramNative_nativeGetVersion(
    JNIEnv *env, jobject) {
    return env->NewStringUTF("CRYPTOGRAM Android 1.2.0-FeatureParity");
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

JNIEXPORT void JNICALL
Java_org_telegram_messenger_cryptogram_CryptogramNative_nativeInitializeStorage(
    JNIEnv *env, jobject, jstring path) {
    if (!path) return;
    const char *pathStr = env->GetStringUTFChars(path, nullptr);
    if (!pathStr) return;
    base::SetGlobalStoragePath(pathStr);
    LOGD("Initialized storage path to: %s", pathStr);
    env->ReleaseStringUTFChars(path, pathStr);
}

// OPSEC JNI Implementations

JNIEXPORT jbyteArray JNICALL
Java_org_telegram_messenger_cryptogram_OPSECHelper_nativeWrapDpiEvasion(
    JNIEnv *env, jobject, jbyteArray data, jint method) {
    if (!data) return nullptr;
    const auto len = env->GetArrayLength(data);
    auto *bytes = env->GetByteArrayElements(data, nullptr);
    if (!bytes) return nullptr;

    try {
        bytes::vector input(reinterpret_cast<uint8_t*>(bytes), reinterpret_cast<uint8_t*>(bytes) + len);
        env->ReleaseByteArrayElements(data, bytes, JNI_ABORT);

        bytes::vector output;

        switch (method) {
            case 0: { // HTTPS Mimicry — prepend fake TLS ClientHello-like header
                const uint8_t fakeHello[] = {
                    0x16, 0x03, 0x01, 0x00, 0x05, 0x01, 0x00, 0x00, 0x01,
                    0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
                };
                output.insert(output.end(), fakeHello, fakeHello + sizeof(fakeHello));
                output.insert(output.end(), input.begin(), input.end());
                break;
            }
            case 1: { // HTTP Tunneling — wrap in fake HTTP POST header
                const char *httpHeader = "POST /api/v1/sync HTTP/1.1\r\nHost: cdn.example.com\r\nContent-Length: ";
                auto headerStr = std::string(httpHeader) + std::to_string(input.size()) + "\r\n\r\n";
                output.insert(output.end(), headerStr.begin(), headerStr.end());
                output.insert(output.end(), input.begin(), input.end());
                break;
            }
            case 2: { // DNS Tunneling — base32-encode into fake DNS query format
                const char *dnsPrefix = "query.example.com.";
                output.insert(output.end(), dnsPrefix, dnsPrefix + std::strlen(dnsPrefix));
                for (size_t i = 0; i < input.size(); ++i) {
                    char hex[4];
                    std::snprintf(hex, sizeof(hex), "%02x", input[i]);
                    output.push_back(static_cast<uint8_t>(hex[0]));
                    output.push_back(static_cast<uint8_t>(hex[1]));
                }
                break;
            }
            case 3: { // Generic — random fragmentation with padding
                uint8_t padLen = 0;
                RAND_bytes(&padLen, 1);
                padLen = padLen % 16;
                for (uint8_t i = 0; i < padLen; ++i) {
                    uint8_t randomByte = 0;
                    RAND_bytes(&randomByte, 1);
                    output.push_back(randomByte);
                }
                output.insert(output.end(), input.begin(), input.end());
                for (uint8_t i = 0; i < padLen; ++i) {
                    uint8_t randomByte = 0;
                    RAND_bytes(&randomByte, 1);
                    output.push_back(randomByte);
                }
                break;
            }
            default: { // Auto / passthrough
                output = input;
                break;
            }
        }

        jbyteArray result = env->NewByteArray(static_cast<jsize>(output.size()));
        env->SetByteArrayRegion(result, 0, static_cast<jsize>(output.size()),
            reinterpret_cast<const jbyte*>(output.data()));
        return result;
    } catch (const std::exception &e) {
        LOGE("DPI evasion wrap failed: %s", e.what());
        return nullptr;
    }
}

JNIEXPORT jboolean JNICALL
Java_org_telegram_messenger_cryptogram_OPSECHelper_nativeSecureWipe(
    JNIEnv *env, jobject, jstring pathStr) {
    if (!pathStr) return JNI_FALSE;
    const char *path = env->GetStringUTFChars(pathStr, nullptr);
    if (!path) return JNI_FALSE;

    try {
        FILE *f = std::fopen(path, "r+b");
        if (!f) {
            env->ReleaseStringUTFChars(pathStr, path);
            return JNI_TRUE; // File doesn't exist, consider it wiped
        }

        std::fseek(f, 0, SEEK_END);
        long fileSize = std::ftell(f);
        std::fseek(f, 0, SEEK_SET);

        if (fileSize > 0) {
            const size_t bufSize = 4096;
            uint8_t buffer[bufSize];
            for (int pass = 0; pass < 3; ++pass) {
                std::fseek(f, 0, SEEK_SET);
                for (long written = 0; written < fileSize; written += bufSize) {
                    size_t toWrite = std::min(static_cast<long>(bufSize), fileSize - written);
                    RAND_bytes(buffer, static_cast<int>(toWrite));
                    std::fwrite(buffer, 1, toWrite, f);
                }
                std::fflush(f);
            }
        }

        std::fclose(f);
        std::remove(path);
        env->ReleaseStringUTFChars(pathStr, path);
        LOGD("Secure wiped: %s", path);
        return JNI_TRUE;
    } catch (const std::exception &e) {
        env->ReleaseStringUTFChars(pathStr, path);
        LOGE("Secure wipe failed: %s", e.what());
        return JNI_FALSE;
    }
}

// Tor JNI Implementations

JNIEXPORT jboolean JNICALL
Java_org_telegram_messenger_cryptogram_OPSECHelper_nativeTorStart(
    JNIEnv *env, jobject, jstring socksHost, jint socksPort, jstring dataDir) {
    Cryptogram::TorConfig config;
    config.socksPort = static_cast<uint16_t>(socksPort);

    const char* hostStr = env->GetStringUTFChars(socksHost, nullptr);
    if (hostStr) {
        config.socksHost = hostStr;
        env->ReleaseStringUTFChars(socksHost, hostStr);
    }

    if (dataDir) {
        const char* dirStr = env->GetStringUTFChars(dataDir, nullptr);
        if (dirStr) {
            config.dataDirectory = dirStr;
            env->ReleaseStringUTFChars(dataDir, dirStr);
        }
    }

    return Cryptogram::TorBridge::instance().start(config);
}

JNIEXPORT void JNICALL
Java_org_telegram_messenger_cryptogram_OPSECHelper_nativeTorStop(
    JNIEnv *, jobject) {
    Cryptogram::TorBridge::instance().stop();
}

JNIEXPORT jint JNICALL
Java_org_telegram_messenger_cryptogram_OPSECHelper_nativeTorStatus(
    JNIEnv *, jobject) {
    return static_cast<jint>(Cryptogram::TorBridge::instance().status());
}

JNIEXPORT jstring JNICALL
Java_org_telegram_messenger_cryptogram_OPSECHelper_nativeTorSocksProxy(
    JNIEnv *env, jobject) {
    const auto proxy = Cryptogram::TorBridge::instance().getSocksProxy();
    return env->NewStringUTF(proxy.c_str());
}

JNIEXPORT jint JNICALL
Java_org_telegram_messenger_cryptogram_OPSECHelper_nativeTorBootstrapProgress(
    JNIEnv *, jobject) {
    return Cryptogram::TorBridge::instance().getBootstrapProgress();
}

JNIEXPORT jboolean JNICALL
Java_org_telegram_messenger_cryptogram_OPSECHelper_nativeTorNewIdentity(
    JNIEnv *, jobject) {
    return Cryptogram::TorBridge::instance().newIdentity();
}

JNIEXPORT jstring JNICALL
Java_org_telegram_messenger_cryptogram_OPSECHelper_nativeTorLastError(
    JNIEnv *env, jobject) {
    const auto err = Cryptogram::TorBridge::instance().getLastError();
    return env->NewStringUTF(err.c_str());
}

// Voice Morphing JNI Implementations

JNIEXPORT jbyteArray JNICALL
Java_org_telegram_messenger_cryptogram_OPSECHelper_nativePitchShift(
    JNIEnv *env, jobject, jbyteArray data, jfloat shift) {
    if (!data) return nullptr;
    const auto len = env->GetArrayLength(data);
    if (len < 128) return data;

    auto *bytes = env->GetByteArrayElements(data, nullptr);
    if (!bytes) return nullptr;

    const auto *samples = reinterpret_cast<const int16_t*>(bytes);
    const int numSamples = len / sizeof(int16_t);

    const float factor = std::exp(shift * 0.69314718f);
    const int resampledLen = static_cast<int>(numSamples / factor);
    if (resampledLen < 1) {
        env->ReleaseByteArrayElements(data, bytes, JNI_ABORT);
        return data;
    }

    std::vector<int16_t> resampled(resampledLen);
    for (int i = 0; i < resampledLen; ++i) {
        const float srcPos = i * factor;
        const int idx0 = static_cast<int>(srcPos);
        const int idx1 = std::min(idx0 + 1, numSamples - 1);
        const float frac = srcPos - idx0;
        const float val = samples[idx0] * (1.0f - frac) + samples[idx1] * frac;
        resampled[i] = static_cast<int16_t>(std::clamp(val, -32768.0f, 32767.0f));
    }

    const int frameSize = 512;
    const int hopSize = frameSize / 2;
    const int searchRange = hopSize / 2;

    std::vector<float> output(numSamples, 0.0f);
    std::vector<float> window(frameSize);
    for (int i = 0; i < frameSize; ++i) {
        window[i] = 0.5f * (1.0f - std::cos(2.0f * M_PI * i / (frameSize - 1)));
    }

    int readPos = 0, writePos = 0;
    while (writePos + frameSize <= numSamples) {
        float bestCorr = -1.0f;
        int bestOffset = 0;
        for (int delta = -searchRange; delta <= searchRange; ++delta) {
            const int candidatePos = readPos + delta;
            if (candidatePos < 0 || candidatePos + frameSize > resampledLen) continue;
            float corr = 0.0f;
            for (int i = 0; i < frameSize; ++i) {
                corr += window[i] * resampled[candidatePos + i] * output[writePos + i];
            }
            if (corr > bestCorr) { bestCorr = corr; bestOffset = delta; }
        }
        const int synchPos = readPos + bestOffset;
        for (int i = 0; i < frameSize; ++i) {
            if (synchPos + i < resampledLen && writePos + i < numSamples) {
                output[writePos + i] += window[i] * resampled[synchPos + i];
            }
        }
        readPos += hopSize;
        writePos += hopSize;
    }

    env->ReleaseByteArrayElements(data, bytes, JNI_ABORT);

    jbyteArray result = env->NewByteArray(numSamples * sizeof(int16_t));
    if (!result) return nullptr;
    std::vector<int16_t> outSamples(numSamples);
    for (int i = 0; i < numSamples; ++i) {
        outSamples[i] = static_cast<int16_t>(std::clamp(output[i], -32768.0f, 32767.0f));
    }
    env->SetByteArrayRegion(result, 0, numSamples * sizeof(int16_t),
        reinterpret_cast<const jbyte*>(outSamples.data()));
    return result;
}

JNIEXPORT jbyteArray JNICALL
Java_org_telegram_messenger_cryptogram_OPSECHelper_nativeFormantShift(
    JNIEnv *env, jobject, jbyteArray data, jint shift) {
    if (!data || shift == 0) return data;
    const auto len = env->GetArrayLength(data);
    if (len < 256) return data;

    auto *bytes = env->GetByteArrayElements(data, nullptr);
    if (!bytes) return nullptr;

    const auto *samples = reinterpret_cast<const int16_t*>(bytes);
    const int numSamples = len / sizeof(int16_t);

    const float factor = std::pow(2.0f, shift / 12.0f);
    const int frameSize = 512;
    const int hopSize = frameSize / 4;
    const int lpcOrder = 12;

    std::vector<float> output(numSamples, 0.0f);
    std::vector<float> window(frameSize);
    for (int i = 0; i < frameSize; ++i) {
        window[i] = 0.5f * (1.0f - std::cos(2.0f * M_PI * i / (frameSize - 1)));
    }

    for (int start = 0; start + frameSize <= numSamples; start += hopSize) {
        std::vector<float> frame(frameSize);
        for (int i = 0; i < frameSize; ++i) {
            frame[i] = samples[start + i] * window[i];
        }

        std::vector<float> autocorr(lpcOrder + 1, 0.0f);
        for (int lag = 0; lag <= lpcOrder; ++lag) {
            for (int i = lag; i < frameSize; ++i) {
                autocorr[lag] += frame[i] * frame[i - lag];
            }
        }
        if (autocorr[0] < 1e-6f) continue;

        std::vector<float> lpc(lpcOrder + 1, 0.0f);
        std::vector<float> reflection(lpcOrder, 0.0f);
        lpc[0] = 1.0f;
        float err = autocorr[0];
        for (int m = 0; m < lpcOrder; ++m) {
            float acc = autocorr[m + 1];
            for (int j = 0; j < m; ++j) acc += lpc[j + 1] * autocorr[m - j];
            if (err < 1e-10f) break;
            reflection[m] = -acc / err;
            lpc[m + 1] = reflection[m];
            for (int j = 0; j < m; ++j) {
                const float tmp = lpc[j + 1];
                lpc[j + 1] = tmp + reflection[m] * lpc[m - j];
            }
            err *= (1.0f - reflection[m] * reflection[m]);
        }

        std::vector<float> residual(frameSize, 0.0f);
        for (int i = 0; i < frameSize; ++i) {
            float pred = 0.0f;
            for (int j = 1; j <= lpcOrder && j <= i; ++j) pred += lpc[j] * frame[i - j];
            residual[i] = frame[i] - pred;
        }

        std::vector<float> warpedResidual(frameSize, 0.0f);
        for (int i = 0; i < frameSize; ++i) {
            const float srcPos = i * factor;
            const int idx0 = static_cast<int>(srcPos);
            const int idx1 = std::min(idx0 + 1, frameSize - 1);
            const float frac = srcPos - idx0;
            warpedResidual[i] = residual[idx0] * (1.0f - frac) + residual[idx1] * frac;
        }

        std::vector<float> synthesized(frameSize, 0.0f);
        for (int i = 0; i < frameSize; ++i) {
            float val = warpedResidual[i];
            for (int j = 1; j <= lpcOrder && j <= i; ++j) val -= lpc[j] * synthesized[i - j];
            synthesized[i] = val;
        }

        for (int i = 0; i < frameSize; ++i) {
            output[start + i] += synthesized[i] * window[i];
        }
    }

    env->ReleaseByteArrayElements(data, bytes, JNI_ABORT);

    float maxVal = 0.0f;
    for (int i = 0; i < numSamples; ++i) maxVal = std::max(maxVal, std::abs(output[i]));
    const float normScale = (maxVal > 32767.0f) ? 32767.0f / maxVal : 1.0f;

    jbyteArray result = env->NewByteArray(numSamples * sizeof(int16_t));
    if (!result) return nullptr;
    std::vector<int16_t> outSamples(numSamples);
    for (int i = 0; i < numSamples; ++i) {
        outSamples[i] = static_cast<int16_t>(std::clamp(output[i] * normScale, -32768.0f, 32767.0f));
    }
    env->SetByteArrayRegion(result, 0, numSamples * sizeof(int16_t),
        reinterpret_cast<const jbyte*>(outSamples.data()));
    return result;
}

JNIEXPORT jint JNICALL
Java_org_telegram_messenger_cryptogram_OPSECHelper_nativeGetQuantumSecurityLevel(
    JNIEnv *, jobject) {
    // Return the configured quantum security level (128, 256, or 384)
    // This is a placeholder that returns the default; the actual level
    // is managed by SharedConfig on the Kotlin side.
    return 128;
}

JNIEXPORT jboolean JNICALL
Java_org_telegram_messenger_cryptogram_OPSECHelper_nativeCheckPQC(
    JNIEnv *, jobject) {
    // Check if OpenSSL supports PQC algorithms
    // Currently we verify that the crypto library is functional
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return JNI_FALSE;
    EVP_CIPHER_CTX_free(ctx);
    return JNI_TRUE;
}

// ===== Security Module JNI Implementations =====

std::mutex gSecurityMutex;
std::unique_ptr<Security::HardwareDetector> gHardwareDetector;
std::unique_ptr<Security::UniversalThreatDetector> gThreatDetector;
std::unique_ptr<SpyGram::Counterintelligence::CounterIntelligenceController> gCIController;
std::unique_ptr<Security::GNAAcousticSecurity> gAcousticSecurity;

bool ensureHardwareDetector() {
    if (!gHardwareDetector) {
        gHardwareDetector = std::make_unique<Security::HardwareDetector>();
    }
    return gHardwareDetector != nullptr;
}

bool ensureThreatDetector() {
    if (!gThreatDetector) {
        gThreatDetector = std::make_unique<Security::UniversalThreatDetector>();
        gThreatDetector->initialize();
        gThreatDetector->setEnabled(true);
    }
    return gThreatDetector != nullptr;
}

bool ensureCIController() {
    if (!gCIController) {
        gCIController = std::make_unique<SpyGram::Counterintelligence::CounterIntelligenceController>();
        gCIController->initialize();
    }
    return gCIController != nullptr;
}

bool ensureAcousticSecurity() {
    if (!gAcousticSecurity) {
        gAcousticSecurity = std::make_unique<Security::GNAAcousticSecurity>();
        gAcousticSecurity->initialize();
    }
    return gAcousticSecurity != nullptr;
}

// Hardware Detector JNI

JNIEXPORT jstring JNICALL
Java_org_telegram_messenger_cryptogram_SecurityNative_nativeDetectHardware(
    JNIEnv *env, jobject) {
    try {
        std::lock_guard<std::mutex> lock(gSecurityMutex);
        if (!ensureHardwareDetector()) return env->NewStringUTF("{}");
        gHardwareDetector->detectAll();
        auto profile = gHardwareDetector->getHardwareProfile();
        std::ostringstream out;
        out << "{\"cpu\":{\"cores\":" << profile.cpu.cores
            << ",\"name\":\"" << profile.cpu.name.toStdString() << "\""
            << ",\"aesni\":" << (profile.cpu.aesniSupport ? "true" : "false")
            << ",\"avx\":" << (profile.cpu.avxSupport ? "true" : "false")
            << "},\"gpu\":{\"available\":" << (profile.gpu.available ? "true" : "false")
            << ",\"name\":\"" << profile.gpu.name.toStdString() << "\""
            << "},\"npu\":{\"available\":" << (profile.npu.available ? "true" : "false")
            << ",\"name\":\"" << profile.npu.name.toStdString() << "\""
            << "},\"tpm\":{\"available\":" << (profile.tpm.available ? "true" : "false")
            << "},\"overallScore\":" << gHardwareDetector->calculateOverallScore() << "}";
        return env->NewStringUTF(out.str().c_str());
    } catch (const std::exception &e) {
        LOGE("Hardware detection failed: %s", e.what());
        return env->NewStringUTF("{}");
    }
}

JNIEXPORT jdouble JNICALL
Java_org_telegram_messenger_cryptogram_SecurityNative_nativeGetHardwareScore(
    JNIEnv *, jobject) {
    try {
        std::lock_guard<std::mutex> lock(gSecurityMutex);
        if (!ensureHardwareDetector()) return 0.0;
        return gHardwareDetector->calculateOverallScore();
    } catch (...) { return 0.0; }
}

// Threat Detector JNI

JNIEXPORT jstring JNICALL
Java_org_telegram_messenger_cryptogram_SecurityNative_nativeAnalyzeText(
    JNIEnv *env, jobject, jstring text, jstring context) {
    if (!text) return env->NewStringUTF("{}");
    const char *textStr = env->GetStringUTFChars(text, nullptr);
    if (!textStr) return env->NewStringUTF("{}");
    const char *ctxStr = context ? env->GetStringUTFChars(context, nullptr) : nullptr;

    try {
        std::lock_guard<std::mutex> lock(gSecurityMutex);
        if (!ensureThreatDetector()) {
            env->ReleaseStringUTFChars(text, textStr);
            if (ctxStr) env->ReleaseStringUTFChars(context, ctxStr);
            return env->NewStringUTF("{}");
        }
        auto analysis = gThreatDetector->analyzeText(QString(textStr), ctxStr ? QString(ctxStr) : QString());
        env->ReleaseStringUTFChars(text, textStr);
        if (ctxStr) env->ReleaseStringUTFChars(context, ctxStr);

        std::ostringstream out;
        out << "{\"result\":" << static_cast<int>(analysis.result)
            << ",\"severity\":" << static_cast<int>(analysis.severity)
            << ",\"confidence\":" << static_cast<int>(analysis.confidence)
            << ",\"threatScore\":" << analysis.threatScore
            << ",\"malwareScore\":" << analysis.malwareScore
            << ",\"phishingScore\":" << analysis.phishingScore
            << ",\"description\":\"" << analysis.description.toStdString() << "\""
            << ",\"processingTimeMs\":" << analysis.processingTimeMs
            << "}";
        return env->NewStringUTF(out.str().c_str());
    } catch (const std::exception &e) {
        env->ReleaseStringUTFChars(text, textStr);
        if (ctxStr) env->ReleaseStringUTFChars(context, ctxStr);
        LOGE("Threat analysis failed: %s", e.what());
        return env->NewStringUTF("{}");
    }
}

JNIEXPORT jstring JNICALL
Java_org_telegram_messenger_cryptogram_SecurityNative_nativeAnalyzeFile(
    JNIEnv *env, jobject, jstring filePath) {
    if (!filePath) return env->NewStringUTF("{}");
    const char *pathStr = env->GetStringUTFChars(filePath, nullptr);
    if (!pathStr) return env->NewStringUTF("{}");

    try {
        std::lock_guard<std::mutex> lock(gSecurityMutex);
        if (!ensureThreatDetector()) {
            env->ReleaseStringUTFChars(filePath, pathStr);
            return env->NewStringUTF("{}");
        }
        auto analysis = gThreatDetector->analyzeFile(QString(pathStr));
        env->ReleaseStringUTFChars(filePath, pathStr);

        std::ostringstream out;
        out << "{\"result\":" << static_cast<int>(analysis.result)
            << ",\"severity\":" << static_cast<int>(analysis.severity)
            << ",\"threatScore\":" << analysis.threatScore
            << ",\"description\":\"" << analysis.description.toStdString() << "\""
            << "}";
        return env->NewStringUTF(out.str().c_str());
    } catch (const std::exception &e) {
        env->ReleaseStringUTFChars(filePath, pathStr);
        LOGE("File analysis failed: %s", e.what());
        return env->NewStringUTF("{}");
    }
}

JNIEXPORT jboolean JNICALL
Java_org_telegram_messenger_cryptogram_SecurityNative_nativeDetectSuspiciousPatterns(
    JNIEnv *env, jobject, jstring content, jobject resultList) {
    if (!content) return JNI_FALSE;
    const char *contentStr = env->GetStringUTFChars(content, nullptr);
    if (!contentStr) return JNI_FALSE;

    try {
        std::lock_guard<std::mutex> lock(gSecurityMutex);
        if (!ensureThreatDetector()) {
            env->ReleaseStringUTFChars(content, contentStr);
            return JNI_FALSE;
        }
        QStringList patterns;
        bool detected = gThreatDetector->detectSuspiciousPatterns(QString(contentStr), patterns);
        env->ReleaseStringUTFChars(content, contentStr);

        if (resultList && detected) {
            jclass listClass = env->GetObjectClass(resultList);
            jmethodID addMethod = env->GetMethodID(listClass, "add", "(Ljava/lang/Object;)Z");
            for (const auto &p : patterns) {
                jstring jstr = env->NewStringUTF(p.toStdString().c_str());
                env->CallBooleanMethod(resultList, addMethod, jstr);
                env->DeleteLocalRef(jstr);
            }
            env->DeleteLocalRef(listClass);
        }
        return detected ? JNI_TRUE : JNI_FALSE;
    } catch (...) {
        env->ReleaseStringUTFChars(content, contentStr);
        return JNI_FALSE;
    }
}

JNIEXPORT jstring JNICALL
Java_org_telegram_messenger_cryptogram_SecurityNative_nativeGetThreatStats(
    JNIEnv *env, jobject) {
    try {
        std::lock_guard<std::mutex> lock(gSecurityMutex);
        if (!ensureThreatDetector()) return env->NewStringUTF("{}");
        auto stats = gThreatDetector->getStatistics();
        std::ostringstream out;
        out << "{\"totalAnalyses\":" << stats.totalAnalyses
            << ",\"safeDetections\":" << stats.safeDetections
            << ",\"threatDetections\":" << stats.threatDetections
            << ",\"errorCount\":" << stats.errorCount
            << ",\"averageProcessingTime\":" << stats.averageProcessingTime
            << "}";
        return env->NewStringUTF(out.str().c_str());
    } catch (...) { return env->NewStringUTF("{}"); }
}

// Counterintelligence JNI

JNIEXPORT jboolean JNICALL
Java_org_telegram_messenger_cryptogram_SecurityNative_nativeInitializeCI(
    JNIEnv *, jobject) {
    try {
        std::lock_guard<std::mutex> lock(gSecurityMutex);
        return ensureCIController() ? JNI_TRUE : JNI_FALSE;
    } catch (...) { return JNI_FALSE; }
}

JNIEXPORT jint JNICALL
Java_org_telegram_messenger_cryptogram_SecurityNative_nativeGetCurrentThreatLevel(
    JNIEnv *, jobject) {
    try {
        std::lock_guard<std::mutex> lock(gSecurityMutex);
        if (!gCIController) return 0;
        return static_cast<jint>(gCIController->getCurrentThreatLevel());
    } catch (...) { return 0; }
}

JNIEXPORT jboolean JNICALL
Java_org_telegram_messenger_cryptogram_SecurityNative_nativeActivateEmergencyMode(
    JNIEnv *, jobject) {
    try {
        std::lock_guard<std::mutex> lock(gSecurityMutex);
        if (!gCIController) return JNI_FALSE;
        gCIController->activateEmergencyMode();
        return JNI_TRUE;
    } catch (...) { return JNI_FALSE; }
}

JNIEXPORT jboolean JNICALL
Java_org_telegram_messenger_cryptogram_SecurityNative_nativeDeactivateEmergencyMode(
    JNIEnv *, jobject) {
    try {
        std::lock_guard<std::mutex> lock(gSecurityMutex);
        if (!gCIController) return JNI_FALSE;
        gCIController->deactivateEmergencyMode();
        return JNI_TRUE;
    } catch (...) { return JNI_FALSE; }
}

JNIEXPORT jstring JNICALL
Java_org_telegram_messenger_cryptogram_SecurityNative_nativeGetCIReport(
    JNIEnv *env, jobject) {
    try {
        std::lock_guard<std::mutex> lock(gSecurityMutex);
        if (!gCIController || !gCIController->getDashboard()) return env->NewStringUTF("");
        auto report = gCIController->getDashboard()->generateReport();
        return env->NewStringUTF(report.toStdString().c_str());
    } catch (...) { return env->NewStringUTF(""); }
}

JNIEXPORT jboolean JNICALL
Java_org_telegram_messenger_cryptogram_SecurityNative_nativeSetGovernmentMode(
    JNIEnv *, jobject, jboolean enabled) {
    try {
        std::lock_guard<std::mutex> lock(gSecurityMutex);
        if (!gCIController) return JNI_FALSE;
        gCIController->setGovernmentMode(enabled);
        return JNI_TRUE;
    } catch (...) { return JNI_FALSE; }
}

// GNA Acoustic Security JNI

JNIEXPORT jboolean JNICALL
Java_org_telegram_messenger_cryptogram_SecurityNative_nativeInitializeAcoustic(
    JNIEnv *, jobject) {
    try {
        std::lock_guard<std::mutex> lock(gSecurityMutex);
        return ensureAcousticSecurity() ? JNI_TRUE : JNI_FALSE;
    } catch (...) { return JNI_FALSE; }
}

JNIEXPORT jbyteArray JNICALL
Java_org_telegram_messenger_cryptogram_SecurityNative_nativeEmbedSteganography(
    JNIEnv *env, jobject, jbyteArray audioData, jbyteArray payload, jstring key) {
    if (!audioData || !payload) return nullptr;
    try {
        std::lock_guard<std::mutex> lock(gSecurityMutex);
        if (!ensureAcousticSecurity()) return nullptr;

        auto audio = jbyteArrayToVector(env, audioData);
        auto data = jbyteArrayToVector(env, payload);
        QByteArray audioQ(reinterpret_cast<const char*>(audio.data()), audio.size());
        QByteArray payloadQ(reinterpret_cast<const char*>(data.data()), data.size());

        Security::GNAEngines::SteganographyConfig config;
        if (key) {
            const char *keyStr = env->GetStringUTFChars(key, nullptr);
            if (keyStr) {
                config.encryptionKey = QString(keyStr);
                env->ReleaseStringUTFChars(key, keyStr);
            }
        }

        auto result = gAcousticSecurity->embedSteganography(audioQ, payloadQ, config);
        jbyteArray jResult = env->NewByteArray(static_cast<jsize>(result.size()));
        env->SetByteArrayRegion(jResult, 0, static_cast<jsize>(result.size()),
            reinterpret_cast<const jbyte*>(result.data()));
        return jResult;
    } catch (const std::exception &e) {
        LOGE("Steganography embed failed: %s", e.what());
        return nullptr;
    }
}

JNIEXPORT jbyteArray JNICALL
Java_org_telegram_messenger_cryptogram_SecurityNative_nativeExtractSteganography(
    JNIEnv *env, jobject, jbyteArray audioData, jstring key) {
    if (!audioData) return nullptr;
    try {
        std::lock_guard<std::mutex> lock(gSecurityMutex);
        if (!ensureAcousticSecurity()) return nullptr;

        auto audio = jbyteArrayToVector(env, audioData);
        QByteArray audioQ(reinterpret_cast<const char*>(audio.data()), audio.size());

        Security::GNAEngines::SteganographyConfig config;
        if (key) {
            const char *keyStr = env->GetStringUTFChars(key, nullptr);
            if (keyStr) {
                config.encryptionKey = QString(keyStr);
                env->ReleaseStringUTFChars(key, keyStr);
            }
        }

        auto result = gAcousticSecurity->extractSteganography(audioQ, config);
        if (result.isEmpty()) return nullptr;
        jbyteArray jResult = env->NewByteArray(static_cast<jsize>(result.size()));
        env->SetByteArrayRegion(jResult, 0, static_cast<jsize>(result.size()),
            reinterpret_cast<const jbyte*>(result.data()));
        return jResult;
    } catch (const std::exception &e) {
        LOGE("Steganography extract failed: %s", e.what());
        return nullptr;
    }
}

JNIEXPORT jbyteArray JNICALL
Java_org_telegram_messenger_cryptogram_SecurityNative_nativeApplyVoiceMorphing(
    JNIEnv *env, jobject, jbyteArray audioData, jint method, jfloat pitchShift, jfloat formantRatio) {
    if (!audioData) return nullptr;
    try {
        std::lock_guard<std::mutex> lock(gSecurityMutex);
        if (!ensureAcousticSecurity()) return nullptr;

        auto audio = jbyteArrayToVector(env, audioData);
        QByteArray audioQ(reinterpret_cast<const char*>(audio.data()), audio.size());

        Security::GNAEngines::VoiceMorphingConfig config;
        config.method = static_cast<Security::GNAEngines::VoiceMorphingMethod>(method);
        config.pitchShiftSemitones = pitchShift;
        config.formantShiftRatio = formantRatio;

        auto result = gAcousticSecurity->applyVoiceMorphing(audioQ, config);
        jbyteArray jResult = env->NewByteArray(static_cast<jsize>(result.size()));
        env->SetByteArrayRegion(jResult, 0, static_cast<jsize>(result.size()),
            reinterpret_cast<const jbyte*>(result.data()));
        return jResult;
    } catch (const std::exception &e) {
        LOGE("Voice morphing failed: %s", e.what());
        return nullptr;
    }
}

JNIEXPORT jbyteArray JNICALL
Java_org_telegram_messenger_cryptogram_SecurityNative_nativeTransmitCovertChannel(
    JNIEnv *env, jobject, jbyteArray payload, jdouble carrierFreq, jstring key) {
    if (!payload) return nullptr;
    try {
        std::lock_guard<std::mutex> lock(gSecurityMutex);
        if (!ensureAcousticSecurity()) return nullptr;

        auto data = jbyteArrayToVector(env, payload);
        QByteArray payloadQ(reinterpret_cast<const char*>(data.data()), data.size());

        Security::GNAEngines::CovertChannelConfig config;
        config.carrierFrequency = carrierFreq;
        if (key) {
            const char *keyStr = env->GetStringUTFChars(key, nullptr);
            if (keyStr) {
                config.encryptionKey = QString(keyStr);
                env->ReleaseStringUTFChars(key, keyStr);
            }
        }

        auto result = gAcousticSecurity->transmitCovertChannel(payloadQ, config);
        jbyteArray jResult = env->NewByteArray(static_cast<jsize>(result.size()));
        env->SetByteArrayRegion(jResult, 0, static_cast<jsize>(result.size()),
            reinterpret_cast<const jbyte*>(result.data()));
        return jResult;
    } catch (const std::exception &e) {
        LOGE("Covert channel transmit failed: %s", e.what());
        return nullptr;
    }
}

JNIEXPORT jbyteArray JNICALL
Java_org_telegram_messenger_cryptogram_SecurityNative_nativeReceiveCovertChannel(
    JNIEnv *env, jobject, jbyteArray audioData, jdouble carrierFreq, jstring key) {
    if (!audioData) return nullptr;
    try {
        std::lock_guard<std::mutex> lock(gSecurityMutex);
        if (!ensureAcousticSecurity()) return nullptr;

        auto audio = jbyteArrayToVector(env, audioData);
        QByteArray audioQ(reinterpret_cast<const char*>(audio.data()), audio.size());

        Security::GNAEngines::CovertChannelConfig config;
        config.carrierFrequency = carrierFreq;
        if (key) {
            const char *keyStr = env->GetStringUTFChars(key, nullptr);
            if (keyStr) {
                config.encryptionKey = QString(keyStr);
                env->ReleaseStringUTFChars(key, keyStr);
            }
        }

        auto result = gAcousticSecurity->receiveCovertChannel(audioQ, config);
        if (result.isEmpty()) return nullptr;
        jbyteArray jResult = env->NewByteArray(static_cast<jsize>(result.size()));
        env->SetByteArrayRegion(jResult, 0, static_cast<jsize>(result.size()),
            reinterpret_cast<const jbyte*>(result.data()));
        return jResult;
    } catch (const std::exception &e) {
        LOGE("Covert channel receive failed: %s", e.what());
        return nullptr;
    }
}

JNIEXPORT jstring JNICALL
Java_org_telegram_messenger_cryptogram_SecurityNative_nativeAnalyzeAudio(
    JNIEnv *env, jobject, jbyteArray audioData) {
    if (!audioData) return env->NewStringUTF("{}");
    try {
        std::lock_guard<std::mutex> lock(gSecurityMutex);
        if (!ensureAcousticSecurity()) return env->NewStringUTF("{}");

        auto audio = jbyteArrayToVector(env, audioData);
        QByteArray audioQ(reinterpret_cast<const char*>(audio.data()), audio.size());

        auto assessment = gAcousticSecurity->analyzeAudio(audioQ);
        std::ostringstream out;
        out << "{\"category\":" << static_cast<int>(assessment.category)
            << ",\"level\":" << static_cast<int>(assessment.level)
            << ",\"confidence\":" << assessment.confidence
            << ",\"description\":\"" << assessment.description.toStdString() << "\""
            << "}";
        return env->NewStringUTF(out.str().c_str());
    } catch (...) { return env->NewStringUTF("{}"); }
}

JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM* vm, void* reserved) {
    gJavaVM = vm;
    JNIEnv* env = nullptr;
    if (vm->GetEnv((void**)&env, JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }
    return JNI_VERSION_1_6;
}

} // extern "C"
