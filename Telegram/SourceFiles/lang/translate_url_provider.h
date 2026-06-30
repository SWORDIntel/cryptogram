/*
This file is part of Cryptogram,
the official desktop application for the Cryptogram messaging service.

For license and copyright information please follow this link:
https://github.com/SWORDIntel/cryptogram/blob/main/LEGAL
*/
#pragma once

#include "translate_provider.h"

namespace Ui {

[[nodiscard]] std::unique_ptr<TranslateProvider> CreateUrlTranslateProvider(
	QString urlTemplate);

} // namespace Ui
