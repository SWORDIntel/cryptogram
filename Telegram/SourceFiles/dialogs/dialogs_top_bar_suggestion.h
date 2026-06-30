/*
This file is part of Cryptogram,
the official desktop application for the Cryptogram messaging service.

For license and copyright information please follow this link:
https://github.com/SWORDIntel/cryptogram/blob/main/LEGAL
*/
#pragma once

template <typename Object>
class object_ptr;

namespace Main {
class Session;
} // namespace Main

namespace Ui {
class RpWidget;
template <typename Widget>
class SlideWrap;
} // namespace Ui

namespace Dialogs {

[[nodiscard]] auto TopBarSuggestionValue(
	not_null<Ui::RpWidget*> parent,
	not_null<Main::Session*>,
	rpl::producer<bool> outerWrapToggleValue,
	rpl::producer<float64> childListShown,
	rpl::producer<> prepareCollapseSnapshot)
-> rpl::producer<Ui::SlideWrap<Ui::RpWidget>*>;

} // namespace Dialogs

