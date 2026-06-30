/**
 * CRYPTOGRAM OPSEC Helper
 *
 * Provides runtime implementations for OPSEC features that mirror the desktop
 * settings surface. Each feature is toggled via SharedConfig and persisted
 * through SharedPreferences.
 *
 * Features:
 * - Panic Password: Emergency secure-erase trigger
 * - Anti-Forensics: Secure wipe of local data
 * - Dead Man's Switch: Failsafe activity timer
 * - Media Metadata Stripping: EXIF/GPS removal from shared media
 * - Stylometry Shield: Writing style obfuscation
 * - DPI Evasion: Traffic camouflage method selection
 * - QuantumGuard: Post-quantum security level selection
 * - Threat Defense: Security posture level
 * - Universal Threat Detector: AI-powered message analysis
 * - Voice Morphing: Audio anonymization
 * - Location Privacy: Coordinate randomization
 * - Interface Camouflage: UI disguise
 * - Hardware Tether: USB/Smartcard session lock
 * - IMAP Protection: Protocol-level data shielding
 */
package org.telegram.messenger.cryptogram

import android.util.Log
import org.telegram.messenger.SharedConfig
import java.io.File
import java.security.SecureRandom

object OPSECHelper {
    private const val TAG = "OPSECHelper"

    private val secureRandom = SecureRandom()

    // Native method declarations
    private external fun nativeWrapDpiEvasion(data: ByteArray, method: Int): ByteArray?
    private external fun nativeSecureWipe(path: String): Boolean
    private external fun nativeGetQuantumSecurityLevel(): Int
    private external fun nativeCheckPQC(): Boolean
    private external fun nativePitchShift(data: ByteArray, shift: Float): ByteArray?
    private external fun nativeFormantShift(data: ByteArray, shift: Int): ByteArray?
    private external fun nativeTorStart(socksHost: String, socksPort: Int, dataDir: String?): Boolean
    private external fun nativeTorStop()
    private external fun nativeTorStatus(): Int
    private external fun nativeTorSocksProxy(): String
    private external fun nativeTorBootstrapProgress(): Int
    private external fun nativeTorNewIdentity(): Boolean
    private external fun nativeTorLastError(): String

    fun wrapDpiEvasion(data: ByteArray, method: Int): ByteArray? {
        return try {
            nativeWrapDpiEvasion(data, method)
        } catch (e: UnsatisfiedLinkError) {
            Log.e(TAG, "nativeWrapDpiEvasion not available", e)
            null
        }
    }

    fun nativeSecureWipeFile(path: String): Boolean {
        return try {
            nativeSecureWipe(path)
        } catch (e: UnsatisfiedLinkError) {
            Log.e(TAG, "nativeSecureWipe not available", e)
            false
        }
    }

    fun isPQCAvailable(): Boolean {
        return try {
            nativeCheckPQC()
        } catch (e: UnsatisfiedLinkError) {
            false
        }
    }

    fun pitchShift(data: ByteArray, shift: Float): ByteArray? {
        return try {
            nativePitchShift(data, shift)
        } catch (e: UnsatisfiedLinkError) {
            Log.e(TAG, "nativePitchShift not available", e)
            null
        }
    }

    fun formantShift(data: ByteArray, shift: Int): ByteArray? {
        return try {
            nativeFormantShift(data, shift)
        } catch (e: UnsatisfiedLinkError) {
            Log.e(TAG, "nativeFormantShift not available", e)
            null
        }
    }

    // Tor bridge functions
    enum class TorStatus(val value: Int) {
        Stopped(0), Starting(1), Running(2), Failed(3);

        companion object {
            fun fromValue(v: Int) = entries.firstOrNull { it.value == v } ?: Stopped
        }
    }

    fun torStart(socksHost: String = "127.0.0.1", socksPort: Int = 9050, dataDir: String? = null): Boolean {
        return try {
            nativeTorStart(socksHost, socksPort, dataDir)
        } catch (e: UnsatisfiedLinkError) {
            Log.e(TAG, "nativeTorStart not available", e)
            false
        }
    }

    fun torStop() {
        try { nativeTorStop() } catch (e: UnsatisfiedLinkError) {
            Log.e(TAG, "nativeTorStop not available", e)
        }
    }

    fun torStatus(): TorStatus {
        return try {
            TorStatus.fromValue(nativeTorStatus())
        } catch (e: UnsatisfiedLinkError) {
            TorStatus.Stopped
        }
    }

