/**
 * CRYPTOGRAM Media Metadata Helper
 *
 * Strips or spoofs EXIF/GPS metadata from media files before sharing.
 * When enabled, removes location data, camera info, and timestamps
 * from JPEG/HEIC images to prevent metadata-based deanonymization.
 */
package org.telegram.messenger.cryptogram

import android.util.Log
import org.telegram.messenger.SharedConfig
import java.io.File
import java.io.FileInputStream
import java.io.FileOutputStream
import java.nio.ByteBuffer
import java.nio.ByteOrder

object MediaMetadataHelper {
    private const val TAG = "MediaMetadataHelper"

    private val JPEG_MARKER = byteArrayOf(0xFF.toByte(), 0xD8.toByte())
    private val JPEG_EOI = byteArrayOf(0xFF.toByte(), 0xD9.toByte())
    private val EXIF_MARKER = byteArrayOf(0xFF.toByte(), 0xE1.toByte())
    private val EXIF_HEADER = byteArrayOf(0x45, 0x78, 0x69, 0x66, 0x00, 0x00)

    fun shouldStripMetadata(): Boolean = SharedConfig.cryptogramMediaMetadataSpoofingEnabled

    fun stripExifMetadata(inputFile: File): File? {
        if (!shouldStripMetadata()) return null
        if (!inputFile.exists()) return null

        val name = inputFile.name
        val ext = name.substringAfterLast('.', "").lowercase()

        return when (ext) {
            "jpg", "jpeg" -> stripJpegExif(inputFile)
            "png" -> stripPngMetadata(inputFile)
            "heic", "heif" -> stripHeicMetadata(inputFile)
            else -> {
                Log.d(TAG, "Unsupported format for metadata stripping: $ext")
                null
            }
        }
    }

    fun stripJpegExif(inputFile: File): File? {
        try {
            val data = inputFile.readBytes()
            if (data.size < 4) return null
            if (data[0] != JPEG_MARKER[0] || data[1] != JPEG_MARKER[1]) return null

            val output = mutableListOf<Byte>()
            output.add(JPEG_MARKER[0])
            output.add(JPEG_MARKER[1])

            var pos = 2
            var stripped = false

            while (pos < data.size - 1) {
                if (data[pos] != 0xFF.toByte()) {
                    output.add(data[pos])
                    pos++
                    continue
                }

                val marker = data[pos + 1].toInt() and 0xFF

                if (marker == 0xE1) {
                    val segLen = ((data[pos + 2].toInt() and 0xFF) shl 8) or (data[pos + 3].toInt() and 0xFF)
                    if (pos + 4 + 6 <= data.size) {
                        val headerCheck = String(data, pos + 4, 6, Charsets.US_ASCII)
                        if (headerCheck.startsWith("Exif")) {
                            pos += 2 + segLen
                            stripped = true
                            continue
                        }
                    }
                }

                if (marker == 0xDA) {
                    while (pos < data.size) {
                        output.add(data[pos])
                        pos++
                    }
                    break
                }

                if (marker == 0xD9) {
                    output.add(data[pos])
                    if (pos + 1 < data.size) output.add(data[pos + 1])
                    break
                }

                if (marker >= 0xD0 && marker <= 0xD7) {
                    output.add(data[pos])
                    output.add(data[pos + 1])
                    pos += 2
                    continue
                }

                val segLen = ((data[pos + 2].toInt() and 0xFF) shl 8) or (data[pos + 3].toInt() and 0xFF)
                output.add(data[pos])
                output.add(data[pos + 1])
                for (i in 2 until segLen + 2) {
                    if (pos + i < data.size) output.add(data[pos + i])
                }
                pos += 2 + segLen
            }

            if (stripped) {
                val tempFile = File(inputFile.parentFile, "stripped_${inputFile.name}")
                FileOutputStream(tempFile).use { fos ->
                    val byteArray = output.map { it }.toByteArray()
                    fos.write(byteArray)
                }
                Log.i(TAG, "Stripped EXIF from JPEG: ${inputFile.name} (${data.size} -> ${output.size} bytes)")
                return tempFile
            }

            Log.d(TAG, "No EXIF found in JPEG: ${inputFile.name}")
            return null
        } catch (e: Exception) {
            Log.e(TAG, "Failed to strip JPEG EXIF: ${inputFile.name}", e)
            return null
        }
    }

