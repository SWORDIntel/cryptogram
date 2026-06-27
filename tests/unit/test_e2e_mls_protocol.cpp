/*
CRYPTOGRAM E2E MLS Protocol Tests
Tests MLS (RFC 9420) group operations: TreeKEM key package generation/verification,
group creation, member add/remove, message encryption/decryption, epoch advancement,
and multi-member group messaging.
*/

#include <catch2/catch_test_macros.hpp>

#include "data/data_mls_protocol.h"
#include "data/data_signal_transport.h"
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

static UserId makeUserId(uint64 id) {
	return UserId(id);
}

// ─── Key Package Generation & Verification E2E ─────────────────────────────────

TEST_CASE("E2E: MLS key package generation and verification", "[mls][e2e]") {
	MLSProtocol protocol;

	const auto keyPackage = protocol.generateKeyPackage(
		MLSCiphersuite::MLS_128_DHKEMX25519_AES128GCM_SHA256_Ed25519);

	REQUIRE(keyPackage.isValid());
	REQUIRE_FALSE(keyPackage.initKey.empty());
	REQUIRE_FALSE(keyPackage.credentialPublicKey.empty());
	REQUIRE_FALSE(keyPackage.signature.empty());
	REQUIRE(protocol.verifyKeyPackage(keyPackage));
}

TEST_CASE("E2E: MLS key package tampered signature fails verification", "[mls][e2e]") {
	MLSProtocol protocol;

	const auto keyPackage = protocol.generateKeyPackage(
		MLSCiphersuite::MLS_128_DHKEMX25519_AES128GCM_SHA256_Ed25519);

	REQUIRE(protocol.verifyKeyPackage(keyPackage));

	auto tampered = keyPackage;
	tampered.signature[0] ^= std::byte{0x01};
	REQUIRE_FALSE(protocol.verifyKeyPackage(tampered));
}

TEST_CASE("E2E: MLS key package tampered init key fails verification", "[mls][e2e]") {
	MLSProtocol protocol;

	const auto keyPackage = protocol.generateKeyPackage(
		MLSCiphersuite::MLS_128_DHKEMX25519_CHACHA20POLY1305_SHA256_Ed25519);

	REQUIRE(protocol.verifyKeyPackage(keyPackage));

	auto tampered = keyPackage;
	tampered.initKey[0] ^= std::byte{0x01};
	REQUIRE_FALSE(protocol.verifyKeyPackage(tampered));
}

TEST_CASE("E2E: MLS key packages for different ciphersuites", "[mls][e2e]") {
	MLSProtocol protocol;

	auto pkg1 = protocol.generateKeyPackage(
		MLSCiphersuite::MLS_128_DHKEMX25519_AES128GCM_SHA256_Ed25519);
	auto pkg2 = protocol.generateKeyPackage(
		MLSCiphersuite::MLS_128_DHKEMX25519_CHACHA20POLY1305_SHA256_Ed25519);

	REQUIRE(pkg1.isValid());
	REQUIRE(pkg2.isValid());
	REQUIRE(pkg1.ciphersuite != pkg2.ciphersuite);
	REQUIRE(protocol.verifyKeyPackage(pkg1));
	REQUIRE(protocol.verifyKeyPackage(pkg2));
}

// ─── Group Creation E2E ────────────────────────────────────────────────────────

TEST_CASE("E2E: MLS group creation with 2 members", "[mls][e2e]") {
	MLSProtocol protocol;

	const QVector<UserId> members = {
		makeUserId(101),
		makeUserId(202),
	};

	const auto groupId = protocol.createGroup(
		members,
		MLSCiphersuite::MLS_128_DHKEMX25519_AES128GCM_SHA256_Ed25519);

	REQUIRE_FALSE(groupId.empty());
	REQUIRE(protocol.hasGroup(groupId));

	auto state = protocol.getGroupState(groupId);
	REQUIRE(state.has_value());
	REQUIRE(state->memberCount() == 2);
	REQUIRE(state->isMember(makeUserId(101)));
	REQUIRE(state->isMember(makeUserId(202)));
}

TEST_CASE("E2E: MLS group creation with 5 members", "[mls][e2e]") {
	MLSProtocol protocol;

	const QVector<UserId> members = {
		makeUserId(1001),
		makeUserId(1002),
		makeUserId(1003),
		makeUserId(1004),
		makeUserId(1005),
	};

	const auto groupId = protocol.createGroup(
		members,
		MLSCiphersuite::MLS_128_DHKEMX25519_AES128GCM_SHA256_Ed25519);

	REQUIRE_FALSE(groupId.empty());
	REQUIRE(protocol.hasGroup(groupId));

	auto state = protocol.getGroupState(groupId);
	REQUIRE(state.has_value());
	REQUIRE(state->memberCount() == 5);

	for (auto uid : members) {
		REQUIRE(state->isMember(uid));
	}
}

