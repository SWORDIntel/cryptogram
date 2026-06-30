/*
This file is part of Cryptogram,
the official desktop application for the Cryptogram messaging service.

For license and copyright information please follow this link:
https://github.com/SWORDIntel/cryptogram/blob/main/LEGAL
*/
#pragma once

#include "base/weak_ptr.h"

namespace Api {
struct SendAction;
} // namespace Api

namespace Ui {
class LocationPicker;
} // namespace Ui

namespace Data {

class LocationPickers final {
public:
	LocationPickers();
	~LocationPickers();

	Ui::LocationPicker *lookup(const Api::SendAction &action);
	void emplace(
		const Api::SendAction &action,
		not_null<Ui::LocationPicker*> picker);

private:
	struct Entry;

	std::vector<Entry> _pickers;

};

} // namespace Data
