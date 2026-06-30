#include "counterintelligence.h"
#include <sys/stat.h>
#include <unistd.h>
#include <ctime>
#include <cmath>
#include <random>
#include <algorithm>

namespace SpyGram::Counterintelligence {

static qint64 currentTimeMillis() {
    return static_cast<qint64>(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count());
}

// ===== SurveillanceDetector =====

void SurveillanceDetector::startDetection() {
    if (_detection_active) return;
    _detection_active = true;
}

void SurveillanceDetector::stopDetection() {
    _detection_active = false;
}

void SurveillanceDetector::processAudioData() {
    if (!_detection_active || !_audio_analysis_enabled) return;

    ThreatAssessment assessment;
    assessment.timestamp = currentTimeMillis();

    if (_gna_available) {
        assessment = analyzeWithGNA(_audio_buffer);
    } else if (_npu_available) {
        assessment = analyzeWithNPU(_audio_buffer);
    } else if (_openvino_available) {
        assessment = analyzeWithOpenVINO(_audio_buffer);
    } else if (_cpu_optimized) {
        assessment = analyzeWithCPUHeuristics(_audio_buffer);
    } else {
        assessment = analyzeWithBasicPatterns(_audio_buffer);
    }

    if (assessment.level != ThreatLevel::None) {
        adjustForGovernmentMode(assessment);
        updateThreatHistory(assessment);
        if (threatDetected) threatDetected(assessment);
    }

    updateThreatLevel();
}

void SurveillanceDetector::analyzeNetworkTraffic() {
    if (!_detection_active || !_network_analysis_enabled) return;

    if (detectNetworkSurveillance()) {
        ThreatAssessment threat;
        threat.level = ThreatLevel::Targeted;
        threat.type = SurveillanceType::NetworkAnalysis;
        threat.confidence = 0.6f;
        threat.timestamp = currentTimeMillis();
        threat.details = "Network surveillance detected";
        threat.mitigation_suggestion = "Enable network obfuscation";
        adjustForGovernmentMode(threat);
        updateThreatHistory(threat);
        if (threatDetected) threatDetected(threat);
    }
}

void SurveillanceDetector::performPeriodicScan() {
    if (!_detection_active) return;

    if (_rf_analysis_enabled && detectRFEmissions()) {
        ThreatAssessment threat;
        threat.level = ThreatLevel::Active;
        threat.type = SurveillanceType::RFEmission;
        threat.confidence = 0.7f;
        threat.timestamp = currentTimeMillis();
        threat.details = "RF emission detected";
        threat.mitigation_suggestion = "Check for nearby surveillance devices";
        adjustForGovernmentMode(threat);
        updateThreatHistory(threat);
        if (threatDetected) threatDetected(threat);
    }

    if (_tempest_analysis_enabled && detectTEMPESTSignals()) {
        ThreatAssessment threat;
        threat.level = ThreatLevel::Hostile;
        threat.type = SurveillanceType::TempestAttack;
        threat.confidence = 0.5f;
        threat.timestamp = currentTimeMillis();
        threat.details = "TEMPEST attack signature detected";
        threat.mitigation_suggestion = "Activate maximum countermeasures";
        adjustForGovernmentMode(threat);
        updateThreatHistory(threat);
        if (threatDetected) threatDetected(threat);
    }

    correlateThreats();
    updateThreatLevel();
}

void SurveillanceDetector::updateThreatLevel() {
    ThreatLevel newLevel = calculateOverallThreatLevel();
    if (newLevel != _current_threat_level) {
        ThreatLevel oldLevel = _current_threat_level;
        _current_threat_level = newLevel;
        if (threatLevelChanged) threatLevelChanged(newLevel, oldLevel);
    }
}

ThreatAssessment SurveillanceDetector::analyzeWithGNA(const QByteArray &audioData) {
    ThreatAssessment assessment;
    assessment.timestamp = currentTimeMillis();
    if (detectUltrasonicBeacons(audioData)) {
        assessment.level = ThreatLevel::Targeted;
        assessment.type = SurveillanceType::UltrasonicBeacon;
        assessment.confidence = 0.85f;
        assessment.details = "Ultrasonic beacon detected via GNA";
    }
    return assessment;
}

ThreatAssessment SurveillanceDetector::analyzeWithNPU(const QByteArray &audioData) {
    ThreatAssessment assessment;
    assessment.timestamp = currentTimeMillis();
    if (detectLaserMicrophone(audioData)) {
        assessment.level = ThreatLevel::Active;
        assessment.type = SurveillanceType::LaserMicrophone;
        assessment.confidence = 0.8f;
        assessment.details = "Laser microphone signature detected via NPU";
    }
    if (detectUltrasonicBeacons(audioData)) {
        assessment.level = std::max(assessment.level, ThreatLevel::Targeted);
        assessment.type = SurveillanceType::UltrasonicBeacon;
        assessment.confidence = std::max(assessment.confidence, 0.75f);
        assessment.details = "Ultrasonic beacon detected via NPU";
    }
    return assessment;
}

ThreatAssessment SurveillanceDetector::analyzeWithOpenVINO(const QByteArray &audioData) {
    ThreatAssessment assessment;
    assessment.timestamp = currentTimeMillis();
    if (detectLaserMicrophone(audioData)) {
        assessment.level = ThreatLevel::Targeted;
        assessment.type = SurveillanceType::LaserMicrophone;
        assessment.confidence = 0.7f;
        assessment.details = "Laser microphone pattern detected via OpenVINO";
    }
    return assessment;
}

ThreatAssessment SurveillanceDetector::analyzeWithCPUHeuristics(const QByteArray &audioData) {
    ThreatAssessment assessment;
    assessment.timestamp = currentTimeMillis();
    if (detectUltrasonicBeacons(audioData)) {
        assessment.level = ThreatLevel::Ambient;
        assessment.type = SurveillanceType::UltrasonicBeacon;
        assessment.confidence = 0.5f;
        assessment.details = "Possible ultrasonic beacon detected via CPU heuristics";
    }
    return assessment;
}

ThreatAssessment SurveillanceDetector::analyzeWithBasicPatterns(const QByteArray &audioData) {
    ThreatAssessment assessment;
    assessment.timestamp = currentTimeMillis();
    assessment.level = ThreatLevel::None;
    return assessment;
}

bool SurveillanceDetector::detectLaserMicrophone(const QByteArray &audioData) {
    if (audioData.size() < 1024) return false;
    return false;
}

bool SurveillanceDetector::detectUltrasonicBeacons(const QByteArray &audioData) {
    if (audioData.size() < 1024) return false;
    return false;
}

bool SurveillanceDetector::detectRFEmissions() {
    return false;
}

bool SurveillanceDetector::detectNetworkSurveillance() {
    return false;
}

bool SurveillanceDetector::detectTEMPESTSignals() {
    return false;
}

void SurveillanceDetector::correlateThreats() {
    if (_threat_history.size() < 3) return;
    int recentCount = 0;
    qint64 now = currentTimeMillis();
    for (const auto &t : _threat_history) {
        if (now - t.timestamp < 60000) recentCount++;
    }
    if (recentCount >= 5 && _current_threat_level < ThreatLevel::Hostile) {
        if (threatLevelChanged) threatLevelChanged(ThreatLevel::Hostile, _current_threat_level);
        _current_threat_level = ThreatLevel::Hostile;
    }
}

void SurveillanceDetector::updateThreatHistory(const ThreatAssessment &threat) {
    _recent_threats.insert(_recent_threats.begin(), threat);
    _threat_history.push_back(threat);
    if (_threat_history.size() > MAX_THREAT_HISTORY) {
        _threat_history.erase(_threat_history.begin());
    }
    if (_recent_threats.size() > 20) {
        _recent_threats.pop_back();
    }
}

ThreatLevel SurveillanceDetector::calculateOverallThreatLevel() {
    if (_recent_threats.isEmpty()) return ThreatLevel::None;
    ThreatLevel maxLevel = ThreatLevel::None;
    for (const auto &t : _recent_threats) {
        if (t.level > maxLevel) maxLevel = t.level;
    }
    return maxLevel;
}

void SurveillanceDetector::adjustForGovernmentMode(ThreatAssessment &threat) {
    if (_government_mode && isAuthorizedGovernmentActivity(threat)) {
        threat.government_exemption = true;
        threat.level = std::max(threat.level, ThreatLevel::None);
        if (threat.level > ThreatLevel::Ambient) {
            threat.level = ThreatLevel::Ambient;
        }
    }
}

bool SurveillanceDetector::isAuthorizedGovernmentActivity(const ThreatAssessment &threat) {
    return threat.type == SurveillanceType::RFEmission ||
           threat.type == SurveillanceType::VisualSurveillance;
}

// ===== AdaptiveCountermeasures =====

void AdaptiveCountermeasures::startCountermeasures() {
    if (_active) return;
    _active = true;
}

void AdaptiveCountermeasures::stopCountermeasures() {
    if (!_active) return;
    _active = false;
    for (auto &cm : _active_countermeasures) {
        cm.active = false;
        if (countermeasureDeactivated) countermeasureDeactivated(cm.type);
    }
    _active_countermeasures.clear();
}

void AdaptiveCountermeasures::respondToThreat(const ThreatAssessment &threat) {
    if (!_active || !_automatic_response) return;

    CountermeasureIntensity required = calculateRequiredIntensity(threat);
    QList<CountermeasureType> measures = selectCountermeasures(threat.level, required);
    deployCountermeasures(measures, required);

    _recent_threats.insert(_recent_threats.begin(), threat);
    _last_threat_time = currentTimeMillis();
}

void AdaptiveCountermeasures::setThreatLevel(ThreatLevel level) {
    _current_threat_level = level;
    if (_automatic_response && _active) {
        CountermeasureIntensity intensity = CountermeasureIntensity::Minimal;
        switch (level) {
            case ThreatLevel::Ambient: intensity = CountermeasureIntensity::Minimal; break;
            case ThreatLevel::Targeted: intensity = CountermeasureIntensity::Moderate; break;
            case ThreatLevel::Active: intensity = CountermeasureIntensity::Aggressive; break;
            case ThreatLevel::Hostile: intensity = CountermeasureIntensity::Maximum; break;
            default: break;
        }
        if (intensity != _current_intensity) {
            auto old = _current_intensity;
            _current_intensity = intensity;
            if (intensityChanged) intensityChanged(_current_intensity, old);
        }
    }
}

CountermeasureIntensity AdaptiveCountermeasures::calculateRequiredIntensity(const ThreatAssessment &threat) {
    switch (threat.level) {
        case ThreatLevel::Ambient: return CountermeasureIntensity::Minimal;
        case ThreatLevel::Targeted: return CountermeasureIntensity::Moderate;
        case ThreatLevel::Active: return CountermeasureIntensity::Aggressive;
        case ThreatLevel::Hostile: return CountermeasureIntensity::Maximum;
        default: return CountermeasureIntensity::Minimal;
    }
}

QList<CountermeasureType> AdaptiveCountermeasures::selectCountermeasures(ThreatLevel threatLevel, CountermeasureIntensity intensity) {
    QList<CountermeasureType> measures;
    if (threatLevel >= ThreatLevel::Ambient) {
        measures.append(CountermeasureType::PrivacyEnhancement);
    }
    if (threatLevel >= ThreatLevel::Targeted) {
        measures.append(CountermeasureType::NetworkObfuscation);
        if (_audio_output_available) {
            measures.append(CountermeasureType::AudioNoise);
        }
    }
    if (threatLevel >= ThreatLevel::Active) {
        measures.append(CountermeasureType::CommunicationHiding);
        if (_audio_output_available) {
            measures.append(CountermeasureType::UltrasonicJamming);
        }
    }
    if (threatLevel >= ThreatLevel::Hostile) {
        measures.append(CountermeasureType::VisualScrambling);
        measures.append(CountermeasureType::EmergencyMode);
    }

    if (_government_mode) {
        measures.erase(std::remove_if(measures.begin(), measures.end(),
            [this](CountermeasureType t) { return !isCountermeasureGovernmentCompatible(t); }),
            measures.end());
    }

    return measures;
}

void AdaptiveCountermeasures::deployCountermeasures(const QList<CountermeasureType> &measures, CountermeasureIntensity intensity) {
    for (auto type : measures) {
        activateCountermeasure(type, intensity);
    }
}

void AdaptiveCountermeasures::activateCountermeasure(CountermeasureType type, CountermeasureIntensity intensity) {
    if (type == CountermeasureType::None) return;

    CountermeasureStatus status;
    status.type = type;
    status.intensity = intensity;
    status.active = true;
    status.activated_time = currentTimeMillis();
    status.effectiveness = assessCountermeasureEffectiveness(type, ThreatAssessment());

    switch (type) {
        case CountermeasureType::AudioNoise:
            status.description = "Audio noise generation";
            activateAudioNoise(intensity);
            break;
        case CountermeasureType::UltrasonicJamming:
            status.description = "Ultrasonic jamming";
            activateUltrasonicJamming(intensity);
            break;
        case CountermeasureType::PrivacyEnhancement:
            status.description = "Privacy enhancement";
            activatePrivacyEnhancement(intensity);
            break;
        case CountermeasureType::NetworkObfuscation:
            status.description = "Network obfuscation";
            activateNetworkObfuscation(intensity);
            break;
        case CountermeasureType::VisualScrambling:
            status.description = "Visual scrambling";
            activateVisualScrambling(intensity);
            break;
        case CountermeasureType::CommunicationHiding:
            status.description = "Communication hiding";
            activateCommunicationHiding(intensity);
            break;
        case CountermeasureType::EmergencyMode:
            status.description = "Emergency mode";
            activateEmergencyMode();
            break;
        default: break;
    }

    adjustForGovernmentMode(status);
    _active_countermeasures.append(status);
    _countermeasure_status[type] = status;
    _activation_count[type]++;

    if (countermeasureActivated) countermeasureActivated(type, intensity);
}

void AdaptiveCountermeasures::deactivateCountermeasure(CountermeasureType type) {
    for (int i = 0; i < _active_countermeasures.size(); ++i) {
        if (_active_countermeasures[i].type == type) {
            _active_countermeasures[i].active = false;
            _active_countermeasures.erase(_active_countermeasures.begin() + i);
            break;
        }
    }
    _countermeasure_status.remove(type);

    switch (type) {
        case CountermeasureType::AudioNoise: deactivateAudioNoise(); break;
        case CountermeasureType::UltrasonicJamming: deactivateUltrasonicJamming(); break;
        case CountermeasureType::PrivacyEnhancement: deactivatePrivacyEnhancement(); break;
        case CountermeasureType::NetworkObfuscation: deactivateNetworkObfuscation(); break;
        case CountermeasureType::VisualScrambling: deactivateVisualScrambling(); break;
        case CountermeasureType::CommunicationHiding: deactivateCommunicationHiding(); break;
        default: break;
    }

    if (countermeasureDeactivated) countermeasureDeactivated(type);
}

void AdaptiveCountermeasures::activateEmergencyMode() {
    _emergency_mode = true;
    activateCountermeasure(CountermeasureType::AudioNoise, CountermeasureIntensity::Maximum);
    activateCountermeasure(CountermeasureType::PrivacyEnhancement, CountermeasureIntensity::Maximum);
    activateCountermeasure(CountermeasureType::NetworkObfuscation, CountermeasureIntensity::Maximum);
    if (emergencyModeActivated) emergencyModeActivated();
}

void AdaptiveCountermeasures::deactivateEmergencyMode() {
    _emergency_mode = false;
    if (emergencyModeDeactivated) emergencyModeDeactivated();
}

void AdaptiveCountermeasures::activateAudioNoise(CountermeasureIntensity intensity) {
    float amplitude = 0.1f;
    switch (intensity) {
        case CountermeasureIntensity::Minimal: amplitude = 0.05f; break;
        case CountermeasureIntensity::Moderate: amplitude = 0.1f; break;
        case CountermeasureIntensity::Aggressive: amplitude = 0.2f; break;
        case CountermeasureIntensity::Maximum: amplitude = 0.3f; break;
    }
    generateWhiteNoise(amplitude);
}

void AdaptiveCountermeasures::activateUltrasonicJamming(CountermeasureIntensity intensity) {
    float amplitude = 0.1f;
    switch (intensity) {
        case CountermeasureIntensity::Minimal: amplitude = 0.05f; break;
        case CountermeasureIntensity::Moderate: amplitude = 0.1f; break;
        case CountermeasureIntensity::Aggressive: amplitude = 0.15f; break;
        case CountermeasureIntensity::Maximum: amplitude = 0.2f; break;
    }
    generateUltrasonicInterference(ULTRASONIC_BASE_FREQ, amplitude);
}

void AdaptiveCountermeasures::activatePrivacyEnhancement(CountermeasureIntensity intensity) {
    enhanceContactObfuscation(intensity);
    enhanceLocationRandomization(intensity);
    enhanceMetadataProtection(intensity);
}

void AdaptiveCountermeasures::activateNetworkObfuscation(CountermeasureIntensity intensity) {
    activateDummyTraffic(intensity);
    activateTrafficPadding(intensity);
    activateTimingObfuscation(intensity);
}

void AdaptiveCountermeasures::activateVisualScrambling(CountermeasureIntensity intensity) {
    (void)intensity;
}

void AdaptiveCountermeasures::activateCommunicationHiding(CountermeasureIntensity intensity) {
    (void)intensity;
}

void AdaptiveCountermeasures::deactivateAudioNoise() {}
void AdaptiveCountermeasures::deactivateUltrasonicJamming() {}
void AdaptiveCountermeasures::deactivatePrivacyEnhancement() {}
void AdaptiveCountermeasures::deactivateNetworkObfuscation() {}
void AdaptiveCountermeasures::deactivateVisualScrambling() {}
void AdaptiveCountermeasures::deactivateCommunicationHiding() {}

void AdaptiveCountermeasures::generateWhiteNoise(float amplitude) {
    static std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> dist(-amplitude, amplitude);
    _noise_buffer.resize(NOISE_BUFFER_SIZE);
    for (int i = 0; i < NOISE_BUFFER_SIZE; ++i) {
        float sample = dist(rng);
        _noise_buffer[i] = static_cast<char>(sample * 127.0f);
    }
}

void AdaptiveCountermeasures::generatePinkNoise(float amplitude) {
    generateWhiteNoise(amplitude * 0.7f);
}

void AdaptiveCountermeasures::generateUltrasonicInterference(float frequency, float amplitude) {
    (void)frequency; (void)amplitude;
}

void AdaptiveCountermeasures::enhanceContactObfuscation(CountermeasureIntensity intensity) {
    (void)intensity;
}

void AdaptiveCountermeasures::enhanceLocationRandomization(CountermeasureIntensity intensity) {
    (void)intensity;
}

void AdaptiveCountermeasures::enhanceMetadataProtection(CountermeasureIntensity intensity) {
    (void)intensity;
}

void AdaptiveCountermeasures::activateDummyTraffic(CountermeasureIntensity intensity) {
    (void)intensity;
}

void AdaptiveCountermeasures::activateTrafficPadding(CountermeasureIntensity intensity) {
    (void)intensity;
}

void AdaptiveCountermeasures::activateTimingObfuscation(CountermeasureIntensity intensity) {
    (void)intensity;
}

bool AdaptiveCountermeasures::isCountermeasureGovernmentCompatible(CountermeasureType type) {
    switch (type) {
        case CountermeasureType::AudioNoise: return false;
        case CountermeasureType::UltrasonicJamming: return false;
        case CountermeasureType::EmergencyMode: return false;
        default: return true;
    }
}

void AdaptiveCountermeasures::adjustForGovernmentMode(CountermeasureStatus &status) {
    if (_government_mode && !status.government_compatible) {
        status.active = false;
    }
}

float AdaptiveCountermeasures::assessCountermeasureEffectiveness(CountermeasureType type, const ThreatAssessment &threat) {
    (void)threat;
    switch (type) {
        case CountermeasureType::AudioNoise: return 0.7f;
        case CountermeasureType::UltrasonicJamming: return 0.8f;
        case CountermeasureType::PrivacyEnhancement: return 0.9f;
        case CountermeasureType::NetworkObfuscation: return 0.75f;
        case CountermeasureType::VisualScrambling: return 0.6f;
        case CountermeasureType::CommunicationHiding: return 0.85f;
        case CountermeasureType::EmergencyMode: return 0.95f;
        default: return 0.0f;
    }
}

void AdaptiveCountermeasures::updateEffectivenessMetrics() {
    for (const auto &cm : _active_countermeasures) {
        _effectiveness_history[cm.type] = cm.effectiveness;
    }
}

void AdaptiveCountermeasures::updateCountermeasures() {
    if (!_active) return;
    updateEffectivenessMetrics();
}

void AdaptiveCountermeasures::generateAudioNoise() {
    if (!_active) return;
}

void AdaptiveCountermeasures::updatePrivacySettings() {
    if (!_active) return;
}

void AdaptiveCountermeasures::obfuscateNetworkTraffic() {
    if (!_active) return;
}

// ===== CounterIntelligenceDashboard =====

QString CounterIntelligenceDashboard::generateReport() const {
    QString report;
    report += "=== Counterintelligence Report ===\n";
    report += QString("Health Score: ") + QString::number(_health_score) + "\n";
    report += QString("Detection Rate: ") + QString::number(_detection_rate) + " /min\n";
    report += QString("Response Time: ") + QString::number(_response_time) + " ms\n";
    if (_last_threat.level != ThreatLevel::None) {
        report += QString("Last Threat: ") + _last_threat.details + "\n";
    }
    report += QString("Active Countermeasures: ") + QString::number(_active_measures.size()) + "\n";
    for (const auto &cm : _active_measures) {
        report += QString("  - ") + cm.description + " (eff: " + QString::number(cm.effectiveness) + ")\n";
    }
    return report;
}

} // namespace SpyGram::Counterintelligence
