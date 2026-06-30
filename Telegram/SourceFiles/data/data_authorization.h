/*
This file is part of Cryptogram,
the official desktop application for the Cryptogram messaging service.

For license and copyright information please follow this link:
https://github.com/SWORDIntel/cryptogram/blob/main/LEGAL
*/
#pragma once

namespace Data {

struct UnreviewedAuth {
	uint64 hash = 0;
	bool unconfirmed = false;
	TimeId date = 0;
	QString device;
	QString location;
};

} // namespace Data
