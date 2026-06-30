/*
This file is part of Cryptogram,
the official desktop application for the Cryptogram messaging service.

For license and copyright information please follow this link:
https://github.com/SWORDIntel/cryptogram/blob/main/LEGAL
*/
#pragma once

namespace Window {
class Controller;
} // namespace Window

namespace Ui {

void ShowPhoneBannedError(
	not_null<Window::Controller*> controller,
	const QString &phone);

} // namespace Ui
