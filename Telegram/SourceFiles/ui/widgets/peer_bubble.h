/*
This file is part of Cryptogram,
the official desktop application for the Cryptogram messaging service.

For license and copyright information please follow this link:
https://github.com/SWORDIntel/cryptogram/blob/main/LEGAL
*/
#pragma once

template <typename Object>
class object_ptr;

class PeerData;

namespace Ui {
class RpWidget;
class VerticalLayout;
} // namespace Ui

namespace Ui {

[[nodiscard]] object_ptr<Ui::RpWidget> CreatePeerBubble(
	not_null<Ui::RpWidget*> parent,
	not_null<PeerData*> peer);

} // namespace Ui
