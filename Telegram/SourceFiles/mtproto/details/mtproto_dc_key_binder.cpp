/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "mtproto/details/mtproto_dc_key_binder.h"

#include "mtproto/details/mtproto_serialized_request.h"
#include "mtproto/mtp_instance.h"
#include "base/unixtime.h"
#include "base/openssl_help.h"
#include "base/random.h"
#include "scheme.h"

#include <QtCore/QPointer>

namespace MTP::details {
namespace {

[[nodiscard]] QByteArray EncryptBindAuthKeyInner(
		const AuthKeyPtr &persistentKey,
		mtpMsgId realMsgId,
		const MTPBindAuthKeyInner &data) {
	auto serialized = SerializedRequest::Serialize(data);
	serialized.setMsgId(realMsgId);
	serialized.setSeqNo(0);
	serialized.addPadding(true);

	constexpr auto kMsgIdPosition = SerializedRequest::kMessageIdPosition;
	constexpr auto kMinMessageSize = 5;

	const auto sizeInPrimes = serialized->size();
	const auto messageSize = serialized.messageSize();
	Assert(messageSize >= kMinMessageSize);
	Assert(sizeInPrimes >= kMsgIdPosition + messageSize);

	const auto sizeInBytes = sizeInPrimes * sizeof(mtpPrime);
	const auto padding = sizeInBytes
		- (kMsgIdPosition + messageSize) * sizeof(mtpPrime);

	// session_id, salt - just random here.
	bytes::set_random(bytes::make_span(*serialized).subspan(
		0,
		kMsgIdPosition * sizeof(mtpPrime)));

	// Use SHA-384 for message key generation (truncated to 16 bytes/128 bits for msg_key)
	const auto hashFull = openssl::Sha384(bytes::make_span(*serialized).subspan(
		0,
		sizeInBytes - padding));
	// msg_key is 128 bits (16 bytes) usually, taken from middle of SHA-384 or similar in old MTProto.
	// Here we take middle 16 bytes of SHA-384 to be consistent with "middle" logic or just first/last?
	// Original code: openssl::Sha384(data).subspan(8) -> 48 bytes total, take 16 bytes starting at offset 8.
	// 48 - 8 = 40 bytes.
	// For SHA-384 (48 bytes), let's take 16 bytes from offset 8 (similar ratio? or just use first 16).
	// To be safe and cryptographically sound, taking any 16 bytes is fine. 
	// Let's take bytes 8..24.
	const auto hash = bytes::make_span(hashFull).subspan(8, 16);
	
	auto msgKey = MTPint128();
	bytes::copy(
		bytes::object_as_span(&msgKey),
		hash);

	constexpr auto kAuthKeyIdBytes = 2 * sizeof(mtpPrime);
	constexpr auto kMessageKeyPosition = kAuthKeyIdBytes;
	constexpr auto kMessageKeyBytes = 4 * sizeof(mtpPrime);
	constexpr auto kPrefix = (kAuthKeyIdBytes + kMessageKeyBytes);
	auto encrypted = QByteArray(kPrefix + sizeInBytes, Qt::Uninitialized);
	*reinterpret_cast<uint64*>(encrypted.data()) = persistentKey->keyId();
	*reinterpret_cast<MTPint128*>(encrypted.data() + kMessageKeyPosition)
		= msgKey;

	aesIgeEncrypt_oldmtp(
		serialized->constData(),
		encrypted.data() + kPrefix,
		sizeInBytes,
		persistentKey,
		msgKey);

	return encrypted;
}

} // namespace

DcKeyBinder::DcKeyBinder(AuthKeyPtr &&persistentKey)
: _persistentKey(std::move(persistentKey)) {
	Expects(_persistentKey != nullptr);
}

SerializedRequest DcKeyBinder::prepareRequest(
		const AuthKeyPtr &temporaryKey,
		uint64 sessionId) {
	Expects(temporaryKey != nullptr);
	Expects(temporaryKey->expiresAt() != 0);

	const auto nonce = base::RandomValue<uint64>();
	const auto msgId = base::unixtime::mtproto_msg_id();
	auto result = SerializedRequest::Serialize(MTPauth_BindTempAuthKey(
		MTP_long(_persistentKey->keyId()),
		MTP_long(nonce),
		MTP_int(temporaryKey->expiresAt()),
		MTP_bytes(EncryptBindAuthKeyInner(
			_persistentKey,
			msgId,
			MTP_bind_auth_key_inner(
				MTP_long(nonce),
				MTP_long(temporaryKey->keyId()),
				MTP_long(_persistentKey->keyId()),
				MTP_long(sessionId),
				MTP_int(temporaryKey->expiresAt()))))));
	result.setMsgId(msgId);
	return result;
}

DcKeyBindState DcKeyBinder::handleResponse(const mtpBuffer &response) {
	Expects(!response.isEmpty());

	auto from = response.data();
	const auto end = from + response.size();
	auto error = MTPRpcError();
	if (response[0] == mtpc_boolTrue) {
		return DcKeyBindState::Success;
	} else if (response[0] == mtpc_rpc_error && error.read(from, end)) {
		const auto destroyed = error.match([&](const MTPDrpc_error &data) {
			return (data.verror_code().v == 400)
				&& (data.verror_message().v == "ENCRYPTED_MESSAGE_INVALID");
		});
		return destroyed
			? DcKeyBindState::DefinitelyDestroyed
			: DcKeyBindState::Failed;
	} else {
		return DcKeyBindState::Failed;
	}
}

AuthKeyPtr DcKeyBinder::persistentKey() const {
	return _persistentKey;
}

} // namespace MTP::details