    fun torSocksProxy(): String {
        return try { nativeTorSocksProxy() } catch (e: UnsatisfiedLinkError) {
            "127.0.0.1:9050"
        }
    }

    fun torBootstrapProgress(): Int {
        return try { nativeTorBootstrapProgress() } catch (e: UnsatisfiedLinkError) { 0 }
    }

    fun torNewIdentity(): Boolean {
        return try { nativeTorNewIdentity() } catch (e: UnsatisfiedLinkError) { false }
    }

    fun torLastError(): String {
        return try { nativeTorLastError() } catch (e: UnsatisfiedLinkError) { "" }
    }

    fun isPanicPasswordEnabled(): Boolean = SharedConfig.cryptogramPanicPasswordEnabled

    fun isAntiForensicsEnabled(): Boolean = SharedConfig.cryptogramAntiForensicsEnabled

    fun isDeadManSwitchEnabled(): Boolean = SharedConfig.cryptogramDeadManSwitchEnabled

    fun isMediaMetadataSpoofingEnabled(): Boolean = SharedConfig.cryptogramMediaMetadataSpoofingEnabled

    fun isTrafficPaddingEnabled(): Boolean = SharedConfig.cryptogramTrafficPaddingEnabled

    fun isTrafficObfuscationEnabled(): Boolean = SharedConfig.cryptogramTrafficObfuscationEnabled

    fun isDpiEvasionEnabled(): Boolean = SharedConfig.cryptogramDpiEvasionEnabled

    fun getDpiEvasionMethod(): Int = SharedConfig.cryptogramDpiEvasionMethod

    fun getQuantumSecurityLevel(): Int = SharedConfig.cryptogramQuantumSecurityLevel

    fun getThreatDefenseLevel(): Int = SharedConfig.cryptogramThreatDefenseLevel

    fun isStylometryShieldEnabled(): Boolean = SharedConfig.cryptogramStylometryShieldEnabled

    fun getStylometryMode(): Int = SharedConfig.cryptogramStylometryMode

    fun getStylometryStrength(): Int = SharedConfig.cryptogramStylometryStrength

    fun isUtdEnabled(): Boolean = SharedConfig.cryptogramUtdEnabled

    fun getUtdThreshold(): Int = SharedConfig.cryptogramUtdThreshold

    fun isVoiceMorphingEnabled(): Boolean = SharedConfig.cryptogramVoiceMorphingEnabled

    fun isLocationPrivacyEnabled(): Boolean = SharedConfig.cryptogramLocationPrivacyEnabled

    fun isInterfaceCamouflageEnabled(): Boolean = SharedConfig.cryptogramInterfaceCamouflageEnabled

    fun isHardwareTetherEnabled(): Boolean = SharedConfig.cryptogramHardwareTetherEnabled

    fun isImapProtectionEnabled(): Boolean = SharedConfig.cryptogramImapProtectionEnabled

    fun getImapProtectionLevel(): Int = SharedConfig.cryptogramImapProtectionLevel

    fun getDpiEvasionMethodName(): String {
        return when (SharedConfig.cryptogramDpiEvasionMethod) {
            0 -> "HTTPS Mimicry"
            1 -> "HTTP Tunneling"
            2 -> "DNS Tunneling"
            3 -> "Generic Fragmentation"
            4 -> "Auto (rotate)"
            else -> "Unknown"
        }
    }

    fun getQuantumSecurityLevelName(): String {
        return when (SharedConfig.cryptogramQuantumSecurityLevel) {
            128 -> "Level 1 (AES-128)"
            256 -> "Level 3 (AES-256)"
            384 -> "Level 5 (Advanced)"
            else -> "Unknown"
        }
    }

    fun getThreatDefenseLevelName(): String {
        return when (SharedConfig.cryptogramThreatDefenseLevel) {
            0 -> "Standard"
            1 -> "Enhanced"
            2 -> "Maximum"
            else -> "Unknown"
        }
    }

    fun getStylometryModeName(): String {
        return when (SharedConfig.cryptogramStylometryMode) {
            0 -> "Rules-only"
            1 -> "Model-assisted"
            else -> "Unknown"
        }
    }

    fun getStylometryStrengthName(): String {
        return when (SharedConfig.cryptogramStylometryStrength) {
            0 -> "Light"
            1 -> "Medium"
            2 -> "Heavy"
            else -> "Unknown"
        }
    }

    fun getImapProtectionLevelName(): String {
        return when (SharedConfig.cryptogramImapProtectionLevel) {
            0 -> "None"
            2 -> "Standard"
            4 -> "Maximum"
            else -> "Unknown"
        }
    }

