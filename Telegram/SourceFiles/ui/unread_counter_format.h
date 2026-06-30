/*
This file is part of Cryptogram,
the official desktop application for the Cryptogram messaging service.

For license and copyright information please follow this link:
https://github.com/SWORDIntel/cryptogram/blob/main/LEGAL
*/
#pragma once

[[nodiscard]] QString FormatUnreadCounter(
	int unreadCounter,
	bool hasMentionOrReaction,
	bool narrow = false);