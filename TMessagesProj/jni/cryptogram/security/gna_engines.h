#pragma once

#include "desktop_shims.h"
#include "base/bytes.h"
#include "security/hardware_detector.h"
#include "security/universal_threat_detector.h"

#include <memory>
#include <cmath>
#include <random>

namespace Security {

enum class SecurityTier {
    Basic,
    CPU,
    GPU,
    NPU,
    GNA
};

enum class ThreatLevel {
    None = 0,
    Low = 1,
    Medium = 2,
    High = 3,
    Critical = 4
};

struct GNACapabilities {
    bool available = false;
    QString deviceName;
    QString driverVersion;
    int computeUnits = 0;
    double powerConsumptionWatts = 0.1;
    bool supportsRealtimeProcessing = false;
    bool supportsVoiceMorphing = false;
    bool supportsSurveillanceDetection = false;
    bool supportsSteganography = false;
    int maxChannels = 2;
    int sampleRateHz = 48000;
    QStringList supportedFormats;
};

namespace GNAEngines {

enum class SteganographyMethod {
    LSB,
    SpectralMasking,
    EchoHiding,
    PhaseModulation,
    SpreadSpectrum,
    AdaptiveLSB,
    PsychoacousticMasking,
    CepstrumModulation
};

struct SteganographyConfig {
    SteganographyMethod method = SteganographyMethod::LSB;
    QString encryptionKey;
    double embeddingStrength = 0.5;
    bool adaptiveEmbedding = true;
    double targetSNR = 40.0;
    int maxPayloadSize = 1024;
    bool errorCorrection = true;
    QString compressionMethod = "zlib";
    double psychoacousticThreshold = 0.1;
};

struct SteganographyAnalysis {
    bool dataDetected = false;
    SteganographyMethod detectedMethod = SteganographyMethod::LSB;
    double confidence = 0.0;
    int estimatedPayloadSize = 0;
    double statisticalSignificance = 0.0;
    QString analysisDetails;
    QDateTime analysisTime;
    qint64 analysisTimeMs = 0;
    double audioQualityScore = 0.0;
    QStringList detectionIndicators;
};

struct SteganographyMetrics {
    qint64 totalEmbeddings = 0;
    qint64 successfulEmbeddings = 0;
    qint64 totalExtractions = 0;
    qint64 successfulExtractions = 0;
    qint64 bytesEmbedded = 0;
    qint64 bytesExtracted = 0;
    double averageEmbeddingTime = 0.0;
    double averageExtractionTime = 0.0;
    double averageSNR = 0.0;
    double averageQualityScore = 0.0;
};

class GNASteganographyEngine {
public:
    GNASteganographyEngine() = default;
    ~GNASteganographyEngine() = default;

    bool initialize(const GNACapabilities &caps) { _caps = caps; _initialized = true; return true; }
    bool isInitialized() const { return _initialized; }

    QByteArray embedData(const QByteArray &audioData, const QByteArray &payload, const SteganographyConfig &config);
    QByteArray extractData(const QByteArray &audioData, const SteganographyConfig &config);
    SteganographyAnalysis analyzeAudio(const QByteArray &audioData);
    SteganographyMetrics getMetrics() const { return _metrics; }

    void setCapabilities(const GNACapabilities &caps) { _caps = caps; }

private:
    QByteArray embedLSB(const QByteArray &audioData, const QByteArray &payload);
    QByteArray extractLSB(const QByteArray &audioData);
    bool detectLSBSteganography(const QByteArray &audioData, SteganographyAnalysis &analysis);

    GNACapabilities _caps;
    bool _initialized = false;
    SteganographyMetrics _metrics;
};

enum class VoiceMorphingMethod {
    PitchShift,
    FormantShift,
    VoiceConversion,
    TimbreModification,
    GenderModification,
    AccentModification,
    AnonymousVoice
};

struct VoiceMorphingConfig {
    VoiceMorphingMethod method = VoiceMorphingMethod::PitchShift;
    double pitchShiftSemitones = 0.0;
    double formantShiftRatio = 1.0;
    double timbreModification = 0.0;
    bool preserveEmotion = true;
    bool preserveProsody = true;
    double targetFundamentalFreq = 0.0;
    QString targetVoiceProfile;
};

struct VoiceMorphingResult {
    bool success = false;
    QByteArray processedAudio;
    double processingTime = 0.0;
    double qualityScore = 0.0;
    QString description;
};

class GNAVoiceMorphingEngine {
public:
    GNAVoiceMorphingEngine() = default;
    ~GNAVoiceMorphingEngine() = default;

    bool initialize(const GNACapabilities &caps) { _caps = caps; _initialized = true; return true; }
    bool isInitialized() const { return _initialized; }

    VoiceMorphingResult processVoice(const QByteArray &audioData, const VoiceMorphingConfig &config);
    void setCapabilities(const GNACapabilities &caps) { _caps = caps; }

private:
    QByteArray applyPitchShift(const QByteArray &audioData, double semitones);
    QByteArray applyFormantShift(const QByteArray &audioData, double ratio);

