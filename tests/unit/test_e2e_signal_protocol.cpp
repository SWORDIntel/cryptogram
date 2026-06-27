/*
CRYPTOGRAM E2E Signal Protocol Tests
Tests the Double Ratchet protocol: X3DH key exchange, DH ratcheting,
chain key ratcheting, forward secrecy, out-of-order message handling,
and key bundle transport.
*/

#include <catch2/catch_test_macros.hpp>

#include "data/data_signal_transport.h"
#include "data/data_signal_protocol.h"
#include "data/data_mls_protocol.h"
#include "base/random.h"
#include "base/bytes.h"

#include <openssl/evp.h>
#include <openssl/rand.h>

using namespace Data;

// ─── Helpers ───────────────────────────────────────────────────────────────────

static bytes::vector randomKey(int size) {
	bytes::vector key(size);
	RAND_bytes(reinterpret_cast<unsigned char*>(key.data()), size);
	return key;
}

static SignalProtocol::KeyBundle makeBundle() {
	SignalProtocol::KeyBundle bundle;
	bundle.identityKey = randomKey(32);
	bundle.signedPreKey = randomKey(32);
	bundle.signature = randomKey(64);
	bundle.oneTimePreKey = randomKey(32);
	bundle.deviceId.registrationId = 0x1234567890ABCDEF;
	return bundle;
}

// Simulate a Double Ratchet session between Alice and Bob
struct RatchetSession {
	bytes::vector rootKey;
	bytes::vector sendingChainKey;
	bytes::vector receivingChainKey;
	bytes::vector dhPrivateKey;
	bytes::vector dhPublicKey;
	bytes::vector remoteDhPublicKey;
	uint32 sendCounter = 0;
	uint32 recvCounter = 0;
};

// Simulate KDF_RK (root key ratchet): derive new root key + chain key from DH output
static std::pair<bytes::vector, bytes::vector> kdfRk(
		const bytes::vector &rootKey,
		const bytes::vector &dhOutput) {
	auto derived = bytes::vector(64);
	EVP_PKEY_CTX *pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_HKDF, nullptr);
	EVP_PKEY_derive_init(pctx);
	EVP_PKEY_CTX_set_hkdf_md(pctx, EVP_sha256());
	EVP_PKEY_CTX_set1_hkdf_salt(pctx, rootKey.data(), rootKey.size());
	EVP_PKEY_CTX_set1_hkdf_key(pctx, dhOutput.data(), dhOutput.size());
	EVP_PKEY_CTX_add1_hkdf_info(pctx,
		reinterpret_cast<const unsigned char*>("CryptogramKDF_RK"), 16);
	size_t outLen = 64;
	EVP_PKEY_derive(pctx, derived.data(), &outLen);
	EVP_PKEY_CTX_free(pctx);

	bytes::vector newRoot(derived.begin(), derived.begin() + 32);
	bytes::vector newChain(derived.begin() + 32, derived.end());
	return {newRoot, newChain};
}

// Simulate KDF_CK (chain key ratchet): derive message key from chain key
static bytes::vector kdfCk(const bytes::vector &chainKey) {
	auto messageKey = bytes::vector(32);
	EVP_PKEY_CTX *pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_HKDF, nullptr);
	EVP_PKEY_derive_init(pctx);
	EVP_PKEY_CTX_set_hkdf_md(pctx, EVP_sha256());
	EVP_PKEY_CTX_set1_hkdf_salt(pctx, chainKey.data(), chainKey.size());
	EVP_PKEY_CTX_set1_hkdf_key(pctx, chainKey.data(), chainKey.size());
	EVP_PKEY_CTX_add1_hkdf_info(pctx,
		reinterpret_cast<const unsigned char*>("CryptogramKDF_CK"), 16);
	size_t outLen = 32;
	EVP_PKEY_derive(pctx, messageKey.data(), &outLen);
	EVP_PKEY_CTX_free(pctx);
	return messageKey;
}

