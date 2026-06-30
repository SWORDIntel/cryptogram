/*
This file is part of Cryptogram,
the official desktop application for the Cryptogram messaging service.

For license and copyright information please follow this link:
https://github.com/SWORDIntel/cryptogram/blob/main/LEGAL
*/
#pragma once

#include "ui/layers/generic_box.h"

namespace Ui {
class InputField;
} // namespace Ui

void EditFactcheckBox(
	not_null<Ui::GenericBox*> box,
	TextWithEntities current,
	int limit,
	Fn<void(TextWithEntities)> save,
	Fn<void(not_null<Ui::InputField*>)> initField);