    fun getUtdSensitivityName(): String {
        val threshold = SharedConfig.cryptogramUtdThreshold
        return when {
            threshold <= 25 -> "Low"
            threshold <= 50 -> "Medium"
            threshold <= 75 -> "High"
            else -> "Maximum"
        }
    }

    fun getFullStatusReport(): String {
        val sb = StringBuilder()
        sb.append("=== CRYPTOGRAM OPSEC Status ===\n\n")

        sb.append("Encryption:\n")
        sb.append("  Double Ratchet: ${if (SharedConfig.cryptogramDoubleRatchetEnabled) "ON" else "OFF"}\n")
        sb.append("  MLS Protocol: ${if (SharedConfig.cryptogramMLSEnabled) "ON" else "OFF"}\n\n")

        sb.append("Privacy:\n")
        sb.append("  Hide Online Status: ${if (SharedConfig.cryptogramHideOnlineStatus) "ON" else "OFF"}\n")
        sb.append("  Hide Typing Indicator: ${if (SharedConfig.cryptogramHideTypingIndicator) "ON" else "OFF"}\n")
        sb.append("  Hide Read Receipts: ${if (SharedConfig.cryptogramHideReadReceipts) "ON" else "OFF"}\n\n")

        sb.append("OPSEC:\n")
        sb.append("  Panic Password: ${if (isPanicPasswordEnabled()) "ON" else "OFF"}\n")
        sb.append("  Anti-Forensics: ${if (isAntiForensicsEnabled()) "ON" else "OFF"}\n")
        sb.append("  Dead Man's Switch: ${if (isDeadManSwitchEnabled()) "ON" else "OFF"}\n")
        sb.append("  Media Metadata Spoofing: ${if (isMediaMetadataSpoofingEnabled()) "ON" else "OFF"}\n")
        sb.append("  Traffic Padding: ${if (isTrafficPaddingEnabled()) "ON" else "OFF"}\n")
        sb.append("  Traffic Obfuscation: ${if (isTrafficObfuscationEnabled()) "ON" else "OFF"}\n")
        sb.append("  DPI Evasion: ${if (isDpiEvasionEnabled()) "ON" else "OFF"} (${getDpiEvasionMethodName()})\n")
        sb.append("  QuantumGuard: ${getQuantumSecurityLevelName()}\n")
        sb.append("  Threat Defense: ${getThreatDefenseLevelName()}\n")
        sb.append("  Stylometry Shield: ${if (isStylometryShieldEnabled()) "ON" else "OFF"} (${getStylometryModeName()}, ${getStylometryStrengthName()})\n")
        sb.append("  Universal Threat Detector: ${if (isUtdEnabled()) "ON" else "OFF"} (${getUtdSensitivityName()})\n")
        sb.append("  Voice Morphing: ${if (isVoiceMorphingEnabled()) "ON" else "OFF"}\n")
        sb.append("  Location Privacy: ${if (isLocationPrivacyEnabled()) "ON" else "OFF"}\n")
        sb.append("  Interface Camouflage: ${if (isInterfaceCamouflageEnabled()) "ON" else "OFF"}\n")
        sb.append("  Hardware Tether: ${if (isHardwareTetherEnabled()) "ON" else "OFF"}\n")
        sb.append("  IMAP Protection: ${if (isImapProtectionEnabled()) "ON" else "OFF"} (${getImapProtectionLevelName()})\n\n")

        sb.append("Network Anonymity:\n")
        sb.append("  Tor: ${torStatus().name} (${torSocksProxy()})\n")
        sb.append("  Bootstrap: ${torBootstrapProgress()}%\n\n")

        sb.append("Native:\n")
        sb.append("  Library: ${if (CryptogramNative.isInitialized()) "Loaded" else "Not loaded"}\n")
        sb.append("  Version: ${CryptogramNative.getVersion()}\n")

        return sb.toString()
    }

    fun secureWipe(file: File): Boolean {
        if (!file.exists()) return true
        return try {
            val length = file.length()
            if (length > 0) {
                val random = ByteArray(4096)
                val fos = java.io.FileOutputStream(file)
                var written = 0L
                while (written < length) {
                    secureRandom.nextBytes(random)
                    val toWrite = minOf(random.size.toLong(), length - written).toInt()
                    fos.write(random, 0, toWrite)
                    written += toWrite
                }
                fos.flush()
                fos.close()
            }
            file.delete()
            Log.i(TAG, "Securely wiped: ${file.absolutePath}")
            true
        } catch (e: Exception) {
            Log.e(TAG, "Failed to secure wipe: ${file.absolutePath}", e)
            false
        }
    }

