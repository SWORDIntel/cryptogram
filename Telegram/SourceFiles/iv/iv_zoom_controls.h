/*
This file is part of Cryptogram,
the official desktop application for the Cryptogram messaging service.

For license and copyright information please follow this link:
https://github.com/SWORDIntel/cryptogram/blob/main/LEGAL
*/
#pragma once

#include "base/basic_types.h"
#include "base/unique_qptr.h"

namespace Ui {
class PopupMenu;
} // namespace Ui

namespace Ui::Menu {
class ItemBase;
} // namespace Ui::Menu

namespace Iv {

class Delegate;

[[nodiscard]] base::unique_qptr<Ui::Menu::ItemBase> CreateZoomMenuAction(
	not_null<Ui::PopupMenu*> parent,
	not_null<Delegate*> delegate);

} // namespace Iv
