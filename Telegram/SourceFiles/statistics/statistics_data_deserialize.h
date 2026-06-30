/*
This file is part of Cryptogram,
the official desktop application for the Cryptogram messaging service.

For license and copyright information please follow this link:
https://github.com/SWORDIntel/cryptogram/blob/main/LEGAL
*/
#pragma once

namespace Data {
struct StatisticalChart;
} // namespace Data

class QByteArray;

namespace Statistic {

[[nodiscard]] Data::StatisticalChart StatisticalChartFromJSON(
	const QByteArray &json);

} // namespace Statistic
