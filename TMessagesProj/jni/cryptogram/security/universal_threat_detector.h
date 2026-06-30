#pragma once

#include "desktop_shims.h"
#include "base/expected.h"
#include "base/bytes.h"

#include <memory>
#include <vector>
#include <map>

namespace Security {

enum class AIAnalysisResult {
    Safe,
    Suspicious,
    Malicious,
    Unknown,
    ProcessingError
};

enum class ThreatSeverity {
    Info = 1,
    Low = 3,
    Medium = 5,
    High = 7,
    Critical = 9,
    Emergency = 10
};

enum class AnalysisConfidence {
    VeryLow = 1,
    Low = 3,
    Medium = 5,
    High = 7,
    VeryHigh = 9,
    Certain = 10
};

enum class AIProcessingTier {
    Tier1_NPU_Accelerated,
    Tier2_GPU_Accelerated,
    Tier3_CPU_Optimized,
    Tier4_Pattern_Only
};

struct ThreatAnalysis {
    QString contentId;
    AIAnalysisResult result = AIAnalysisResult::Unknown;
    ThreatSeverity severity = ThreatSeverity::Info;
    AnalysisConfidence confidence = AnalysisConfidence::Medium;

    QString description;
    QStringList detectedPatterns;
    QStringList suspiciousElements;

    double threatScore = 0.0;
    double malwareScore = 0.0;
    double phishingScore = 0.0;
    double socialEngScore = 0.0;

    QDateTime analysisTime;
    double processingTimeMs = 0.0;
    AIProcessingTier tierUsed = AIProcessingTier::Tier4_Pattern_Only;
    QString modelVersion;
    QJsonObject rawResults;

    QStringList recommendations;
    bool blockContent = false;
    bool quarantine = false;
    bool requireHumanReview = false;
};

struct AIModelInfo {
    QString name;
    QString version;
    QString format;
    QString path;
    size_t sizeBytes = 0;
    bool loaded = false;
    bool optimized = false;
    QStringList inputFormats;
    QStringList outputFormats;
    double accuracy = 0.0;
    QString description;
    QDateTime lastUpdated;
};

struct ProcessingStatistics {
    int totalAnalyses = 0;
    int safeDetections = 0;
    int threatDetections = 0;
    int errorCount = 0;
    double averageProcessingTime = 0.0;
    double peakProcessingTime = 0.0;
    QDateTime lastAnalysis;
    QMap<AIProcessingTier, int> tierUsage;
    QMap<ThreatSeverity, int> severityCount;
};

struct AnalysisRequest {
    QString requestId;
    QString content;
    QString contentType;
    QString source;
    QString context;
    QVariantMap metadata;
    AIProcessingTier preferredTier = AIProcessingTier::Tier1_NPU_Accelerated;
    bool priorityRequest = false;
    QDateTime requestTime;
};

class UniversalThreatDetector {
public:
    explicit UniversalThreatDetector(QObject *parent = nullptr);
    ~UniversalThreatDetector();

    void initialize();
    bool isInitialized() const { return _initialized; }
    void setEnabled(bool enabled) { _enabled = enabled; }
    bool isEnabled() const { return _enabled; }

    void setProcessingTier(AIProcessingTier tier);
    AIProcessingTier getCurrentProcessingTier() const { return _currentTier; }
    QVector<AIProcessingTier> getAvailableTiers() const;
    bool isProcessingTierAvailable(AIProcessingTier tier) const;

    ThreatAnalysis analyzeText(const QString &text, const QString &context = QString());
    ThreatAnalysis analyzeFile(const QString &filePath);
    ThreatAnalysis analyzeUrl(const QString &url);
    ThreatAnalysis analyzeBinaryData(const QByteArray &data, const QString &contentType);

    QString requestAnalysis(const AnalysisRequest &request);
    void cancelAnalysis(const QString &requestId);
    QStringList getPendingAnalyses() const;
    int getQueueSize() const;

    QVector<AIModelInfo> getAvailableModels() const;
    AIModelInfo getCurrentModel() const { return _currentModel; }
    bool loadModel(const QString &modelName);
    void unloadModel();
    bool isModelLoaded() const { return _modelLoaded; }
    void optimizeModel();

    void enableRealTimeDetection(bool enabled) { _realTimeDetectionEnabled = enabled; }
    bool isRealTimeDetectionEnabled() const { return _realTimeDetectionEnabled; }
    void setRealTimeThreshold(double threshold) { _realTimeThreshold = threshold; }
    double getRealTimeThreshold() const { return _realTimeThreshold; }

    bool detectSuspiciousPatterns(const QString &content, QStringList &patterns);
    bool detectMalwareSignatures(const QByteArray &data, QStringList &signatures);
    bool detectPhishingIndicators(const QString &content, QStringList &indicators);
    bool detectSocialEngineering(const QString &text, QStringList &techniques);

    void updateThreatDatabase();
    QDateTime getLastDatabaseUpdate() const { return _lastDatabaseUpdate; }
    int getThreatDatabaseVersion() const { return _threatDatabaseVersion; }
    void enableAutomaticUpdates(bool enabled) { _automaticUpdatesEnabled = enabled; }
    bool isAutomaticUpdatesEnabled() const { return _automaticUpdatesEnabled; }
    bool hasValidLicense() const;

    ProcessingStatistics getStatistics() const;
    void resetStatistics();
    double getCurrentCPUUsage() const { return 0.0; }
    double getCurrentMemoryUsage() const { return 0.0; }
    double getAverageLatency() const;

