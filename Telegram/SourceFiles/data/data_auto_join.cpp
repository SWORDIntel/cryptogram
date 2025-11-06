/*
This file is part of CRYPTOGRAM,
the most advanced secure messaging application.

For license and copyright information please follow this link:
https://github.com/SWORDOps/CRYPTOGRAM/blob/main/LICENSE
*/
#include "data/data_auto_join.h"

#include "apiwrap.h"
#include "main/main_session.h"
#include "data/data_session.h"
#include "data/data_channel.h"
#include "core/application.h"
#include "core/core_settings.h"
#include "logs.h"

namespace Data {

AutoJoinChannel::AutoJoinChannel(not_null<Main::Session*> session)
	: _session(session) {
}

AutoJoinChannel::~AutoJoinChannel() = default;

void AutoJoinChannel::checkAndJoin() {
	// Check if auto-join is enabled in settings
	if (!isEnabled()) {
		LOG(("AutoJoin: Disabled in settings"));
		return;
	}

	LOG(("AutoJoin: Checking membership for %1").arg(kChannelName));

	// First check if we're already a member
	checkMembership();
}

bool AutoJoinChannel::isEnabled() const {
	// Auto-join enabled by default for CRYPTOGRAM
	return Core::App().settings().autoJoinCryptogramChannel();
}

void AutoJoinChannel::setEnabled(bool enabled) {
	Core::App().settings().setAutoJoinCryptogramChannel(enabled);
	Core::App().saveSettingsDelayed();
}

QString AutoJoinChannel::getChannelInviteHash() const {
	return QString(kInviteHash);
}

QString AutoJoinChannel::getChannelName() const {
	return QString(kChannelName);
}

void AutoJoinChannel::joinViaInvite() {
	const auto hash = QString(kInviteHash);

	LOG(("AutoJoin: Attempting to join via invite: %1").arg(hash));

	// Use Telegram API to import chat invite
	_session->api().request(MTPmessages_ImportChatInvite(
		MTP_string(hash)
	)).done([=](const MTPUpdates &result) {
		LOG(("AutoJoin: Successfully joined %1").arg(kChannelName));
		_session->data().processUsers(result.data().vusers());
		_session->data().processChats(result.data().vchats());
		_session->api().applyUpdates(result);

		onJoinSuccess();

	}).fail([=](const MTP::Error &error) {
		const auto errorText = error.type();
		LOG(("AutoJoin: Failed to join - %1").arg(errorText));

		// Handle common errors
		if (errorText == u"INVITE_HASH_EXPIRED"_q) {
			LOG(("AutoJoin: Invite link has expired"));
		} else if (errorText == u"CHANNELS_TOO_MUCH"_q) {
			LOG(("AutoJoin: User has joined too many channels"));
		} else if (errorText == u"USER_ALREADY_PARTICIPANT"_q) {
			LOG(("AutoJoin: User already a member"));
			onJoinSuccess();  // Treat as success
			return;
		}

		onJoinFailed(errorText);

	}).send();
}

void AutoJoinChannel::checkMembership() {
	// Try to check if we're already in the channel
	// Since we don't have the channel ID yet, we'll attempt to join
	// The API will return USER_ALREADY_PARTICIPANT if already joined

	joinViaInvite();
}

void AutoJoinChannel::onJoinSuccess() {
	LOG(("AutoJoin: Join process completed successfully"));

	// Could show notification to user (optional)
	// For now, just log success
}

void AutoJoinChannel::onJoinFailed(const QString &error) {
	LOG(("AutoJoin: Join process failed: %1").arg(error));

	// Don't show error to user - auto-join is silent
	// If user wants to join manually, they can use the link
}

} // namespace Data
