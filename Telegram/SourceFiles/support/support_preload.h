/*
This file is part of Cryptogram,
the official desktop application for the Cryptogram messaging service.

For license and copyright information please follow this link:
https://github.com/SWORDIntel/cryptogram/blob/main/LEGAL
*/
#pragma once

class History;

namespace Support {

// Returns histories().request, not api().request.
[[nodiscard]] int SendPreloadRequest(
	not_null<History*> history,
	Fn<void()> retry);

} // namespace Support
