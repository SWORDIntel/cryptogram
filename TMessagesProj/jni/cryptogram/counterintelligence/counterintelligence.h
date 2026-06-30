#pragma once

#include "desktop_shims.h"
#include <memory>
#include <array>

namespace SpyGram::Counterintelligence {

enum class ThreatLevel {
    None = 0,
    Ambient = 1,
    Targeted = 2,
    Active = 3,
    Hostile = 4
};

enum class SurveillanceType {
    Unknown,
    AudioRecording,
    LaserMicrophone,
    RFEmission,
    UltrasonicBeacon,
    NetworkAnalysis,
    VisualSurveillance,
    TempestAttack
};

struct ThreatAssessment {
    ThreatLevel level = ThreatLevel::None;
    SurveillanceType type = SurveillanceType::Unknown;
    float confidence = 0.0f;
    QString details;
    qint64 timestamp = 0;
    QString mitigation_suggestion;
    bool government_exemption = false;
};

enum class CountermeasureType {
    None,
    AudioNoise,
    UltrasonicJamming,
    PrivacyEnhancement,
    NetworkObfuscation,
    VisualScrambling,
    CommunicationHiding,
    EmergencyMode
};

enum class CountermeasureIntensity {
    Minimal = 1,
    Moderate = 2,
    Aggressive = 3,
    Maximum = 4
};

struct CountermeasureStatus {
    CountermeasureType type = CountermeasureType::None;
    CountermeasureIntensity intensity = CountermeasureIntensity::Minimal;
    bool active = false;
    float effectiveness = 0.0f;
    QString description;
    qint64 activated_time = 0;
    bool government_compatible = true;
};

class SurveillanceDetector {
public:
    explicit SurveillanceDetector(QObject *parent = nullptr) { (void)parent; }
    ~SurveillanceDetector() { stopDetection(); }

    void startDetection();
    void stopDetection();
    bool isDetectionActive() const { return _detection_active; }

    void setGNAAvailable(bool available) { _gna_available = available; }
    void setNPUAvailable(bool available) { _npu_available = available; }
    void setOpenVINOAvailable(bool available) { _openvino_available = available; }
    void setGovernmentMode(bool enabled) { _government_mode = enabled; }
    bool isGovernmentMode() const { return _government_mode; }

    ThreatLevel getCurrentThreatLevel() const { return _current_threat_level; }
    QList<ThreatAssessment> getRecentThreats() const { return _recent_threats; }

    void setSensitivity(float sensitivity) { _sensitivity = sensitivity; }
    void setAudioAnalysisEnabled(bool enabled) { _audio_analysis_enabled = enabled; }
    void setNetworkAnalysisEnabled(bool enabled) { _network_analysis_enabled = enabled; }

    std::function<void(const ThreatAssessment &)> threatDetected;
    std::function<void(ThreatLevel, ThreatLevel)> threatLevelChanged;
    std::function<void(const QString &, bool)> hardwareCapabilityChanged;
    std::function<void(const QString &)> securityViolationDetected;

private:
    void processAudioData();
    void analyzeNetworkTraffic();
    void performPeriodicScan();
    void updateThreatLevel();

    ThreatAssessment analyzeWithGNA(const QByteArray &audioData);
    ThreatAssessment analyzeWithNPU(const QByteArray &audioData);
    ThreatAssessment analyzeWithOpenVINO(const QByteArray &audioData);
    ThreatAssessment analyzeWithCPUHeuristics(const QByteArray &audioData);
    ThreatAssessment analyzeWithBasicPatterns(const QByteArray &audioData);

    bool detectLaserMicrophone(const QByteArray &audioData);
    bool detectUltrasonicBeacons(const QByteArray &audioData);
    bool detectRFEmissions();
    bool detectNetworkSurveillance();
    bool detectTEMPESTSignals();

    void correlateThreats();
    void updateThreatHistory(const ThreatAssessment &threat);
    ThreatLevel calculateOverallThreatLevel();
    void adjustForGovernmentMode(ThreatAssessment &threat);
    bool isAuthorizedGovernmentActivity(const ThreatAssessment &threat);

