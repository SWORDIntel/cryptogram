/*
This file is part of Cryptogram,
the official desktop application for the Cryptogram messaging service.

For license and copyright information please follow this link:
https://github.com/SWORDIntel/cryptogram/blob/main/LEGAL
*/
#pragma once

namespace Ui {

class GenericBox;

[[nodiscard]] std::vector<TimeId> DefaultTimePickerValues();

[[nodiscard]] Fn<TimeId()> TimePickerBox(
	not_null<GenericBox*> box,
	std::vector<TimeId> values,
	std::vector<QString> phrases,
	TimeId startValue);

} // namespace Ui