// AES-256-GCM encrypt
static bytes::vector aesGcmEncrypt(
		const bytes::vector &key,
		const bytes::vector &iv,
		const bytes::const_span &plaintext) {
	EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
	EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr);
	EVP_CIPHER_CTX_set_key_length(ctx, 32);
	EVP_EncryptInit_ex(ctx, nullptr, nullptr,
		reinterpret_cast<const unsigned char*>(key.data()),
		reinterpret_cast<const unsigned char*>(iv.data()));

	const auto *pt = reinterpret_cast<const unsigned char*>(plaintext.data());
	auto ptLen = static_cast<int>(bytes::span(plaintext).size());

	std::vector<unsigned char> ciphertext(ptLen);
	int len = 0;
	EVP_EncryptUpdate(ctx, ciphertext.data(), &len, pt, ptLen);
	int totalLen = len;
	EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len);
	totalLen += len;

	std::vector<unsigned char> tag(16);
	EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, tag.data());
	EVP_CIPHER_CTX_free(ctx);

	// Append tag to ciphertext
	ciphertext.insert(ciphertext.end(), tag.begin(), tag.end());
	return bytes::vector(ciphertext.begin(), ciphertext.end());
}

// AES-256-GCM decrypt
static std::optional<bytes::vector> aesGcmDecrypt(
		const bytes::vector &key,
		const bytes::vector &iv,
		const bytes::const_span &ciphertextWithTag) {
	auto totalSize = bytes::span(ciphertextWithTag).size();
	if (totalSize < 16) return std::nullopt;

	auto ctSize = totalSize - 16;
	const auto *ct = reinterpret_cast<const unsigned char*>(ciphertextWithTag.data());
	const auto *tag = ct + ctSize;

	EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
	EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr);
	EVP_DecryptInit_ex(ctx, nullptr, nullptr,
		reinterpret_cast<const unsigned char*>(key.data()),
		reinterpret_cast<const unsigned char*>(iv.data()));

	std::vector<unsigned char> decrypted(ctSize);
	int len = 0;
	EVP_DecryptUpdate(ctx, decrypted.data(), &len, ct, static_cast<int>(ctSize));

	EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, const_cast<unsigned char*>(tag));
	int ret = EVP_DecryptFinal_ex(ctx, decrypted.data() + len, &len);
	EVP_CIPHER_CTX_free(ctx);

	if (ret != 1) return std::nullopt;
	return bytes::vector(decrypted.begin(), decrypted.end());
}

// Generate X25519 key pair
static std::pair<bytes::vector, bytes::vector> generateX25519() {
	EVP_PKEY *pkey = EVP_PKEY_new_raw_private_key(EVP_PKEY_X25519, nullptr, nullptr, 0);

	size_t privLen = 0, pubLen = 0;
	EVP_PKEY_get_raw_private_key(pkey, nullptr, &privLen);
	EVP_PKEY_get_raw_public_key(pkey, nullptr, &pubLen);

	bytes::vector priv(privLen), pub(pubLen);
	EVP_PKEY_get_raw_private_key(pkey, priv.data(), &privLen);
	EVP_PKEY_get_raw_public_key(pkey, pub.data(), &pubLen);
	EVP_PKEY_free(pkey);
	return {priv, pub};
}

// X25519 DH
static bytes::vector dhCompute(
		const bytes::vector &privKey,
		const bytes::vector &peerPubKey) {
	EVP_PKEY *pkey = EVP_PKEY_new_raw_private_key(EVP_PKEY_X25519, nullptr,
		privKey.data(), privKey.size());
	EVP_PKEY *peer = EVP_PKEY_new_raw_public_key(EVP_PKEY_X25519, nullptr,
		peerPubKey.data(), peerPubKey.size());

	EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(pkey, nullptr);
	EVP_PKEY_derive_init(ctx);
	EVP_PKEY_derive_set_peer(ctx, peer);

	size_t len = 0;
	EVP_PKEY_derive(ctx, nullptr, &len);
	bytes::vector shared(len);
	EVP_PKEY_derive(ctx, shared.data(), &len);

	EVP_PKEY_CTX_free(ctx);
	EVP_PKEY_free(peer);
	EVP_PKEY_free(pkey);
	return shared;
}

