/*
This file is part of Cryptogram,
the official desktop application for the Cryptogram messaging service.

For license and copyright information please follow this link:
https://github.com/SWORDIntel/cryptogram/blob/main/LEGAL
*/
#pragma once

namespace Data {
struct StatisticalGraph;
} // namespace Data

namespace Api {

[[nodiscard]] Data::StatisticalGraph StatisticalGraphFromTL(
	const MTPStatsGraph &tl);

} // namespace Api
