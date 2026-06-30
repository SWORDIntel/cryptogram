/*
This file is part of Cryptogram,
the official desktop application for the Cryptogram messaging service.

For license and copyright information please follow this link:
https://github.com/SWORDIntel/cryptogram/blob/main/LEGAL
*/
#pragma once

namespace HistoryView {

struct WebPageText {
	QString title;
	QString description;
};

WebPageText TitleAndDescriptionFromWebPage(not_null<WebPageData*> d);
bool DrawWebPageDataPreview(
	QPainter &p,
	not_null<WebPageData*> webpage,
	not_null<PeerData*> context,
	QRect to);

} // namespace HistoryView
