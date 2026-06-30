#pragma once

#include "desktop_shims.h"
#include "base/expected.h"
#include "base/bytes.h"

#include <memory>
#include <vector>

namespace Security {

enum class HardwareDetectionResult {
    Success,
    NotAvailable,
    PermissionDenied,
    DriverMissing,
    VersionMismatch,
    TestFailed,
    UnknownError
};

struct NPUInfo {
    bool available = false;
    QString name;
    QString driver;
    QString version;
    int computeUnits = 0;
    double maxTOPS = 0.0;
    bool openvinoSupport = false;
    QString deviceId;
    QString vendorId;
    double powerConsumption = 0.0;
    QStringList supportedFormats;
};

struct TPMInfo {
    bool available = false;
    QString version;
    QString manufacturer;
    QString firmwareVersion;
    QString devicePath;
    bool fipsCompliant = false;
    QStringList algorithms;
    int keySlots = 0;
    bool attestationSupport = false;
    QString spec;
};

struct OpenVINOInfo {
    bool available = false;
    QString version;
    QString installPath;
    QStringList availableDevices;
    QStringList supportedFormats;
    bool optimizationsEnabled = false;
    QString runtimeVersion;
    QStringList pluginVersions;
};

struct CPUInfo {
    QString name;
    QString architecture;
    int cores = 0;
    int threads = 0;
    QStringList features;
    bool aesniSupport = false;
    bool avxSupport = false;
    bool avx512Support = false;
    double frequency = 0.0;
    int cacheSize = 0;
    QString vendor;
};

struct GPUInfo {
    bool available = false;
    QString name;
    QString vendor;
    QString driver;
    int computeUnits = 0;
    size_t memory = 0;
    bool openclSupport = false;
    bool vulkanSupport = false;
    QString deviceId;
};

struct HardwareProfile {
    NPUInfo npu;
    TPMInfo tpm;
    OpenVINOInfo openvino;
    CPUInfo cpu;
    GPUInfo gpu;
    QDateTime detectionTime;
    QString systemInfo;
    double overallScore = 0.0;
};

struct BenchmarkResults {
    struct CryptoTest {
        double aes256Throughput = 0.0;
        double sha256Throughput = 0.0;
        double ed25519SignLatency = 0.0;
        double x25519DhLatency = 0.0;
    };

    struct AITest {
        double inferenceLatency = 0.0;
        double throughput = 0.0;
        double accuracy = 0.0;
        QString modelUsed;
    };

    CryptoTest crypto;
    AITest ai;
    double memoryBandwidth = 0.0;
    double diskIOLatency = 0.0;
    QDateTime benchmarkTime;
    QString hardwareUsed;
};

class HardwareDetector {
public:
    explicit HardwareDetector(QObject *parent = nullptr) { (void)parent; }
    ~HardwareDetector() { stopContinuousMonitoring(); }

    void detectAll();
    void detectNPU();
    void detectTPM();
    void detectOpenVINO();
    void detectCPU();
    void detectGPU();

    HardwareProfile getHardwareProfile() const { return _profile; }
    NPUInfo getNPUInfo() const { return _profile.npu; }
    TPMInfo getTPMInfo() const { return _profile.tpm; }
    OpenVINOInfo getOpenVINOInfo() const { return _profile.openvino; }
    CPUInfo getCPUInfo() const { return _profile.cpu; }
    GPUInfo getGPUInfo() const { return _profile.gpu; }

    bool isNPUAvailable() const { return _profile.npu.available; }
    bool isTPMAvailable() const { return _profile.tpm.available; }
    bool isOpenVINOAvailable() const { return _profile.openvino.available; }
    bool hasHardwareAcceleration() const { return _profile.gpu.available || _profile.npu.available; }
    bool hasSecureHardware() const { return _profile.tpm.available; }

    void runBenchmarks();
    BenchmarkResults getBenchmarkResults() const { return _benchmarkResults; }
    double calculateOverallScore() const;
    QString getPerformanceReport() const;

    bool testNPUPerformance();
    bool testTPMOperations();
    bool testOpenVINOInference();
    bool testCryptographicAcceleration();

