/*
This file is part of Cryptogram,
the official desktop application for the Cryptogram messaging service.

For license and copyright information please follow this link:
https://github.com/SWORDIntel/cryptogram/blob/main/LEGAL
*/
#pragma once

#include "ui/text/text_entity.h"

namespace Main {
class Session;
} // namespace Main

namespace Api {

enum class ConvertOption {
	WithLocal,
	SkipLocal,
};

[[nodiscard]] EntitiesInText EntitiesFromMTP(
	Main::Session *session,
	const QVector<MTPMessageEntity> &entities,
	int32 length = 0, int msglen = 0);

[[nodiscard]] MTPVector<MTPMessageEntity> EntitiesToMTP(
	Main::Session *session,
	const EntitiesInText &entities,
	ConvertOption option = ConvertOption::WithLocal);

[[nodiscard]] TextWithEntities ParseTextWithEntities(
	Main::Session *session,
	const MTPTextWithEntities &text);

} // namespace Api
