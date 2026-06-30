/*
This file is part of Cryptogram,
the official desktop application for the Cryptogram messaging service.

For license and copyright information please follow this link:
https://github.com/SWORDIntel/cryptogram/blob/main/LEGAL
*/
#pragma once

#ifdef Q_OS_MAC

#include <QtGui/QImage>

namespace Media::Streaming {

struct NativeFrame;

[[nodiscard]] QImage ConvertNativeFrameToARGB32(const NativeFrame &frame);

} // namespace Media::Streaming

#endif // Q_OS_MAC
