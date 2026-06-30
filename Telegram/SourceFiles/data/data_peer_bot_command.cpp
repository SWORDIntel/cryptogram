/*
This file is part of Cryptogram,
the official desktop application for the Cryptogram messaging service.

For license and copyright information please follow this link:
https://github.com/SWORDIntel/cryptogram/blob/main/LEGAL
*/
#include "data/data_peer_bot_command.h"

namespace Data {

BotCommand BotCommandFromTL(const MTPBotCommand &result) {
	return result.match([](const MTPDbotCommand &data) {
		return BotCommand {
			.command = qs(data.vcommand().v),
			.description = qs(data.vdescription().v),
		};
	});
}

} // namespace Data
