/*
This file is part of Cryptogram,
the official desktop application for the Cryptogram messaging service.

For license and copyright information please follow this link:
https://github.com/SWORDIntel/cryptogram/blob/main/LEGAL
*/
#include "dialogs/suggestions/suggestion.h"

namespace Dialogs::TopBarSuggestions {

std::vector<Spec> AllSpecs() {
	auto result = std::vector<Spec>();
	result.push_back(MakeBirthdayContactsSpec());
	result.push_back(MakeBirthdaySetupSpec());
	result.push_back(MakeCustomPromoSpec());
	result.push_back(MakeGiftAuctionsSpec());
	result.push_back(MakeLowCreditsSubsSpec());
	result.push_back(MakePremiumGraceSpec());
	result.push_back(MakePremiumOfferSpec());
	result.push_back(MakeUnreviewedAuthSpec());
	result.push_back(MakeUserpicSetupSpec());
	return result;
}

} // namespace Dialogs::TopBarSuggestions
