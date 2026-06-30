/*
This file is part of Cryptogram,
the official desktop application for the Cryptogram messaging service.

For license and copyright information please follow this link:
https://github.com/SWORDIntel/cryptogram/blob/main/LEGAL
*/
#pragma once

#include <QtCore/QJsonObject>
#include <vector>

namespace Stripe {

[[nodiscard]] bool ContainsFields(
	const QJsonObject &object,
	std::vector<QStringView> keys);

} // namespace Stripe
