/*
This file is part of Cryptogram,
the official desktop application for the Cryptogram messaging service.

For license and copyright information please follow this link:
https://github.com/SWORDIntel/cryptogram/blob/main/LEGAL
*/
#pragma once

namespace Info::Stories {

[[nodiscard]] int ArchiveId();

struct Tag {
	explicit Tag(
		not_null<PeerData*> peer,
		int albumId = 0,
		int addingToAlbumId = 0)
	: peer(peer)
	, albumId(albumId)
	, addingToAlbumId(addingToAlbumId) {
	}

	not_null<PeerData*> peer;
	int albumId = 0;
	int addingToAlbumId = 0;
};

} // namespace Info::Stories
