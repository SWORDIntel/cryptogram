/*
This file is part of Cryptogram,
the official desktop application for the Cryptogram messaging service.

For license and copyright information please follow this link:
https://github.com/SWORDIntel/cryptogram/blob/main/LEGAL
*/
#pragma once

namespace Ui {
struct MarkdownEnabledState;
} // namespace Ui

namespace Platform {

void CreateGlobalMenu();
void DestroyGlobalMenu();
void RequestUpdateGlobalMenu();

[[nodiscard]] rpl::producer<Ui::MarkdownEnabledState> GlobalMenuMarkdownState();

} // namespace Platform