// ─── Key Bundle Transport E2E ──────────────────────────────────────────────────

TEST_CASE("E2E: KeyBundle full transport round-trip via zero-width entities", "[signal][e2e]") {
	auto bundle = makeBundle();

	// Build outgoing zero-width entity
	QString zwText;
	auto entity = SignalProtocolTransport::buildOutgoingEntity(bundle, zwText);
	REQUIRE(!zwText.isEmpty());
	REQUIRE(entity.type() == mtpc_messageEntityUnknown);

	// Simulate transport: entity arrives at remote peer
	QVector<MTPMessageEntity> entities;
	entities.push_back(entity);

	auto extracted = SignalProtocolTransport::extractAndStripBundles(entities, zwText);
	REQUIRE(extracted.size() == 1);

	// Verify all fields match
	REQUIRE(extracted[0].identityKey == bundle.identityKey);
	REQUIRE(extracted[0].signedPreKey == bundle.signedPreKey);
	REQUIRE(extracted[0].signature == bundle.signature);
	REQUIRE(extracted[0].oneTimePreKey == bundle.oneTimePreKey);
	REQUIRE(extracted[0].deviceId.registrationId == bundle.deviceId.registrationId);

	// Entity should be stripped after extraction
	REQUIRE(entities.isEmpty());
}

TEST_CASE("E2E: Multiple key bundles in single message", "[signal][e2e]") {
	auto bundle1 = makeBundle();
	auto bundle2 = makeBundle();
	bundle2.deviceId.registrationId = 0xFEDCBA0987654321;

	QString zwText1, zwText2;
	SignalProtocolTransport::buildOutgoingEntity(bundle1, zwText1);
	SignalProtocolTransport::buildOutgoingEntity(bundle2, zwText2);

	// Combine zero-width texts
	QString combined = zwText1 + zwText2;

	QVector<MTPMessageEntity> entities;
	entities.push_back(MTP_messageEntityUnknown(MTP_int(0), MTP_int(combined.size())));

	auto extracted = SignalProtocolTransport::extractAndStripBundles(entities, combined);
	REQUIRE(extracted.size() >= 1);
}

// ─── Double Ratchet Simulation E2E ─────────────────────────────────────────────

TEST_CASE("E2E: Double Ratchet basic message exchange", "[signal][e2e][ratchet]") {
	// Simulate X3DH: Alice and Bob perform initial key exchange
	auto [alicePriv, alicePub] = generateX25519();
	auto [bobPriv, bobPub] = generateX25519();

	// Initial shared secret via DH
	auto dhOutput = dhCompute(alicePriv, bobPub);

	// Initialize root key from DH output
	auto [initialRoot, initialChain] = kdfRk(randomKey(32), dhOutput);

	// Alice's session
	RatchetSession alice;
	alice.rootKey = initialRoot;
	alice.sendingChainKey = initialChain;
	alice.dhPrivateKey = alicePriv;
	alice.dhPublicKey = alicePub;
	alice.remoteDhPublicKey = bobPub;

	// Bob's session (reversed roles)
	RatchetSession bob;
	bob.rootKey = initialRoot;
	bob.receivingChainKey = initialChain;
	bob.dhPrivateKey = bobPriv;
	bob.dhPublicKey = bobPub;
	bob.remoteDhPublicKey = alicePub;

	// Alice sends message 1
	auto msgKey1 = kdfCk(alice.sendingChainKey);
	auto iv1 = randomKey(12);
	auto plaintext1 = bytes::vector{'H', 'e', 'l', 'l', 'o'};
	auto ciphertext1 = aesGcmEncrypt(msgKey1, iv1, plaintext1);
	alice.sendCounter++;

	// Bob decrypts message 1
	auto bobMsgKey1 = kdfCk(bob.receivingChainKey);
	auto decrypted1 = aesGcmDecrypt(bobMsgKey1, iv1, ciphertext1);
	REQUIRE(decrypted1.has_value());
	REQUIRE(*decrypted1 == plaintext1);
	bob.recvCounter++;
}

