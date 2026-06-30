/*
This file is part of Cryptogram,
the official desktop application for the Cryptogram messaging service.

For license and copyright information please follow this link:
https://github.com/SWORDIntel/cryptogram/blob/main/LEGAL
*/
#pragma once

namespace Ui {

class RpWidget;

[[nodiscard]] QPainterPath ToggleUpDownArrowPath(
	float64 x,
	float64 y,
	float64 size,
	float64 fourStrokes,
	float64 progress);

void AddToggleUpDownArrowToMoreButton(not_null<Ui::RpWidget*> parent);

} // namespace Ui
