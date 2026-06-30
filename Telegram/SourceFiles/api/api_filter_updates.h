/*
This file is part of Cryptogram,
the official desktop application for the Cryptogram messaging service.

For license and copyright information please follow this link:
https://github.com/SWORDIntel/cryptogram/blob/main/LEGAL
*/
#pragma once

namespace Api {

template <typename Type>
void PerformForUpdate(
		const MTPUpdates &updates,
		Fn<void(const Type &)> callback) {
	updates.match([&](const MTPDupdates &updates) {
		for (const auto &update : updates.vupdates().v) {
			update.match([&](const Type &d) {
				callback(d);
			}, [](const auto &) {
			});
		}
	}, [](const auto &) {
	});
}

} // namespace Api