    QByteArray _audio_buffer;
    bool _gna_available = false;
    bool _npu_available = false;
    bool _openvino_available = false;
    bool _cpu_optimized = true;
    bool _detection_active = false;
    bool _government_mode = false;
    ThreatLevel _current_threat_level = ThreatLevel::None;
    float _sensitivity = 0.7f;
    bool _audio_analysis_enabled = true;
    bool _network_analysis_enabled = true;
    bool _rf_analysis_enabled = true;
    bool _tempest_analysis_enabled = false;

    QList<ThreatAssessment> _recent_threats;
    QList<ThreatAssessment> _threat_history;
    static constexpr int MAX_THREAT_HISTORY = 100;

    qint64 _last_analysis_time = 0;
    int _analysis_count = 0;
    float _average_analysis_time = 0.0f;

    static constexpr int SAMPLE_RATE = 44100;
    static constexpr int SAMPLE_SIZE = 16;
    static constexpr int CHANNEL_COUNT = 1;
    static constexpr int ANALYSIS_WINDOW_MS = 100;
    static constexpr float ULTRASONIC_LOW_FREQ = 18000.0f;
    static constexpr float ULTRASONIC_HIGH_FREQ = 25000.0f;
    static constexpr float LASER_MIC_FREQ_LOW = 1000.0f;
    static constexpr float LASER_MIC_FREQ_HIGH = 8000.0f;
};

class AdaptiveCountermeasures {
public:
    explicit AdaptiveCountermeasures(QObject *parent = nullptr) { (void)parent; }
    ~AdaptiveCountermeasures() { stopCountermeasures(); }

    void startCountermeasures();
    void stopCountermeasures();
    bool isActive() const { return _active; }

    void respondToThreat(const ThreatAssessment &threat);
    void setThreatLevel(ThreatLevel level);

    void setMaxIntensity(CountermeasureIntensity max) { _max_intensity = max; }
    void setGovernmentMode(bool enabled) { _government_mode = enabled; }
    void setAutomaticResponse(bool enabled) { _automatic_response = enabled; }

    void setGNAAvailable(bool available) { _gna_available = available; }
    void setNPUAvailable(bool available) { _npu_available = available; }
    void setAudioOutputAvailable(bool available) { _audio_output_available = available; }

    void activateCountermeasure(CountermeasureType type, CountermeasureIntensity intensity);
    void deactivateCountermeasure(CountermeasureType type);
    void activateEmergencyMode();
    void deactivateEmergencyMode();

    QList<CountermeasureStatus> getActiveCountermeasures() const { return _active_countermeasures; }
    CountermeasureIntensity getCurrentIntensity() const { return _current_intensity; }
    bool isEmergencyModeActive() const { return _emergency_mode; }

    std::function<void(CountermeasureType, CountermeasureIntensity)> countermeasureActivated;
    std::function<void(CountermeasureType)> countermeasureDeactivated;
    std::function<void()> emergencyModeActivated;
    std::function<void()> emergencyModeDeactivated;
    std::function<void(CountermeasureIntensity, CountermeasureIntensity)> intensityChanged;

private:
    void updateCountermeasures();
    void generateAudioNoise();
    void updatePrivacySettings();
    void obfuscateNetworkTraffic();

    CountermeasureIntensity calculateRequiredIntensity(const ThreatAssessment &threat);
    QList<CountermeasureType> selectCountermeasures(ThreatLevel threatLevel, CountermeasureIntensity intensity);
    void deployCountermeasures(const QList<CountermeasureType> &measures, CountermeasureIntensity intensity);

    void activateAudioNoise(CountermeasureIntensity intensity);
    void activateUltrasonicJamming(CountermeasureIntensity intensity);
    void activatePrivacyEnhancement(CountermeasureIntensity intensity);
    void activateNetworkObfuscation(CountermeasureIntensity intensity);
    void activateVisualScrambling(CountermeasureIntensity intensity);
    void activateCommunicationHiding(CountermeasureIntensity intensity);