    GNACapabilities _caps;
    bool _initialized = false;
};

enum class CovertChannelProtocol {
    AudioFrequencyShift,
    UltrasonicCarrier,
    SpectralEncoding,
    PhaseShiftKeying,
    AmplitudeModulation
};

struct CovertChannelConfig {
    CovertChannelProtocol protocol = CovertChannelProtocol::AudioFrequencyShift;
    double carrierFrequency = 19000.0;
    double bandwidth = 1000.0;
    int dataRate = 100;
    bool encryptionEnabled = true;
    QString encryptionKey;
    bool errorCorrection = true;
    double signalAmplitude = 0.01;
};

struct CovertChannelMetrics {
    qint64 totalBytesTransmitted = 0;
    qint64 totalBytesReceived = 0;
    double averageLatency = 0.0;
    double bitErrorRate = 0.0;
    int successfulTransmissions = 0;
    int failedTransmissions = 0;
};

class GNACovertChannelEngine {
public:
    GNACovertChannelEngine() = default;
    ~GNACovertChannelEngine() = default;

    bool initialize(const GNACapabilities &caps) { _caps = caps; _initialized = true; return true; }
    bool isInitialized() const { return _initialized; }

    QByteArray transmitData(const QByteArray &payload, const CovertChannelConfig &config);
    QByteArray receiveData(const QByteArray &audioData, const CovertChannelConfig &config);
    CovertChannelMetrics getMetrics() const { return _metrics; }

    void setCapabilities(const GNACapabilities &caps) { _caps = caps; }

private:
    QByteArray encodeFrequencyShift(const QByteArray &payload, double carrierFreq, double bandwidth);
    QByteArray decodeFrequencyShift(const QByteArray &audioData, double carrierFreq, double bandwidth);

    GNACapabilities _caps;
    bool _initialized = false;
    CovertChannelMetrics _metrics;
};

} // namespace GNAEngines

enum class AcousticThreatCategory {
    SurveillanceDevice,
    VoiceRecognition,
    AudioKeylogger,
    RoomMicrophone,
    PhoneHome,
    VoiceDeepfake,
    AudioSteganography,
    UltrasonicBeacon,
    InaudibleCommand,
    Unknown
};

enum class VoiceSecurityMode {
    Disabled,
    Monitoring,
    ActiveProtection,
    MaximumSecurity
};

struct AcousticThreatAssessment {
    AcousticThreatCategory category = AcousticThreatCategory::Unknown;
    ThreatLevel level = ThreatLevel::None;
    double confidence = 0.0;
    QString description;
    QString mitigation;
    qint64 timestamp = 0;
    QByteArray threatSignature;
};

struct AcousticSecurityConfig {
    VoiceSecurityMode mode = VoiceSecurityMode::Monitoring;
    float sensitivity = 0.7f;
    bool detectUltrasonic = true;
    bool detectInfrasound = false;
    bool detectVoiceDeepfake = true;
    bool enableVoiceMorphing = false;
    bool enableSteganography = false;
    bool enableCovertChannel = false;
    int sampleRate = 48000;
    int channels = 1;
};

class GNAAcousticSecurity {
public:
    GNAAcousticSecurity(QObject *parent = nullptr) { (void)parent; }
    ~GNAAcousticSecurity() = default;

    bool initialize();
    bool isInitialized() const { return _initialized; }
    void shutdown();

    GNACapabilities detectCapabilities() const;
    void setCapabilities(const GNACapabilities &caps) { _capabilities = caps; }
    GNACapabilities getCapabilities() const { return _capabilities; }

    void setConfig(const AcousticSecurityConfig &config) { _config = config; }
    AcousticSecurityConfig getConfig() const { return _config; }

    AcousticThreatAssessment analyzeAudio(const QByteArray &audioData);
    QList<AcousticThreatAssessment> getRecentThreats() const { return _recent_threats; }

    QByteArray applyVoiceMorphing(const QByteArray &audioData, const GNAEngines::VoiceMorphingConfig &config);
    QByteArray embedSteganography(const QByteArray &audioData, const QByteArray &payload, const GNAEngines::SteganographyConfig &config);
    QByteArray extractSteganography(const QByteArray &audioData, const GNAEngines::SteganographyConfig &config);
    QByteArray transmitCovertChannel(const QByteArray &payload, const GNAEngines::CovertChannelConfig &config);
    QByteArray receiveCovertChannel(const QByteArray &audioData, const GNAEngines::CovertChannelConfig &config);

    std::function<void(const AcousticThreatAssessment &)> threatDetected;
    std::function<void(ThreatLevel)> threatLevelChanged;

private:
    bool detectSurveillanceDevice(const QByteArray &audioData, AcousticThreatAssessment &assessment);
    bool detectUltrasonicBeacon(const QByteArray &audioData, AcousticThreatAssessment &assessment);
    bool detectVoiceDeepfake(const QByteArray &audioData, AcousticThreatAssessment &assessment);
    bool detectAudioSteganography(const QByteArray &audioData, AcousticThreatAssessment &assessment);
    bool detectInaudibleCommand(const QByteArray &audioData, AcousticThreatAssessment &assessment);

    void updateThreatLevel(ThreatLevel newLevel);

    bool _initialized = false;
    GNACapabilities _capabilities;
    AcousticSecurityConfig _config;
    QList<AcousticThreatAssessment> _recent_threats;
    ThreatLevel _current_threat_level = ThreatLevel::None;

    std::unique_ptr<GNAEngines::GNASteganographyEngine> _steganography_engine;
    std::unique_ptr<GNAEngines::GNAVoiceMorphingEngine> _voice_morphing_engine;
    std::unique_ptr<GNAEngines::GNACovertChannelEngine> _covert_channel_engine;
};

} // namespace Security
