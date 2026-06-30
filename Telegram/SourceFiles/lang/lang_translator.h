/*
This file is part of Cryptogram,
the official desktop application for the Cryptogram messaging service.

For license and copyright information please follow this link:
https://github.com/SWORDIntel/cryptogram/blob/main/LEGAL
*/
#pragma once

#include <QtCore/QTranslator>

namespace Lang {

class Translator : public QTranslator {
public:
	QString translate(const char *context, const char *sourceText, const char *disambiguation = 0, int n = -1) const override;

};

} // namespace Lang