TEST_CASE("E2E: MLS group creation with different ciphersuites", "[mls][e2e]") {
	MLSProtocol protocol;

	const QVector<UserId> members = {makeUserId(1), makeUserId(2)};

	auto group1 = protocol.createGroup(
		members,
		MLSCiphersuite::MLS_128_DHKEMX25519_AES128GCM_SHA256_Ed25519);
	auto group2 = protocol.createGroup(
		members,
		MLSCiphersuite::MLS_128_DHKEMX25519_CHACHA20POLY1305_SHA256_Ed25519);

	REQUIRE_FALSE(group1.empty());
	REQUIRE_FALSE(group2.empty());
	REQUIRE(group1 != group2);

	auto state1 = protocol.getGroupState(group1);
	auto state2 = protocol.getGroupState(group2);
	REQUIRE(state1->ciphersuite() != state2->ciphersuite());
}

// ─── Group Messaging E2E ───────────────────────────────────────────────────────

TEST_CASE("E2E: MLS group message encrypt/decrypt round-trip", "[mls][e2e]") {
	MLSProtocol protocol;

	const QVector<UserId> members = {makeUserId(101), makeUserId(202)};
	const auto groupId = protocol.createGroup(
		members,
		MLSCiphersuite::MLS_128_DHKEMX25519_AES128GCM_SHA256_Ed25519);

	const bytes::vector plaintext = {
		std::byte{'H'}, std::byte{'e'}, std::byte{'l'}, std::byte{'l'}, std::byte{'o'}
	};

	const auto ciphertext = protocol.encryptMessage(groupId, plaintext);
	REQUIRE_FALSE(ciphertext.empty());

	const auto decrypted = protocol.decryptMessage(groupId, ciphertext);
	REQUIRE(decrypted.has_value());
	REQUIRE(decrypted.value() == plaintext);
}

TEST_CASE("E2E: MLS group message tampered ciphertext fails decryption", "[mls][e2e]") {
	MLSProtocol protocol;

	const QVector<UserId> members = {makeUserId(101), makeUserId(202)};
	const auto groupId = protocol.createGroup(
		members,
		MLSCiphersuite::MLS_128_DHKEMX25519_AES128GCM_SHA256_Ed25519);

	const bytes::vector plaintext = {
		std::byte{'T'}, std::byte{'a'}, std::byte{'m'}, std::byte{'p'}, std::byte{'e'}, std::byte{'r'}
	};

	const auto ciphertext = protocol.encryptMessage(groupId, plaintext);
	REQUIRE_FALSE(ciphertext.empty());

	auto tampered = ciphertext;
	tampered.back() ^= std::byte{0x01};

	const auto result = protocol.decryptMessage(groupId, tampered);
	REQUIRE_FALSE(result.has_value());
}

TEST_CASE("E2E: MLS multiple messages in same epoch", "[mls][e2e]") {
	MLSProtocol protocol;

	const QVector<UserId> members = {makeUserId(1), makeUserId(2)};
	const auto groupId = protocol.createGroup(
		members,
		MLSCiphersuite::MLS_128_DHKEMX25519_AES128GCM_SHA256_Ed25519);

	for (int i = 0; i < 10; i++) {
		bytes::vector plaintext;
		plaintext.push_back(static_cast<std::byte>('A' + i));
		plaintext.push_back(static_cast<std::byte>('0' + (i % 10)));

		auto ciphertext = protocol.encryptMessage(groupId, plaintext);
		REQUIRE_FALSE(ciphertext.empty());

		auto decrypted = protocol.decryptMessage(groupId, ciphertext);
		REQUIRE(decrypted.has_value());
		REQUIRE(decrypted.value() == plaintext);
	}
}

TEST_CASE("E2E: MLS large message encryption", "[mls][e2e]") {
	MLSProtocol protocol;

	const QVector<UserId> members = {makeUserId(1), makeUserId(2)};
	const auto groupId = protocol.createGroup(
		members,
		MLSCiphersuite::MLS_128_DHKEMX25519_AES128GCM_SHA256_Ed25519);

	// 4KB message
	bytes::vector plaintext(4096);
	RAND_bytes(reinterpret_cast<unsigned char*>(plaintext.data()), 4096);

	auto ciphertext = protocol.encryptMessage(groupId, plaintext);
	REQUIRE_FALSE(ciphertext.empty());
	REQUIRE(ciphertext.size() >= plaintext.size());

	auto decrypted = protocol.decryptMessage(groupId, ciphertext);
	REQUIRE(decrypted.has_value());
	REQUIRE(decrypted.value() == plaintext);
}

