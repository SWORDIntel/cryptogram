/*
This file is part of Cryptogram,
the official desktop application for the Cryptogram messaging service.

For license and copyright information please follow this link:
https://github.com/SWORDIntel/cryptogram/blob/main/LEGAL
*/
#pragma once

#include "platform/platform_translate_provider.h"

namespace Platform {

inline bool IsTranslateProviderAvailable() {
	return false;
}

inline std::unique_ptr<Ui::TranslateProvider> CreateTranslateProvider() {
	return nullptr;
}

} // namespace Platform