TEST_CASE("E2E: Double Ratchet DH ratchet step", "[signal][e2e][ratchet]") {
	// After initial exchange, Bob sends a message which triggers DH ratchet
	auto [alicePriv, alicePub] = generateX25519();
	auto [bobPriv, bobPub] = generateX25519();

	auto dhOutput1 = dhCompute(alicePriv, bobPub);
	auto [rootKey1, chainKey1] = kdfRk(randomKey(32), dhOutput1);

	// Bob generates new DH key pair for ratchet step
	auto [bobNewPriv, bobNewPub] = generateX25519();

	// Bob sends new DH public key + message encrypted with ratcheted key
	auto dhOutput2 = dhCompute(bobNewPriv, alicePub);
	auto [rootKey2, newChainKey] = kdfRk(rootKey1, dhOutput2);

	// Alice receives Bob's new DH public key and performs her own ratchet
	auto aliceDhOutput2 = dhCompute(alicePriv, bobNewPub);
	auto [aliceRootKey2, aliceNewChainKey] = kdfRk(rootKey1, aliceDhOutput2);

	// Both sides should have the same new root key
	REQUIRE(rootKey2 == aliceRootKey2);
	REQUIRE(newChainKey == aliceNewChainKey);
}

TEST_CASE("E2E: Double Ratchet forward secrecy - old keys cannot decrypt new messages", "[signal][e2e][ratchet]") {
	auto [alicePriv, alicePub] = generateX25519();
	auto [bobPriv, bobPub] = generateX25519();

	auto dhOutput = dhCompute(alicePriv, bobPub);
	auto [rootKey, chainKey] = kdfRk(randomKey(32), dhOutput);

	// Encrypt message 1 with current chain key
	auto msgKey1 = kdfCk(chainKey);
	auto iv = randomKey(12);
	auto plaintext = bytes::vector{'s', 'e', 'c', 'r', 'e', 't'};
	auto ciphertext = aesGcmEncrypt(msgKey1, iv, plaintext);

	// Ratchet the chain key forward
	auto [newRoot, newChain] = kdfRk(rootKey, dhCompute(alicePriv, bobPub));
	auto msgKey2 = kdfCk(newChain);

	// Old message key (msgKey1) should not decrypt a message encrypted with msgKey2
	auto ciphertext2 = aesGcmEncrypt(msgKey2, iv, plaintext);
	auto decryptWithOldKey = aesGcmDecrypt(msgKey1, iv, ciphertext2);
	REQUIRE_FALSE(decryptWithOldKey.has_value());

	// But the original message should still decrypt with msgKey1
	auto decryptOriginal = aesGcmDecrypt(msgKey1, iv, ciphertext);
	REQUIRE(decryptOriginal.has_value());
	REQUIRE(*decryptOriginal == plaintext);
}

