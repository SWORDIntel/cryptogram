/*
This file is part of CRYPTOGRAM,
the most advanced secure messaging application.

For license and copyright information please follow this link:
https://github.com/SWORDOps/CRYPTOGRAM/blob/main/LICENSE
*/
#pragma once

#include "base/timer.h"
#include "base/weak_ptr.h"

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QDateTime>
#include <QtCore/QThread>
#include <QtCore/QMutex>
#include <memory>

namespace Data {

// Forward declarations
class Session;

// Akashi Network contribution types
enum class ContributionType {
    None,           // No contribution
    Relay,          // Act as relay node
    Storage,        // Provide distributed storage
    Compute,        // Donate CPU cycles
    Bandwidth       // Share bandwidth
};

// CPU contribution level
enum class CPUContributionLevel {
    None = 0,       // 0% - Disabled
    Minimal = 5,    // 5% CPU
    Low = 10,       // 10% CPU
    Default = 20,   // 20% CPU (default)
    Medium = 30,    // 30% CPU
    High = 50,      // 50% CPU
    Maximum = 75    // 75% CPU (max for stability)
};

// Idle detection level
enum class IdleDetectionLevel {
    Never,          // Never consider idle (always respect CPU limit)
    Conservative,   // Idle after 30 minutes of no input
    Moderate,       // Idle after 15 minutes of no input
    Aggressive      // Idle after 5 minutes of no input
};

// Akashi contribution statistics
struct AkashiStatistics {
    // Contribution stats
    qint64 totalComputeTime = 0;     // Total seconds contributed
    qint64 totalTasksCompleted = 0;   // Tasks completed
    qint64 totalDataRelayed = 0;      // Bytes relayed
    double averageCPUUsage = 0.0;     // Average CPU usage %

    // Network stats
    int activeConnections = 0;        // Active peer connections
    int totalPeersHelped = 0;         // Number of peers helped
    QString currentTask;              // Current task description

    // Rewards/Credits (if applicable)
    qint64 creditsEarned = 0;         // Credits earned
    int reputation = 0;               // Network reputation score

    // Session stats
    QDateTime sessionStart;
    QDateTime lastContribution;
    bool isContributing = false;
};

// Akashi configuration
struct AkashiConfiguration {
    bool enabled = false;
    CPUContributionLevel cpuLevel = CPUContributionLevel::Default;
    IdleDetectionLevel idleLevel = IdleDetectionLevel::Moderate;

    // Contribution types enabled
    bool enableRelay = true;
    bool enableCompute = true;
    bool enableStorage = false;  // Disabled by default (requires space)
    bool enableBandwidth = true;

    // Limits
    int maxBandwidthMbps = 10;        // Max bandwidth to donate (Mbps)
    int maxStorageGB = 5;             // Max storage to donate (GB)
    bool onlyWhenCharging = true;     // Only contribute when device is charging
    bool onlyWhenIdle = true;         // Only contribute when idle

    // Advanced
    bool enableWhenBatteryLow = false;  // Continue even on battery
    int minBatteryPercent = 50;         // Minimum battery % to continue
};

// Idle state information
struct IdleState {
    bool isIdle = false;
    qint64 idleTimeSeconds = 0;
    qint64 lastInputTime = 0;
    bool isCharging = false;
    int batteryPercent = 100;
    double cpuUsage = 0.0;
};

/**
 * Akashi Network Contribution System
 *
 * Allows users to optionally donate their CPU cycles and resources to the
 * Akashi distributed network, helping maintain privacy infrastructure for
 * the CRYPTOGRAM community.
 *
 * Features:
 * - Optional CPU donation (default 20%)
 * - Intelligent idle detection
 * - Battery-aware contribution
 * - Multiple contribution types (relay, compute, storage, bandwidth)
 * - Privacy-preserving (anonymous contribution)
 * - Earn reputation/credits in the network
 * - Completely optional and user-controlled
 */
class AkashiNetwork : public QObject, public base::has_weak_ptr {
    Q_OBJECT

public:
    explicit AkashiNetwork(Session *session);
    ~AkashiNetwork();

    // Initialization
    bool initialize();
    bool isInitialized() const { return _initialized; }
    void shutdown();

    // Configuration
    void setEnabled(bool enabled);
    bool isEnabled() const { return _config.enabled; }

    void setConfiguration(const AkashiConfiguration &config);
    AkashiConfiguration getConfiguration() const { return _config; }

    void setCPUContributionLevel(CPUContributionLevel level);
    CPUContributionLevel getCPUContributionLevel() const { return _config.cpuLevel; }

    void setIdleDetectionLevel(IdleDetectionLevel level);
    IdleDetectionLevel getIdleDetectionLevel() const { return _config.idleLevel; }

    // Status
    bool isContributing() const;
    IdleState getIdleState() const;
    AkashiStatistics getStatistics() const { return _statistics; }
    void resetStatistics();

    // Manual control
    void startContributing();
    void stopContributing();
    void pauseContributing();
    void resumeContributing();

    // Idle detection
    bool detectIdleState();
    qint64 getIdleTimeSeconds() const;
    void updateIdleState();

Q_SIGNALS:
    void contributionStarted();
    void contributionStopped();
    void contributionPaused();
    void contributionResumed();
    void idleStateChanged(bool isIdle);
    void statisticsUpdated(const AkashiStatistics &stats);
    void taskCompleted(const QString &taskId, qint64 creditsEarned);
    void error(const QString &error);

private Q_SLOTS:
    void checkIdleState();
    void updateContributionState();
    void processNetworkTasks();
    void updateStatistics();
    void handleBatteryChange();

private:
    // Idle detection helpers
    qint64 getSystemIdleTime();  // Platform-specific idle time detection
    bool isBatteryCharging();    // Check if device is charging
    int getBatteryPercentage();  // Get current battery level
    double getCurrentCPUUsage(); // Get current system CPU usage

    // Contribution helpers
    void startCPUContribution();
    void stopCPUContribution();
    void adjustCPUUsage();

    // Network communication
    bool connectToAkashiNetwork();
    void disconnectFromAkashiNetwork();
    bool sendHeartbeat();
    void requestTask();
    void submitTaskResult(const QString &taskId, const QByteArray &result);

    // Task processing
    void processComputeTask(const QString &taskId, const QByteArray &taskData);
    void processRelayTask(const QString &taskId);

    Session *_session = nullptr;
    bool _initialized = false;
    bool _paused = false;

    AkashiConfiguration _config;
    AkashiStatistics _statistics;
    IdleState _idleState;

    // Worker thread for CPU-intensive tasks
    QThread *_workerThread = nullptr;

    // Timers
    base::Timer _idleCheckTimer;
    base::Timer _statsUpdateTimer;
    base::Timer _heartbeatTimer;

    // Synchronization
    QMutex _statsMutex;
    QMutex _configMutex;

    // Current state
    bool _isContributing = false;
    QString _currentTaskId;
    QDateTime _contributionStartTime;
};

} // namespace Data