    void deactivateAudioNoise();
    void deactivateUltrasonicJamming();
    void deactivatePrivacyEnhancement();
    void deactivateNetworkObfuscation();
    void deactivateVisualScrambling();
    void deactivateCommunicationHiding();

    void generateWhiteNoise(float amplitude);
    void generatePinkNoise(float amplitude);
    void generateUltrasonicInterference(float frequency, float amplitude);

    void enhanceContactObfuscation(CountermeasureIntensity intensity);
    void enhanceLocationRandomization(CountermeasureIntensity intensity);
    void enhanceMetadataProtection(CountermeasureIntensity intensity);

    void activateDummyTraffic(CountermeasureIntensity intensity);
    void activateTrafficPadding(CountermeasureIntensity intensity);
    void activateTimingObfuscation(CountermeasureIntensity intensity);

    bool isCountermeasureGovernmentCompatible(CountermeasureType type);
    void adjustForGovernmentMode(CountermeasureStatus &status);
    float assessCountermeasureEffectiveness(CountermeasureType type, const ThreatAssessment &threat);
    void updateEffectivenessMetrics();

    bool _active = false;
    bool _government_mode = false;
    bool _automatic_response = true;
    bool _emergency_mode = false;
    bool _gna_available = false;
    bool _npu_available = false;
    bool _audio_output_available = false;

    ThreatLevel _current_threat_level = ThreatLevel::None;
    CountermeasureIntensity _current_intensity = CountermeasureIntensity::Minimal;
    CountermeasureIntensity _max_intensity = CountermeasureIntensity::Maximum;

    QList<CountermeasureStatus> _active_countermeasures;
    QHash<CountermeasureType, CountermeasureStatus> _countermeasure_status;

    QByteArray _noise_buffer;

    QList<ThreatAssessment> _recent_threats;
    qint64 _last_threat_time = 0;
    int _threat_escalation_count = 0;

    QHash<CountermeasureType, float> _effectiveness_history;
    QHash<CountermeasureType, int> _activation_count;
    qint64 _total_active_time = 0;

    static constexpr int NOISE_SAMPLE_RATE = 44100;
    static constexpr int NOISE_BUFFER_SIZE = 4096;
    static constexpr float ULTRASONIC_BASE_FREQ = 19000.0f;
    static constexpr float ULTRASONIC_MAX_FREQ = 23000.0f;
    static constexpr float MINIMAL_OBFUSCATION = 0.3f;
    static constexpr float MODERATE_OBFUSCATION = 0.6f;
    static constexpr float AGGRESSIVE_OBFUSCATION = 0.8f;
    static constexpr float MAXIMUM_OBFUSCATION = 1.0f;
};

class CounterIntelligenceDashboard {
public:
    explicit CounterIntelligenceDashboard(QObject *parent = nullptr) { (void)parent; }
    ~CounterIntelligenceDashboard() = default;

    void initialize() { _initialized = true; }
    bool isInitialized() const { return _initialized; }

    void updateThreatDisplay(const ThreatAssessment &threat) { _last_threat = threat; }
    void updateCountermeasureDisplay(const QList<CountermeasureStatus> &measures) { _active_measures = measures; }
    void updateSystemMetrics(float healthScore, float detectionRate, float responseTime) {
        _health_score = healthScore;
        _detection_rate = detectionRate;
        _response_time = responseTime;
    }

    ThreatAssessment getLastThreat() const { return _last_threat; }
    QList<CountermeasureStatus> getActiveMeasures() const { return _active_measures; }
    float getHealthScore() const { return _health_score; }

    QString generateReport() const;

    std::function<void(const QString &)> alertTriggered;
    std::function<void(const QString &)> statusUpdated;

private:
    bool _initialized = false;
    ThreatAssessment _last_threat;
    QList<CountermeasureStatus> _active_measures;
    float _health_score = 1.0f;
    float _detection_rate = 0.0f;
    float _response_time = 0.0f;
};

} // namespace SpyGram::Counterintelligence
