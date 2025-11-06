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
#include "data/data_akashi_network.h"

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
    [[nodiscard]] QPointer<Ui::RpWidget> createAkashiContribution();

private:
    void setupContent();
    void setupNetworkAnonymitySection();
    void setupAkashiSection();

    // Network Anonymity Settings
    void createTorSettings(not_null<Ui::VerticalLayout*> container);
    void createI2PSettings(not_null<Ui::VerticalLayout*> container);
    void createBridgeSettings(not_null<Ui::VerticalLayout*> container);

    // Akashi Network Settings
    void createContributionToggle(not_null<Ui::VerticalLayout*> container);
    void createCPULevelSelector(not_null<Ui::VerticalLayout*> container);
    void createIdleDetectionSelector(not_null<Ui::VerticalLayout*> container);
    void createContributionTypesCheckboxes(not_null<Ui::VerticalLayout*> container);
    void createAdvancedOptions(not_null<Ui::VerticalLayout*> container);
    void createStatisticsDisplay(not_null<Ui::VerticalLayout*> container);

    // Helper functions
    void updateI2PStatus();
    void updateAkashiStatistics();
    void saveSettings();

    not_null<Window::SessionController*> _controller;
};

/**
 * Network Anonymity Settings
 *
 * Configure Tor, I2P, and bridge networks for censorship resistance
 * and anonymity.
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

/**
 * Akashi Network Contribution Settings
 *
 * Configure CPU and resource donation to the Akashi distributed network.
 * Helps maintain privacy infrastructure for the CRYPTOGRAM community.
 */
class AkashiContribution : public Section<AkashiContribution> {
public:
    AkashiContribution(
        QWidget *parent,
        not_null<Window::SessionController*> controller);

    [[nodiscard]] rpl::producer<QString> title() override;

private:
    void setupContent();
    void createMainSettings(not_null<Ui::VerticalLayout*> container);
    void createStatistics(not_null<Ui::VerticalLayout*> container);

    void updateStatisticsDisplay();

    not_null<Window::SessionController*> _controller;
    base::Timer _statsUpdateTimer;
};

// Settings storage keys
namespace CryptogramSettings {

// I2P Settings
inline constexpr auto kI2PEnabled = "cryptogram/i2p/enabled"_cs;
inline constexpr auto kI2PAutoStart = "cryptogram/i2p/auto_start"_cs;
inline constexpr auto kI2PRouterAddress = "cryptogram/i2p/router_address"_cs;
inline constexpr auto kI2PRouterPort = "cryptogram/i2p/router_port"_cs;

// Tor Settings (already in network security)
inline constexpr auto kTorEnabled = "cryptogram/tor/enabled"_cs;
inline constexpr auto kTorAutoStart = "cryptogram/tor/auto_start"_cs;

// Akashi Network Settings
inline constexpr auto kAkashiEnabled = "cryptogram/akashi/enabled"_cs;
inline constexpr auto kAkashiCPULevel = "cryptogram/akashi/cpu_level"_cs;
inline constexpr auto kAkashiIdleLevel = "cryptogram/akashi/idle_level"_cs;
inline constexpr auto kAkashiEnableRelay = "cryptogram/akashi/enable_relay"_cs;
inline constexpr auto kAkashiEnableCompute = "cryptogram/akashi/enable_compute"_cs;
inline constexpr auto kAkashiEnableStorage = "cryptogram/akashi/enable_storage"_cs;
inline constexpr auto kAkashiEnableBandwidth = "cryptogram/akashi/enable_bandwidth"_cs;
inline constexpr auto kAkashiOnlyWhenCharging = "cryptogram/akashi/only_charging"_cs;
inline constexpr auto kAkashiOnlyWhenIdle = "cryptogram/akashi/only_idle"_cs;
inline constexpr auto kAkashiMaxBandwidth = "cryptogram/akashi/max_bandwidth"_cs;
inline constexpr auto kAkashiMaxStorage = "cryptogram/akashi/max_storage"_cs;

} // namespace CryptogramSettings

} // namespace Settings
