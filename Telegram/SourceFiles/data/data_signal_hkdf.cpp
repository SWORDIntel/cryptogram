/*
This file is part of CRYPTOGRAM,
the most advanced secure messaging application for secure messaging.

For license and copyright information please follow this link:
https://github.com/SWORDOps/CRYPTOGRAM/blob/main/LICENSE
*/
#include "data/data_signal_hkdf.h"

#include <cstdint>

#include "../../ThirdParty/libsignal/src/signal_protocol.h"
#include "../../ThirdParty/libsignal/src/hkdf.h"

#include <QtCore/QByteArray>
#include <QtCore/QMutex>
#include <QtCore/QDebug>
#include <cstdlib>

namespace Data {
namespace {

signal_context *gSignalContext = nullptr;
QMutex gSignalContextMutex;

signal_context *ensureSignalContext() {
    QMutexLocker locker(&gSignalContextMutex);
    if (!gSignalContext) {
        if (signal_context_create(&gSignalContext, nullptr) != 0) {
            qWarning() << "Signal HKDF: failed to create global signal_context";
            return nullptr;
        }
    }
    return gSignalContext;
}

inline const uint8_t *asUInt8(const bytes::const_span &span) {
    return reinterpret_cast<const uint8_t *>(span.data());
}

} // namespace

bytes::vector deriveSignalHKDF(
        const bytes::const_span &inputKeyMaterial,
        const QString &info,
        size_t outputLength) {
    if (outputLength == 0) {
        return {};
    }

    auto *context = ensureSignalContext();
    if (!context) {
        return {};
    }

    hkdf_context *kdf = nullptr;
    if (hkdf_create(&kdf, 1, context) != 0 || !kdf) {
        qWarning() << "Signal HKDF: failed to create hkdf_context";
        return {};
    }

    QByteArray infoBytes = info.toUtf8();

    uint8_t *outputPtr = nullptr;
    auto result = hkdf_derive_secrets(
        kdf,
        &outputPtr,
        asUInt8(inputKeyMaterial), inputKeyMaterial.size(),
        nullptr, 0,
        reinterpret_cast<const uint8_t*>(infoBytes.constData()), infoBytes.size(),
        outputLength);

    hkdf_destroy(reinterpret_cast<signal_type_base*>(kdf));

    if (result < 0) {
        qWarning() << "Signal HKDF: derive failed";
        return {};
    }
    bytes::vector output;
    output.reserve(result);
    for (int i = 0; i < result; ++i) {
        output.push_back(bytes::type(outputPtr[i]));
    }
    free(outputPtr);
    return output;
}

} // namespace Data
