/*
This file is part of Cryptogram,
the official desktop application for the Cryptogram messaging service.

For license and copyright information please follow this link:
https://github.com/SWORDIntel/cryptogram/blob/main/LEGAL
*/
#pragma once

#include "base/object_ptr.h"
#include "lang/lang_keys.h"

class PeerData;
class UserData;

namespace Ui {
class BoxContent;
} // namespace Ui

namespace Window {
class SessionController;
} // namespace Window

[[nodiscard]] object_ptr<Ui::BoxContent> MakeVerifyPeersBox(
	not_null<Window::SessionController*> window,
	not_null<UserData*> bot);

struct BotVerifyPhrases {
	tr::phrase<> title;
	tr::phrase<lngtag_name> text;
	tr::phrase<> about;
	tr::phrase<> submit;
	tr::phrase<lngtag_name> sent;
	tr::phrase<> remove;
};
[[nodiscard]] BotVerifyPhrases PeerVerifyPhrases(not_null<PeerData*> peer);