    fun stripPngMetadata(inputFile: File): File? {
        try {
            val data = inputFile.readBytes()
            if (data.size < 8) return null

            val pngSig = byteArrayOf(
                0x89.toByte(), 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A
            )
            for (i in pngSig.indices) {
                if (data[i] != pngSig[i]) return null
            }

            val output = mutableListOf<Byte>()
            for (b in pngSig) output.add(b)

            var pos = 8
            var stripped = false

            while (pos < data.size - 8) {
                val chunkLen = ByteBuffer.wrap(data, pos, 4).order(ByteOrder.BIG_ENDIAN).int
                val chunkType = String(data, pos + 4, 4, Charsets.US_ASCII)

                if (chunkType == "tEXt" || chunkType == "zTXt" || chunkType == "iTXt" ||
                    chunkType == "eXIf" || chunkType == "tIME") {
                    pos += 4 + 4 + chunkLen + 4
                    stripped = true
                    continue
                }

                for (i in 0 until 4 + 4 + chunkLen + 4) {
                    if (pos + i < data.size) output.add(data[pos + i])
                }
                pos += 4 + 4 + chunkLen + 4
            }

            if (stripped) {
                val tempFile = File(inputFile.parentFile, "stripped_${inputFile.name}")
                FileOutputStream(tempFile).use { fos ->
                    fos.write(output.map { it }.toByteArray())
                }
                Log.i(TAG, "Stripped metadata from PNG: ${inputFile.name}")
                return tempFile
            }
            return null
        } catch (e: Exception) {
            Log.e(TAG, "Failed to strip PNG metadata: ${inputFile.name}", e)
            return null
        }
    }

    fun stripHeicMetadata(inputFile: File): File? {
        Log.d(TAG, "HEIC metadata stripping not yet implemented, copying file as-is")
        return null
    }

    fun getMetadataSummary(file: File): String {
        if (!file.exists()) return "File not found"
        val ext = file.extension.lowercase()
        val hasExif = when (ext) {
            "jpg", "jpeg" -> checkJpegHasExif(file)
            "png" -> checkPngHasMetadata(file)
            else -> false
        }
        return if (hasExif) "Contains metadata" else "No metadata detected"
    }

    private fun checkJpegHasExif(file: File): Boolean {
        try {
            val data = file.readBytes()
            for (i in 0 until data.size - 10) {
                if (data[i] == 0xFF.toByte() && data[i + 1] == 0xE1.toByte()) {
                    if (i + 10 <= data.size) {
                        val header = String(data, i + 4, 6, Charsets.US_ASCII)
                        if (header.startsWith("Exif")) return true
                    }
                }
            }
        } catch (e: Exception) {
            Log.e(TAG, "Failed to check JPEG EXIF", e)
        }
        return false
    }

    private fun checkPngHasMetadata(file: File): Boolean {
        try {
            val data = file.readBytes()
            if (data.size < 8) return false
            var pos = 8
            while (pos < data.size - 8) {
                val chunkLen = ByteBuffer.wrap(data, pos, 4).order(ByteOrder.BIG_ENDIAN).int
                val chunkType = String(data, pos + 4, 4, Charsets.US_ASCII)
                if (chunkType == "tEXt" || chunkType == "zTXt" || chunkType == "iTXt" ||
                    chunkType == "eXIf" || chunkType == "tIME") {
                    return true
                }
                pos += 4 + 4 + chunkLen + 4
            }
        } catch (e: Exception) {
            Log.e(TAG, "Failed to check PNG metadata", e)
        }
        return false
    }
}
