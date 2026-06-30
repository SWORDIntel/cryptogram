/*
This file is part of Cryptogram,
the official desktop application for the Cryptogram messaging service.

For license and copyright information please follow this link:
https://github.com/SWORDIntel/cryptogram/blob/main/LEGAL
*/
#include "data/data_game.h"

GameData::GameData(not_null<Data::Session*> owner, const GameId &id)
: owner(owner)
, id(id) {
}