// ─── Member Operations E2E ─────────────────────────────────────────────────────

TEST_CASE("E2E: MLS add member to group", "[mls][e2e]") {
	MLSProtocol protocol;

	const QVector<UserId> initialMembers = {makeUserId(101), makeUserId(202)};
	const auto groupId = protocol.createGroup(
		initialMembers,
		MLSCiphersuite::MLS_128_DHKEMX25519_AES128GCM_SHA256_Ed25519);

	REQUIRE(protocol.addMember(groupId, makeUserId(303)));

	auto state = protocol.getGroupState(groupId);
	REQUIRE(state.has_value());
	REQUIRE(state->memberCount() == 3);
	REQUIRE(state->isMember(makeUserId(303)));
}

TEST_CASE("E2E: MLS remove member from group", "[mls][e2e]") {
	MLSProtocol protocol;

	const QVector<UserId> members = {makeUserId(101), makeUserId(202), makeUserId(303)};
	const auto groupId = protocol.createGroup(
		members,
		MLSCiphersuite::MLS_128_DHKEMX25519_AES128GCM_SHA256_Ed25519);

	REQUIRE(protocol.removeMember(groupId, makeUserId(202)));

	auto state = protocol.getGroupState(groupId);
	REQUIRE(state.has_value());
	REQUIRE(state->memberCount() == 2);
	REQUIRE_FALSE(state->isMember(makeUserId(202)));
	REQUIRE(state->isMember(makeUserId(101)));
	REQUIRE(state->isMember(makeUserId(303)));
}

TEST_CASE("E2E: MLS add then remove member preserves encryption", "[mls][e2e]") {
	MLSProtocol protocol;

	const QVector<UserId> members = {makeUserId(1), makeUserId(2)};
	const auto groupId = protocol.createGroup(
		members,
		MLSCiphersuite::MLS_128_DHKEMX25519_AES128GCM_SHA256_Ed25519);

	// Add member 3
	REQUIRE(protocol.addMember(groupId, makeUserId(3)));

	// Encrypt after add
	bytes::vector msg1 = {std::byte{'A'}, std::byte{'f'}, std::byte{'t'}, std::byte{'e'}, std::byte{'r'}};
	auto ct1 = protocol.encryptMessage(groupId, msg1);
	auto pt1 = protocol.decryptMessage(groupId, ct1);
	REQUIRE(pt1.has_value());
	REQUIRE(*pt1 == msg1);

	// Remove member 3
	REQUIRE(protocol.removeMember(groupId, makeUserId(3)));

	// Encrypt after remove
	bytes::vector msg2 = {std::byte{'R'}, std::byte{'e'}, std::byte{'m'}, std::byte{'o'}, std::byte{'v'}, std::byte{'e'}};
	auto ct2 = protocol.encryptMessage(groupId, msg2);
	auto pt2 = protocol.decryptMessage(groupId, ct2);
	REQUIRE(pt2.has_value());
	REQUIRE(*pt2 == msg2);
}

TEST_CASE("E2E: MLS add multiple members sequentially", "[mls][e2e]") {
	MLSProtocol protocol;

	const QVector<UserId> initial = {makeUserId(1)};
	const auto groupId = protocol.createGroup(
		initial,
		MLSCiphersuite::MLS_128_DHKEMX25519_AES128GCM_SHA256_Ed25519);

	for (uint64 i = 2; i <= 10; i++) {
		REQUIRE(protocol.addMember(groupId, makeUserId(i)));
	}

	auto state = protocol.getGroupState(groupId);
	REQUIRE(state->memberCount() == 10);

	// Verify all members present
	for (uint64 i = 1; i <= 10; i++) {
		REQUIRE(state->isMember(makeUserId(i)));
	}
}

// ─── Proposal Processing E2E ───────────────────────────────────────────────────

