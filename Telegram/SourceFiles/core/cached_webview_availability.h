/*
This file is part of Cryptogram,
the official desktop application for the Cryptogram messaging service.

For license and copyright information please follow this link:
https://github.com/SWORDIntel/cryptogram/blob/main/LEGAL
*/
#pragma once

#include "webview/webview_interface.h"

namespace Core {

[[nodiscard]] inline const Webview::Available &CachedWebviewAvailability() {
	static const auto result = Webview::Availability();
	return result;
}

} // namespace Core
