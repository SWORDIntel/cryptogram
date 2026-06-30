/*
This file is part of Cryptogram,
the official desktop application for the Cryptogram messaging service.

For license and copyright information please follow this link:
https://github.com/SWORDIntel/cryptogram/blob/main/LEGAL
*/
#pragma once

namespace Data {

struct ColorProfileSet {
	std::vector<QColor> palette;
	std::vector<QColor> bg;
	std::vector<QColor> story;
};

struct ColorProfileData {
	ColorProfileSet light;
	ColorProfileSet dark;
};

} // namespace Data
