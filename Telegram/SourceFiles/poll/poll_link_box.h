/*
This file is part of Cryptogram,
the official desktop application for the Cryptogram messaging service.

For license and copyright information please follow this link:
https://github.com/SWORDIntel/cryptogram/blob/main/LEGAL
*/
#pragma once

namespace Ui {
class GenericBox;
} // namespace Ui

namespace Poll {

void AddPollOptionLinkBox(
	not_null<Ui::GenericBox*> box,
	const QString &initial,
	Fn<void(QString)> callback);

} // namespace Poll
