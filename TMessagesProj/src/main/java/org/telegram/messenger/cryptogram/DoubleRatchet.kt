/**
 * CRYPTOGRAM Double Ratchet (Signal Protocol)
 *
 * Provides end-to-end encryption for 1-on-1 messages using the Signal Protocol.
 *
 * Features:
 * - Forward secrecy: Past messages remain secure even if keys are compromised
 * - Post-compromise security: Security restored after key compromise
 * - X25519 ECDH for key agreement
 * - Ed25519 for signatures
 * - AES-256-GCM for message encryption
 *
 * Usage:
 * ```kotlin
 * val ratchet = DoubleRatchet
 *
 * // Initialize session with a user
 * ratchet.initializeSession(userId)
 *
 * // Encrypt outgoing message
 * val encrypted = ratchet.encrypt(userId, "Hello, secure world!")
 *
 * // Decrypt incoming message
 * val plaintext = ratchet.decrypt(userId, encryptedBytes)
 * ```
 */
package org.telegram.messenger.cryptogram

import android.util.Log

object DoubleRatchet {
    private const val TAG = "DoubleRatchet"

    init {
        try {
            System.loadLibrary("cryptogram")
            Log.d(TAG, "CRYPTOGRAM native library loaded successfully")
        } catch (e: UnsatisfiedLinkError) {
            Log.e(TAG, "Failed to load CRYPTOGRAM native library", e)
        }
    }

    // Native method declarations
    private external fun nativeInitializeSession(userId: Long): Boolean
    private external fun nativeInitializeWithRemoteBundle(userId: Long, bundle: ByteArray): Boolean
    private external fun nativeEncrypt(userId: Long, plaintext: String): ByteArray?
    private external fun nativeDecrypt(userId: Long, ciphertext: ByteArray): String?
    private external fun nativeRotateSession(userId: Long): Boolean
    private external fun nativeGetFingerprint(userId: Long): String?
    private external fun nativeGenerateKeyBundle(): ByteArray?
    private external fun nativeGetState(userId: Long): String?

    /**
     * Generate a local key bundle (Identity + Signed PreKey + One-time PreKeys)
     * for X3DH key agreement.
     *
     * @return Byte array containing the serialized key bundle
     */
    fun generateKeyBundle(): ByteArray? {
        return try {
            nativeGenerateKeyBundle()
        } catch (e: Exception) {
            Log.e(TAG, "Failed to generate key bundle", e)
            null
        }
    }

    /**
     * Initialize a Double Ratchet session using a peer's key bundle (X3DH)
     *
     * @param userId Telegram user ID
     * @param bundle Serialized remote key bundle
     * @return true if session initialized successfuly
     */
    fun initializeWithRemoteBundle(userId: Long, bundle: ByteArray): Boolean {
        if (bundle.isEmpty()) {
            Log.w(TAG, "Empty remote bundle for user $userId")
            return false
        }
        return try {
            val result = nativeInitializeWithRemoteBundle(userId, bundle)
            Log.d(TAG, "Session initialized with bundle for user $userId: $result")
            result
        } catch (e: Exception) {
            Log.e(TAG, "Failed to initialize session with bundle for user $userId", e)
            false
        }
    }

    /**
     * Manually trigger a key rotation for a session
     *
     * @param userId Telegram user ID
     * @return true if rotation successful
     */
    fun rotateSession(userId: Long): Boolean {
        return try {
            nativeRotateSession(userId)
        } catch (e: Exception) {
            Log.e(TAG, "Failed to rotate session for user $userId", e)
            false
        }
    }

    /**
     * Get the "Safety Number" fingerprint for a user's session
     * for out-of-band verification.
     *
     * @param userId Telegram user ID
     * @return Fingerprint string, or null on failure
     */
    fun getFingerprint(userId: Long): String? {
        return try {
            nativeGetFingerprint(userId)
        } catch (e: Exception) {
            Log.e(TAG, "Failed to get fingerprint for user $userId", e)
            null
        }
    }

    /**
     * Initialize a Double Ratchet session with a user (basic)
     *
     * @param userId Telegram user ID
     * @return true if initialization successful
     */
    fun initializeSession(userId: Long): Boolean {
        return try {
            val result = nativeInitializeSession(userId)
            Log.d(TAG, "Session initialized for user $userId: $result")
            result
        } catch (e: Exception) {
            Log.e(TAG, "Failed to initialize session for user $userId", e)
            false
        }
    }

    /**
     * Encrypt a message for a user
     *
     * @param userId Telegram user ID
     * @param plaintext Message to encrypt
     * @return Encrypted ciphertext, or null on failure
     */
    fun encrypt(userId: Long, plaintext: String): ByteArray? {
        if (plaintext.isEmpty()) {
            Log.w(TAG, "Cannot encrypt empty message")
            return null
        }

        return try {
            val ciphertext = nativeEncrypt(userId, plaintext)
            if (ciphertext != null) {
                Log.d(TAG, "Encrypted message for user $userId (${ciphertext.size} bytes)")
            } else {
                Log.e(TAG, "Encryption returned null for user $userId")
            }
            ciphertext
        } catch (e: Exception) {
            Log.e(TAG, "Failed to encrypt message for user $userId", e)
            null
        }
    }

    /**
     * Decrypt a message from a user
     *
     * @param userId Telegram user ID
     * @param ciphertext Encrypted message
     * @return Decrypted plaintext, or null on failure
     */
    fun decrypt(userId: Long, ciphertext: ByteArray): String? {
        if (ciphertext.isEmpty()) {
            Log.w(TAG, "Cannot decrypt empty ciphertext")
            return null
        }

        return try {
            val plaintext = nativeDecrypt(userId, ciphertext)
            if (plaintext != null) {
                Log.d(TAG, "Decrypted message from user $userId")
            } else {
                Log.e(TAG, "Decryption returned null for user $userId")
            }
            plaintext
        } catch (e: Exception) {
            Log.e(TAG, "Failed to decrypt message from user $userId", e)
            null
        }
    }

    /**
     * Get current ratchet state for a user (for debugging)
     *
     * @param userId Telegram user ID
     * @return JSON string with ratchet state
     */
    fun getState(userId: Long): String? {
        return try {
            nativeGetState(userId)
        } catch (e: Exception) {
            Log.e(TAG, "Failed to get state for user $userId", e)
            null
        }
    }

    /**
     * Check if a session exists for a user
     *
     * @param userId Telegram user ID
     * @return true if session exists
     */
    fun hasSession(userId: Long): Boolean {
        val state = getState(userId)
        return state != null && state.contains("\"initialized\": true")
    }
}
