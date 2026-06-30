/*
This file is part of Cryptogram,
the official desktop application for the Cryptogram messaging service.

For license and copyright information please follow this link:
https://github.com/SWORDIntel/cryptogram/blob/main/LEGAL
*/
#pragma once

namespace Statistic {

[[nodiscard]] QString LangDayMonthYear(crl::time seconds);
[[nodiscard]] QString LangDayMonth(crl::time seconds);
[[nodiscard]] QString LangDetailedDayMonth(crl::time seconds);

} // namespace Statistic
