/*
This file is part of Cryptogram,
the official desktop application for the Cryptogram messaging service.

For license and copyright information please follow this link:
https://github.com/SWORDIntel/cryptogram/blob/main/LEGAL
*/
#pragma once

namespace HistoryView {

enum class DoubleClickQuickAction {
	Reply, // Default.
	React,
	None,
};

[[nodiscard]] DoubleClickQuickAction CurrentQuickAction();

} // namespace HistoryView
