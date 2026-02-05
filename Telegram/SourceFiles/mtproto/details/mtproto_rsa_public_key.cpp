/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "mtproto/details/mtproto_rsa_public_key.h"

#include "base/openssl_help.h"
#include "core/utils.h"

#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/err.h>

namespace MTP::details {
namespace {

struct BIODeleter {
	void operator()(BIO *value) {
		BIO_free(value);
	}
};

EVP_PKEY *CreateRaw(bytes::const_span key) {
	const auto bio = std::unique_ptr<BIO, BIODeleter>{
		BIO_new_mem_buf(
			const_cast<gsl::byte*>(key.data()),
			key.size()),
	};
	return PEM_read_bio_PUBKEY(bio.get(), nullptr, nullptr, nullptr);
}

} // namespace

class RSAPublicKey::Private {
public:
	explicit Private(bytes::const_span key);
	Private(bytes::const_span nBytes, bytes::const_span eBytes);
	~Private();

	[[nodiscard]] bool valid() const;
	[[nodiscard]] uint64 fingerprint() const;
	[[nodiscard]] bytes::vector getN() const;
	[[nodiscard]] bytes::vector getE() const;
	[[nodiscard]] bytes::vector encrypt(bytes::const_span data) const;
	[[nodiscard]] bytes::vector decrypt(bytes::const_span data) const;
	[[nodiscard]] bytes::vector encryptOAEPpadding(
		bytes::const_span data) const;

private:
	void computeFingerprint();
	[[nodiscard]] static bytes::vector ToBytes(const BIGNUM *number);

	EVP_PKEY *_pkey = nullptr;
	uint64 _fingerprint = 0;

};

RSAPublicKey::Private::Private(bytes::const_span key)
	: _pkey(CreateRaw(key)) {
	if (_pkey) {
		computeFingerprint();
	}
}

RSAPublicKey::Private::Private(bytes::const_span nBytes, bytes::const_span eBytes)
	: _pkey(nullptr) {
	// For ECC, we expect the full key in nBytes (SPKI format). eBytes is ignored.
	// If it was legacy RSA usage, this constructor is deprecated/unsupported for RSA composition.
	if (!nBytes.empty()) {
		const unsigned char *p = reinterpret_cast<const unsigned char*>(nBytes.data());
		_pkey = d2i_PUBKEY(nullptr, &p, nBytes.size());
		if (_pkey) {
			computeFingerprint();
		}
	}
}

bool RSAPublicKey::Private::valid() const {
	return _pkey != nullptr;
}

uint64 RSAPublicKey::Private::fingerprint() const {
	return _fingerprint;
}

bytes::vector RSAPublicKey::Private::getN() const {
	Expects(valid());
	// Return full key in N for ECC serialization support
	int len = i2d_PUBKEY(_pkey, nullptr);
	if (len > 0) {
		auto result = bytes::vector(len);
		unsigned char *p = reinterpret_cast<unsigned char*>(result.data());
		i2d_PUBKEY(_pkey, &p);
		return result;
	}
	return {};
}

bytes::vector RSAPublicKey::Private::getE() const {
	Expects(valid());
	return {}; // Empty for ECC
}

bytes::vector RSAPublicKey::Private::encrypt(bytes::const_span data) const {
	Expects(valid());

	EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(_pkey, nullptr);
	if (!ctx) return {};
	if (EVP_PKEY_encrypt_init(ctx) <= 0) {
		EVP_PKEY_CTX_free(ctx);
		return {};
	}

	size_t outlen = 0;
	if (EVP_PKEY_encrypt(ctx, nullptr, &outlen, reinterpret_cast<const unsigned char*>(data.data()), data.size()) <= 0) {
		EVP_PKEY_CTX_free(ctx);
		return {};
	}

	auto result = bytes::vector(outlen);
	if (EVP_PKEY_encrypt(ctx, reinterpret_cast<unsigned char*>(result.data()), &outlen, reinterpret_cast<const unsigned char*>(data.data()), data.size()) <= 0) {
		EVP_PKEY_CTX_free(ctx);
		return {};
	}
	
	EVP_PKEY_CTX_free(ctx);
	result.resize(outlen);
	return result;
}

bytes::vector RSAPublicKey::Private::decrypt(bytes::const_span data) const {
	Expects(valid());

	EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(_pkey, nullptr);
	if (!ctx) return {};
	if (EVP_PKEY_decrypt_init(ctx) <= 0) {
		EVP_PKEY_CTX_free(ctx);
		return {};
	}

	size_t outlen = 0;
	if (EVP_PKEY_decrypt(ctx, nullptr, &outlen, reinterpret_cast<const unsigned char*>(data.data()), data.size()) <= 0) {
		EVP_PKEY_CTX_free(ctx);
		return {};
	}

	auto result = bytes::vector(outlen);
	if (EVP_PKEY_decrypt(ctx, reinterpret_cast<unsigned char*>(result.data()), &outlen, reinterpret_cast<const unsigned char*>(data.data()), data.size()) <= 0) {
		EVP_PKEY_CTX_free(ctx);
		return {};
	}

	EVP_PKEY_CTX_free(ctx);
	result.resize(outlen);
	return result;
}

bytes::vector RSAPublicKey::Private::encryptOAEPpadding(bytes::const_span data) const {
	Expects(valid());
	// Fallback to standard encrypt for ECC, OAEP is RSA specific
	return encrypt(data);
}

RSAPublicKey::Private::~Private() {
	EVP_PKEY_free(_pkey);
}

void RSAPublicKey::Private::computeFingerprint() {
	Expects(valid());

	unsigned char *buf = nullptr;
	int len = i2d_PUBKEY(_pkey, &buf);
	if (len > 0 && buf) {
		auto sha384 = hashSha384(buf, len);
		// Use the last 8 bytes as fingerprint
		_fingerprint = *(uint64*)(sha384.data() + 40); 
		OPENSSL_free(buf);
	}
}

bytes::vector RSAPublicKey::Private::ToBytes(const BIGNUM *number) {
	return {};
}

RSAPublicKey::RSAPublicKey(bytes::const_span key)
: _private(std::make_shared<Private>(key)) {
}

RSAPublicKey::RSAPublicKey(
	bytes::const_span nBytes,
	bytes::const_span eBytes)
: _private(std::make_shared<Private>(nBytes, eBytes)) {
}

bool RSAPublicKey::empty() const {
	return !_private;
}

bool RSAPublicKey::valid() const {
	return !empty() && _private->valid();
}

uint64 RSAPublicKey::fingerprint() const {
	Expects(valid());

	return _private->fingerprint();
}

bytes::vector RSAPublicKey::getN() const {
	Expects(valid());

	return _private->getN();
}

bytes::vector RSAPublicKey::getE() const {
	Expects(valid());

	return _private->getE();
}

bytes::vector RSAPublicKey::encrypt(bytes::const_span data) const {
	Expects(valid());

	return _private->encrypt(data);
}

bytes::vector RSAPublicKey::decrypt(bytes::const_span data) const {
	Expects(valid());

	return _private->decrypt(data);
}

bytes::vector RSAPublicKey::encryptOAEPpadding(
		bytes::const_span data) const {
	return _private->encryptOAEPpadding(data);
}

} // namespace MTP::details
