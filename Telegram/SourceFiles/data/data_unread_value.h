/*
This file is part of Cryptogram,
the official desktop application for the Cryptogram messaging service.

For license and copyright information please follow this link:
https://github.com/SWORDIntel/cryptogram/blob/main/LEGAL
*/
#pragma once

namespace Dialogs {
struct UnreadState;
} // namespace Dialogs

namespace Main {
class Session;
} // namespace Main

namespace Data {

[[nodiscard]] Dialogs::UnreadState MainListMapUnreadState(
	not_null<Main::Session*> session,
	const Dialogs::UnreadState &state);

[[nodiscard]] rpl::producer<Dialogs::UnreadState> UnreadStateValue(
	not_null<Main::Session*> session,
	FilterId filterId);

[[nodiscard]] rpl::producer<bool> IncludeMutedCounterFoldersValue();

} // namespace Data
