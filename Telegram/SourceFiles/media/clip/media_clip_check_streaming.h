/*
This file is part of Cryptogram,
the official desktop application for the Cryptogram messaging service.

For license and copyright information please follow this link:
https://github.com/SWORDIntel/cryptogram/blob/main/LEGAL
*/
#pragma once

namespace Core {
class FileLocation;
} // namespace Core

namespace Media {
namespace Clip {

bool CheckStreamingSupport(
	const Core::FileLocation &location,
	QByteArray data);

} // namespace Clip
} // namespace Media
