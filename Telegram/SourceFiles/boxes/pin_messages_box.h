/*
This file is part of Cryptogram,
the official desktop application for the Cryptogram messaging service.

For license and copyright information please follow this link:
https://github.com/SWORDIntel/cryptogram/blob/main/LEGAL
*/
#pragma once

namespace Ui {
class GenericBox;
} // namespace Ui

void PinMessageBox(
	not_null<Ui::GenericBox*> box,
	not_null<HistoryItem*> item);
