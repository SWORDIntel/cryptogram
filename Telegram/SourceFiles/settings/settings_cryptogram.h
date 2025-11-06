/*
This file is part of CRYPTOGRAM,
the most advanced secure messaging application.

For license and copyright information please follow this link:
https://github.com/SWORDOps/CRYPTOGRAM/blob/main/LICENSE
*/
#pragma once

#include "settings/settings_common_session.h"
#include "settings/settings_type.h"
#include "data/data_i2p_integration.h"

namespace Settings {

/**
 * CRYPTOGRAM Settings Menu
 *
 * Advanced security and privacy settings for CRYPTOGRAM features.
 * Located in: Settings → CRYPTOGRAM (2 levels deep, near bottom)
 */

class Cryptogram : public Section<Cryptogram> {
public:
    Cryptogram(
        QWidget *parent,
        not_null<Window::SessionController*> controller);

    [[nodiscard]] rpl::producer<QString> title() override;

    // Subsections
    [[nodiscard]] QPointer<Ui::RpWidget> createNetworkAnonymity();

private:
    void setupContent();
    void setupNetworkAnonymitySection();

    // Network Anonymity Settings
    void createTorSettings(not_null<Ui::VerticalLayout*> container);
    void createI2PSettings(not_null<Ui::VerticalLayout*> container);
    void createBridgeSettings(not_null<Ui::VerticalLayout*> container);
    void createTorSnowflakeSettings(not_null<Ui::VerticalLayout*> container);
    void createI2PRelaySettings(not_null<Ui::VerticalLayout*> container);

    // Helper functions
    void updateI2PStatus();
    void saveSettings();

    not_null<Window::SessionController*> _controller;
};

/**
 * Network Anonymity Settings
 *
 * Configure Tor, I2P, and bridge networks for censorship resistance
 * and anonymity. Includes optional Tor Snowflake and I2P relay contribution.
 */
class NetworkAnonymity : public Section<NetworkAnonymity> {
public:
    NetworkAnonymity(
        QWidget *parent,
        not_null<Window::SessionController*> controller);

    [[nodiscard]] rpl::producer<QString> title() override;

private:
    void setupContent();

    not_null<Window::SessionController*> _controller;
};

// Settings storage keys
namespace CryptogramSettings {

// I2P Settings
inline constexpr auto kI2PEnabled = "cryptogram/i2p/enabled"_cs;
inline constexpr auto kI2PAutoStart = "cryptogram/i2p/auto_start"_cs;
inline constexpr auto kI2PRouterAddress = "cryptogram/i2p/router_address"_cs;
inline constexpr auto kI2PRouterPort = "cryptogram/i2p/router_port"_cs;

// Tor Settings
inline constexpr auto kTorEnabled = "cryptogram/tor/enabled"_cs;
inline constexpr auto kTorAutoStart = "cryptogram/tor/auto_start"_cs;
inline constexpr auto kTorSnowflakeEnabled = "cryptogram/tor/snowflake_enabled"_cs;
inline constexpr auto kTorSnowflakeCPU = "cryptogram/tor/snowflake_cpu"_cs;

// I2P Relay Settings
inline constexpr auto kI2PRelayEnabled = "cryptogram/i2p/relay_enabled"_cs;
inline constexpr auto kI2PRelayCPU = "cryptogram/i2p/relay_cpu"_cs;

// Contribution Settings (shared)
inline constexpr auto kContributionOnlyWhenIdle = "cryptogram/contribution/only_idle"_cs;
inline constexpr auto kContributionOnlyWhenCharging = "cryptogram/contribution/only_charging"_cs;
inline constexpr auto kContributionIdleMinutes = "cryptogram/contribution/idle_minutes"_cs;

} // namespace CryptogramSettings

} // namespace Settings
