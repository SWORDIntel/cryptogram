/*
This file is part of Cryptogram,
the official desktop application for the Cryptogram messaging service.

For license and copyright information please follow this link:
https://github.com/SWORDIntel/cryptogram/blob/main/LEGAL
*/
#pragma once

#include "platform/platform_tray.h"

namespace Core::TrayAccountsMenu {

void SetupChangesSubscription(
	Fn<void()> callback,
	rpl::lifetime &lifetime);
void Fill(Platform::Tray &tray);

} // namespace Core::TrayAccountsMenu
