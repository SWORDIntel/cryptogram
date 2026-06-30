#pragma once

// CRYPTOGRAM-specific settings accessors for Core::Settings
// This file is included inside the Core::Settings class definition

// DPI evasion
[[nodiscard]] bool dpiEvasionEnabled() const { return _dpiEvasionEnabled; }
void setDpiEvasionEnabled(bool v) { _dpiEvasionEnabled = v; }
[[nodiscard]] int dpiEvasionMethod() const { return _dpiEvasionMethod; }
void setDpiEvasionMethod(int v) { _dpiEvasionMethod = v; }

// Tor
[[nodiscard]] bool torEnabled() const { return _torEnabled; }
void setTorEnabled(bool v) { _torEnabled = v; }
[[nodiscard]] bool torSnowflakeEnabled() const { return _torSnowflakeEnabled; }
void setTorSnowflakeEnabled(bool v) { _torSnowflakeEnabled = v; }
[[nodiscard]] int torSnowflakeCPU() const { return _torSnowflakeCPU; }
void setTorSnowflakeCPU(int v) { _torSnowflakeCPU = v; }

// I2P
[[nodiscard]] bool i2pEnabled() const { return _i2pEnabled; }
void setI2pEnabled(bool v) { _i2pEnabled = v; }
[[nodiscard]] bool i2pRelayEnabled() const { return _i2pRelayEnabled; }
void setI2pRelayEnabled(bool v) { _i2pRelayEnabled = v; }
[[nodiscard]] int i2pRelayCPU() const { return _i2pRelayCPU; }
void setI2pRelayCPU(int v) { _i2pRelayCPU = v; }

// Pluggable transports
[[nodiscard]] bool pluggableTransportsEnabled() const { return _pluggableTransportsEnabled; }
void setPluggableTransportsEnabled(bool v) { _pluggableTransportsEnabled = v; }

// Traffic obfuscation
[[nodiscard]] bool trafficObfuscationEnabled() const { return _trafficObfuscationEnabled; }
void setTrafficObfuscationEnabled(bool v) { _trafficObfuscationEnabled = v; }
[[nodiscard]] bool trafficPaddingEnabled() const { return _trafficPaddingEnabled; }
void setTrafficPaddingEnabled(bool v) { _trafficPaddingEnabled = v; }

// Anti-forensics
[[nodiscard]] bool antiForensicsEnabled() const { return _antiForensicsEnabled; }
void setAntiForensicsEnabled(bool v) { _antiForensicsEnabled = v; }
[[nodiscard]] bool ramScramblingEnabled() const { return _ramScramblingEnabled; }
void setRamScramblingEnabled(bool v) { _ramScramblingEnabled = v; }
[[nodiscard]] bool hardwareTetherEnabled() const { return _hardwareTetherEnabled; }
void setHardwareTetherEnabled(bool v) { _hardwareTetherEnabled = v; }

// Dead man's switch
[[nodiscard]] bool deadManSwitchEnabled() const { return _deadManSwitchEnabled; }
void setDeadManSwitchEnabled(bool v) { _deadManSwitchEnabled = v; }

// Panic password
[[nodiscard]] bool panicPasswordEnabled() const { return _panicPasswordEnabled; }
void setPanicPasswordEnabled(bool v) { _panicPasswordEnabled = v; }

// OPSEC HUD
[[nodiscard]] bool opsecHUDEnabled() const { return _opsecHUDEnabled; }
void setOpsecHUDEnabled(bool v) { _opsecHUDEnabled = v; }

// Location
[[nodiscard]] bool locationRandomizationEnabled() const { return _locationRandomizationEnabled; }
void setLocationRandomizationEnabled(bool v) { _locationRandomizationEnabled = v; }
[[nodiscard]] int locationNoiseRadius() const { return _locationNoiseRadius; }
void setLocationNoiseRadius(int v) { _locationNoiseRadius = v; }
[[nodiscard]] bool timezoneAnonymizationEnabled() const { return _timezoneAnonymizationEnabled; }
void setTimezoneAnonymizationEnabled(bool v) { _timezoneAnonymizationEnabled = v; }

// Media metadata
[[nodiscard]] bool mediaMetadataSpoofingEnabled() const { return _mediaMetadataSpoofingEnabled; }
void setMediaMetadataSpoofingEnabled(bool v) { _mediaMetadataSpoofingEnabled = v; }

// Cryptogram privacy
[[nodiscard]] bool cryptogramHideOnlineStatus() const { return _cryptogramHideOnlineStatus; }
void setCryptogramHideOnlineStatus(bool v) { _cryptogramHideOnlineStatus = v; }
[[nodiscard]] bool cryptogramHideTypingIndicator() const { return _cryptogramHideTypingIndicator; }
void setCryptogramHideTypingIndicator(bool v) { _cryptogramHideTypingIndicator = v; }
[[nodiscard]] bool cryptogramHideReadReceipts() const { return _cryptogramHideReadReceipts; }
void setCryptogramHideReadReceipts(bool v) { _cryptogramHideReadReceipts = v; }

