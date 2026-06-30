/*
This file is part of Cryptogram,
the official desktop application for the Cryptogram messaging service.

For license and copyright information please follow this link:
https://github.com/SWORDIntel/cryptogram/blob/main/LEGAL
*/
#pragma once

namespace Platform::AutostartTask {

void Toggle(bool enabled, Fn<void(bool)> done);
void RequestState(Fn<void(bool)> callback);
void OpenSettings();

} // namespace Platform::AutostartTask
