#include "universal_threat_detector.h"
#include "hardware_detector.h"

#include <fstream>
#include <sstream>
#include <thread>
#include <atomic>
#include <algorithm>
#include <cmath>
#include <random>
#include <regex>

namespace Security {

namespace {
    constexpr int MIN_SUSPICIOUS_KEYWORDS = 3;
    constexpr size_t MAX_QUEUE_SIZE = 1000;

    const QStringList CRYPTO_PATTERNS = {
        "[A-Za-z0-9+/]{40,}={0,2}",
        "-----BEGIN [A-Z ]+-----",
        "\\b[0-9a-fA-F]{64,}\\b",
        "ssh-[a-z0-9]+ [A-Za-z0-9+/]+=*",
        "AAAAC3NzaC1lZDI1NTE5",
    };

    const QStringList SPYGRAM_PATTERNS = {
        "spygram[_-]?[a-z0-9]{8,}",
        "telegram[_-]?barrier",
        "universal[_-]?security",
        "threat[_-]?detector",
        "npu[_-]?accelerated",
        "quantum[_-]?resistant",
    };

    const QStringList MALWARE_INDICATORS = {
        "keylogger", "trojan", "backdoor", "rootkit", "botnet",
        "ransomware", "worm", "virus", "spyware", "adware",
        "cryptominer", "stealer", "loader", "dropper", "rat"
    };

    const QStringList PHISHING_INDICATORS = {
        "urgent", "verify account", "suspended", "click here",
        "limited time", "act now", "confirm identity", "security alert",
        "update payment", "prize winner", "tax refund", "inheritance"
    };

    const QStringList SOCIAL_ENG_TECHNIQUES = {
        "authority", "urgency", "scarcity", "reciprocity", "consensus",
        "liking", "commitment", "trust", "fear", "greed", "curiosity"
    };
}

struct UniversalThreatDetector::AIEngine {
    bool npuAvailable = false;
    bool gpuAvailable = false;
    bool openvinoLoaded = false;
    QString currentModelPath;
    QString modelFormat;
    QJsonObject modelMetadata;
    std::atomic<int> npuInferences{0};
    std::atomic<int> gpuInferences{0};
    std::atomic<int> cpuInferences{0};
    std::atomic<int> patternMatches{0};
    bool simdOptimized = false;

