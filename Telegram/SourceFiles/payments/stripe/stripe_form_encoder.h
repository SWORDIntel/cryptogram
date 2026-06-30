/*
This file is part of Cryptogram,
the official desktop application for the Cryptogram messaging service.

For license and copyright information please follow this link:
https://github.com/SWORDIntel/cryptogram/blob/main/LEGAL
*/
#pragma once

#include "stripe/stripe_form_encodable.h"

namespace Stripe {

class FormEncoder {
public:
	[[nodiscard]] static QByteArray formEncodedDataForObject(
		FormEncodable &&object);

};

} // namespace Stripe
