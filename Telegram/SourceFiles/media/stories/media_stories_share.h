/*
This file is part of Cryptogram,
the official desktop application for the Cryptogram messaging service.

For license and copyright information please follow this link:
https://github.com/SWORDIntel/cryptogram/blob/main/LEGAL
*/
#pragma once

#include "base/object_ptr.h"

namespace ChatHelpers {
class Show;
} // namespace ChatHelpers

namespace Ui {
class BoxContent;
} // namespace Ui

namespace Media::Stories {

[[nodiscard]] object_ptr<Ui::BoxContent> PrepareShareBox(
	std::shared_ptr<ChatHelpers::Show> show,
	FullStoryId id,
	bool viewerStyle = false);

[[nodiscard]] QString FormatShareAtTime(TimeId seconds);

[[nodiscard]] object_ptr<Ui::BoxContent> PrepareShareAtTimeBox(
	std::shared_ptr<ChatHelpers::Show> show,
	not_null<HistoryItem*> item,
	TimeId videoTimestamp);

} // namespace Media::Stories
