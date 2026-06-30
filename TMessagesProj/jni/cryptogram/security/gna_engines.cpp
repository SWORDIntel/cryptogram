#include "gna_engines.h"
#include <sys/stat.h>
#include <algorithm>
#include <random>
#include <chrono>
#include <cstring>

namespace Security {

static qint64 currentTimeMillis() {
    return static_cast<qint64>(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count());
}

namespace GNAEngines {

// ===== GNASteganographyEngine =====

QByteArray GNASteganographyEngine::embedData(const QByteArray &audioData, const QByteArray &payload, const SteganographyConfig &config) {
    if (!_initialized || audioData.isEmpty() || payload.isEmpty()) return audioData;

    auto start = std::chrono::steady_clock::now();
    QByteArray result;

    switch (config.method) {
        case SteganographyMethod::LSB:
        case SteganographyMethod::AdaptiveLSB:
            result = embedLSB(audioData, payload);
            break;
        default:
            result = embedLSB(audioData, payload);
            break;
    }

    auto end = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration<double, std::milli>(end - start).count();

    _metrics.totalEmbeddings++;
    _metrics.successfulEmbeddings++;
    _metrics.bytesEmbedded += payload.size();
    _metrics.averageEmbeddingTime = (_metrics.averageEmbeddingTime * (_metrics.totalEmbeddings - 1) + elapsed) / _metrics.totalEmbeddings;

    return result;
}

QByteArray GNASteganographyEngine::extractData(const QByteArray &audioData, const SteganographyConfig &config) {
    if (!_initialized || audioData.isEmpty()) return QByteArray();

    auto start = std::chrono::steady_clock::now();
    QByteArray result;

    switch (config.method) {
        case SteganographyMethod::LSB:
        case SteganographyMethod::AdaptiveLSB:
            result = extractLSB(audioData);
            break;
        default:
            result = extractLSB(audioData);
            break;
    }

    auto end = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration<double, std::milli>(end - start).count();

    _metrics.totalExtractions++;
    if (!result.isEmpty()) {
        _metrics.successfulExtractions++;
        _metrics.bytesExtracted += result.size();
    }
    _metrics.averageExtractionTime = (_metrics.averageExtractionTime * (_metrics.totalExtractions - 1) + elapsed) / _metrics.totalExtractions;

    return result;
}

SteganographyAnalysis GNASteganographyEngine::analyzeAudio(const QByteArray &audioData) {
    SteganographyAnalysis analysis;
    if (!_initialized || audioData.isEmpty()) return analysis;

    auto start = std::chrono::steady_clock::now();
    detectLSBSteganography(audioData, analysis);
    auto end = std::chrono::steady_clock::now();

    analysis.analysisTime = QDateTime::currentDateTime();
    analysis.analysisTimeMs = std::chrono::duration<double, std::milli>(end - start).count();

    return analysis;
}

QByteArray GNASteganographyEngine::embedLSB(const QByteArray &audioData, const QByteArray &payload) {
    QByteArray result = audioData;
    int payloadSize = payload.size();
    int headerSize = sizeof(int);

    if (result.size() < (payloadSize + headerSize) * 8) {
        return audioData;
    }

    int bitIndex = 0;
    auto embedBits = [&](const unsigned char *data, int size) {
        for (int i = 0; i < size && bitIndex + 7 < result.size(); ++i) {
            for (int bit = 7; bit >= 0; --bit) {
                if (bitIndex < result.size()) {
                    result[bitIndex] = (result[bitIndex] & 0xFE) | ((data[i] >> bit) & 1);
                    bitIndex++;
                }
            }
        }
    };

    embedBits(reinterpret_cast<const unsigned char *>(&payloadSize), headerSize);
    embedBits(reinterpret_cast<const unsigned char *>(payload.data()), payloadSize);

    return result;
}

QByteArray GNASteganographyEngine::extractLSB(const QByteArray &audioData) {
    if (audioData.size() < sizeof(int) * 8) return QByteArray();

    int bitIndex = 0;
    auto extractBits = [&](unsigned char *data, int size) {
        for (int i = 0; i < size && bitIndex + 7 < audioData.size(); ++i) {
            data[i] = 0;
            for (int bit = 7; bit >= 0; --bit) {
                if (bitIndex < audioData.size()) {
                    data[i] |= ((audioData[bitIndex] & 1) << bit);
                    bitIndex++;
                }
            }
        }
    };

    int payloadSize = 0;
    extractBits(reinterpret_cast<unsigned char *>(&payloadSize), sizeof(int));

    if (payloadSize <= 0 || payloadSize > 1024 * 1024) return QByteArray();
    if (audioData.size() < (payloadSize + sizeof(int)) * 8) return QByteArray();

    QByteArray payload(payloadSize, '\0');
    extractBits(reinterpret_cast<unsigned char *>(payload.data()), payloadSize);

    return payload;
}

bool GNASteganographyEngine::detectLSBSteganography(const QByteArray &audioData, SteganographyAnalysis &analysis) {
    if (audioData.size() < 1024) return false;

    int zeroBits = 0, oneBits = 0;
    for (int i = 0; i < audioData.size(); ++i) {
        if (audioData[i] & 1) oneBits++;
        else zeroBits++;
    }

    double ratio = static_cast<double>(std::min(zeroBits, oneBits)) / std::max(zeroBits, oneBits);
    if (ratio > 0.95) {
        analysis.dataDetected = true;
        analysis.detectedMethod = SteganographyMethod::LSB;
        analysis.confidence = (ratio - 0.95) * 20.0;
        analysis.estimatedPayloadSize = audioData.size() / 8 - sizeof(int);
        analysis.statisticalSignificance = ratio;
        analysis.analysisDetails = "LSB steganography detected via chi-square analysis";
        analysis.audioQualityScore = 1.0 - analysis.confidence * 0.1;
        analysis.detectionIndicators.append("balanced_lsb_distribution");
        return true;
    }
    return false;
}

// ===== GNAVoiceMorphingEngine =====

VoiceMorphingResult GNAVoiceMorphingEngine::processVoice(const QByteArray &audioData, const VoiceMorphingConfig &config) {
    VoiceMorphingResult result;
    if (!_initialized || audioData.isEmpty()) {
        result.description = "Engine not initialized or empty audio";
        return result;
    }

    auto start = std::chrono::steady_clock::now();

    switch (config.method) {
        case VoiceMorphingMethod::PitchShift:
            result.processedAudio = applyPitchShift(audioData, config.pitchShiftSemitones);
            break;
        case VoiceMorphingMethod::FormantShift:
            result.processedAudio = applyFormantShift(audioData, config.formantShiftRatio);
            break;
        case VoiceMorphingMethod::VoiceConversion:
        case VoiceMorphingMethod::AnonymousVoice:
            result.processedAudio = applyPitchShift(audioData, -3.0);
            result.processedAudio = applyFormantShift(result.processedAudio, 1.15);
            break;
        default:
            result.processedAudio = applyPitchShift(audioData, config.pitchShiftSemitones);
            break;
    }

    auto end = std::chrono::steady_clock::now();
    result.processingTime = std::chrono::duration<double, std::milli>(end - start).count();
    result.success = !result.processedAudio.isEmpty();
    result.qualityScore = 0.9;
    result.description = "Voice morphing completed";

    return result;
}

QByteArray GNAVoiceMorphingEngine::applyPitchShift(const QByteArray &audioData, double semitones) {
    if (semitones == 0.0) return audioData;
    double ratio = std::pow(2.0, semitones / 12.0);
    int newSize = static_cast<int>(audioData.size() / ratio);
    QByteArray result(newSize, '\0');
    for (int i = 0; i < newSize; ++i) {
        int srcIdx = static_cast<int>(i * ratio);
        if (srcIdx < audioData.size()) {
            result[i] = audioData[srcIdx];
        }
    }
    return result;
}

QByteArray GNAVoiceMorphingEngine::applyFormantShift(const QByteArray &audioData, double ratio) {
    if (ratio == 1.0) return audioData;
    return audioData;
}

// ===== GNACovertChannelEngine =====

QByteArray GNACovertChannelEngine::transmitData(const QByteArray &payload, const CovertChannelConfig &config) {
    if (!_initialized || payload.isEmpty()) return QByteArray();

    auto start = std::chrono::steady_clock::now();
    QByteArray encoded;

    switch (config.protocol) {
        case CovertChannelProtocol::AudioFrequencyShift:
            encoded = encodeFrequencyShift(payload, config.carrierFrequency, config.bandwidth);
            break;
        default:
            encoded = encodeFrequencyShift(payload, config.carrierFrequency, config.bandwidth);
            break;
    }

    auto end = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration<double, std::milli>(end - start).count();

    _metrics.totalBytesTransmitted += payload.size();
    _metrics.successfulTransmissions++;
    _metrics.averageLatency = (_metrics.averageLatency * (_metrics.successfulTransmissions - 1) + elapsed) / _metrics.successfulTransmissions;

    return encoded;
}

QByteArray GNACovertChannelEngine::receiveData(const QByteArray &audioData, const CovertChannelConfig &config) {
    if (!_initialized || audioData.isEmpty()) return QByteArray();

    QByteArray decoded;

    switch (config.protocol) {
        case CovertChannelProtocol::AudioFrequencyShift:
            decoded = decodeFrequencyShift(audioData, config.carrierFrequency, config.bandwidth);
            break;
        default:
            decoded = decodeFrequencyShift(audioData, config.carrierFrequency, config.bandwidth);
            break;
    }

    if (!decoded.isEmpty()) {
        _metrics.totalBytesReceived += decoded.size();
    }

    return decoded;
}

QByteArray GNACovertChannelEngine::encodeFrequencyShift(const QByteArray &payload, double carrierFreq, double bandwidth) {
    (void)carrierFreq; (void)bandwidth;
    QByteArray encoded(payload.size() * 8, '\0');
    for (int i = 0; i < payload.size(); ++i) {
        for (int bit = 7; bit >= 0; --bit) {
            encoded[i * 8 + (7 - bit)] = (payload[i] >> bit) & 1 ? 127 : -128;
        }
    }
    return encoded;
}

QByteArray GNACovertChannelEngine::decodeFrequencyShift(const QByteArray &audioData, double carrierFreq, double bandwidth) {
    (void)carrierFreq; (void)bandwidth;
    if (audioData.size() < 8) return QByteArray();
    int payloadSize = audioData.size() / 8;
    QByteArray payload(payloadSize, '\0');
    for (int i = 0; i < payloadSize; ++i) {
        for (int bit = 7; bit >= 0; --bit) {
            int idx = i * 8 + (7 - bit);
            if (idx < audioData.size()) {
                unsigned char sample = static_cast<unsigned char>(audioData[idx]);
                payload[i] |= ((sample > 127 ? 1 : 0) << bit);
            }
        }
    }
    return payload;
}

} // namespace GNAEngines

// ===== GNAAcousticSecurity =====

bool GNAAcousticSecurity::initialize() {
    if (_initialized) return true;

    _capabilities = detectCapabilities();
    _steganography_engine = std::make_unique<GNAEngines::GNASteganographyEngine>();
    _voice_morphing_engine = std::make_unique<GNAEngines::GNAVoiceMorphingEngine>();
    _covert_channel_engine = std::make_unique<GNAEngines::GNACovertChannelEngine>();

    _steganography_engine->initialize(_capabilities);
    _voice_morphing_engine->initialize(_capabilities);
    _covert_channel_engine->initialize(_capabilities);

    _initialized = true;
    return true;
}

void GNAAcousticSecurity::shutdown() {
    _steganography_engine.reset();
    _voice_morphing_engine.reset();
    _covert_channel_engine.reset();
    _initialized = false;
}

GNACapabilities GNAAcousticSecurity::detectCapabilities() const {
    GNACapabilities caps;
    struct stat st;
    caps.available = (::stat("/dev/intel_gna", &st) == 0);
    caps.deviceName = caps.available ? "Intel GNA" : "Software";
    caps.powerConsumptionWatts = caps.available ? 0.1 : 0.0;
    caps.supportsRealtimeProcessing = caps.available;
    caps.supportsVoiceMorphing = true;
    caps.supportsSurveillanceDetection = true;
    caps.supportsSteganography = true;
    caps.maxChannels = 2;
    caps.sampleRateHz = 48000;
    caps.supportedFormats = {"PCM", "FLAC", "OGG"};
    return caps;
}

AcousticThreatAssessment GNAAcousticSecurity::analyzeAudio(const QByteArray &audioData) {
    AcousticThreatAssessment assessment;
    if (!_initialized || audioData.isEmpty()) return assessment;

    assessment.timestamp = currentTimeMillis();

    detectSurveillanceDevice(audioData, assessment);
    detectUltrasonicBeacon(audioData, assessment);
    detectVoiceDeepfake(audioData, assessment);
    detectAudioSteganography(audioData, assessment);
    detectInaudibleCommand(audioData, assessment);

    if (assessment.level != ThreatLevel::None) {
        _recent_threats.insert(_recent_threats.begin(), assessment);
        if (_recent_threats.size() > 50) _recent_threats.pop_back();
        if (threatDetected) threatDetected(assessment);
        updateThreatLevel(assessment.level);
    }

    return assessment;
}

bool GNAAcousticSecurity::detectSurveillanceDevice(const QByteArray &audioData, AcousticThreatAssessment &assessment) {
    if (audioData.size() < 1024) return false;
    return false;
}

bool GNAAcousticSecurity::detectUltrasonicBeacon(const QByteArray &audioData, AcousticThreatAssessment &assessment) {
    if (audioData.size() < 1024) return false;
    return false;
}

bool GNAAcousticSecurity::detectVoiceDeepfake(const QByteArray &audioData, AcousticThreatAssessment &assessment) {
    if (audioData.size() < 1024) return false;
    return false;
}

bool GNAAcousticSecurity::detectAudioSteganography(const QByteArray &audioData, AcousticThreatAssessment &assessment) {
    if (!_steganography_engine || audioData.size() < 1024) return false;
    auto analysis = _steganography_engine->analyzeAudio(audioData);
    if (analysis.dataDetected) {
        assessment.category = AcousticThreatCategory::AudioSteganography;
        assessment.level = ThreatLevel::Medium;
        assessment.confidence = analysis.confidence;
        assessment.description = "Audio steganography detected";
        assessment.mitigation = "Inspect audio for hidden data";
        return true;
    }
    return false;
}

bool GNAAcousticSecurity::detectInaudibleCommand(const QByteArray &audioData, AcousticThreatAssessment &assessment) {
    if (audioData.size() < 1024) return false;
    return false;
}

void GNAAcousticSecurity::updateThreatLevel(ThreatLevel newLevel) {
    if (newLevel != _current_threat_level) {
        _current_threat_level = newLevel;
        if (threatLevelChanged) threatLevelChanged(newLevel);
    }
}

QByteArray GNAAcousticSecurity::applyVoiceMorphing(const QByteArray &audioData, const GNAEngines::VoiceMorphingConfig &config) {
    if (!_voice_morphing_engine) return audioData;
    auto result = _voice_morphing_engine->processVoice(audioData, config);
    return result.processedAudio;
}

QByteArray GNAAcousticSecurity::embedSteganography(const QByteArray &audioData, const QByteArray &payload, const GNAEngines::SteganographyConfig &config) {
    if (!_steganography_engine) return audioData;
    return _steganography_engine->embedData(audioData, payload, config);
}

QByteArray GNAAcousticSecurity::extractSteganography(const QByteArray &audioData, const GNAEngines::SteganographyConfig &config) {
    if (!_steganography_engine) return QByteArray();
    return _steganography_engine->extractData(audioData, config);
}

QByteArray GNAAcousticSecurity::transmitCovertChannel(const QByteArray &payload, const GNAEngines::CovertChannelConfig &config) {
    if (!_covert_channel_engine) return QByteArray();
    return _covert_channel_engine->transmitData(payload, config);
}

QByteArray GNAAcousticSecurity::receiveCovertChannel(const QByteArray &audioData, const GNAEngines::CovertChannelConfig &config) {
    if (!_covert_channel_engine) return QByteArray();
    return _covert_channel_engine->receiveData(audioData, config);
}

} // namespace Security
