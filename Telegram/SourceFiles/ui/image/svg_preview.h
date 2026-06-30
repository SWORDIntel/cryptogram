/*
This file is part of Cryptogram,
the official desktop application for the Cryptogram messaging service.

For license and copyright information please follow this link:
https://github.com/SWORDIntel/cryptogram/blob/main/LEGAL
*/
#pragma once

#include <QtCore/QByteArray>
#include <QtCore/QSize>
#include <QtGui/QImage>

namespace Ui {

[[nodiscard]] int SvgPreviewBytesLimit();
[[nodiscard]] QImage RenderSvgPreview(
	const QByteArray &bytes,
	QSize maxSize);

} // namespace Ui
