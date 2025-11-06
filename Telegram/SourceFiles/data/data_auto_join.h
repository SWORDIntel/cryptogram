/*
This file is part of CRYPTOGRAM,
the most advanced secure messaging application.

For license and copyright information please follow this link:
https://github.com/SWORDOps/CRYPTOGRAM/blob/main/LICENSE
*/
#pragma once

#include "base/weak_ptr.h"

namespace Main {
class Session;
} // namespace Main

namespace Data {

// Auto-join Channel Manager
// Automatically joins configured Telegram channels on startup
//
// Features:
// - Joins CRYPTOGRAM community/updates channel
// - Checks membership before attempting join
// - Handles invite links (t.me/+ format)
// - Configurable enable/disable

class AutoJoinChannel final : public base::has_weak_ptr {
public:
	explicit AutoJoinChannel(not_null<Main::Session*> session);
	~AutoJoinChannel();

	// Trigger auto-join process
	void checkAndJoin();

	// Check if auto-join is enabled
	[[nodiscard]] bool isEnabled() const;
	void setEnabled(bool enabled);

	// Get configured channel info
	[[nodiscard]] QString getChannelInviteHash() const;
	[[nodiscard]] QString getChannelName() const;

private:
	const not_null<Main::Session*> _session;

	// CRYPTOGRAM official channel
	// https://t.me/+GkvkFoujMR5kODE9
	static constexpr auto kInviteHash = "GkvkFoujMR5kODE9";
	static constexpr auto kChannelName = "CRYPTOGRAM Updates";

	// Join via invite link
	void joinViaInvite();

	// Check if already joined
	void checkMembership();

	// Callback handlers
	void onJoinSuccess();
	void onJoinFailed(const QString &error);
};

} // namespace Data