TEST_CASE("E2E: MLS proposal add member via processProposal", "[mls][e2e]") {
	MLSProtocol protocol;

	const QVector<UserId> members = {makeUserId(101), makeUserId(202)};
	const auto groupId = protocol.createGroup(
		members,
		MLSCiphersuite::MLS_128_DHKEMX25519_AES128GCM_SHA256_Ed25519);

	auto state = protocol.getGroupState(groupId);
	REQUIRE(state->memberCount() == 2);

	MLSProposal addProposal;
	addProposal.type = MLSProposalType::Add;
	addProposal.sender = makeUserId(303);
	addProposal.addKeyPackage = protocol.generateKeyPackage(
		MLSCiphersuite::MLS_128_DHKEMX25519_AES128GCM_SHA256_Ed25519);
	addProposal.timestamp = QDateTime::currentDateTime();

	REQUIRE(protocol.processProposal(groupId, addProposal));

	state = protocol.getGroupState(groupId);
	REQUIRE(state->memberCount() == 3);
	REQUIRE(state->isMember(makeUserId(303)));
}

TEST_CASE("E2E: MLS proposal remove member via processProposal", "[mls][e2e]") {
	MLSProtocol protocol;

	const QVector<UserId> members = {makeUserId(101), makeUserId(202), makeUserId(303)};
	const auto groupId = protocol.createGroup(
		members,
		MLSCiphersuite::MLS_128_DHKEMX25519_AES128GCM_SHA256_Ed25519);

	MLSProposal removeProposal;
	removeProposal.type = MLSProposalType::Remove;
	removeProposal.sender = makeUserId(101);
	removeProposal.removeLeaf = MLSLeafIndex(0);
	removeProposal.timestamp = QDateTime::currentDateTime();

	REQUIRE(protocol.processProposal(groupId, removeProposal));

	auto state = protocol.getGroupState(groupId);
	REQUIRE_FALSE(state->isMember(makeUserId(101)));
}

// ─── MLS Transport E2E ─────────────────────────────────────────────────────────

TEST_CASE("E2E: MLS key package transport via zero-width entities", "[mls][e2e][transport]") {
	MLSProtocol protocol;

	auto pkg = protocol.generateKeyPackage(
		MLSCiphersuite::MLS_128_DHKEMX25519_AES128GCM_SHA256_Ed25519);

	QString zwText;
	auto entity = SignalProtocolTransport::buildOutgoingMLSKeyPackage(pkg, zwText);

	REQUIRE(!zwText.isEmpty());
	REQUIRE(entity.type() == mtpc_messageEntityUnknown);

	// All chars should be zero-width
	for (const auto &ch : zwText) {
		const auto cp = ch.unicode();
		REQUIRE((cp == 0x200B || cp == 0x200C || cp == 0x200D || cp == 0xFEFF));
	}

	// Decode back
	const auto rawBytes = SignalProtocolTransport::zeroWidthToBytes(zwText);
	REQUIRE(!rawBytes.isEmpty());

	const auto decoded = SignalProtocolTransport::decodeMLSKeyPackage(rawBytes);
	REQUIRE(decoded.has_value());
	REQUIRE(decoded->initKey == pkg.initKey);
	REQUIRE(decoded->credentialPublicKey == pkg.credentialPublicKey);
	REQUIRE(decoded->ciphersuite == pkg.ciphersuite);
}

TEST_CASE("E2E: MLS welcome transport via zero-width entities", "[mls][e2e][transport]") {
	MLSWelcome welcome;
	welcome.version = kMLSProtocolVersion;
	welcome.ciphersuite = MLSCiphersuite::MLS_128_DHKEMX25519_AES128GCM_SHA256_Ed25519;
	welcome.encryptedGroupSecrets = randomKey(64);
	welcome.encryptedGroupInfo = randomKey(128);

	QString zwText;
	auto entity = SignalProtocolTransport::buildOutgoingMLSWelcome(welcome, zwText);

	REQUIRE(!zwText.isEmpty());

	const auto rawBytes = SignalProtocolTransport::zeroWidthToBytes(zwText);
	REQUIRE(!rawBytes.isEmpty());

	const auto decoded = SignalProtocolTransport::decodeMLSWelcome(rawBytes);
	REQUIRE(decoded.has_value());
	REQUIRE(decoded->encryptedGroupSecrets == welcome.encryptedGroupSecrets);
	REQUIRE(decoded->encryptedGroupInfo == welcome.encryptedGroupInfo);
}

// ─── Epoch Advancement E2E ─────────────────────────────────────────────────────

