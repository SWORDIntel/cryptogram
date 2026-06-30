/*
This file is part of Cryptogram,
the official desktop application for the Cryptogram messaging service.

For license and copyright information please follow this link:
https://github.com/SWORDIntel/cryptogram/blob/main/LEGAL
*/
#pragma once

class ClickHandler;

namespace HistoryView {

[[nodiscard]] std::shared_ptr<ClickHandler> SponsoredLink(
	const QString &link,
	bool isInternal);

} // namespace HistoryView
