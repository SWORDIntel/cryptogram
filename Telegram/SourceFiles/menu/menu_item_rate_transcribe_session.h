/*
This file is part of Cryptogram,
the official desktop application for the Cryptogram messaging service.

For license and copyright information please follow this link:
https://github.com/SWORDIntel/cryptogram/blob/main/LEGAL
*/
#pragma once

class HistoryItem;

namespace Menu {

[[nodiscard]] Fn<void(bool)> RateTranscribeCallbackFactory(
	not_null<HistoryItem*>);

[[nodiscard]] bool HasRateTranscribeItem(not_null<HistoryItem*>);

} // namespace Menu