TEST_CASE("E2E: MLS epoch advances after member operations", "[mls][e2e]") {
	MLSProtocol protocol;

	const QVector<UserId> members = {makeUserId(1), makeUserId(2)};
	const auto groupId = protocol.createGroup(
		members,
		MLSCiphersuite::MLS_128_DHKEMX25519_AES128GCM_SHA256_Ed25519);

	auto stateBefore = protocol.getGroupState(groupId);
	auto epochBefore = stateBefore->epoch();

	// Add a member (should trigger epoch advancement)
	protocol.addMember(groupId, makeUserId(3));

	auto stateAfter = protocol.getGroupState(groupId);
	REQUIRE(stateAfter->epoch() > epochBefore);
}

TEST_CASE("E2E: MLS encryption works across epoch boundaries", "[mls][e2e]") {
	MLSProtocol protocol;

	const QVector<UserId> members = {makeUserId(1), makeUserId(2)};
	const auto groupId = protocol.createGroup(
		members,
		MLSCiphersuite::MLS_128_DHKEMX25519_AES128GCM_SHA256_Ed25519);

	// Message in epoch 0
	bytes::vector msg0 = {std::byte{'E'}, std::byte{'0'}};
	auto ct0 = protocol.encryptMessage(groupId, msg0);
	auto pt0 = protocol.decryptMessage(groupId, ct0);
	REQUIRE(pt0.has_value());
	REQUIRE(*pt0 == msg0);

	// Add member -> epoch advances
	protocol.addMember(groupId, makeUserId(3));

	// Message in epoch 1
	bytes::vector msg1 = {std::byte{'E'}, std::byte{'1'}};
	auto ct1 = protocol.encryptMessage(groupId, msg1);
	auto pt1 = protocol.decryptMessage(groupId, ct1);
	REQUIRE(pt1.has_value());
	REQUIRE(*pt1 == msg1);

	// Remove member -> epoch advances again
	protocol.removeMember(groupId, makeUserId(3));

	// Message in epoch 2
	bytes::vector msg2 = {std::byte{'E'}, std::byte{'2'}};
	auto ct2 = protocol.encryptMessage(groupId, msg2);
	auto pt2 = protocol.decryptMessage(groupId, ct2);
	REQUIRE(pt2.has_value());
	REQUIRE(*pt2 == msg2);
}

// ─── Group State Queries E2E ───────────────────────────────────────────────────

TEST_CASE("E2E: MLS group state returns nullopt for unknown group", "[mls][e2e]") {
	MLSProtocol protocol;

	auto state = protocol.getGroupState({std::byte{0xFF}, std::byte{0xFF}});
	REQUIRE_FALSE(state.has_value());
}

TEST_CASE("E2E: MLS hasGroup returns false for unknown group", "[mls][e2e]") {
	MLSProtocol protocol;

	REQUIRE_FALSE(protocol.hasGroup({std::byte{0x00}, std::byte{0x01}}));
}

TEST_CASE("E2E: MLS group context has valid ciphersuite", "[mls][e2e]") {
	MLSProtocol protocol;

	const QVector<UserId> members = {makeUserId(1), makeUserId(2)};
	const auto groupId = protocol.createGroup(
		members,
		MLSCiphersuite::MLS_128_DHKEMX25519_CHACHA20POLY1305_SHA256_Ed25519);

	auto state = protocol.getGroupState(groupId);
	REQUIRE(state->ciphersuite() == MLSCiphersuite::MLS_128_DHKEMX25519_CHACHA20POLY1305_SHA256_Ed25519);
}

// ─── Zero-Width Transport Edge Cases E2E ───────────────────────────────────────

TEST_CASE("E2E: Zero-width transport handles empty payload", "[mls][e2e][transport]") {
	const QByteArray empty;
	const auto zw = SignalProtocolTransport::bytesToZeroWidth(empty);
	REQUIRE(zw.isEmpty());

	const auto decoded = SignalProtocolTransport::zeroWidthToBytes(zw);
	REQUIRE(decoded.isEmpty());
}

TEST_CASE("E2E: Zero-width transport handles binary data with null bytes", "[mls][e2e][transport]") {
	const QByteArray binaryData("\x00\xFF\x42\x00\xAB\xCD", 6);
	const auto zw = SignalProtocolTransport::bytesToZeroWidth(binaryData);
	REQUIRE(zw.size() == binaryData.size() * 4);

	const auto decoded = SignalProtocolTransport::zeroWidthToBytes(zw);
	REQUIRE(decoded == binaryData);
}

TEST_CASE("E2E: Zero-width transport rejects mixed content", "[mls][e2e][transport]") {
	// Mix of ZW chars and regular text
	QString mixed = QString(QChar(0x200B)) + "X" + QString(QChar(0x200C));
	REQUIRE(SignalProtocolTransport::zeroWidthToBytes(mixed).isEmpty());
}
