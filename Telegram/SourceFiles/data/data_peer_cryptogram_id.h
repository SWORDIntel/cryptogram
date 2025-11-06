/*
This file is part of CRYPTOGRAM Desktop,
the privacy-enhanced desktop application for secure messaging.

For license and copyright information please follow this link:
https://github.com/SWORDOps/CRYPTOGRAM/blob/main/LEGAL
*/
#pragma once

#include "data/data_peer.h"
#include "ui/style/style_core.h"

namespace Data {

// CRYPTOGRAM User Visual Identification
// Displays known CRYPTOGRAM users with red-colored names
// Only visible to other CRYPTOGRAM users (local client-side feature)
//
// Usage:
// - When rendering peer names, check if peer is CRYPTOGRAM user
// - If yes, override name color to red
// - Non-CRYPTOGRAM clients see normal colored names
//
// This helps users identify who they can safely use:
// - Covert channels with
// - Double Ratchet encryption
// - Enhanced privacy features

// Get the color for a peer's name
// Returns red for CRYPTOGRAM users, normal color otherwise
[[nodiscard]] QColor GetPeerNameColor(not_null<PeerData*> peer);

// Get the style color for a peer's name (for style system)
[[nodiscard]] style::color GetPeerNameStyleColor(not_null<PeerData*> peer);

// Check if peer should be displayed with red name
[[nodiscard]] bool ShouldShowAsRedName(not_null<PeerData*> peer);

// Auto-register CRYPTOGRAM user when they send encrypted message
void AutoDetectCryptogramUser(not_null<PeerData*> peer);

// Constants
namespace {
    // Red color for CRYPTOGRAM users (visible only to CRYPTOGRAM users)
    constexpr auto kCryptogramUserColor = QColor(220, 20, 60); // Crimson red
    constexpr auto kCryptogramUserColorHex = "#DC143C";
}

} // namespace Data
