#include "counterintelligence_controller.h"
#include <sys/stat.h>
#include <ctime>
#include <algorithm>

namespace SpyGram::Counterintelligence {

static qint64 currentTimeMillis() {
    return static_cast<qint64>(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count());
}

void CounterIntelligenceController::initialize() {
    if (_initialized) return;

    initializeComponents();
    detectHardwareCapabilities();
    connectSignals();
    loadDefaultConfiguration();

    _surveillance_detector->startDetection();
    _adaptive_countermeasures->startCountermeasures();
    _dashboard->initialize();

    _initialization_time = currentTimeMillis();
    _initialized = true;

    if (systemInitialized) systemInitialized();
}

void CounterIntelligenceController::shutdown() {
    if (!_initialized) return;

    _adaptive_countermeasures->stopCountermeasures();
    _surveillance_detector->stopDetection();

    _initialized = false;
    if (systemShutdown) systemShutdown();
}

void CounterIntelligenceController::initializeComponents() {
    _surveillance_detector = std::make_unique<SurveillanceDetector>();
    _adaptive_countermeasures = std::make_unique<AdaptiveCountermeasures>();
    _dashboard = std::make_unique<CounterIntelligenceDashboard>();
}

void CounterIntelligenceController::connectSignals() {
    if (_surveillance_detector) {
        _surveillance_detector->threatDetected = [this](const ThreatAssessment &threat) {
            onThreatDetected(threat);
        };
        _surveillance_detector->threatLevelChanged = [this](ThreatLevel newLevel, ThreatLevel oldLevel) {
            onThreatLevelChanged(newLevel, oldLevel);
        };
    }
    if (_adaptive_countermeasures) {
        _adaptive_countermeasures->countermeasureActivated = [this](CountermeasureType type, CountermeasureIntensity intensity) {
            onCountermeasureActivated(type, intensity);
        };
        _adaptive_countermeasures->countermeasureDeactivated = [this](CountermeasureType type) {
            onCountermeasureDeactivated(type);
        };
        _adaptive_countermeasures->emergencyModeActivated = [this]() {
            _emergency_mode = true;
            if (emergencyModeActivated) emergencyModeActivated();
        };
        _adaptive_countermeasures->emergencyModeDeactivated = [this]() {
            _emergency_mode = false;
            if (emergencyModeDeactivated) emergencyModeDeactivated();
        };
    }
}

void CounterIntelligenceController::detectHardwareCapabilities() {
    _gna_available = detectGNACapability();
    _npu_available = detectNPUCapability();
    _openvino_available = detectOpenVINOCapability();
    _audio_available = detectAudioCapability();

    if (_surveillance_detector) {
        _surveillance_detector->setGNAAvailable(_gna_available);
        _surveillance_detector->setNPUAvailable(_npu_available);
        _surveillance_detector->setOpenVINOAvailable(_openvino_available);
    }
    if (_adaptive_countermeasures) {
        _adaptive_countermeasures->setGNAAvailable(_gna_available);
        _adaptive_countermeasures->setNPUAvailable(_npu_available);
        _adaptive_countermeasures->setAudioOutputAvailable(_audio_available);
    }
}

bool CounterIntelligenceController::detectGNACapability() {
    struct stat st;
    return ::stat("/dev/intel_gna", &st) == 0;
}

bool CounterIntelligenceController::detectNPUCapability() {
    struct stat st;
    return ::stat("/dev/accel/accel0", &st) == 0;
}

bool CounterIntelligenceController::detectOpenVINOCapability() {
    struct stat st;
    return ::stat("/opt/intel/openvino", &st) == 0;
}

bool CounterIntelligenceController::detectAudioCapability() {
    return true;
}

void CounterIntelligenceController::setHardwareAcceleration(bool gna, bool npu, bool openvino) {
    _gna_available = gna;
    _npu_available = npu;
    _openvino_available = openvino;

    if (_surveillance_detector) {
        _surveillance_detector->setGNAAvailable(gna);
        _surveillance_detector->setNPUAvailable(npu);
        _surveillance_detector->setOpenVINOAvailable(openvino);
    }
    if (_adaptive_countermeasures) {
        _adaptive_countermeasures->setGNAAvailable(gna);
        _adaptive_countermeasures->setNPUAvailable(npu);
    }

    if (hardwareCapabilityChanged) {
        hardwareCapabilityChanged("GNA", gna);
        hardwareCapabilityChanged("NPU", npu);
        hardwareCapabilityChanged("OpenVINO", openvino);
    }
}

void CounterIntelligenceController::refreshHardwareCapabilities() {
    detectHardwareCapabilities();
}

void CounterIntelligenceController::loadDefaultConfiguration() {
    if (_surveillance_detector) {
        _surveillance_detector->setSensitivity(0.7f);
        _surveillance_detector->setAudioAnalysisEnabled(true);
        _surveillance_detector->setNetworkAnalysisEnabled(true);
    }
    if (_adaptive_countermeasures) {
        _adaptive_countermeasures->setMaxIntensity(CountermeasureIntensity::Maximum);
        _adaptive_countermeasures->setAutomaticResponse(true);
    }
}

void CounterIntelligenceController::loadConfiguration() {
    loadDefaultConfiguration();
}

void CounterIntelligenceController::saveConfiguration() {}

void CounterIntelligenceController::reportExternalThreat(const ThreatAssessment &threat) {
    correlateThreatData(threat);
    if (_adaptive_countermeasures && _automatic_response) {
        _adaptive_countermeasures->respondToThreat(threat);
    }
}

void CounterIntelligenceController::activateEmergencyMode() {
    if (_adaptive_countermeasures) {
        _adaptive_countermeasures->activateEmergencyMode();
    }
    _emergency_mode = true;
}

void CounterIntelligenceController::deactivateEmergencyMode() {
    if (_adaptive_countermeasures) {
        _adaptive_countermeasures->deactivateEmergencyMode();
    }
    _emergency_mode = false;
}

ThreatLevel CounterIntelligenceController::getCurrentThreatLevel() const {
    if (_surveillance_detector) return _surveillance_detector->getCurrentThreatLevel();
    return ThreatLevel::None;
}

QList<ThreatAssessment> CounterIntelligenceController::getRecentThreats() const {
    if (_surveillance_detector) return _surveillance_detector->getRecentThreats();
    return {};
}

QList<CountermeasureStatus> CounterIntelligenceController::getActiveCountermeasures() const {
    if (_adaptive_countermeasures) return _adaptive_countermeasures->getActiveCountermeasures();
    return {};
}

void CounterIntelligenceController::onThreatDetected(const ThreatAssessment &threat) {
    _total_threats_detected++;
    _threat_count_last_minute++;

    correlateThreatData(threat);

    if (threat.level >= ThreatLevel::Active && criticalThreatDetected) {
        criticalThreatDetected(threat);
    }

    if (_dashboard) {
        _dashboard->updateThreatDisplay(threat);
    }

    if (_adaptive_countermeasures && _automatic_response) {
        _adaptive_countermeasures->respondToThreat(threat);
    }
}

void CounterIntelligenceController::onThreatLevelChanged(ThreatLevel newLevel, ThreatLevel oldLevel) {
    if (newLevel > oldLevel && threatLevelEscalated) {
        threatLevelEscalated(newLevel, oldLevel);
    }
    if (_adaptive_countermeasures) {
        _adaptive_countermeasures->setThreatLevel(newLevel);
    }
}

void CounterIntelligenceController::onCountermeasureActivated(CountermeasureType type, CountermeasureIntensity intensity) {
    (void)type; (void)intensity;
    if (_dashboard) {
        _dashboard->updateCountermeasureDisplay(_adaptive_countermeasures->getActiveCountermeasures());
    }
}

void CounterIntelligenceController::onCountermeasureDeactivated(CountermeasureType type) {
    (void)type;
    if (_dashboard) {
        _dashboard->updateCountermeasureDisplay(_adaptive_countermeasures->getActiveCountermeasures());
    }
}

void CounterIntelligenceController::onEmergencyModeChanged() {
    if (_emergency_mode && emergencyModeActivated) emergencyModeActivated();
    else if (!_emergency_mode && emergencyModeDeactivated) emergencyModeDeactivated();
}

void CounterIntelligenceController::correlateThreatData(const ThreatAssessment &threat) {
    _threat_intelligence.append(threat);
    if (_threat_intelligence.size() > MAX_THREAT_INTELLIGENCE_SIZE) {
        _threat_intelligence.erase(_threat_intelligence.begin());
    }
}

void CounterIntelligenceController::analyzeThreatPatterns() {
    if (_threat_intelligence.size() < 10) return;
}

void CounterIntelligenceController::updateThreatIntelligence() {
    analyzeThreatPatterns();
}

void CounterIntelligenceController::optimizeForHardwareConfiguration() {
    if (_npu_available && _surveillance_detector) {
        _surveillance_detector->setNPUAvailable(true);
    }
}

void CounterIntelligenceController::balancePerformanceAndSecurity() {}

void CounterIntelligenceController::adjustResourceAllocation() {}

void CounterIntelligenceController::applyConfiguration() {
    if (_surveillance_detector) {
        _surveillance_detector->setGovernmentMode(_government_mode);
    }
    if (_adaptive_countermeasures) {
        _adaptive_countermeasures->setGovernmentMode(_government_mode);
        _adaptive_countermeasures->setAutomaticResponse(_automatic_response);
    }
}

void CounterIntelligenceController::validateConfiguration() {}

float CounterIntelligenceController::calculateSystemHealth() {
    float health = 1.0f;
    if (!_surveillance_detector || !_surveillance_detector->isDetectionActive()) {
        health -= 0.3f;
    }
    if (!_adaptive_countermeasures || !_adaptive_countermeasures->isActive()) {
        health -= 0.2f;
    }
    if (_system_metrics.system_load > 0.8f) {
        health -= 0.2f;
    }
    if (_system_metrics.average_response_time > 1000.0f) {
        health -= 0.1f;
    }
    return std::max(0.0f, health);
}

void CounterIntelligenceController::checkComponentHealth() {
    float health = calculateSystemHealth();
    if (health != _system_health) {
        _system_health = health;
        if (systemHealthChanged) systemHealthChanged(health);
    }
}

void CounterIntelligenceController::detectPerformanceIssues() {
    if (_system_metrics.system_load > 0.8f && performanceAlert) {
        performanceAlert("System", "High CPU load detected");
    }
    if (_system_metrics.average_response_time > 1000.0f && performanceAlert) {
        performanceAlert("Response", "Slow response time detected");
    }
}

void CounterIntelligenceController::updateSystemHealth() {
    checkComponentHealth();
}

void CounterIntelligenceController::updatePerformanceMetrics() {
    if (_initialization_time > 0) {
        _system_metrics.uptime = (currentTimeMillis() - _initialization_time) / 1000;
    }
    _system_metrics.active_countermeasures = _adaptive_countermeasures ? 
        _adaptive_countermeasures->getActiveCountermeasures().size() : 0;
    _system_metrics.detection_rate = static_cast<float>(_threat_count_last_minute);
    _threat_count_last_minute = 0;

    if (_dashboard) {
        _dashboard->updateSystemMetrics(_system_health, _system_metrics.detection_rate, _system_metrics.average_response_time);
    }
}

void CounterIntelligenceController::performHealthCheck() {
    _last_health_check = currentTimeMillis();
    checkComponentHealth();
    detectPerformanceIssues();
}

void CounterIntelligenceController::optimizeSystemPerformance() {
    optimizeForHardwareConfiguration();
    balancePerformanceAndSecurity();
}

} // namespace SpyGram::Counterintelligence