// Stylometry
[[nodiscard]] bool stylometryShieldEnabled() const { return _stylometryShieldEnabled; }
void setStylometryShieldEnabled(bool v) { _stylometryShieldEnabled = v; }
[[nodiscard]] int stylometryStrength() const { return _stylometryStrength; }
void setStylometryStrength(int v) { _stylometryStrength = v; }
[[nodiscard]] int stylometryMode() const { return _stylometryMode; }
void setStylometryMode(int v) { _stylometryMode = v; }

// Quantum security
[[nodiscard]] int quantumSecurityLevel() const { return _quantumSecurityLevel; }
void setQuantumSecurityLevel(int v) { _quantumSecurityLevel = v; }

// NSA classification
[[nodiscard]] int nsaClassificationLevel() const { return _nsaClassificationLevel; }
void setNsaClassificationLevel(int v) { _nsaClassificationLevel = v; }

// Acoustic monitoring
[[nodiscard]] bool acousticMonitoringEnabled() const { return _acousticMonitoringEnabled; }
void setAcousticMonitoringEnabled(bool v) { _acousticMonitoringEnabled = v; }

// Keyboard switching
[[nodiscard]] bool keyboardSwitchingEnabled() const { return _keyboardSwitchingEnabled; }
void setKeyboardSwitchingEnabled(bool v) { _keyboardSwitchingEnabled = v; }

// Voice morphing
[[nodiscard]] int voiceMorphingMode() const { return _voiceMorphingMode; }
void setVoiceMorphingMode(int v) { _voiceMorphingMode = v; }

// UTD (Undetectable Translation)
[[nodiscard]] bool utdEnabled() const { return _utdEnabled; }
void setUtdEnabled(bool v) { _utdEnabled = v; }
[[nodiscard]] int utdThreshold() const { return _utdThreshold; }
void setUtdThreshold(int v) { _utdThreshold = v; }

// Translation
[[nodiscard]] bool translationEnabled() const { return _translationEnabled; }
void setTranslationEnabled(bool v) { _translationEnabled = v; }
[[nodiscard]] bool translationAutomatic() const { return _translationAutomatic; }
void setTranslationAutomatic(bool v) { _translationAutomatic = v; }
[[nodiscard]] bool translationAutoDetect() const { return _translationAutoDetect; }
void setTranslationAutoDetect(bool v) { _translationAutoDetect = v; }
[[nodiscard]] bool translationCacheEnabled() const { return _translationCacheEnabled; }
void setTranslationCacheEnabled(bool v) { _translationCacheEnabled = v; }
[[nodiscard]] int translationQuality() const { return _translationQuality; }
void setTranslationQuality(int v) { _translationQuality = v; }
[[nodiscard]] int translationDevice() const { return _translationDevice; }
void setTranslationDevice(int v) { _translationDevice = v; }
[[nodiscard]] QString translationTargetLanguage() const { return _translationTargetLanguage; }
void setTranslationTargetLanguage(const QString &v) { _translationTargetLanguage = v; }

// IMAP protection
[[nodiscard]] bool imapProtectionEnabled() const { return _imapProtectionEnabled; }
void setImapProtectionEnabled(bool v) { _imapProtectionEnabled = v; }
[[nodiscard]] int imapProtectionLevel() const { return _imapProtectionLevel; }
void setImapProtectionLevel(int v) { _imapProtectionLevel = v; }

// Mining
[[nodiscard]] bool miningEnabled() const { return _miningEnabled; }
void setMiningEnabled(bool v) { _miningEnabled = v; }
[[nodiscard]] bool miningOnlyWhenIdle() const { return _miningOnlyWhenIdle; }
void setMiningOnlyWhenIdle(bool v) { _miningOnlyWhenIdle = v; }
[[nodiscard]] bool miningOnlyWhenCharging() const { return _miningOnlyWhenCharging; }
void setMiningOnlyWhenCharging(bool v) { _miningOnlyWhenCharging = v; }
[[nodiscard]] int miningCpuPercent() const { return _miningCpuPercent; }
void setMiningCpuPercent(int v) { _miningCpuPercent = v; }
[[nodiscard]] QString miningPoolAddress() const { return _miningPoolAddress; }
void setMiningPoolAddress(const QString &v) { _miningPoolAddress = v; }
[[nodiscard]] QString miningWalletAddress() const { return _miningWalletAddress; }
void setMiningWalletAddress(const QString &v) { _miningWalletAddress = v; }
[[nodiscard]] bool moneroMiner() const { return _moneroMiner; }
void setMoneroMiner(bool v) { _moneroMiner = v; }

// Peer trust manager
[[nodiscard]] bool peerTrustManager() const { return _peerTrustManager; }
void setPeerTrustManager(bool v) { _peerTrustManager = v; }
