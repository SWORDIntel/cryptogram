/*
This file is part of Cryptogram,
the official desktop application for the Cryptogram messaging service.

For license and copyright information please follow this link:
https://github.com/SWORDIntel/cryptogram/blob/main/LEGAL
*/
#pragma once

class UserData;

namespace Ui {
class RpWidget;
} // namespace Ui

namespace Info::Profile {

void StartProfileBirthdayEffect(
	not_null<Ui::RpWidget*> cover,
	not_null<UserData*> user,
	Fn<QRect()> userpicGeometry,
	Fn<bool()> paused);

} // namespace Info::Profile