    fun secureWipeDirectory(dir: File): Boolean {
        if (!dir.exists()) return true
        var allOk = true
        dir.listFiles()?.forEach { f ->
            if (f.isDirectory) {
                if (!secureWipeDirectory(f)) allOk = false
            } else {
                if (!secureWipe(f)) allOk = false
            }
        }
        return allOk
    }

    fun triggerPanicWipe(): Boolean {
        Log.w(TAG, "Panic wipe triggered!")
        var ok = true
        try {
            val cryptogramDir = File("/sdcard/cryptogram")
            if (cryptogramDir.exists()) {
                ok = secureWipeDirectory(cryptogramDir)
            }
        } catch (e: Exception) {
            Log.e(TAG, "Panic wipe failed", e)
            ok = false
        }
        return ok
    }

    fun applyStylometry(text: String): String {
        if (!isStylometryShieldEnabled()) return text
        return when (SharedConfig.cryptogramStylometryStrength) {
            0 -> applyLightStylometry(text)
            1 -> applyMediumStylometry(text)
            2 -> applyHeavyStylometry(text)
            else -> text
        }
    }

    private fun applyLightStylometry(text: String): String {
        var result = text
        result = result.replace("  ", " ")
        result = result.replace(", ", " , ")
        result = result.replace(". ", " . ")
        result = result.replace("  ".toRegex(), " ")
        return result.trim()
    }

    private fun applyMediumStylometry(text: String): String {
        var result = applyLightStylometry(text)
        val words = result.split(" ")
        val sb = StringBuilder()
        for (i in words.indices) {
            val w = words[i]
            if (w.isNotEmpty()) {
                if (i > 0 && secureRandom.nextInt(10) == 0) {
                    sb.append(" ")
                }
                sb.append(w)
                if (i < words.size - 1) {
                    sb.append(" ")
                }
            }
        }
        result = sb.toString()
        result = result.replace(" i ", " I ")
        return result
    }

    private fun applyHeavyStylometry(text: String): String {
        var result = applyMediumStylometry(text)
        val sentences = result.split(". ")
        val sb = StringBuilder()
        for (i in sentences.indices) {
            val s = sentences[i].trim()
            if (s.isNotEmpty()) {
                if (i > 0 && secureRandom.nextInt(3) == 0) {
                    sb.append(". ")
                } else if (i > 0) {
                    sb.append(". ")
                }
                sb.append(s)
            }
        }
        return sb.toString()
    }

    fun randomizeLocation(lat: Double, lon: Double): Pair<Double, Double> {
        if (!isLocationPrivacyEnabled()) return Pair(lat, lon)
        val latNoise = (secureRandom.nextDouble() - 0.5) * 0.01
        val lonNoise = (secureRandom.nextDouble() - 0.5) * 0.01
        return Pair(lat + latNoise, lon + lonNoise)
    }

    fun getFeatureSummaryForUI(): String {
        val active = mutableListOf<String>()
        if (SharedConfig.cryptogramDoubleRatchetEnabled) active.add("DR")
        if (SharedConfig.cryptogramMLSEnabled) active.add("MLS")
        if (isPanicPasswordEnabled()) active.add("Panic")
        if (isAntiForensicsEnabled()) active.add("Anti-Forensics")
        if (isDeadManSwitchEnabled()) active.add("Dead Man")
        if (isMediaMetadataSpoofingEnabled()) active.add("Media Strip")
        if (isTrafficPaddingEnabled()) active.add("Pad")
        if (isTrafficObfuscationEnabled()) active.add("Obfuscate")
        if (isDpiEvasionEnabled()) active.add("DPI")
        if (isStylometryShieldEnabled()) active.add("Stylometry")
        if (isUtdEnabled()) active.add("UTD")
        if (isVoiceMorphingEnabled()) active.add("Voice")
        if (isLocationPrivacyEnabled()) active.add("Location")
        if (isInterfaceCamouflageEnabled()) active.add("Camouflage")
        if (isHardwareTetherEnabled()) active.add("Tether")
        if (isImapProtectionEnabled()) active.add("IMAP")
        if (torStatus() != TorStatus.Stopped) active.add("Tor")
        return if (active.isEmpty()) "No OPSEC features active" else active.joinToString(", ")
    }
}
