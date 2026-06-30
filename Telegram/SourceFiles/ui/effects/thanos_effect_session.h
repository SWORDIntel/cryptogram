/*
This file is part of Cryptogram,
the official desktop application for the Cryptogram messaging service.

For license and copyright information please follow this link:
https://github.com/SWORDIntel/cryptogram/blob/main/LEGAL
*/
#pragma once

namespace Main {
class Session;
} // namespace Main

namespace Ui {

void ScheduleThanosEffectWarmUp(
	not_null<Main::Session*> session,
	rpl::lifetime &lifetime);

} // namespace Ui
