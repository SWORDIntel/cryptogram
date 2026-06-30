/*
This file is part of Cryptogram,
the official desktop application for the Cryptogram messaging service.

For license and copyright information please follow this link:
https://github.com/SWORDIntel/cryptogram/blob/main/LEGAL
*/
#pragma once

#include "iv/iv_rich_page.h"

#include <optional>

namespace Main {
class Session;
} // namespace Main

namespace Iv {

[[nodiscard]] std::optional<MTPInputRichMessage> SerializeInputRichMessage(
	not_null<Main::Session*> session,
	const RichPage &page);

} // namespace Iv