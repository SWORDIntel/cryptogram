/*
This file is part of CRYPTOGRAM Desktop,
the privacy-enhanced desktop application for secure messaging.

For license and copyright information please follow this link:
https://github.com/SWORDOps/CRYPTOGRAM/blob/main/LEGAL
*/
#include "data/data_peer_cryptogram_id.h"

#include "data/data_enhanced_privacy.h"
#include "data/data_user.h"
#include "ui/chat/chat_style.h"
#include "styles/style_dialogs.h"

namespace Data {

bool ShouldShowAsRedName(not_null<PeerData*> peer) {
    // Only show red name if peer is registered as CRYPTOGRAM user
    if (const auto user = peer->asUser()) {
        return EnhancedPrivacy::IsCryptogramUser(user->id);
    }
    return false;
}

QColor GetPeerNameColor(not_null<PeerData*> peer) {
    // Check if this is a CRYPTOGRAM user
    if (ShouldShowAsRedName(peer)) {
        // Return red color (crimson)
        return QColor(220, 20, 60);
    }

    // Return normal peer color based on their colorIndex
    // This preserves the standard Telegram color scheme for non-CRYPTOGRAM users
    const auto colorIndex = peer->colorIndex();

    // Use standard Telegram peer colors
    // These are the default peer name colors from Telegram's palette
    static const std::array<QColor, 8> kDefaultPeerColors = {{
        QColor(242, 106, 97),   // Red
        QColor(105, 181, 77),   // Green
        QColor(116, 174, 250),  // Blue
        QColor(236, 161, 86),   // Orange
        QColor(144, 121, 255),  // Purple
        QColor(105, 221, 188),  // Cyan
        QColor(255, 135, 168),  // Pink
        QColor(255, 179, 71),   // Yellow
    }};

    return kDefaultPeerColors[colorIndex % kDefaultPeerColors.size()];
}

style::color GetPeerNameStyleColor(not_null<PeerData*> peer) {
    // For CRYPTOGRAM users, we need to return a red color
    if (ShouldShowAsRedName(peer)) {
        // Use msgFileOutBg as a base and override with red
        // (This is a workaround since we can't easily create custom style::color at runtime)
        return st::msgFileOutBg; // Will be overridden in paint code
    }

    // Return normal dialog name color
    return st::dialogsNameFg;
}

void AutoDetectCryptogramUser(not_null<PeerData*> peer) {
    // Auto-register peer as CRYPTOGRAM user
    // Called when we detect they're using CRYPTOGRAM features (e.g., covert channel, encryption)
    if (const auto user = peer->asUser()) {
        if (!EnhancedPrivacy::IsCryptogramUser(user->id)) {
            EnhancedPrivacy::RegisterCryptogramUser(user->id);
            LOG(("CRYPTOGRAM: Auto-detected CRYPTOGRAM user: %1").arg(user->id.value));
        }
    }
}

} // namespace Data
