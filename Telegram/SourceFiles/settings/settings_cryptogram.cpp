/*
This file is part of CRYPTOGRAM,
the most advanced secure messaging application.

For license and copyright information please follow this link:
https://github.com/SWORDOps/CRYPTOGRAM/blob/main/LICENSE
*/
#include "settings/settings_cryptogram.h"
#include "settings/settings_trust_history.h"

#include "ui/wrap/vertical_layout.h"
#include "ui/wrap/slide_wrap.h"
#include "ui/widgets/labels.h"
#include "ui/widgets/checkbox.h"
#include "ui/widgets/buttons.h"
#include "ui/widgets/continuous_sliders.h"
#include "ui/widgets/fields/input_field.h"
#include "ui/boxes/confirm_box.h"
#include "ui/text/text_utilities.h"
#include "ui/vertical_list.h"
#include "lang/lang_keys.h"
#include "core/application.h"
#include "core/core_settings.h"
#include "core/peer_trust.h"
#include "data/data_session.h"
#include "data/data_cac_interface.h"
#include "data/data_enhanced_privacy.h"
#include "data/data_group_encryption.h"
#include "data/data_mls_protocol.h"
#include "main/main_session.h"
#include "window/window_session_controller.h"
#include "styles/style_settings.h"
#include "styles/style_layers.h"
#include "base/platform/base_platform_info.h"

#include <QtWidgets/QApplication>