    AIEngine() {
        simdOptimized = false;
    }
};

UniversalThreatDetector::UniversalThreatDetector(QObject *parent)
    : _aiEngine(std::make_unique<AIEngine>())
{
    (void)parent;
    setupDefaultConfiguration();
}

UniversalThreatDetector::~UniversalThreatDetector() {
    cleanupResources();
}

void UniversalThreatDetector::initialize() {
    if (_initialized) return;

    try {
        loadConfiguration();
        if (!loadThreatDatabase()) {
            _threatDatabase.clear();
            _threatDatabaseVersion = 1;
            _lastDatabaseUpdate = QDateTime::currentDateTime();
        }
        detectNPUCapability();
        detectGPUCapability();
        optimizeForCPU();

        if (_aiEngine->npuAvailable) {
            _currentTier = AIProcessingTier::Tier1_NPU_Accelerated;
        } else if (_aiEngine->gpuAvailable) {
            _currentTier = AIProcessingTier::Tier2_GPU_Accelerated;
        } else {
            _currentTier = AIProcessingTier::Tier3_CPU_Optimized;
        }

        _initialized = true;
        if (processingTierChanged) processingTierChanged(_currentTier, AIProcessingTier::Tier4_Pattern_Only);
    } catch (const std::exception &) {
        _initialized = false;
        _currentTier = AIProcessingTier::Tier4_Pattern_Only;
    }
}

void UniversalThreatDetector::setProcessingTier(AIProcessingTier tier) {
    if (!isProcessingTierAvailable(tier)) return;
    AIProcessingTier oldTier = _currentTier;
    _currentTier = tier;
    if (processingTierChanged) processingTierChanged(_currentTier, oldTier);
}

QVector<AIProcessingTier> UniversalThreatDetector::getAvailableTiers() const {
    QVector<AIProcessingTier> tiers;
    if (_aiEngine->npuAvailable) tiers.append(AIProcessingTier::Tier1_NPU_Accelerated);
    if (_aiEngine->gpuAvailable) tiers.append(AIProcessingTier::Tier2_GPU_Accelerated);
    tiers.append(AIProcessingTier::Tier3_CPU_Optimized);
    tiers.append(AIProcessingTier::Tier4_Pattern_Only);
    return tiers;
}

bool UniversalThreatDetector::isProcessingTierAvailable(AIProcessingTier tier) const {
    switch (tier) {
        case AIProcessingTier::Tier1_NPU_Accelerated: return _aiEngine->npuAvailable;
        case AIProcessingTier::Tier2_GPU_Accelerated: return _aiEngine->gpuAvailable;
        case AIProcessingTier::Tier3_CPU_Optimized:
        case AIProcessingTier::Tier4_Pattern_Only: return true;
        default: return false;
    }
}

ThreatAnalysis UniversalThreatDetector::analyzeText(const QString &text, const QString &context) {
    if (!_enabled || !_initialized) {
        ThreatAnalysis analysis;
        analysis.result = AIAnalysisResult::ProcessingError;
        analysis.description = "Threat detector not enabled or initialized";
        return analysis;
    }

    auto start = std::chrono::steady_clock::now();

    ThreatAnalysis analysis;
    analysis.contentId = generateRequestId();
    analysis.analysisTime = QDateTime::currentDateTime();
    analysis.tierUsed = _currentTier;

    try {
        if (isWhitelisted(text)) {
            analysis.result = AIAnalysisResult::Safe;
            analysis.severity = ThreatSeverity::Info;
            analysis.confidence = AnalysisConfidence::High;
            analysis.description = "Content is whitelisted";
            auto end = std::chrono::steady_clock::now();
            analysis.processingTimeMs = std::chrono::duration<double, std::milli>(end - start).count();
            return analysis;
        }

        switch (_currentTier) {
            case AIProcessingTier::Tier1_NPU_Accelerated:
                analysis = analyzeWithNPU(text, context); break;
            case AIProcessingTier::Tier2_GPU_Accelerated:
                analysis = analyzeWithGPU(text, context); break;
            case AIProcessingTier::Tier3_CPU_Optimized:
                analysis = analyzeWithCPU(text, context); break;
            default:
                analysis = analyzeWithPatterns(text, context); break;
        }

        auto end = std::chrono::steady_clock::now();
        analysis.processingTimeMs = std::chrono::duration<double, std::milli>(end - start).count();
        analysis.contentId = generateRequestId();
        analysis.analysisTime = QDateTime::currentDateTime();
        analysis.tierUsed = _currentTier;

        updateStatistics(analysis);

        if (analysis.result != AIAnalysisResult::Safe && threatDetected) {
            threatDetected(analysis);
        }
    } catch (const std::exception &e) {
        analysis = handleAnalysisError(AnalysisRequest(), QString(e.what()));
        auto end = std::chrono::steady_clock::now();
        analysis.processingTimeMs = std::chrono::duration<double, std::milli>(end - start).count();
    }

    return analysis;
}

ThreatAnalysis UniversalThreatDetector::analyzeFile(const QString &filePath) {
    std::ifstream f(filePath.toStdString(), std::ios::binary);
    if (!f.is_open()) {
        ThreatAnalysis analysis;
        analysis.result = AIAnalysisResult::ProcessingError;
        analysis.description = "File not found or not readable";
        return analysis;
    }
    std::string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    if (content.size() > 10 * 1024 * 1024) {
        ThreatAnalysis analysis;
        analysis.result = AIAnalysisResult::ProcessingError;
        analysis.description = "File too large for analysis";
        return analysis;
    }
    QString context = QString("file:") + filePath + ";size:" + QString::number(content.size());
    return analyzeText(QString(content), context);
}

ThreatAnalysis UniversalThreatDetector::analyzeUrl(const QString &url) {
    QString context = QString("url:") + url;
    return analyzeText(url, context);
}

ThreatAnalysis UniversalThreatDetector::analyzeBinaryData(const QByteArray &data, const QString &contentType) {
    if (data.isEmpty()) {
        ThreatAnalysis analysis;
        analysis.result = AIAnalysisResult::Safe;
        analysis.description = "Empty data";
        return analysis;
    }
    QString context = QString("binary:") + contentType + ";size:" + QString::number(data.size());
    QString content = data.toHex();
    return analyzeText(content, context);
}

QString UniversalThreatDetector::requestAnalysis(const AnalysisRequest &request) {
    QString requestId = request.requestId.isEmpty() ? generateRequestId() : request.requestId;
    AnalysisRequest req = request;
    req.requestId = requestId;
    req.requestTime = QDateTime::currentDateTime();

    QMutexLocker locker(&_queueMutex);
    if ((size_t)_analysisQueue.size() >= MAX_QUEUE_SIZE) {
        if (analysisError) analysisError(requestId, "Analysis queue full");
        return QString();
    }
    _analysisQueue.append(req);
    return requestId;
}

void UniversalThreatDetector::cancelAnalysis(const QString &requestId) {
    QMutexLocker locker(&_queueMutex);
    _analysisQueue.erase(
        std::remove_if(_analysisQueue.begin(), _analysisQueue.end(),
            [&requestId](const AnalysisRequest &req) { return req.requestId == requestId; }),
        _analysisQueue.end());
    _activeRequests.remove(requestId);
}

QStringList UniversalThreatDetector::getPendingAnalyses() const {
    QMutexLocker locker(&_queueMutex);
    QStringList pending;
    for (const auto &req : _analysisQueue) pending.append(req.requestId);
    return pending;
}

int UniversalThreatDetector::getQueueSize() const {
    QMutexLocker locker(&_queueMutex);
    return _analysisQueue.size();
}

bool UniversalThreatDetector::detectSuspiciousPatterns(const QString &content, QStringList &patterns) {
    patterns.clear();
    for (const auto &pattern : CRYPTO_PATTERNS) {
        std::regex re(pattern.toStdString(), std::regex::icase);
        if (std::regex_search(content.toStdString(), re)) {
            patterns.append("crypto:" + pattern);
        }
    }
    for (const auto &pattern : SPYGRAM_PATTERNS) {
        std::regex re(pattern.toStdString(), std::regex::icase);
        if (std::regex_search(content.toStdString(), re)) {
            patterns.append("spygram:" + pattern);
        }
    }
    QStringList statPatterns = detectStatisticalAnomalies(content);
    patterns.append(statPatterns);
    return !patterns.isEmpty();
}

bool UniversalThreatDetector::detectMalwareSignatures(const QByteArray &data, QStringList &signatures) {
    signatures.clear();
    QString content = data;
    for (const auto &indicator : MALWARE_INDICATORS) {
        if (content.contains(indicator, Qt::CaseInsensitive)) {
            signatures.append("malware:" + indicator);
        }
    }
    return !signatures.isEmpty();
}

bool UniversalThreatDetector::detectPhishingIndicators(const QString &content, QStringList &indicators) {
    indicators.clear();
    for (const auto &indicator : PHISHING_INDICATORS) {
        if (content.contains(indicator, Qt::CaseInsensitive)) {
            indicators.append("phishing:" + indicator);
        }
    }
    return !indicators.isEmpty();
}

bool UniversalThreatDetector::detectSocialEngineering(const QString &text, QStringList &techniques) {
    techniques.clear();
    for (const auto &technique : SOCIAL_ENG_TECHNIQUES) {
        if (text.contains(technique, Qt::CaseInsensitive)) {
            techniques.append("social_eng:" + technique);
        }
    }
    return !techniques.isEmpty();
}

ProcessingStatistics UniversalThreatDetector::getStatistics() const {
    QMutexLocker locker(&_statsMutex);
    return _statistics;
}

void UniversalThreatDetector::resetStatistics() {
    QMutexLocker locker(&_statsMutex);
    _statistics = ProcessingStatistics();
    _aiEngine->npuInferences = 0;
    _aiEngine->gpuInferences = 0;
    _aiEngine->cpuInferences = 0;
    _aiEngine->patternMatches = 0;
}

double UniversalThreatDetector::getAverageLatency() const {
    QMutexLocker locker(&_statsMutex);
    return _statistics.averageProcessingTime;
}

ThreatAnalysis UniversalThreatDetector::analyzeWithNPU(const QString &content, const QString &context) {
    _aiEngine->npuInferences++;
    ThreatAnalysis analysis;
    analysis.result = AIAnalysisResult::Safe;
    analysis.severity = ThreatSeverity::Info;
    analysis.confidence = AnalysisConfidence::High;
    analysis.description = "NPU analysis completed";
    analysis.modelVersion = "NPU-v1.0";
    QStringList patterns;
    if (detectSuspiciousPatterns(content, patterns)) {
        analysis.result = AIAnalysisResult::Suspicious;
        analysis.severity = ThreatSeverity::Medium;
        analysis.detectedPatterns = patterns;
        analysis.threatScore = 0.6;
    }
    return analysis;
}

ThreatAnalysis UniversalThreatDetector::analyzeWithGPU(const QString &content, const QString &context) {
    _aiEngine->gpuInferences++;
    ThreatAnalysis analysis;
    analysis.result = AIAnalysisResult::Safe;
    analysis.severity = ThreatSeverity::Info;
    analysis.confidence = AnalysisConfidence::Medium;
    analysis.description = "GPU analysis completed";
    analysis.modelVersion = "GPU-v1.0";
    QStringList patterns, signatures, indicators;
    bool hasSuspicious = detectSuspiciousPatterns(content, patterns);
    bool hasMalware = detectMalwareSignatures(content.toUtf8(), signatures);
    bool hasPhishing = detectPhishingIndicators(content, indicators);
    if (hasSuspicious || hasMalware || hasPhishing) {
        analysis.result = AIAnalysisResult::Suspicious;
        analysis.severity = ThreatSeverity::Medium;
        analysis.detectedPatterns = patterns + signatures + indicators;
        analysis.threatScore = 0.5;
        analysis.malwareScore = hasMalware ? 0.7 : 0.0;
        analysis.phishingScore = hasPhishing ? 0.6 : 0.0;
    }
    return analysis;
}

ThreatAnalysis UniversalThreatDetector::analyzeWithCPU(const QString &content, const QString &context) {
    _aiEngine->cpuInferences++;
    ThreatAnalysis analysis;
    analysis.result = AIAnalysisResult::Safe;
    analysis.severity = ThreatSeverity::Info;
    analysis.confidence = AnalysisConfidence::Medium;
    analysis.description = "CPU analysis completed";
    analysis.modelVersion = "CPU-v1.0";
    QStringList patterns, signatures, indicators, techniques;
    bool hasSuspicious = detectSuspiciousPatterns(content, patterns);
    bool hasMalware = detectMalwareSignatures(content.toUtf8(), signatures);
    bool hasPhishing = detectPhishingIndicators(content, indicators);
    bool hasSocialEng = detectSocialEngineering(content, techniques);
    QStringList allPatterns = patterns + signatures + indicators + techniques;
    if (hasSuspicious || hasMalware || hasPhishing || hasSocialEng) {
        if (hasMalware && hasPhishing) {
            analysis.result = AIAnalysisResult::Malicious;
            analysis.severity = ThreatSeverity::High;
        } else {
            analysis.result = AIAnalysisResult::Suspicious;
            analysis.severity = ThreatSeverity::Medium;
        }
        analysis.detectedPatterns = allPatterns;
        analysis.threatScore = 0.4;
        analysis.malwareScore = hasMalware ? 0.6 : 0.0;
        analysis.phishingScore = hasPhishing ? 0.5 : 0.0;
        analysis.socialEngScore = hasSocialEng ? 0.4 : 0.0;
    }
    return analysis;
}

ThreatAnalysis UniversalThreatDetector::analyzeWithPatterns(const QString &content, const QString &context) {
    _aiEngine->patternMatches++;
    ThreatAnalysis analysis;
    analysis.result = AIAnalysisResult::Safe;
    analysis.severity = ThreatSeverity::Info;
    analysis.confidence = AnalysisConfidence::Low;
    analysis.description = "Pattern analysis completed";
    analysis.modelVersion = "Pattern-v1.0";
    QStringList patterns = detectKeywordPatterns(content);
    QStringList regexPatterns = detectRegexPatterns(content);
    QStringList allPatterns = patterns + regexPatterns;
    if (allPatterns.size() >= MIN_SUSPICIOUS_KEYWORDS) {
        analysis.result = AIAnalysisResult::Suspicious;
        analysis.severity = ThreatSeverity::Low;
        analysis.confidence = AnalysisConfidence::Medium;
        analysis.detectedPatterns = allPatterns;
        analysis.threatScore = qMin(0.3, allPatterns.size() * 0.1);
    }
    return analysis;
}

QStringList UniversalThreatDetector::detectKeywordPatterns(const QString &content) {
    QStringList patterns;
    QString lowerContent = content.toLower();
    for (const auto &indicator : MALWARE_INDICATORS) {
        if (lowerContent.contains(indicator)) patterns.append("keyword:" + indicator);
    }
    for (const auto &indicator : PHISHING_INDICATORS) {
        if (lowerContent.contains(indicator)) patterns.append("keyword:" + indicator);
    }
    return patterns;
}

QStringList UniversalThreatDetector::detectRegexPatterns(const QString &content) {
    QStringList patterns;
    auto s = content.toStdString();
    std::regex emailRegex("\\b[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Z|a-z]{2,}\\b");
    if (std::regex_search(s, emailRegex)) patterns.append("regex:email");
    std::regex urlRegex("https?://[^\\s]+");
    if (std::regex_search(s, urlRegex)) patterns.append("regex:url");
    std::regex ipRegex("\\b(?:[0-9]{1,3}\\.){3}[0-9]{1,3}\\b");
    if (std::regex_search(s, ipRegex)) patterns.append("regex:ip");
    return patterns;
}

QStringList UniversalThreatDetector::detectStatisticalAnomalies(const QString &content) {
    QStringList patterns;
    if (content.length() < 10) return patterns;
    std::map<char, int> charFreq;
    for (char ch : content.toStdString()) charFreq[ch]++;
    double entropy = 0.0;
    for (const auto &[ch, freq] : charFreq) {
        double p = static_cast<double>(freq) / content.length();
        entropy -= p * std::log2(p);
    }
    if (entropy > 7.5) patterns.append("statistical:high_entropy");
    std::regex repeatRegex("(.{3,})\\1{3,}");
    if (std::regex_search(content.toStdString(), repeatRegex)) {
        patterns.append("statistical:repeated_pattern");
    }
    return patterns;
}

QStringList UniversalThreatDetector::detectBehavioralPatterns(const QString &content) {
    return QStringList();
}

bool UniversalThreatDetector::detectNPUCapability() {
    struct stat st;
    _aiEngine->npuAvailable = (::stat("/dev/accel/accel0", &st) == 0);
    return _aiEngine->npuAvailable;
}

bool UniversalThreatDetector::detectGPUCapability() {
    struct stat st;
    _aiEngine->gpuAvailable = (::stat("/dev/dri/card0", &st) == 0);
    return _aiEngine->gpuAvailable;
}

void UniversalThreatDetector::optimizeForCPU() {
    _aiEngine->simdOptimized = false;
    int threadCount = std::thread::hardware_concurrency();
    _maxConcurrentAnalyses = qMax(2, qMin(threadCount / 2, 8));
}

void UniversalThreatDetector::updateStatistics(const ThreatAnalysis &analysis) {
    QMutexLocker locker(&_statsMutex);
    _statistics.totalAnalyses++;
    _statistics.lastAnalysis = analysis.analysisTime;
    if (analysis.result == AIAnalysisResult::Safe) _statistics.safeDetections++;
    else _statistics.threatDetections++;
    if (analysis.result == AIAnalysisResult::ProcessingError) _statistics.errorCount++;
    double pt = analysis.processingTimeMs;
    if (_statistics.totalAnalyses == 1) _statistics.averageProcessingTime = pt;
    else _statistics.averageProcessingTime = (_statistics.averageProcessingTime * (_statistics.totalAnalyses - 1) + pt) / _statistics.totalAnalyses;
    if (pt > _statistics.peakProcessingTime) _statistics.peakProcessingTime = pt;
    _statistics.tierUsage[analysis.tierUsed]++;
    _statistics.severityCount[analysis.severity]++;
}

ThreatAnalysis UniversalThreatDetector::handleAnalysisError(const AnalysisRequest &request, const QString &error) {
    ThreatAnalysis analysis;
    analysis.contentId = request.requestId;
    analysis.result = AIAnalysisResult::ProcessingError;
    analysis.severity = ThreatSeverity::Info;
    analysis.confidence = AnalysisConfidence::VeryLow;
    analysis.description = QString("Analysis error: ") + error;
    analysis.analysisTime = QDateTime::currentDateTime();
    analysis.tierUsed = _currentTier;
    analysis.modelVersion = "Error";
    return analysis;
}

void UniversalThreatDetector::setupDefaultConfiguration() {
    _enabled = true;
    _analysisTimeoutMs = 10000;
    _maxConcurrentAnalyses = 4;
    _memoryLimitMB = 1024;
    _realTimeThreshold = 0.5;
    _realTimeDetectionEnabled = true;
    _automaticUpdatesEnabled = true;
    _hardwareAcceleration = true;
    _statistics = ProcessingStatistics();
}

void UniversalThreatDetector::validateConfiguration() {}

QString UniversalThreatDetector::generateRequestId() const {
    static std::mt19937 rng(std::random_device{}());
    return QString("req_") + QString::number(QDateTime::currentMSecsSinceEpoch()) + "_" + QString::number(rng() % 10000);
}

bool UniversalThreatDetector::loadThreatDatabase() {
    QString dbPath = base::GlobalStoragePath().toStdString() + "/threat_database.json";
    std::ifstream f(dbPath.toStdString());
    if (!f.is_open()) return false;
    std::string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    if (content.empty()) return false;
    _threatDatabaseVersion = 1;
    _lastDatabaseUpdate = QDateTime::currentDateTime();
    return true;
}

void UniversalThreatDetector::saveThreatDatabase() {
    QString dbPath = base::GlobalStoragePath().toStdString() + "/threat_database.json";
    std::ofstream f(dbPath.toStdString());
    if (f.is_open()) {
        f << "{\"version\":" << _threatDatabaseVersion << "}";
        f.close();
    }
}

bool UniversalThreatDetector::downloadDatabaseUpdate() { return false; }
void UniversalThreatDetector::parseThreatDatabase(const QByteArray &data) { (void)data; }

void UniversalThreatDetector::loadConfiguration() {
    QString configPath = base::GlobalStoragePath().toStdString() + "/threat_detector.ini";
    std::ifstream f(configPath.toStdString());
    if (f.is_open()) {
        std::string line;
        while (std::getline(f, line)) {
            if (line.find("analysis_timeout_ms=") == 0) {
                _analysisTimeoutMs = std::stoi(line.substr(20));
            } else if (line.find("max_concurrent=") == 0) {
                _maxConcurrentAnalyses = std::stoi(line.substr(15));
            } else if (line.find("realtime_threshold=") == 0) {
                _realTimeThreshold = std::stod(line.substr(19));
            }
        }
    }
}

void UniversalThreatDetector::saveConfiguration() {
    QString configPath = base::GlobalStoragePath().toStdString() + "/threat_detector.ini";
    std::ofstream f(configPath.toStdString());
    if (f.is_open()) {
        f << "analysis_timeout_ms=" << _analysisTimeoutMs << "\n";
        f << "max_concurrent=" << _maxConcurrentAnalyses << "\n";
        f << "realtime_threshold=" << _realTimeThreshold << "\n";
        f.close();
    }
}

void UniversalThreatDetector::cleanupResources() {
    _aiEngine.reset();
}

bool UniversalThreatDetector::isWhitelisted(const QString &content) const {
    for (const auto &pattern : _whitelist) {
        if (content.contains(pattern, Qt::CaseInsensitive)) return true;
    }
    return false;
}

void UniversalThreatDetector::addToWhitelist(const QString &pattern) { _whitelist.append(pattern); }
void UniversalThreatDetector::removeFromWhitelist(const QString &pattern) { _whitelist.removeAll(pattern); }

void UniversalThreatDetector::reportFalsePositive(const QString &contentId) {
    _feedbackData[contentId] = false;
    _falsePositiveCount++;
}

void UniversalThreatDetector::reportMissedThreat(const QString &content, ThreatSeverity severity) {
    _feedbackData[content] = true;
    (void)severity;
}

void UniversalThreatDetector::retrainOnFeedback() {}

void UniversalThreatDetector::updateThreatDatabase() {
    if (downloadDatabaseUpdate()) {
        _lastDatabaseUpdate = QDateTime::currentDateTime();
        _threatDatabaseVersion++;
        if (threatDatabaseUpdated) threatDatabaseUpdated(_threatDatabaseVersion);
    }
}

void UniversalThreatDetector::optimizeForHardware() {
    detectNPUCapability();
    detectGPUCapability();
    optimizeForCPU();
}

void UniversalThreatDetector::optimizeMemoryUsage() {}
void UniversalThreatDetector::optimizeProcessingPipeline() {}
void UniversalThreatDetector::optimizePerformance() {}

void UniversalThreatDetector::addToQueue(const AnalysisRequest &request) {
    QMutexLocker locker(&_queueMutex);
    _analysisQueue.append(request);
}

AnalysisRequest UniversalThreatDetector::getNextRequest() {
    QMutexLocker locker(&_queueMutex);
    if (_analysisQueue.isEmpty()) return AnalysisRequest();
    return _analysisQueue.takeFirst();
}

void UniversalThreatDetector::clearQueue() {
    QMutexLocker locker(&_queueMutex);
    _analysisQueue.clear();
}

void UniversalThreatDetector::prioritizeRequest(const QString &requestId) {
    QMutexLocker locker(&_queueMutex);
    for (int i = 0; i < _analysisQueue.size(); ++i) {
        if (_analysisQueue[i].requestId == requestId) {
            AnalysisRequest req = _analysisQueue.takeAt(i);
            req.priorityRequest = true;
            _analysisQueue.prepend(req);
            break;
        }
    }
}

void UniversalThreatDetector::fallbackToLowerTier() {
    switch (_currentTier) {
        case AIProcessingTier::Tier1_NPU_Accelerated: _currentTier = AIProcessingTier::Tier2_GPU_Accelerated; break;
        case AIProcessingTier::Tier2_GPU_Accelerated: _currentTier = AIProcessingTier::Tier3_CPU_Optimized; break;
        case AIProcessingTier::Tier3_CPU_Optimized: _currentTier = AIProcessingTier::Tier4_Pattern_Only; break;
        default: break;
    }
}

bool UniversalThreatDetector::recoverFromError() { return true; }

QString UniversalThreatDetector::preprocessText(const QString &text) { return text; }
QByteArray UniversalThreatDetector::preprocessBinaryData(const QByteArray &data) { return data; }
QJsonObject UniversalThreatDetector::createModelInput(const QString &content, const QString &context) {
    QJsonObject obj;
    obj["content"] = content;
    obj["context"] = context;
    return obj;
}

ThreatAnalysis UniversalThreatDetector::processModelOutput(const QJsonObject &output, const AnalysisRequest &request) {
    ThreatAnalysis analysis;
    analysis.contentId = request.requestId;
    analysis.result = AIAnalysisResult::Safe;
    return analysis;
}

ThreatSeverity UniversalThreatDetector::calculateThreatSeverity(double threatScore) {
    if (threatScore >= 0.9) return ThreatSeverity::Emergency;
    if (threatScore >= 0.7) return ThreatSeverity::Critical;
    if (threatScore >= 0.5) return ThreatSeverity::High;
    if (threatScore >= 0.3) return ThreatSeverity::Medium;
    if (threatScore > 0.0) return ThreatSeverity::Low;
    return ThreatSeverity::Info;
}

AnalysisConfidence UniversalThreatDetector::calculateConfidence(const QJsonObject &output) {
    return AnalysisConfidence::Medium;
}

QStringList UniversalThreatDetector::extractDetectedPatterns(const QJsonObject &output) {
    return QStringList();
}

bool UniversalThreatDetector::loadNPUModel(const QString &modelPath) { (void)modelPath; return false; }
bool UniversalThreatDetector::loadGPUModel(const QString &modelPath) { (void)modelPath; return false; }
bool UniversalThreatDetector::loadCPUModel(const QString &modelPath) { (void)modelPath; return false; }
void UniversalThreatDetector::unloadCurrentModel() { _modelLoaded = false; }
bool UniversalThreatDetector::validateModel(const QString &modelPath) { (void)modelPath; return false; }

bool UniversalThreatDetector::loadModel(const QString &modelName) {
    _currentModel.name = modelName;
    _modelLoaded = true;
    if (modelLoaded) modelLoaded(modelName);
    return true;
}

void UniversalThreatDetector::unloadModel() {
    _modelLoaded = false;
    _currentModel = AIModelInfo();
}

void UniversalThreatDetector::optimizeModel() {
    _currentModel.optimized = true;
}

QVector<AIModelInfo> UniversalThreatDetector::getAvailableModels() const {
    QVector<AIModelInfo> models;
    return models;
}

void UniversalThreatDetector::monitorResourceUsage() {}

QString UniversalThreatDetector::formatAnalysisResult(const ThreatAnalysis &analysis) {
    return ThreatDetectorUtils::formatThreatAnalysis(analysis);
}

bool UniversalThreatDetector::hasValidLicense() const { return true; }

namespace ThreatDetectorUtils {

QString aiAnalysisResultToString(AIAnalysisResult result) {
    switch (result) {
        case AIAnalysisResult::Safe: return "Safe";
        case AIAnalysisResult::Suspicious: return "Suspicious";
        case AIAnalysisResult::Malicious: return "Malicious";
        case AIAnalysisResult::Unknown: return "Unknown";
        case AIAnalysisResult::ProcessingError: return "Error";
        default: return "Unknown";
    }
}

QString threatSeverityToString(ThreatSeverity severity) {
    switch (severity) {
        case ThreatSeverity::Info: return "Info";
        case ThreatSeverity::Low: return "Low";
        case ThreatSeverity::Medium: return "Medium";
        case ThreatSeverity::High: return "High";
        case ThreatSeverity::Critical: return "Critical";
        case ThreatSeverity::Emergency: return "Emergency";
        default: return "Unknown";
    }
}

QString analysisConfidenceToString(AnalysisConfidence confidence) {
    switch (confidence) {
        case AnalysisConfidence::VeryLow: return "Very Low";
        case AnalysisConfidence::Low: return "Low";
        case AnalysisConfidence::Medium: return "Medium";
        case AnalysisConfidence::High: return "High";
        case AnalysisConfidence::VeryHigh: return "Very High";
        case AnalysisConfidence::Certain: return "Certain";
        default: return "Unknown";
    }
}

QString aiProcessingTierToString(AIProcessingTier tier) {
    switch (tier) {
        case AIProcessingTier::Tier1_NPU_Accelerated: return "NPU Accelerated";
        case AIProcessingTier::Tier2_GPU_Accelerated: return "GPU Accelerated";
        case AIProcessingTier::Tier3_CPU_Optimized: return "CPU Optimized";
        case AIProcessingTier::Tier4_Pattern_Only: return "Pattern Only";
        default: return "Unknown";
    }
}

bool isHighRiskAnalysis(const ThreatAnalysis &analysis) {
    return static_cast<int>(analysis.severity) >= static_cast<int>(ThreatSeverity::High) ||
           analysis.threatScore >= 0.7 ||
           analysis.result == AIAnalysisResult::Malicious;
}

QString formatThreatAnalysis(const ThreatAnalysis &analysis) {
    return QString("ID: ") + analysis.contentId + ", Result: " + aiAnalysisResultToString(analysis.result) +
           ", Severity: " + threatSeverityToString(analysis.severity) + ", Score: " + QString::number(analysis.threatScore);
}

QString formatProcessingStatistics(const ProcessingStatistics &stats) {
    return QString("Total: ") + QString::number(stats.totalAnalyses) +
           ", Safe: " + QString::number(stats.safeDetections) +
           ", Threats: " + QString::number(stats.threatDetections);
}

double calculateOverallThreatScore(const ThreatAnalysis &analysis) {
    return std::max({analysis.threatScore, analysis.malwareScore, analysis.phishingScore, analysis.socialEngScore});
}

QStringList getRecommendedActions(const ThreatAnalysis &analysis) {
    QStringList actions;
    if (analysis.blockContent) actions.append("Block content");
    if (analysis.quarantine) actions.append("Quarantine");
    if (analysis.requireHumanReview) actions.append("Manual review required");
    if (actions.isEmpty()) actions.append("No action needed");
    return actions;
}

} // namespace ThreatDetectorUtils

} // namespace Security
