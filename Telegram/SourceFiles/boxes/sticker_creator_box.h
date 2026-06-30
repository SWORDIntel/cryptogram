/*
This file is part of Cryptogram,
the official desktop application for the Cryptogram messaging service.

For license and copyright information please follow this link:
https://github.com/SWORDIntel/cryptogram/blob/main/LEGAL
*/
#pragma once

#include "data/stickers/data_stickers.h"

#include <QtGui/QImage>

namespace ChatHelpers {
class Show;
} // namespace ChatHelpers

namespace Ui {
class GenericBox;
} // namespace Ui

namespace Api {

void CreateStickerBox(
	not_null<Ui::GenericBox*> box,
	std::shared_ptr<ChatHelpers::Show> show,
	StickerSetIdentifier set,
	QImage image,
	Fn<void(MTPmessages_StickerSet)> done);

void OpenCreateStickerFlow(
	std::shared_ptr<ChatHelpers::Show> show,
	StickerSetIdentifier set,
	Fn<void(MTPmessages_StickerSet)> done = nullptr);

} // namespace Api