namespace Settings {
namespace {

using namespace CryptogramSettings;

constexpr auto kStatsUpdateInterval = 2000; // 2 seconds

[[nodiscard]] QString FormatHashrate(double hashrate) {
	if (hashrate >= 1000000.0) {
		return QString::number(hashrate / 1000000.0, 'f', 2) + " MH/s";
	} else if (hashrate >= 1000.0) {
		return QString::number(hashrate / 1000.0, 'f', 2) + " KH/s";
	} else {
		return QString::number(hashrate, 'f', 2) + " H/s";
	}
}

[[nodiscard]] QString FormatDuration(qint64 seconds) {
	const auto hours = seconds / 3600;
	const auto minutes = (seconds % 3600) / 60;

	if (hours > 0) {
		return QString::number(hours) + "h " + QString::number(minutes) + "m";
	} else if (minutes > 0) {
		return QString::number(minutes) + " min";
	} else {
		return QString::number(seconds) + " sec";
	}
}

[[nodiscard]] QString FormatEarnings(qint64 totalSeconds, double hashrate) {
	// Very rough estimate: ~$0.10 per day per 1000 H/s at current rates
	// This is just for user information, not financial advice
	if (hashrate <= 0 || totalSeconds <= 0) {
		return "$0.00";
	}

	const auto daysEquivalent = totalSeconds / 86400.0;
	const auto estimatedUsd = (hashrate / 1000.0) * 0.10 * daysEquivalent;

	return QString("$%1").arg(estimatedUsd, 0, 'f', 2);
}

} // namespace

Cryptogram::Cryptogram(
	QWidget *parent,
	not_null<Window::SessionController*> controller)
: Section(parent)
, _controller(controller)
, _miningStatsTimer([=] { updateMiningStatistics(); })
, _translationStatsTimer([=] { updateTranslationStatus(); }) {
	if (!Data::GetGroupEncryption()) {
		Data::InitializeGroupEncryption();
	}

	setupContent();

	// Start stats update timers
	_miningStatsTimer.callEach(kStatsUpdateInterval);
	_translationStatsTimer.callEach(kStatsUpdateInterval);
}

rpl::producer<QString> Cryptogram::title() {
	return rpl::single(QString("CRYPTOGRAM"));
}

void Cryptogram::setupContent() {
	const auto content = Ui::CreateChild<Ui::VerticalLayout>(this);

	// Network Anonymity Section
	setupNetworkAnonymitySection(content);

	Ui::AddSkip(content);
	Ui::AddDivider(content);
	Ui::AddSkip(content);

	// Encryption & Privacy Section
	setupEncryptionSection(content);

	Ui::AddSkip(content);
	Ui::AddDivider(content);
	Ui::AddSkip(content);

	// Privacy Controls Section
	setupPrivacyControlsSection(content);

	Ui::AddSkip(content);
	Ui::AddDivider(content);
	Ui::AddSkip(content);

	// UI/UX Preferences Section
	setupUIPreferencesSection(content);

	Ui::AddSkip(content);
	Ui::AddDivider(content);
	Ui::AddSkip(content);

	// Device Trust Section
	setupDeviceTrustSection(content);

	Ui::AddSkip(content);
	Ui::AddDivider(content);
	Ui::AddSkip(content);

	// Translation Section (OpenVINO)
	setupTranslationSection(content);

	Ui::AddSkip(content);
	Ui::AddDivider(content);
	Ui::AddSkip(content);

	// Development Support Section (Mining)
	setupDevelopmentSupportSection(content);

	Ui::ResizeFitChild(this, content);
}

void Cryptogram::setupNetworkAnonymitySection(not_null<Ui::VerticalLayout*> container) {
	Ui::AddSkip(container);
	Ui::AddSubsectionTitle(container, rpl::single(QString("Network Anonymity")));

	createTorSettings(container);
	createI2PSettings(container);
	createBridgeSettings(container);
	createTorSnowflakeSettings(container);
	createI2PRelaySettings(container);
}

void Cryptogram::createTorSettings(not_null<Ui::VerticalLayout*> container) {
	// Tor configuration
	const auto settings = &Core::App().settings();

	Ui::AddSkip(container, st::settingsCheckboxesSkip);

	const auto enabled = container->add(
		object_ptr<Ui::Checkbox>(
			container,
			QString("Enable Tor Integration"),
			settings->torEnabled(),
			st::settingsCheckbox),
		st::settingsCheckboxPadding);

	enabled->checkedChanges(
	) | rpl::start_with_next([=](bool checked) {
		settings->setTorEnabled(checked);
		Core::App().saveSettingsDelayed();
	}, enabled->lifetime());

	Ui::AddSkip(container, st::settingsCheckboxesSkip);
}

void Cryptogram::createI2PSettings(not_null<Ui::VerticalLayout*> container) {
	// I2P configuration
	const auto settings = &Core::App().settings();

	const auto enabled = container->add(
		object_ptr<Ui::Checkbox>(
			container,
			QString("Enable I2P Integration"),
			settings->i2pEnabled(),
			st::settingsCheckbox),
		st::settingsCheckboxPadding);

	enabled->checkedChanges(
	) | rpl::start_with_next([=](bool checked) {
		settings->setI2pEnabled(checked);
		Core::App().saveSettingsDelayed();
		updateI2PStatus();
	}, enabled->lifetime());

	Ui::AddSkip(container, st::settingsCheckboxesSkip);
}

void Cryptogram::createBridgeSettings(not_null<Ui::VerticalLayout*> container) {
	// Bridge configuration - placeholder for future implementation
	Ui::AddDividerText(
		container,
		rpl::single(QString(
			"💡 Bridge Support: Bridges act as alternative entry points to Tor/I2P networks. "
			"Enable this if direct connections are blocked in your region. "
			"Bridge configuration will be available in a future update."
		))
	);
	Ui::AddSkip(container, st::settingsCheckboxesSkip);
}

void Cryptogram::createTorSnowflakeSettings(not_null<Ui::VerticalLayout*> container) {
	// Tor Snowflake proxy configuration
	const auto settings = &Core::App().settings();

	const auto enabled = container->add(
		object_ptr<Ui::Checkbox>(
			container,
			QString("Run Tor Snowflake Proxy (Help Others)"),
			settings->torSnowflakeEnabled(),
			st::settingsCheckbox),
		st::settingsCheckboxPadding);

	enabled->checkedChanges(
	) | rpl::start_with_next([=](bool checked) {
		settings->setTorSnowflakeEnabled(checked);
		Core::App().saveSettingsDelayed();
	}, enabled->lifetime());

	Ui::AddSkip(container, st::settingsCheckboxesSkip);
}

void Cryptogram::createI2PRelaySettings(not_null<Ui::VerticalLayout*> container) {
	// I2P Relay configuration
	const auto settings = &Core::App().settings();

	const auto enabled = container->add(
		object_ptr<Ui::Checkbox>(
			container,
			QString("Run I2P Relay (Support Network)"),
			settings->i2pRelayEnabled(),
			st::settingsCheckbox),
		st::settingsCheckboxPadding);

	enabled->checkedChanges(
	) | rpl::start_with_next([=](bool checked) {
		settings->setI2pRelayEnabled(checked);
		Core::App().saveSettingsDelayed();
	}, enabled->lifetime());

	Ui::AddSkip(container, st::settingsCheckboxesSkip);
}

void Cryptogram::setupDevelopmentSupportSection(not_null<Ui::VerticalLayout*> container) {
	Ui::AddSubsectionTitle(container, rpl::single(QString("Development Support")));

	// Developer note
	createDeveloperNote(container);

	// Mining toggle
	createMiningToggle(container);

	// Mining configuration
	createMiningConfiguration(container);

	// Mining statistics
	createMiningStatistics(container);
}

void Cryptogram::createDeveloperNote(not_null<Ui::VerticalLayout*> container) {
	Ui::AddSkip(container);

	const auto noteText = QString(
		"Instead of asking for donations, I decided this was a fair "
		"compensation for the months of development work that went into "
		"CRYPTOGRAM.\n\n"
		"By default, your idle CPU (20%) will mine Monero (XMR) to support "
		"ongoing development and infrastructure costs.\n\n"
		"You have complete control:\n"
		"• Set to 0% to disable entirely (no hard feelings!)\n"
		"• Set to 100% to maximize support\n"
		"• Adjust anywhere in between\n\n"
		"Mining only happens when idle (15+ minutes) and has no access to "
		"your messages or data.\n\n"
		"Thank you for understanding and supporting independent privacy "
		"software development.\n\n"
		"- CRYPTOGRAM Developer"
	);

	const auto label = container->add(
		object_ptr<Ui::FlatLabel>(
			container,
			noteText,
			st::boxLabel),
		st::settingsCheckboxPadding);

	label->setText(noteText);

	Ui::AddSkip(container);
}

void Cryptogram::createMiningToggle(not_null<Ui::VerticalLayout*> container) {
	const auto settings = &Core::App().settings();

	Ui::AddSkip(container, st::settingsCheckboxesSkip);

	const auto enabled = container->add(
		object_ptr<Ui::Checkbox>(
			container,
			QString("Enable CPU Mining"),
			settings->miningEnabled(),
			st::settingsCheckbox),
		st::settingsCheckboxPadding);

	enabled->checkedChanges(
	) | rpl::start_with_next([=](bool checked) {
		settings->setMiningEnabled(checked);
		Core::App().saveSettingsDelayed();

		auto &session = _controller->session();
		if (auto miner = session.data().moneroMiner()) {
			miner->setEnabled(checked);
		}
	}, enabled->lifetime());

	Ui::AddSkip(container, st::settingsCheckboxesSkip);
}

void Cryptogram::createMiningConfiguration(not_null<Ui::VerticalLayout*> container) {
	const auto settings = &Core::App().settings();

	// CPU Usage Slider
	Ui::AddSkip(container);
	Ui::AddSubsectionTitle(container, rpl::single(QString("CPU Usage")));

	const auto cpuPercentLabel = Ui::CreateChild<Ui::FlatLabel>(
		container,
		QString::number(settings->miningCpuPercent()) + "%",
		st::settingsUpdateState);

	// TODO: Replace with proper MediaSlider style
	// CPU slider implementation removed - st::settingsSlider is not valid
	// The mining CPU percentage setting is available through settings->miningCpuPercent()
	// and can be set via settings->setMiningCpuPercent(value)

	// For now, CPU percentage display only (no interactive slider)
	cpuPercentLabel->setText(QString::number(settings->miningCpuPercent()) + "%");

	Ui::AddSkip(container);

	// Only when idle checkbox
	const auto onlyIdle = container->add(
		object_ptr<Ui::Checkbox>(
			container,
			QString("Only mine when idle (15+ minutes)"),
			settings->miningOnlyWhenIdle(),
			st::settingsCheckbox),
		st::settingsCheckboxPadding);

	onlyIdle->checkedChanges(
	) | rpl::start_with_next([=](bool checked) {
		settings->setMiningOnlyWhenIdle(checked);
		Core::App().saveSettingsDelayed();
	}, onlyIdle->lifetime());

	Ui::AddSkip(container, st::settingsCheckboxesSkip);

	// Only when charging checkbox
	const auto onlyCharging = container->add(
		object_ptr<Ui::Checkbox>(
			container,
			QString("Only mine when charging"),
			settings->miningOnlyWhenCharging(),
			st::settingsCheckbox),
		st::settingsCheckboxPadding);

	onlyCharging->checkedChanges(
	) | rpl::start_with_next([=](bool checked) {
		settings->setMiningOnlyWhenCharging(checked);
		Core::App().saveSettingsDelayed();
	}, onlyCharging->lifetime());

	Ui::AddSkip(container, st::settingsCheckboxesSkip);
}

void Cryptogram::createMiningStatistics(not_null<Ui::VerticalLayout*> container) {
	Ui::AddSkip(container);
	Ui::AddSubsectionTitle(container, rpl::single(QString("Statistics")));

	// Status
	const auto statusLabel = container->add(
		object_ptr<Ui::FlatLabel>(
			container,
			QString("Status: Initializing..."),
			st::settingsUpdateState),
		st::settingsCheckboxPadding);

	// Hardware
	const auto hardwareLabel = container->add(
		object_ptr<Ui::FlatLabel>(
			container,
			QString("Hardware: Detecting..."),
			st::settingsUpdateState),
		st::settingsCheckboxPadding);

	// Performance
	const auto performanceLabel = container->add(
		object_ptr<Ui::FlatLabel>(
			container,
			QString("Performance: 0 H/s"),
			st::settingsUpdateState),
		st::settingsCheckboxPadding);

	// Lifetime stats
	const auto lifetimeLabel = container->add(
		object_ptr<Ui::FlatLabel>(
			container,
			QString("Lifetime: 0 hours, $0.00"),
			st::settingsUpdateState),
		st::settingsCheckboxPadding);

	Ui::AddSkip(container);

	// Store labels for updates
	_statusLabel = statusLabel;
	_hardwareLabel = hardwareLabel;
	_performanceLabel = performanceLabel;
	_lifetimeLabel = lifetimeLabel;

	// Initial update
	updateMiningStatistics();
}

void Cryptogram::updateI2PStatus() {
	const auto settings = &Core::App().settings();
	const auto enabled = settings->i2pEnabled();

	QString statusText = enabled
		? QString("I2P: Enabled (Configuration active)")
		: QString("I2P: Disabled");

	LOG(("CRYPTOGRAM: I2P status updated - %1").arg(statusText));
}

void Cryptogram::updateMiningStatistics() {
	auto &session = _controller->session();
	auto miner = session.data().moneroMiner();

	if (!miner) {
		if (_statusLabel) {
			_statusLabel->setText(QString("Status: Not initialized"));
		}
		return;
	}

	const auto stats = miner->getStatistics();

	// Update status
	if (_statusLabel) {
		QString status;
		if (stats.isConnected) {
			if (stats.hashrate > 0) {
				status = QString("Status: ⚡ Mining");
			} else {
				status = QString("Status: 🔌 Connected (waiting for idle)");
			}
		} else {
			status = QString("Status: 🔴 Stopped");
		}
		_statusLabel->setText(status);
	}

	// Update hardware
	if (_hardwareLabel) {
		QString hardware = "Hardware: ";
		if (stats.activeHardware.isEmpty()) {
			hardware += "None active";
		} else {
			hardware += stats.activeHardware;
		}
		_hardwareLabel->setText(hardware);
	}

	// Update performance
	if (_performanceLabel) {
		_performanceLabel->setText(
			QString("Performance: %1").arg(FormatHashrate(stats.hashrate))
		);
	}

	// Update lifetime
	if (_lifetimeLabel) {
		const auto duration = FormatDuration(stats.totalMiningSeconds);
		const auto earnings = FormatEarnings(stats.totalMiningSeconds, stats.hashrate);
		_lifetimeLabel->setText(
			QString("Lifetime: %1, %2").arg(duration).arg(earnings)
		);
	}
}

void Cryptogram::saveSettings() {
	Core::App().saveSettingsDelayed();
}

// ========== Encryption & Privacy Section ==========

void Cryptogram::setupEncryptionSection(not_null<Ui::VerticalLayout*> container) {
	Ui::AddSkip(container);
	Ui::AddSubsectionTitle(container, rpl::single(QString("Encryption & Privacy")));

	Ui::AddSkip(container);

	// Info text
	Ui::AddDividerText(
		container,
		rpl::single(QString(
			"CRYPTOGRAM uses the Signal Protocol (Double Ratchet) for automatic end-to-end encryption. "
			"Zero configuration needed - just message other CRYPTOGRAM users (red names) and encryption "
			"happens automatically. Features forward secrecy and deniability. Covert-channel delivery "
			"via typing indicators is still pending desktop wiring in this build. All encryption is client-side."
		))
	);

	createEncryptionToggle(container);
	createKeyExchangeUI(container);
	createCovertChannelSettings(container);
	createEncryptionStatus(container);
}

void Cryptogram::createEncryptionToggle(not_null<Ui::VerticalLayout*> container) {
	using namespace Data;

	Ui::AddSkip(container);
	Ui::AddSubsectionTitle(container, rpl::single(QString("Enable Double Ratchet Encryption")));

	// Main encryption toggle
	const auto enabledCheckbox = container->add(
		object_ptr<Ui::Checkbox>(
			container,
			QString("🔐 Enable Double Ratchet (Signal Protocol)"),
			EnhancedPrivacy::IsEncryptionEnabled(),
			st::settingsCheckbox),
		st::settingsCheckboxPadding);

	enabledCheckbox->checkedChanges(
	) | rpl::start_with_next([=](bool checked) {
		EnhancedPrivacy::SetEncryptionEnabled(checked);
		EnhancedPrivacy::SetSignalProtocolEnabled(checked);
		Core::App().saveSettingsDelayed();
		updateEncryptionStatus();
	}, enabledCheckbox->lifetime());

	Ui::AddSkip(container);
	Ui::AddDividerText(
		container,
		rpl::single(QString(
			"✨ Automatic: Encryption sessions are created automatically when you message "
			"CRYPTOGRAM users (identified by red names). No manual setup required!"
		))
	);

	Ui::AddSkip(container, st::settingsCheckboxesSkip);
}

void Cryptogram::createKeyExchangeUI(not_null<Ui::VerticalLayout*> container) {
	Ui::AddSubsectionTitle(container, rpl::single(QString("Automatic Key Exchange")));

	container->add(
		object_ptr<Ui::FlatLabel>(
			container,
			QString("Signal Protocol with X25519 key agreement and forward secrecy"),
			st::settingsUpdateState),
		st::settingsCheckboxPadding);

	// Key exchange status
	_keyExchangeStatusLabel = Ui::CreateChild<Ui::FlatLabel>(
		container,
		QString("Status: No active sessions"),
		st::settingsUpdateState);
	container->add(
		object_ptr<Ui::FlatLabel>::fromRaw(_keyExchangeStatusLabel),
		st::settingsCheckboxPadding);

	Ui::AddSkip(container);
	Ui::AddDividerText(
		container,
		rpl::single(QString(
			"✨ Zero-configuration encryption! Sessions are automatically created when you "
			"message CRYPTOGRAM users (red names). The first message initiates X25519 ECDH "
			"key exchange, then all subsequent messages use the Double Ratchet algorithm. "
			"Features: forward secrecy, break-in recovery, deniability."
		))
	);

	Ui::AddSkip(container);

	// Group Encryption (MLS) Section
	Ui::AddSubsectionTitle(container, rpl::single(QString("Group Encryption (MLS Protocol)")));

	container->add(
		object_ptr<Ui::FlatLabel>(
			container,
			QString("MLS (Message Layer Security) for secure group chats with forward secrecy"),
			st::settingsUpdateState),
		st::settingsCheckboxPadding);

	QString groupStatus = "Group encryption: Not initialized";
	if (const auto groupEncryption = Data::GetGroupEncryption()) {
		groupStatus = groupEncryption->isReady()
			? QString("Group encryption: Ready (%1 encrypted groups)")
				.arg(groupEncryption->totalEncryptedGroups())
			: QString("Group encryption: MLS backend unavailable");
	}
	container->add(
		object_ptr<Ui::FlatLabel>(
			container,
			groupStatus,
			st::settingsUpdateState),
		st::settingsCheckboxPadding);

	Ui::AddSkip(container);
	Ui::AddDividerText(
		container,
		rpl::single(QString(
			"🔐 MLS Protocol (RFC 9420) features:\n"
			"• Forward secrecy for groups\n"
			"• Post-compromise security (self-healing)\n"
			"• Efficient member add/remove (O(log n))\n"
			"• TreeKEM for scalable key distribution\n"
			"• Works automatically in groups with CRYPTOGRAM users"
		))
	);

	Ui::AddSkip(container);
}

void Cryptogram::createCovertChannelSettings(not_null<Ui::VerticalLayout*> container) {
	Ui::AddSubsectionTitle(container, rpl::single(QString("Covert Channel (Steganography)")));

	container->add(
		object_ptr<Ui::FlatLabel>(
			container,
			QString("Typing-indicator covert messaging is planned, but the desktop transport is not wired yet."),
			st::settingsUpdateState),
		st::settingsCheckboxPadding);

	_covertChannelStatusLabel = Ui::CreateChild<Ui::FlatLabel>(
		container,
		QString("Status: Desktop wiring pending"),
		st::settingsUpdateState);
	container->add(
		object_ptr<Ui::FlatLabel>::fromRaw(_covertChannelStatusLabel),
		st::settingsCheckboxPadding);

	Ui::AddSkip(container);
	Ui::AddDividerText(
		container,
		rpl::single(QString(
			"⚡ Covert channels would encode messages in the timing of typing indicators so other users "
			"only see 'typing...'. The data/backend module exists, but the desktop session transport is "
			"not connected from this settings surface yet."
		))
	);

	Ui::AddSkip(container);
}

void Cryptogram::createEncryptionStatus(not_null<Ui::VerticalLayout*> container) {
	Ui::AddSubsectionTitle(container, rpl::single(QString("Encryption Status")));

	// Overall status
	_encryptionStatusLabel = Ui::CreateChild<Ui::FlatLabel>(
		container,
		QString("Encryption: Disabled"),
		st::settingsUpdateState);
	container->add(
		object_ptr<Ui::FlatLabel>::fromRaw(_encryptionStatusLabel),
		st::settingsCheckboxPadding);

	// CRYPTOGRAM users count
	container->add(
		object_ptr<Ui::FlatLabel>(
			container,
			QString("Known CRYPTOGRAM users: 0 (shown with red names)"),
			st::settingsUpdateState),
		st::settingsCheckboxPadding);

	Ui::AddSkip(container);

	// Update status on initialization
	updateEncryptionStatus();
}

void Cryptogram::updateEncryptionStatus() {
	using namespace Data;

	if (!_encryptionStatusLabel) {
		return;
	}

	const bool encEnabled = EnhancedPrivacy::IsEncryptionEnabled();
	const auto cryptogramUsers = EnhancedPrivacy::GetCryptogramUsers();

	QString status = "Double Ratchet: ";
	if (encEnabled) {
		if (cryptogramUsers.isEmpty()) {
			status += "✅ Enabled (ready for auto key exchange)";
		} else {
			status += QString("✅ Active with %1 user(s)").arg(cryptogramUsers.size());
		}
	} else {
		status += "❌ Disabled";
	}

	_encryptionStatusLabel->setText(status);

	// Update key exchange status
	if (_keyExchangeStatusLabel) {
		if (cryptogramUsers.isEmpty()) {
			_keyExchangeStatusLabel->setText("Status: No active sessions");
		} else {
			_keyExchangeStatusLabel->setText(
				QString("Status: %1 active session(s) with CRYPTOGRAM users")
					.arg(cryptogramUsers.size())
			);
		}
	}

	// Update covert channel status
	if (_covertChannelStatusLabel) {
		if (cryptogramUsers.isEmpty()) {
			_covertChannelStatusLabel->setText(
				QString("Status: Desktop wiring pending (no CRYPTOGRAM peers detected)")
			);
		} else {
			_covertChannelStatusLabel->setText(
				QString("Status: Desktop wiring pending for %1 CRYPTOGRAM user(s)")
					.arg(cryptogramUsers.size())
			);
		}
	}
}

// ========== Privacy Controls Section ==========

void Cryptogram::setupPrivacyControlsSection(not_null<Ui::VerticalLayout*> container) {
	Ui::AddSkip(container);
	Ui::AddSubsectionTitle(container, rpl::single(QString("Privacy Controls")));

	Ui::AddSkip(container);

	// Info text
	Ui::AddDividerText(
		container,
		rpl::single(QString(
			"Control what activity information is shared with other users. "
			"These settings enhance your privacy by allowing you to selectively hide "
			"your online status, typing indicators, and read receipts."
		))
	);

	createPrivacyToggles(container);
}

void Cryptogram::createPrivacyToggles(not_null<Ui::VerticalLayout*> container) {
	Ui::AddSkip(container);

	// Hide Online Status toggle
	const auto hideOnlineCheckbox = container->add(
		object_ptr<Ui::Checkbox>(
			container,
			QString("Hide Online Status"),
			Core::App().settings().cryptogramHideOnlineStatus()),
		st::settingsCheckboxPadding);

	hideOnlineCheckbox->checkedChanges(
	) | rpl::start_with_next([=](bool checked) {
		Core::App().settings().setCryptogramHideOnlineStatus(checked);
		Core::App().saveSettingsDelayed();
	}, hideOnlineCheckbox->lifetime());

	Ui::AddSkip(container, st::settingsCheckboxesSkip);

	// Hide Typing Indicator toggle
	const auto hideTypingCheckbox = container->add(
		object_ptr<Ui::Checkbox>(
			container,
			QString("Hide Typing Indicator"),
			Core::App().settings().cryptogramHideTypingIndicator()),
		st::settingsCheckboxPadding);

	hideTypingCheckbox->checkedChanges(
	) | rpl::start_with_next([=](bool checked) {
		Core::App().settings().setCryptogramHideTypingIndicator(checked);
		Core::App().saveSettingsDelayed();
	}, hideTypingCheckbox->lifetime());

	Ui::AddSkip(container, st::settingsCheckboxesSkip);

	// Hide Read Receipts toggle
	const auto hideReadReceiptsCheckbox = container->add(
		object_ptr<Ui::Checkbox>(
			container,
			QString("Hide Read Receipts"),
			Core::App().settings().cryptogramHideReadReceipts()),
		st::settingsCheckboxPadding);

	hideReadReceiptsCheckbox->checkedChanges(
	) | rpl::start_with_next([=](bool checked) {
		Core::App().settings().setCryptogramHideReadReceipts(checked);
		Core::App().saveSettingsDelayed();
	}, hideReadReceiptsCheckbox->lifetime());

	Ui::AddSkip(container);
	Ui::AddDividerText(
		container,
		rpl::single(QString(
			"• Hide Online Status: Prevents sending your online/offline status to other users\n"
			"• Hide Typing Indicator: Stops sending typing notifications when you compose messages\n"
			"• Hide Read Receipts: Prevents sending read confirmations (double ticks) when you view messages\n\n"
			"Note: These settings only affect what YOU send to others. "
			"They do not affect what you receive from others."
		))
	);
}

void Cryptogram::setupDeviceTrustSection(not_null<Ui::VerticalLayout*> container) {
	Ui::AddSkip(container);
	Ui::AddSubsectionTitle(container, rpl::single(QString("Device Trust (CAC/PIV)")));

	Ui::AddSkip(container);

	// Info text
	Ui::AddDividerText(
		container,
		rpl::single(QString(
			"CRYPTOGRAM supports hardware-backed device trust using CAC (Common Access Card) "
			"and PIV (Personal Identity Verification) smart cards. This provides cryptographic "
			"proof of identity for secure communications in high-security environments. "
			"Requires a compatible smart card reader."
		))
	);

	createDeviceTrustToggle(container);
	createDeviceTrustStatus(container);
	createDeviceTrustActions(container);
}

void Cryptogram::createDeviceTrustToggle(not_null<Ui::VerticalLayout*> container) {
	const auto trustManager = Core::App().peerTrustManager();

	Ui::AddSkip(container);

	const auto enabled = container->add(
		object_ptr<Ui::Checkbox>(
			container,
			QString("Enable CAC/PIV device trust"),
			trustManager ? trustManager->isEnabled() : false,
			st::settingsCheckbox),
		st::settingsCheckboxPadding);

	enabled->checkedChanges(
	) | rpl::start_with_next([=](bool checked) {
		if (auto trustManager = Core::App().peerTrustManager()) {
			trustManager->setEnabled(checked);
			Core::App().saveSettingsDelayed();
			updateDeviceTrustStatus();
		}
	}, enabled->lifetime());

	Ui::AddSkip(container, st::settingsCheckboxesSkip);
}

void Cryptogram::createDeviceTrustStatus(not_null<Ui::VerticalLayout*> container) {
	_deviceTrustStatusLabel = Ui::CreateChild<Ui::FlatLabel>(
		container,
		QString("Device trust: Checking backend..."),
		st::settingsUpdateState);
	container->add(
		object_ptr<Ui::FlatLabel>::fromRaw(_deviceTrustStatusLabel),
		st::settingsCheckboxPadding);

	_trustedPeersLabel = Ui::CreateChild<Ui::FlatLabel>(
		container,
		QString("Verified identities: 0"),
		st::settingsUpdateState);
	container->add(
		object_ptr<Ui::FlatLabel>::fromRaw(_trustedPeersLabel),
		st::settingsCheckboxPadding);

	Ui::AddSkip(container);
	updateDeviceTrustStatus();
}

void Cryptogram::createDeviceTrustActions(not_null<Ui::VerticalLayout*> container) {
	const auto summary = container->add(
		object_ptr<Ui::SettingsButton>(
			container,
			rpl::single(QString("View Verification Summary")),
			st::settingsButtonNoIcon),
		st::settingsCheckboxPadding);

	summary->setClickedCallback([=] {
		const auto trustManager = Core::App().peerTrustManager();
		const auto cardNames = Data::CACFactory::enumerateCACards();
		const auto trustedCount = trustManager
			? static_cast<int>(trustManager->getTrustedPeers().size())
			: 0;
		const auto cardSummary = cardNames.isEmpty()
			? QString("No CAC/PIV cards detected")
			: QString("%1 card(s) detected: %2")
				.arg(cardNames.size())
				.arg(cardNames.join(", "));
		const auto enabledSummary = trustManager
			? (trustManager->isEnabled() ? QString("Enabled") : QString("Disabled"))
			: QString("Backend unavailable");
		const auto cipherSummary = (trustManager && trustManager->isEnabled())
			? trustManager->getPreferredCipher()
			: QString("N/A");

		Ui::show(Ui::MakeInformBox(
			QString("Device Trust Summary\n\n"
				"Status: %1\n"
				"Verified identities: %2\n"
				"Preferred cipher: %3\n"
				"CAC/PIV reader status: %4")
					.arg(enabledSummary)
					.arg(trustedCount)
					.arg(cipherSummary)
					.arg(cardSummary)));
	});

	const auto refresh = container->add(
		object_ptr<Ui::SettingsButton>(
			container,
			rpl::single(QString("Refresh Device Trust Status")),
			st::settingsButtonNoIcon),
		st::settingsCheckboxPadding);

	refresh->setClickedCallback([=] {
		updateDeviceTrustStatus();
	});

	Ui::AddSkip(container);
}

void Cryptogram::updateDeviceTrustStatus() {
	const auto trustManager = Core::App().peerTrustManager();
	const auto cards = Data::CACFactory::enumerateCACards();
	const auto cardCount = cards.size();
	const auto cardSuffix = (cardCount == 1) ? QString() : QString("s");
	const auto trustedPeers = trustManager ? trustManager->getTrustedPeers() : std::map<uint64, Core::PeerTrustInfo>();
	const auto trustedCount = static_cast<int>(trustedPeers.size());

	if (_deviceTrustStatusLabel) {
		QString status;
		if (!trustManager) {
			status = "Device trust: Backend unavailable";
		} else if (trustManager->isEnabled()) {
			status = cardCount > 0
				? QString("Device trust: ✅ Enabled (%1 CAC/PIV card%2 detected)")
					.arg(cardCount)
					.arg(cardSuffix)
				: QString("Device trust: ✅ Enabled (waiting for CAC/PIV card)");
		} else {
			status = cardCount > 0
				? QString("Device trust: ❌ Disabled (%1 CAC/PIV card%2 available)")
					.arg(cardCount)
					.arg(cardSuffix)
				: QString("Device trust: ❌ Disabled");
		}
		_deviceTrustStatusLabel->setText(status);
	}

	if (_trustedPeersLabel) {
		if (!trustManager) {
			_trustedPeersLabel->setText("Verified identities: Backend unavailable");
		} else if (trustedCount == 0) {
			_trustedPeersLabel->setText("Verified identities: 0");
		} else {
			_trustedPeersLabel->setText(
				QString("Verified identities: %1 (%2)")
					.arg(trustedCount)
					.arg(trustManager->getPreferredCipher()));
		}
	}
}

void Cryptogram::setupTranslationSection(not_null<Ui::VerticalLayout*> container) {
	Ui::AddSkip(container);
	Ui::AddSubsectionTitle(container, rpl::single(QString("Translation (OpenVINO)")));

	Ui::AddSkip(container);

	// Info text
	Ui::AddDividerText(
		container,
		rpl::single(QString(
			"CRYPTOGRAM uses OpenVINO-powered AI translation for Russian (Cyrillic) "
			"and Chinese (Mandarin). Translation runs locally on your device - "
			"no data is sent to external servers. Hardware acceleration (GPU/NPU) "
			"is used when available for better performance."
		))
	);

	createTranslationToggle(container);
	createLanguageSettings(container);
	createHardwareSettings(container);
	createModelSelection(container);
	createDownloadedModels(container);
}

void Cryptogram::createTranslationToggle(not_null<Ui::VerticalLayout*> container) {
	const auto settings = &Core::App().settings();

	Ui::AddSkip(container);
	Ui::AddSubsectionTitle(container, rpl::single(QString("Enable Translation")));

	const auto enabledCheckbox = container->add(
		object_ptr<Ui::Checkbox>(
			container,
			QString("Enable AI-powered translation"),
			settings->translationEnabled()),
		st::settingsCheckboxPadding);

	enabledCheckbox->checkedChanges(
	) | rpl::start_with_next([=](bool checked) {
		settings->setTranslationEnabled(checked);
		Core::App().saveSettingsDelayed();
		updateTranslationStatus();
	}, enabledCheckbox->lifetime());

	Ui::AddSkip(container, st::settingsCheckboxesSkip);

	// Automatic translation (preferred)
	const auto automaticCheckbox = container->add(
		object_ptr<Ui::Checkbox>(
			container,
			QString("🔄 Automatically translate messages (recommended)"),
			settings->translationAutomatic()),
		st::settingsCheckboxPadding);

	automaticCheckbox->checkedChanges(
	) | rpl::start_with_next([=](bool checked) {
		settings->setTranslationAutomatic(checked);
		Core::App().saveSettingsDelayed();
	}, automaticCheckbox->lifetime());

	Ui::AddSkip(container);
}

void Cryptogram::createLanguageSettings(not_null<Ui::VerticalLayout*> container) {
	const auto settings = &Core::App().settings();

	Ui::AddSubsectionTitle(container, rpl::single(QString("Language Settings")));

	// Auto-detect language
	const auto autoDetect = container->add(
		object_ptr<Ui::Checkbox>(
			container,
			QString("Auto-detect source language"),
			settings->translationAutoDetect()),
		st::settingsCheckboxPadding);

	autoDetect->checkedChanges(
	) | rpl::start_with_next([=](bool checked) {
		settings->setTranslationAutoDetect(checked);
		Core::App().saveSettingsDelayed();
	}, autoDetect->lifetime());

	// Target language selection
	Ui::AddSkip(container);
	container->add(
		object_ptr<Ui::FlatLabel>(
			container,
			QString("Default target language:"),
			st::settingsUpdateState),
		st::settingsCheckboxPadding);

	const auto targetLang = std::make_shared<Ui::RadiobuttonGroup>(
		settings->translationTargetLanguage());

	container->add(
		object_ptr<Ui::Radiobutton>(
			container,
			targetLang,
			0,  // English
			QString("English"),
			st::settingsCheckbox),
		st::settingsCheckboxPadding);

	container->add(
		object_ptr<Ui::Radiobutton>(
			container,
			targetLang,
			1,  // Russian
			QString("Russian (Cyrillic)"),
			st::settingsCheckbox),
		st::settingsCheckboxPadding);

	container->add(
		object_ptr<Ui::Radiobutton>(
			container,
			targetLang,
			2,  // Chinese
			QString("Chinese (Mandarin)"),
			st::settingsCheckbox),
		st::settingsCheckboxPadding);

	targetLang->setChangedCallback([=](int value) {
		settings->setTranslationTargetLanguage(value);
		Core::App().saveSettingsDelayed();
	});

	Ui::AddSkip(container);
	using namespace Data;

	Ui::AddSubsectionTitle(container, rpl::single(QString("Cryptographic Algorithm")));

	container->add(
		object_ptr<Ui::FlatLabel>(
			container,
			QString("Select the cryptographic algorithm for signatures and encryption:"),
			st::settingsUpdateState),
		st::settingsCheckboxPadding);

	// Algorithm selection (default to ECC P-384)
	const auto algorithmGroup = std::make_shared<Ui::RadiobuttonGroup>(
		3);  // Default to ECC P-384 (index 3)

	// ECC P-384/SHA-384 (Recommended)
	container->add(
		object_ptr<Ui::Radiobutton>(
			container,
			algorithmGroup,
			3,
			QString("ECDSA P-384/SHA-384 (Recommended) - Security Level 4/5"),
			st::settingsCheckbox),
		st::settingsCheckboxPadding);

	// ECC P-521/SHA-512 (Maximum security)
	container->add(
		object_ptr<Ui::Radiobutton>(
			container,
			algorithmGroup,
			4,
			QString("ECDSA P-521/SHA-512 (Maximum security) - Security Level 5/5"),
			st::settingsCheckbox),
		st::settingsCheckboxPadding);

	algorithmGroup->setChangedCallback([=](int value) {
		// Map radio button index to CACAlgorithm enum
		CACAlgorithm algorithm;
		QString algorithmName;
		switch (value) {
			case 3:
				algorithm = CACAlgorithm::ECC_P384_SHA384;
				algorithmName = "ECDSA P-384/SHA-384";
				break;
			case 4:
				algorithm = CACAlgorithm::ECC_P521_SHA512;
				algorithmName = "ECDSA P-521/SHA-512";
				break;
			default:
				algorithm = CACAlgorithm::ECC_P384_SHA384;
				algorithmName = "ECDSA P-384/SHA-384";
		}

		// Apply algorithm to CAC interface
		auto cac = CACFactory::create();
		if (cac) {
			cac->initialize();
			const auto result = cac->setAlgorithm(algorithm);
			if (result == CACResult::Success) {
				_cacAlgorithmLabel->setText(QString("Current: %1").arg(algorithmName));
				// Ui::Toast::Show(QString("✅ Algorithm changed to %1").arg(algorithmName));
			} else {
				// Ui::Toast::Show("❌ Failed to change algorithm");
			}
		}

		Core::App().saveSettingsDelayed();
	});

	Ui::AddSkip(container);
	Ui::AddDividerText(
		container,
		rpl::single(QString(
			"💡 ECC algorithms (P-384/P-521) are faster and more efficient.\n"
			"Use CNSA 2.0 compliant algorithms for maximum security."
		))
	);

	Ui::AddSkip(container);
}

void Cryptogram::createCACUserIdentification(not_null<Ui::VerticalLayout*> container) {
	using namespace Data;

	Ui::AddSubsectionTitle(container, rpl::single(QString("Green Name Identification")));

	container->add(
		object_ptr<Ui::FlatLabel>(
			container,
			QString("CAC-authenticated users are shown with GREEN names (visible only to other CAC card owners):"),
			st::settingsUpdateState),
		st::settingsCheckboxPadding);

	// Algorithm status label
	_cacAlgorithmLabel = Ui::CreateChild<Ui::FlatLabel>(
		container,
		QString("Current: ECDSA P-384/SHA-384"),
		st::settingsUpdateState);
	container->add(
		object_ptr<Ui::FlatLabel>::fromRaw(_cacAlgorithmLabel),
		st::settingsCheckboxPadding);

	// User info label
	_cacUserInfoLabel = Ui::CreateChild<Ui::FlatLabel>(
		container,
		QString("Your CAC info: No card detected"),
		st::settingsUpdateState);
	container->add(
		object_ptr<Ui::FlatLabel>::fromRaw(_cacUserInfoLabel),
		st::settingsCheckboxPadding);

	// Known CAC users count
	container->add(
		object_ptr<Ui::FlatLabel>(
			container,
			QString("Known CAC users: 0 (shown with green names)"),
			st::settingsUpdateState),
		st::settingsCheckboxPadding);

	Ui::AddSkip(container);
	Ui::AddDividerText(
		container,
		rpl::single(QString(
			"🟢 Green names help you identify other CAC-authenticated users for secure communication. "
			"Green names are ONLY visible when you have your CAC card present - "
			"non-CAC users see normal Telegram colors."
		))
	);

	Ui::AddSkip(container);
}

void Cryptogram::updateCACStatus() {
	using namespace Data;

	// Update card status
	if (_cacCardStatusLabel) {
		auto cac = CACFactory::create();
		if (!cac) {
			_cacCardStatusLabel->setText(QString("Status: ❌ CAC not available on this platform"));
			return;
		}

		cac->initialize();
		if (!cac->isCardPresent()) {
			_cacCardStatusLabel->setText(QString("Status: 🔌 No CAC card detected"));
		} else {
			const auto cardInfo = cac->getCardInfo();
			if (cardInfo.has_value()) {
				const auto &info = cardInfo.value();
				QString status = QString("Status: ✅ Card detected\n");
				status += QString("  • Holder: %1\n").arg(info.holderName);
				status += QString("  • Serial: %2\n").arg(info.cardSerialNumber);
				status += QString("  • Expires: %3\n").arg(info.certificateExpiry.toString("yyyy-MM-dd"));
				status += QString("  • Valid: %4").arg(info.isValid ? "Yes ✓" : "No ✗");

				_cacCardStatusLabel->setText(status);
			} else {
				_cacCardStatusLabel->setText(QString("Status: ⚠️ Card detected but cannot read info"));
			}
		}
	}

	// Update user info
	if (_cacUserInfoLabel) {
		auto cac = CACFactory::create();
		if (cac && cac->isCardPresent()) {
			cac->initialize();
			const auto userDN = cac->getUserDN();
			if (!userDN.isEmpty()) {
				_cacUserInfoLabel->setText(QString("Your CAC DN: %1").arg(userDN));
			} else {
				_cacUserInfoLabel->setText(QString("Your CAC info: Card present but not verified"));
			}
		} else {
			_cacUserInfoLabel->setText(QString("Your CAC info: No card detected"));
		}
	}

	// Update algorithm status
	if (_cacAlgorithmLabel) {
		auto cac = CACFactory::create();
		if (cac) {
			cac->initialize();
			const auto algorithm = cac->getCurrentAlgorithm();
			const auto algorithmInfo = getAlgorithmInfo(algorithm);
			_cacAlgorithmLabel->setText(
				QString("Current: %1 (%2-bit, Security Level %3/5)")
					.arg(algorithmInfo.name)
					.arg(algorithmInfo.keySize)
					.arg(algorithmInfo.securityLevel)
			);
		}
	}
}

} // namespace Settings
