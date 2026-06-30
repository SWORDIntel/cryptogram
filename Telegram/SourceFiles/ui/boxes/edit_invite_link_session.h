/*
This file is part of Cryptogram,
the official desktop application for the Cryptogram messaging service.

For license and copyright information please follow this link:
https://github.com/SWORDIntel/cryptogram/blob/main/LEGAL
*/
#pragma once

class PeerData;

namespace Ui {

class GenericBox;
class SettingsButton;

struct InviteLinkSubscriptionToggle;

InviteLinkSubscriptionToggle FillCreateInviteLinkSubscriptionToggle(
	not_null<Ui::GenericBox*> box,
	not_null<PeerData*> peer);

} // namespace Ui