    QStringList getOptimalDeviceOrder() const;
    QString getBestAIDevice() const;
    QString getBestCryptoDevice() const;
    void optimizeForSecurity();
    void optimizeForPerformance();

    void startContinuousMonitoring(int intervalMs = 30000);
    void stopContinuousMonitoring() { _continuousMonitoring = false; }
    bool isContinuousMonitoringActive() const { return _continuousMonitoring; }
    void refreshHardwareStatus();

    base::expected<QStringList, HardwareDetectionResult> getNPUDeviceList();
    base::expected<QString, HardwareDetectionResult> getTPMDevicePath();
    base::expected<QStringList, HardwareDetectionResult> getOpenVINODevices();

    QString getSystemInfo() const;
    QString getKernelVersion() const;
    QString getDistribution() const;
    QStringList getLoadedDrivers() const;

    std::function<void(const HardwareProfile &)> detectionComplete;
    std::function<void(const NPUInfo &)> npuDetected;
    std::function<void(const TPMInfo &)> tpmDetected;
    std::function<void(const OpenVINOInfo &)> openvinoDetected;
    std::function<void(const BenchmarkResults &)> benchmarkComplete;
    std::function<void(const QString &, bool)> hardwareStatusChanged;
    std::function<void(const QString &, HardwareDetectionResult)> detectionError;

private:
    NPUInfo detectNPUInternal();
    TPMInfo detectTPMInternal();
    OpenVINOInfo detectOpenVINOInternal();
    CPUInfo detectCPUInternal();
    GPUInfo detectGPUInternal();

    NPUInfo detectIntelNPU();
    NPUInfo detectAMDNPU();
    NPUInfo detectNVIDIANPU();
    TPMInfo detectLinuxTPM();

    bool detectOpenVINOInstallation();
    QStringList detectOpenVINODevices();
    QString getOpenVINOVersion();
    bool testOpenVINODevice(const QString &device);

    bool detectCPUFeature(const QString &feature);
    QStringList getCPUFlags();
    double getCPUFrequency();
    int getCPUCacheSize();

    GPUInfo detectIntelGPU();
    GPUInfo detectAMDGPU();
    GPUInfo detectNVIDIAGPU();
    bool testGPUCompute(const QString &device);

    BenchmarkResults::CryptoTest runCryptoBenchmark();
    BenchmarkResults::AITest runAIBenchmark();
    double measureMemoryBandwidth();
    double measureDiskIOLatency();

    bool testDevice(const QString &devicePath);
    bool loadKernelModule(const QString &moduleName);
    QString executeCommand(const QString &command) const;
    QStringList parseCommandOutput(const QString &output);

    double calculateNPUScore(const NPUInfo &npu) const;
    double calculateTPMScore(const TPMInfo &tpm) const;
    double calculateCPUScore(const CPUInfo &cpu) const;
    double calculateGPUScore(const GPUInfo &gpu) const;

    mutable QMutex _mutex;
    HardwareProfile _profile;
    BenchmarkResults _benchmarkResults;
    bool _detectionComplete = false;
    bool _benchmarkCompleteFlag = false;
    bool _continuousMonitoring = false;

    QStringList _cachedNPUDevices;
    QString _cachedTPMPath;
    QStringList _cachedOpenVINODevices;
    QDateTime _cacheTime;
    static constexpr int CacheValidityMs = 60000;

    void updateHardwareProfile();
    void clearCache();
    bool isCacheValid() const;
    void logDetectionResult(const QString &component, HardwareDetectionResult result);
    QString formatHardwareInfo(const HardwareProfile &profile) const;
    void validateDetectionResults();
    bool checkPermissions(const QString &devicePath);
};

namespace HardwareUtils {
    QString hardwareDetectionResultToString(HardwareDetectionResult result);
    bool isHardwareSecurityCapable(const HardwareProfile &profile);
    bool isAIAccelerationCapable(const HardwareProfile &profile);
    QString formatHardwareProfile(const HardwareProfile &profile);
    QString formatBenchmarkResults(const BenchmarkResults &results);
    QStringList getRequiredKernelModules();
    bool checkSystemRequirements();
    QString getHardwareRecommendations(const HardwareProfile &profile);
}

} // namespace Security