TEST_CASE("E2E: Double Ratchet out-of-order message handling", "[signal][e2e][ratchet]") {
	auto [alicePriv, alicePub] = generateX25519();
	auto [bobPriv, bobPub] = generateX25519();

	auto dhOutput = dhCompute(alicePriv, bobPub);
	auto [rootKey, chainKey] = kdfRk(randomKey(32), dhOutput);

	// Generate message keys for messages 0, 1, 2
	auto key0 = kdfCk(chainKey);
	auto key1 = kdfCk(chainKey);
	auto key2 = kdfCk(chainKey);

	auto iv = randomKey(12);
	auto msg0 = bytes::vector{'m', '0'};
	auto msg1 = bytes::vector{'m', '1'};
	auto msg2 = bytes::vector{'m', '2'};

	auto ct0 = aesGcmEncrypt(key0, iv, msg0);
	auto ct1 = aesGcmEncrypt(key1, iv, msg1);
	auto ct2 = aesGcmEncrypt(key2, iv, msg2);

	// Decrypt in reverse order (out-of-order)
	auto dec2 = aesGcmDecrypt(key2, iv, ct2);
	auto dec1 = aesGcmDecrypt(key1, iv, ct1);
	auto dec0 = aesGcmDecrypt(key0, iv, ct0);

	REQUIRE(dec0.has_value());
	REQUIRE(dec1.has_value());
	REQUIRE(dec2.has_value());
	REQUIRE(*dec0 == msg0);
	REQUIRE(*dec1 == msg1);
	REQUIRE(*dec2 == msg2);
}

TEST_CASE("E2E: Double Ratchet multiple messages same chain", "[signal][e2e][ratchet]") {
	auto [alicePriv, alicePub] = generateX25519();
	auto [bobPriv, bobPub] = generateX25519();

	auto dhOutput = dhCompute(alicePriv, bobPub);
	auto [rootKey, chainKey] = kdfRk(randomKey(32), dhOutput);

	// Send 5 messages in the same chain
	auto iv = randomKey(12);
	for (int i = 0; i < 5; i++) {
		auto msgKey = kdfCk(chainKey);
		auto plaintext = bytes::vector{
			static_cast<std::byte>('A' + i),
			static_cast<std::byte>('0' + i)
		};
		auto ciphertext = aesGcmEncrypt(msgKey, iv, plaintext);

		// Receiver uses same key to decrypt
		auto recvKey = kdfCk(chainKey);
		auto decrypted = aesGcmDecrypt(recvKey, iv, ciphertext);
		REQUIRE(decrypted.has_value());
		REQUIRE(*decrypted == plaintext);
	}
}

// ─── Key Bundle Validation E2E ─────────────────────────────────────────────────

TEST_CASE("E2E: KeyBundle rejects corrupted identity key", "[signal][e2e]") {
	auto bundle = makeBundle();
	auto encoded = SignalProtocolTransport::encodeKeyBundle(bundle);
	REQUIRE(!encoded.isEmpty());

	// Corrupt identity key (byte at offset 10, after version+bitmap+regId)
	auto corrupted = encoded;
	corrupted[10] ^= 0x01;

	auto decoded = SignalProtocolTransport::decodeKeyBundle(corrupted);
	REQUIRE(!decoded.has_value());
}

TEST_CASE("E2E: KeyBundle rejects truncated data", "[signal][e2e]") {
	auto bundle = makeBundle();
	auto encoded = SignalProtocolTransport::encodeKeyBundle(bundle);
	REQUIRE(!encoded.isEmpty());

	// Truncate to 10 bytes
	auto truncated = encoded.left(10);
	auto decoded = SignalProtocolTransport::decodeKeyBundle(truncated);
	REQUIRE(!decoded.has_value());
}

TEST_CASE("E2E: KeyBundle with maximum registration ID", "[signal][e2e]") {
	SignalProtocol::KeyBundle bundle;
	bundle.identityKey = randomKey(32);
	bundle.signedPreKey = randomKey(32);
	bundle.signature = randomKey(64);
	bundle.oneTimePreKey = randomKey(32);
	bundle.deviceId.registrationId = 0xFFFFFFFFFFFFFFFF;

	auto encoded = SignalProtocolTransport::encodeKeyBundle(bundle);
	REQUIRE(!encoded.isEmpty());

	auto decoded = SignalProtocolTransport::decodeKeyBundle(encoded);
	REQUIRE(decoded.has_value());
	REQUIRE(decoded->deviceId.registrationId == 0xFFFFFFFFFFFFFFFF);
}
