/*
This file is part of Cryptogram,
the official desktop application for the Cryptogram messaging service.

For license and copyright information please follow this link:
https://github.com/SWORDIntel/cryptogram/blob/main/LEGAL
*/
#include "platform/platform_integration.h"

#if defined Q_OS_WINRT || defined Q_OS_WIN
#include "platform/win/integration_win.h"
#elif defined Q_OS_MAC // Q_OS_WINRT || Q_OS_WIN
#include "platform/mac/integration_mac.h"
#else // Q_OS_WINRT || Q_OS_WIN || Q_OS_MAC
#include "platform/linux/integration_linux.h"
#endif // else Q_OS_WINRT || Q_OS_WIN || Q_OS_MAC

namespace Platform {
namespace {

Integration *GlobalInstance/* = nullptr*/;

} // namespace

Integration::~Integration() {
	GlobalInstance = nullptr;
}

std::unique_ptr<Integration> Integration::Create() {
	Expects(GlobalInstance == nullptr);

	auto result = CreateIntegration();
	GlobalInstance = result.get();
	return result;
}

Integration &Integration::Instance() {
	Expects(GlobalInstance != nullptr);

	return *GlobalInstance;
};

} // namespace Platform