    void setAnalysisTimeout(int timeoutMs) { _analysisTimeoutMs = timeoutMs; }
    int getAnalysisTimeout() const { return _analysisTimeoutMs; }
    void setMaxConcurrentAnalyses(int max) { _maxConcurrentAnalyses = max; }
    int getMaxConcurrentAnalyses() const { return _maxConcurrentAnalyses; }
    void setMemoryLimit(size_t limitMB) { _memoryLimitMB = limitMB; }
    size_t getMemoryLimit() const { return _memoryLimitMB; }

    void optimizeForHardware();
    void enableHardwareAcceleration(bool enabled) { _hardwareAcceleration = enabled; }
    bool isHardwareAccelerationEnabled() const { return _hardwareAcceleration; }
    QString getHardwareStatus() const { return _hardwareStatus; }

    void addToWhitelist(const QString &pattern);
    void removeFromWhitelist(const QString &pattern);
    QStringList getWhitelist() const { return _whitelist; }
    bool isWhitelisted(const QString &content) const;

    void reportFalsePositive(const QString &contentId);
    void reportMissedThreat(const QString &content, ThreatSeverity severity);
    int getFalsePositiveCount() const { return _falsePositiveCount; }
    void retrainOnFeedback();

    std::function<void(const ThreatAnalysis &)> threatDetected;
    std::function<void(const QString &, const ThreatAnalysis &)> analysisComplete;
    std::function<void(const QString &, const QString &)> analysisError;
    std::function<void(const QString &)> modelLoaded;
    std::function<void(const QString &, const QString &)> modelLoadError;
    std::function<void(AIProcessingTier, AIProcessingTier)> processingTierChanged;
    std::function<void(const ProcessingStatistics &)> statisticsUpdated;
    std::function<void(int)> threatDatabaseUpdated;

private:
    ThreatAnalysis analyzeWithNPU(const QString &content, const QString &context);
    ThreatAnalysis analyzeWithGPU(const QString &content, const QString &context);
    ThreatAnalysis analyzeWithCPU(const QString &content, const QString &context);
    ThreatAnalysis analyzeWithPatterns(const QString &content, const QString &context);

    bool loadNPUModel(const QString &modelPath);
    bool loadGPUModel(const QString &modelPath);
    bool loadCPUModel(const QString &modelPath);
    void unloadCurrentModel();
    bool validateModel(const QString &modelPath);

    QString preprocessText(const QString &text);
    QByteArray preprocessBinaryData(const QByteArray &data);
    QJsonObject createModelInput(const QString &content, const QString &context);

    ThreatAnalysis processModelOutput(const QJsonObject &output, const AnalysisRequest &request);
    ThreatSeverity calculateThreatSeverity(double threatScore);
    AnalysisConfidence calculateConfidence(const QJsonObject &output);
    QStringList extractDetectedPatterns(const QJsonObject &output);

    QStringList detectKeywordPatterns(const QString &content);
    QStringList detectRegexPatterns(const QString &content);
    QStringList detectStatisticalAnomalies(const QString &content);
    QStringList detectBehavioralPatterns(const QString &content);

    bool detectNPUCapability();
    bool detectGPUCapability();
    void optimizeForCPU();
    void optimizeMemoryUsage();
    void optimizeProcessingPipeline();

    bool loadThreatDatabase();
    void saveThreatDatabase();
    bool downloadDatabaseUpdate();
    void parseThreatDatabase(const QByteArray &data);

    void updateStatistics(const ThreatAnalysis &analysis);
    void monitorResourceUsage();
    void optimizePerformance();

    void addToQueue(const AnalysisRequest &request);
    AnalysisRequest getNextRequest();
    void clearQueue();
    void prioritizeRequest(const QString &requestId);

    ThreatAnalysis handleAnalysisError(const AnalysisRequest &request, const QString &error);
    void fallbackToLowerTier();
    bool recoverFromError();

    bool _initialized = false;
    bool _enabled = false;
    AIProcessingTier _currentTier = AIProcessingTier::Tier4_Pattern_Only;
    mutable QMutex _queueMutex;
    mutable QMutex _statsMutex;

    struct AIEngine;
    std::unique_ptr<AIEngine> _aiEngine;
    AIModelInfo _currentModel;
    bool _modelLoaded = false;

    QVector<AnalysisRequest> _analysisQueue;
    QMap<QString, AnalysisRequest> _activeRequests;
    int _maxConcurrentAnalyses = 4;

    ProcessingStatistics _statistics;

    int _analysisTimeoutMs = 10000;
    size_t _memoryLimitMB = 1024;
    double _realTimeThreshold = 0.5;
    bool _realTimeDetectionEnabled = true;
    bool _automaticUpdatesEnabled = true;

    QMap<QString, QVariant> _threatDatabase;
    QDateTime _lastDatabaseUpdate;
    int _threatDatabaseVersion = 0;

    QStringList _whitelist;
    QMap<QString, bool> _feedbackData;
    int _falsePositiveCount = 0;

    bool _hardwareAcceleration = true;
    QString _hardwareStatus;

    void setupDefaultConfiguration();
    void validateConfiguration();
    QString generateRequestId() const;
    void loadConfiguration();
    void saveConfiguration();
    void cleanupResources();
    QString formatAnalysisResult(const ThreatAnalysis &analysis);
};

namespace ThreatDetectorUtils {
    QString aiAnalysisResultToString(AIAnalysisResult result);
    QString threatSeverityToString(ThreatSeverity severity);
    QString analysisConfidenceToString(AnalysisConfidence confidence);
    QString aiProcessingTierToString(AIProcessingTier tier);
    bool isHighRiskAnalysis(const ThreatAnalysis &analysis);
    QString formatThreatAnalysis(const ThreatAnalysis &analysis);
    QString formatProcessingStatistics(const ProcessingStatistics &stats);
    double calculateOverallThreatScore(const ThreatAnalysis &analysis);
    QStringList getRecommendedActions(const ThreatAnalysis &analysis);
}

} // namespace Security
