/*
This file is part of Cryptogram,
the official desktop application for the Cryptogram messaging service.

For license and copyright information please follow this link:
https://github.com/SWORDIntel/cryptogram/blob/main/LEGAL
*/
#pragma once

class PeerData;

namespace Data {
struct CreditsHistoryEntry;
} // namespace Data

namespace Api {

[[nodiscard]] Data::CreditsHistoryEntry CreditsHistoryEntryFromTL(
	const MTPStarsTransaction &tl,
	not_null<PeerData*> peer);

} // namespace Api
