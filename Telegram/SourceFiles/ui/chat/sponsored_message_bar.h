/*
This file is part of Cryptogram,
the official desktop application for the Cryptogram messaging service.

For license and copyright information please follow this link:
https://github.com/SWORDIntel/cryptogram/blob/main/LEGAL
*/
#pragma once

namespace Data {
struct SponsoredFrom;
} // namespace Data

namespace Main {
class Session;
} // namespace Main

namespace Ui {

class RpWidget;

void FillSponsoredMessageBar(
	not_null<RpWidget*> container,
	not_null<Main::Session*> session,
	FullMsgId fullId,
	Data::SponsoredFrom from,
	const TextWithEntities &textWithEntities);

} // namespace Ui
