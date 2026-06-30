/*
This file is part of Cryptogram,
the official desktop application for the Cryptogram messaging service.

For license and copyright information please follow this link:
https://github.com/SWORDIntel/cryptogram/blob/main/LEGAL
*/
#pragma once

#include <AppKit/NSImageView.h>

namespace Main {
class Session;
} // namespace Main

API_AVAILABLE(macos(10.12.2))
@interface PinnedDialogsPanel : NSImageView
- (id)init:(not_null<Main::Session*>)session
	destroyEvent:(rpl::producer<>)touchBarSwitches;
@end
