/**
 * CRYPTOGRAM Universal Threat Detector (UTD)
 *
 * Scans incoming messages for phishing, social engineering, and suspicious
 * patterns. Uses rule-based heuristics with configurable sensitivity.
 */
package org.telegram.messenger.cryptogram

import android.util.Log
import org.telegram.messenger.SharedConfig
import java.util.regex.Pattern

object ThreatDetector {
    private const val TAG = "ThreatDetector"

    private val PHISHING_URL_PATTERNS = listOf(
        Pattern.compile("(?i)https?://[^\\s]*(telegram|tg)[^\\s]*\\.(ru|cn|tk|ml|ga|cf|xyz)"),
        Pattern.compile("(?i)https?://[^\\s]*bit\\.ly/[^\\s]*"),
        Pattern.compile("(?i)https?://[^\\s]*tinyurl\\.com/[^\\s]*"),
        Pattern.compile("(?i)https?://[^\\s]*t\\.me/[^\\s]*share[^\\s]*"),
        Pattern.compile("(?i)https?://[^\\s]*login[^\\s]*verify[^\\s]*"),
        Pattern.compile("(?i)https?://[^\\s]*secure[^\\s]*update[^\\s]*")
    )

    private val SOCIAL_ENGINEERING_PATTERNS = listOf(
        "(?i)urgent.{0,20}(action|verify|confirm|password|account)",
        "(?i)(congratulations|you.{0,10}won|prize|lottery|winner).{0,30}(claim|click|send)",
        "(?i)(send|transfer).{0,20}(crypto|bitcoin|eth|usdt|wallet).{0,20}(address|now|immediately)",
        "(?i)(admin|support|moderator).{0,20}(password|2fa|code|verification)",
        "(?i)your.{0,10}(account|profile).{0,20}(blocked|suspended|restricted|locked)",
        "(?i)click.{0,10}(here|link|below).{0,20}(verify|confirm|restore|unlock)"
    ).map { Pattern.compile(it) }

    private val SUSPICIOUS_KEYWORDS = listOf(
        "verify your account", "confirm your identity", "suspended",
        "click here to restore", "enter your password", "2fa code",
        "seed phrase", "recovery phrase", "private key",
        "send us your", "limited time", "act now"
    )

    fun scanMessage(message: String?): String? {
        if (message == null || message.isEmpty()) return null
        if (!SharedConfig.cryptogramUtdEnabled) return null

        val threshold = SharedConfig.cryptogramUtdThreshold
        val score = calculateThreatScore(message)
        Log.d(TAG, "Threat score: $score (threshold: $threshold)")

        return if (score >= threshold) {
            val warning = buildWarning(score)
            Log.w(TAG, "Threat detected: score=$score, warning=$warning")
            warning
        } else {
            null
        }
    }

    private fun calculateThreatScore(message: String): Int {
        var score = 0
        val lower = message.lowercase()

        for (pattern in PHISHING_URL_PATTERNS) {
            if (pattern.matcher(message).find()) {
                score += 30
            }
        }

        for (pattern in SOCIAL_ENGINEERING_PATTERNS) {
            if (pattern.matcher(message).find()) {
                score += 20
            }
        }

        for (keyword in SUSPICIOUS_KEYWORDS) {
            if (lower.contains(keyword)) {
                score += 15
            }
        }

        if (message.contains("http://") && !message.contains("https://")) {
            score += 10
        }

        val exclamationCount = message.count { it == '!' }
        if (exclamationCount >= 3) {
            score += 5
        }

        val upperRatio = message.count { it.isUpperCase() }.toDouble() / message.length
        if (upperRatio > 0.5 && message.length > 20) {
            score += 10
        }

        return minOf(score, 100)
    }

    private fun buildWarning(score: Int): String {
        return "[⚠️ UTD Alert] Threat score: $score/100. This message triggered the Universal Threat Detector. " +
               "Exercise caution — do not click links, share credentials, or send funds based on this message."
    }

    fun getThreatLevel(message: String?): String {
        if (message == null || message.isEmpty()) return "None"
        val score = calculateThreatScore(message)
        return when {
            score >= 75 -> "Critical"
            score >= 50 -> "High"
            score >= 25 -> "Medium"
            score > 0 -> "Low"
            else -> "None"
        }
    }
}
