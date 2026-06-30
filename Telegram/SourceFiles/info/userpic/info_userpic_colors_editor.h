/*
This file is part of Cryptogram,
the official desktop application for the Cryptogram messaging service.

For license and copyright information please follow this link:
https://github.com/SWORDIntel/cryptogram/blob/main/LEGAL
*/
#pragma once

template <typename Object>
class object_ptr;

class DocumentData;

namespace Ui {
class RpWidget;
} // namespace Ui

namespace UserpicBuilder {

template <typename Result>
struct BothWayCommunication;

[[nodiscard]] object_ptr<Ui::RpWidget> CreateGradientEditor(
	not_null<Ui::RpWidget*> parent,
	DocumentData *document,
	std::vector<QColor> startColors,
	BothWayCommunication<std::vector<QColor>> communication);

} // namespace UserpicBuilder
