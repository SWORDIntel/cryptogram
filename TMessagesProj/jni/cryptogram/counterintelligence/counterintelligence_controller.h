#pragma once

#include "counterintelligence.h"
#include "desktop_shims.h"
#include <memory>

namespace SpyGram::Counterintelligence {

class CounterIntelligenceController {
public:
    explicit CounterIntelligenceController(QObject *parent = nullptr) { (void)parent; }
    ~CounterIntelligenceController() { shutdown(); }

    void initialize();
    void shutdown();
    bool isInitialized() const { return _initialized; }

    SurveillanceDetector* getSurveillanceDetector() const { return _surveillance_detector.get(); }
    AdaptiveCountermeasures* getAdaptiveCountermeasures() const { return _adaptive_countermeasures.get(); }
    CounterIntelligenceDashboard* getDashboard() const { return _dashboard.get(); }

    void setGovernmentMode(bool enabled) { _government_mode = enabled; }
    void setExpertMode(bool enabled) { _expert_mode = enabled; }
    void setHardwareAcceleration(bool gna, bool npu, bool openvino);
    void loadConfiguration();
    void saveConfiguration();

    void reportExternalThreat(const ThreatAssessment &threat);
    void setThreatResponseMode(bool automatic) { _automatic_response = automatic; }
    void activateEmergencyMode();
    void deactivateEmergencyMode();

    ThreatLevel getCurrentThreatLevel() const;
    bool isEmergencyModeActive() const { return _emergency_mode; }
    QList<ThreatAssessment> getRecentThreats() const;
    QList<CountermeasureStatus> getActiveCountermeasures() const;

    bool isGNAAvailable() const { return _gna_available; }
    bool isNPUAvailable() const { return _npu_available; }
    bool isOpenVINOAvailable() const { return _openvino_available; }
    void refreshHardwareCapabilities();

    struct SystemMetrics {
        float detection_rate = 0.0f;
        float average_response_time = 0.0f;
        float system_load = 0.0f;
        float countermeasure_effectiveness = 0.0f;
        int active_countermeasures = 0;
        qint64 uptime = 0;
    };

    SystemMetrics getSystemMetrics() const { return _system_metrics; }

    std::function<void()> systemInitialized;
    std::function<void()> systemShutdown;
    std::function<void(const QString &, bool)> hardwareCapabilityChanged;
    std::function<void(const ThreatAssessment &)> criticalThreatDetected;
    std::function<void(ThreatLevel, ThreatLevel)> threatLevelEscalated;
    std::function<void()> emergencyModeActivated;
    std::function<void()> emergencyModeDeactivated;
    std::function<void(float)> systemHealthChanged;
    std::function<void(const QString &, const QString &)> performanceAlert;

private:
    void initializeComponents();
    void connectSignals();
    void detectHardwareCapabilities();
    void loadDefaultConfiguration();

    bool detectGNACapability();
    bool detectNPUCapability();
    bool detectOpenVINOCapability();
    bool detectAudioCapability();

    void correlateThreatData(const ThreatAssessment &threat);
    void analyzeThreatPatterns();
    void updateThreatIntelligence();

    void optimizeForHardwareConfiguration();
    void balancePerformanceAndSecurity();
    void adjustResourceAllocation();

    void applyConfiguration();
    void validateConfiguration();

    float calculateSystemHealth();
    void checkComponentHealth();
    void detectPerformanceIssues();

    void onThreatDetected(const ThreatAssessment &threat);
    void onThreatLevelChanged(ThreatLevel newLevel, ThreatLevel oldLevel);
    void onCountermeasureActivated(CountermeasureType type, CountermeasureIntensity intensity);
    void onCountermeasureDeactivated(CountermeasureType type);
    void onEmergencyModeChanged();

    void updateSystemHealth();
    void updatePerformanceMetrics();
    void performHealthCheck();
    void optimizeSystemPerformance();

    std::unique_ptr<SurveillanceDetector> _surveillance_detector;
    std::unique_ptr<AdaptiveCountermeasures> _adaptive_countermeasures;
    std::unique_ptr<CounterIntelligenceDashboard> _dashboard;

    bool _initialized = false;
    bool _government_mode = false;
    bool _expert_mode = false;
    bool _emergency_mode = false;
    bool _automatic_response = true;

    bool _gna_available = false;
    bool _npu_available = false;
    bool _openvino_available = false;
    bool _audio_available = false;

    SystemMetrics _system_metrics;
    float _system_health = 1.0f;
    QList<ThreatAssessment> _threat_intelligence;

    qint64 _initialization_time = 0;
    qint64 _last_health_check = 0;
    int _threat_count_last_minute = 0;
    int _total_threats_detected = 0;
    QList<qint64> _response_times;

    static constexpr int MAX_THREAT_INTELLIGENCE_SIZE = 500;
    static constexpr float HEALTH_WARNING_THRESHOLD = 0.8f;
    static constexpr float HEALTH_CRITICAL_THRESHOLD = 0.6f;
};

} // namespace SpyGram::Counterintelligence
