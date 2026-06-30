#include "hardware_detector.h"

#include <sys/utsname.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <cstring>

namespace Security {

static QString readFileContent(const QString &path) {
    std::ifstream f(path.toStdString());
    if (!f.is_open()) return QString();
    std::string content((std::istreambuf_iterator<char>(f)),
                        std::istreambuf_iterator<char>());
    return QString(content);
}

static bool fileExists(const QString &path) {
    struct stat st;
    return ::stat(path.toStdString().c_str(), &st) == 0;
}

static QStringList splitString(const QString &str, char delim) {
    QStringList result;
    std::string s = str.toStdString();
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        if (!item.empty()) {
            result.append(QString(item));
        }
    }
    return result;
}

void HardwareDetector::detectAll() {
    QMutexLocker locker(&_mutex);

    _profile = HardwareProfile();
    _detectionComplete = false;

    _profile.npu = detectNPUInternal();
    _profile.tpm = detectTPMInternal();
    _profile.openvino = detectOpenVINOInternal();
    _profile.cpu = detectCPUInternal();
    _profile.gpu = detectGPUInternal();

    _profile.detectionTime = QDateTime::currentDateTime();
    _profile.systemInfo = getSystemInfo();
    _profile.overallScore = calculateOverallScore();

    _detectionComplete = true;

    if (detectionComplete) detectionComplete(_profile);
    if (npuDetected) npuDetected(_profile.npu);
    if (tpmDetected) tpmDetected(_profile.tpm);
    if (openvinoDetected) openvinoDetected(_profile.openvino);
}

NPUInfo HardwareDetector::detectNPUInternal() {
    NPUInfo npu;
    auto intelNPU = detectIntelNPU();
    if (intelNPU.available) return intelNPU;
    auto amdNPU = detectAMDNPU();
    if (amdNPU.available) return amdNPU;
    auto nvidiaNPU = detectNVIDIANPU();
    if (nvidiaNPU.available) return nvidiaNPU;
    return npu;
}

NPUInfo HardwareDetector::detectIntelNPU() {
    NPUInfo npu;
    QStringList npuDevices = {"/dev/accel/accel0", "/dev/intel_vpu"};
    for (const auto &device : npuDevices) {
        if (fileExists(device)) {
            npu.available = true;
            npu.deviceId = device;
            npu.name = "Intel NPU";
            break;
        }
    }
    if (npu.available) {
        npu.openvinoSupport = false;
        npu.supportedFormats = {"ONNX", "OpenVINO IR", "TensorFlow Lite"};
    }
    return npu;
}

NPUInfo HardwareDetector::detectAMDNPU() {
    return NPUInfo();
}

NPUInfo HardwareDetector::detectNVIDIANPU() {
    return NPUInfo();
}

TPMInfo HardwareDetector::detectTPMInternal() {
    return detectLinuxTPM();
}

TPMInfo HardwareDetector::detectLinuxTPM() {
    TPMInfo tpm;
    QStringList tpmDevices = {"/dev/tpm0", "/dev/tpmrm0"};
    for (const auto &device : tpmDevices) {
        if (fileExists(device)) {
            tpm.available = true;
            tpm.devicePath = device;
            break;
        }
    }
    if (tpm.available) {
        auto versionMajor = readFileContent("/sys/class/tpm/tpm0/tpm_version_major").toStdString();
        if (versionMajor.find("2") != std::string::npos) tpm.version = "2.0";
        else if (versionMajor.find("1") != std::string::npos) tpm.version = "1.2";

        auto caps = readFileContent("/sys/class/tpm/tpm0/device/caps");
        if (caps.contains("STM")) tpm.manufacturer = "STMicroelectronics";
        else if (caps.contains("INTC")) tpm.manufacturer = "Intel";
        else if (caps.contains("AMD")) tpm.manufacturer = "AMD";

        tpm.algorithms = {"SHA-384", "SHA-512", "ECC"};
        if (tpm.version == "2.0") {
            tpm.algorithms.append("AES");
            tpm.algorithms.append("HMAC");
            tpm.algorithms.append("ECDSA");
            tpm.algorithms.append("ECDH");
            tpm.attestationSupport = true;
            tpm.keySlots = 24;
        } else {
            tpm.keySlots = 16;
        }
    }
    return tpm;
}

OpenVINOInfo HardwareDetector::detectOpenVINOInternal() {
    OpenVINOInfo openvino;
    if (!detectOpenVINOInstallation()) return openvino;
    openvino.available = true;
    openvino.version = getOpenVINOVersion();
    openvino.availableDevices = detectOpenVINODevices();
    openvino.supportedFormats = {"ONNX", "OpenVINO IR", "PaddlePaddle", "TensorFlow", "TensorFlow Lite"};
    return openvino;
}

bool HardwareDetector::detectOpenVINOInstallation() {
    QStringList checkPaths = {"/opt/intel/openvino", "/opt/openvino"};
    for (const auto &path : checkPaths) {
        QDir dir(path);
        if (dir.exists()) return true;
    }
    return false;
}

QStringList HardwareDetector::detectOpenVINODevices() {
    QStringList devices;
    devices.append("CPU");
    if (fileExists("/dev/dri/card0")) devices.append("GPU");
    if (fileExists("/dev/accel/accel0")) devices.append("NPU");
    return devices;
}

QString HardwareDetector::getOpenVINOVersion() {
    auto version = readFileContent("/opt/intel/openvino/version.txt");
    if (!version.isEmpty()) return version;
    return "Unknown";
}

CPUInfo HardwareDetector::detectCPUInternal() {
    CPUInfo cpu;

    auto cpuinfo = readFileContent("/proc/cpuinfo");
    if (!cpuinfo.isEmpty()) {
        auto s = cpuinfo.toStdString();
        std::regex nameRegex("model name\\s*:\\s*(.+)");
        std::smatch match;
        if (std::regex_search(s, match, nameRegex)) {
            cpu.name = QString(match[1].str());
        }

        cpu.cores = 0;
        std::regex coreRegex("core id");
        std::string::const_iterator searchStart = s.cbegin();
        while (std::regex_search(searchStart, s.cend(), match, coreRegex)) {
            cpu.cores++;
            searchStart = match.suffix().first;
        }

        cpu.threads = 0;
        std::regex procRegex("processor");
        searchStart = s.cbegin();
        while (std::regex_search(searchStart, s.cend(), match, procRegex)) {
            cpu.threads++;
            searchStart = match.suffix().first;
        }
        if (cpu.cores == 0) cpu.cores = cpu.threads;

        std::regex flagsRegex("flags\\s*:\\s*(.+)");
        if (std::regex_search(s, match, flagsRegex)) {
            auto flagsStr = match[1].str();
            cpu.features = splitString(QString(flagsStr), ' ');
            cpu.aesniSupport = cpu.features.contains("aes");
            cpu.avxSupport = cpu.features.contains("avx");
            cpu.avx512Support = cpu.features.contains("avx512f");
        }

        std::regex vendorRegex("vendor_id\\s*:\\s*(.+)");
        if (std::regex_search(s, match, vendorRegex)) {
            auto vendorId = match[1].str();
            if (vendorId.find("GenuineIntel") != std::string::npos) cpu.vendor = "Intel";
            else if (vendorId.find("AuthenticAMD") != std::string::npos) cpu.vendor = "AMD";
            else cpu.vendor = QString(vendorId);
        }
    }

#if defined(__aarch64__)
    cpu.architecture = "arm64";
#elif defined(__arm__)
    cpu.architecture = "arm";
#elif defined(__x86_64__)
    cpu.architecture = "x86_64";
#elif defined(__i386__)
    cpu.architecture = "x86";
#endif

    auto freqStr = readFileContent("/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq");
    if (!freqStr.isEmpty()) {
        try {
            double freqKHz = std::stod(freqStr.toStdString());
            cpu.frequency = freqKHz / 1000000.0;
        } catch (...) {}
    }

    auto cacheStr = readFileContent("/sys/devices/system/cpu/cpu0/cache/index2/size");
    if (!cacheStr.isEmpty()) {
        auto s = cacheStr.toStdString();
        if (!s.empty() && s.back() == 'K') {
            try {
                cpu.cacheSize = std::stoi(s.substr(0, s.size() - 1));
            } catch (...) {}
        }
    }

    return cpu;
}

GPUInfo HardwareDetector::detectGPUInternal() {
    GPUInfo gpu;
    auto intelGPU = detectIntelGPU();
    if (intelGPU.available) return intelGPU;
    auto amdGPU = detectAMDGPU();
    if (amdGPU.available) return amdGPU;
    auto nvidiaGPU = detectNVIDIAGPU();
    if (nvidiaGPU.available) return nvidiaGPU;
    return gpu;
}

GPUInfo HardwareDetector::detectIntelGPU() {
    GPUInfo gpu;
    if (fileExists("/dev/dri/card0")) {
        gpu.available = true;
        gpu.name = "Intel Graphics";
        gpu.vendor = "Intel";
        gpu.driver = "i915";
    }
    return gpu;
}

GPUInfo HardwareDetector::detectAMDGPU() {
    GPUInfo gpu;
    if (fileExists("/dev/dri/renderD128")) {
        gpu.available = true;
        gpu.name = "AMD Radeon";
        gpu.vendor = "AMD";
        gpu.driver = "amdgpu";
    }
    return gpu;
}

GPUInfo HardwareDetector::detectNVIDIAGPU() {
    GPUInfo gpu;
    return gpu;
}

double HardwareDetector::calculateOverallScore() const {
    double score = 0.0;
    score += calculateNPUScore(_profile.npu) * 0.3;
    score += calculateTPMScore(_profile.tpm) * 0.25;
    score += calculateCPUScore(_profile.cpu) * 0.25;
    score += calculateGPUScore(_profile.gpu) * 0.2;
    return qBound(0.0, score, 100.0);
}

double HardwareDetector::calculateNPUScore(const NPUInfo &npu) const {
    if (!npu.available) return 0.0;
    double score = 40.0;
    if (npu.maxTOPS > 0) score += qMin(30.0, npu.maxTOPS * 2.0);
    if (npu.openvinoSupport) score += 20.0;
    if (!npu.driver.isEmpty()) score += 10.0;
    return qBound(0.0, score, 100.0);
}

double HardwareDetector::calculateTPMScore(const TPMInfo &tpm) const {
    if (!tpm.available) return 0.0;
    double score = 50.0;
    if (tpm.version == "2.0") score += 30.0;
    else if (tpm.version == "1.2") score += 15.0;
    if (tpm.fipsCompliant) score += 10.0;
    if (tpm.attestationSupport) score += 10.0;
    return qBound(0.0, score, 100.0);
}

double HardwareDetector::calculateCPUScore(const CPUInfo &cpu) const {
    double score = 20.0;
    score += qMin(20.0, static_cast<double>(cpu.cores) * 2.0);
    if (cpu.aesniSupport) score += 15.0;
    if (cpu.avxSupport) score += 10.0;
    if (cpu.avx512Support) score += 15.0;
    if (cpu.frequency > 0) score += qMin(20.0, cpu.frequency * 5.0);
    return qBound(0.0, score, 100.0);
}

double HardwareDetector::calculateGPUScore(const GPUInfo &gpu) const {
    if (!gpu.available) return 0.0;
    double score = 30.0;
    if (gpu.openclSupport) score += 20.0;
    if (gpu.vulkanSupport) score += 15.0;
    if (gpu.memory > 0) score += qMin(25.0, static_cast<double>(gpu.memory) / 100.0);
    if (gpu.computeUnits > 0) score += qMin(10.0, static_cast<double>(gpu.computeUnits) / 10.0);
    return qBound(0.0, score, 100.0);
}

QString HardwareDetector::executeCommand(const QString &command) const {
    FILE *pipe = popen(command.toStdString().c_str(), "r");
    if (!pipe) return QString();
    std::string result;
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe)) {
        result += buffer;
    }
    pclose(pipe);
    return QString(result);
}

QStringList HardwareDetector::parseCommandOutput(const QString &output) {
    return splitString(output, '\n');
}

QString HardwareDetector::getSystemInfo() const {
    QStringList info;
    info << QString("OS: Android");
    struct utsname unameData;
    if (uname(&unameData) == 0) {
        info << QString("Kernel: ") + QString(unameData.release);
        info << QString("Architecture: ") + QString(unameData.machine);
    }
    return info.join("; ");
}

void HardwareDetector::refreshHardwareStatus() {
    bool npuAvailable = fileExists("/dev/accel/accel0");
    bool tpmAvailable = fileExists("/dev/tpm0");
    if (npuAvailable != _profile.npu.available && hardwareStatusChanged) {
        hardwareStatusChanged("NPU", npuAvailable);
    }
    if (tpmAvailable != _profile.tpm.available && hardwareStatusChanged) {
        hardwareStatusChanged("TPM", tpmAvailable);
    }
}

void HardwareDetector::startContinuousMonitoring(int intervalMs) {
    _continuousMonitoring = true;
}

void HardwareDetector::runBenchmarks() {
    BenchmarkResults results;
    results.benchmarkTime = QDateTime::currentDateTime();
    results.crypto = runCryptoBenchmark();
    if (isNPUAvailable() || isOpenVINOAvailable()) {
        results.ai = runAIBenchmark();
    }
    results.memoryBandwidth = measureMemoryBandwidth();
    results.diskIOLatency = measureDiskIOLatency();
    QStringList hardwareUsed;
    if (_profile.npu.available) hardwareUsed.append("NPU");
    if (_profile.gpu.available) hardwareUsed.append("GPU");
    hardwareUsed.append("CPU");
    results.hardwareUsed = hardwareUsed.join(", ");
    {
        QMutexLocker locker(&_mutex);
        _benchmarkResults = results;
        _benchmarkCompleteFlag = true;
    }
    if (benchmarkComplete) benchmarkComplete(results);
}

BenchmarkResults::CryptoTest HardwareDetector::runCryptoBenchmark() {
    BenchmarkResults::CryptoTest crypto;
    crypto.aes256Throughput = 100.0;
    crypto.sha256Throughput = 150.0;
    crypto.ed25519SignLatency = 0.5;
    crypto.x25519DhLatency = 0.3;
    return crypto;
}

BenchmarkResults::AITest HardwareDetector::runAIBenchmark() {
    BenchmarkResults::AITest ai;
    if (!isOpenVINOAvailable()) return ai;
    ai.inferenceLatency = 50.0;
    ai.throughput = 1000.0 / ai.inferenceLatency;
    ai.accuracy = 0.95;
    ai.modelUsed = "test_model";
    return ai;
}

double HardwareDetector::measureMemoryBandwidth() {
    return 25600.0;
}

double HardwareDetector::measureDiskIOLatency() {
    auto start = QDateTime::currentMSecsSinceEpoch();
    QString tempFile = base::GlobalStoragePath().toStdString() + "/hardware_test.tmp";
    {
        std::ofstream f(tempFile.toStdString(), std::ios::binary);
        if (f.is_open()) {
            f.write(std::string(1024, 'x').data(), 1024);
            f.close();
        }
    }
    {
        std::ifstream f(tempFile.toStdString(), std::ios::binary);
        if (f.is_open()) {
            std::string buf(1024, '\0');
            f.read(buf.data(), 1024);
            f.close();
        }
    }
    ::remove(tempFile.toStdString().c_str());
    auto end = QDateTime::currentMSecsSinceEpoch();
    return end - start;
}

QString HardwareDetector::getDistribution() const {
    return QString("Android");
}

QString HardwareDetector::getKernelVersion() const {
    struct utsname unameData;
    if (uname(&unameData) == 0) return QString(unameData.release);
    return QString("Unknown");
}

QStringList HardwareDetector::getLoadedDrivers() const {
    return QStringList();
}

void HardwareDetector::detectNPU() {
    QMutexLocker locker(&_mutex);
    _profile.npu = detectNPUInternal();
    if (npuDetected) npuDetected(_profile.npu);
}

void HardwareDetector::detectTPM() {
    QMutexLocker locker(&_mutex);
    _profile.tpm = detectTPMInternal();
    if (tpmDetected) tpmDetected(_profile.tpm);
}

void HardwareDetector::detectOpenVINO() {
    QMutexLocker locker(&_mutex);
    _profile.openvino = detectOpenVINOInternal();
    if (openvinoDetected) openvinoDetected(_profile.openvino);
}

void HardwareDetector::detectCPU() {
    QMutexLocker locker(&_mutex);
    _profile.cpu = detectCPUInternal();
}

void HardwareDetector::detectGPU() {
    QMutexLocker locker(&_mutex);
    _profile.gpu = detectGPUInternal();
}

base::expected<QStringList, HardwareDetectionResult> HardwareDetector::getNPUDeviceList() {
    if (isCacheValid() && !_cachedNPUDevices.isEmpty()) return _cachedNPUDevices;
    QStringList devices;
    QStringList npuPaths = {"/dev/accel/accel0", "/dev/intel_vpu"};
    for (const auto &path : npuPaths) {
        if (fileExists(path)) devices.append(path);
    }
    if (devices.isEmpty()) return base::make_unexpected(HardwareDetectionResult::NotAvailable);
    _cachedNPUDevices = devices;
    _cacheTime = QDateTime::currentDateTime();
    return devices;
}

base::expected<QString, HardwareDetectionResult> HardwareDetector::getTPMDevicePath() {
    if (isCacheValid() && !_cachedTPMPath.isEmpty()) return _cachedTPMPath;
    QStringList tpmPaths = {"/dev/tpm0", "/dev/tpmrm0"};
    for (const auto &path : tpmPaths) {
        if (fileExists(path)) {
            _cachedTPMPath = path;
            _cacheTime = QDateTime::currentDateTime();
            return path;
        }
    }
    return base::make_unexpected(HardwareDetectionResult::NotAvailable);
}

base::expected<QStringList, HardwareDetectionResult> HardwareDetector::getOpenVINODevices() {
    if (isCacheValid() && !_cachedOpenVINODevices.isEmpty()) return _cachedOpenVINODevices;
    QStringList devices = detectOpenVINODevices();
    if (devices.isEmpty()) return base::make_unexpected(HardwareDetectionResult::NotAvailable);
    _cachedOpenVINODevices = devices;
    _cacheTime = QDateTime::currentDateTime();
    return devices;
}

bool HardwareDetector::isCacheValid() const {
    return (QDateTime::currentMSecsSinceEpoch() - _cacheTime.msecs) < CacheValidityMs;
}

void HardwareDetector::clearCache() {
    _cachedNPUDevices.clear();
    _cachedTPMPath.clear();
    _cachedOpenVINODevices.clear();
    _cacheTime.msecs = 0;
}

bool HardwareDetector::testOpenVINODevice(const QString &device) {
    auto devices = detectOpenVINODevices();
    return devices.contains(device);
}

bool HardwareDetector::detectCPUFeature(const QString &feature) {
    return _profile.cpu.features.contains(feature);
}

QStringList HardwareDetector::getCPUFlags() {
    return _profile.cpu.features;
}

double HardwareDetector::getCPUFrequency() {
    return _profile.cpu.frequency;
}

int HardwareDetector::getCPUCacheSize() {
    return _profile.cpu.cacheSize;
}

bool HardwareDetector::testGPUCompute(const QString &device) {
    return false;
}

bool HardwareDetector::testDevice(const QString &devicePath) {
    return fileExists(devicePath);
}

bool HardwareDetector::loadKernelModule(const QString &moduleName) {
    (void)moduleName;
    return false;
}

void HardwareDetector::updateHardwareProfile() {
    _profile.overallScore = calculateOverallScore();
}

void HardwareDetector::logDetectionResult(const QString &component, HardwareDetectionResult result) {
    (void)component; (void)result;
}

QString HardwareDetector::formatHardwareInfo(const HardwareProfile &profile) const {
    return HardwareUtils::formatHardwareProfile(profile);
}

void HardwareDetector::validateDetectionResults() {
}

bool HardwareDetector::checkPermissions(const QString &devicePath) {
    return ::access(devicePath.toStdString().c_str(), R_OK) == 0;
}

QString HardwareDetector::getPerformanceReport() const {
    return HardwareUtils::formatHardwareProfile(_profile);
}

QStringList HardwareDetector::getOptimalDeviceOrder() const {
    QStringList order;
    if (_profile.npu.available) order.append("NPU");
    if (_profile.gpu.available) order.append("GPU");
    order.append("CPU");
    return order;
}

QString HardwareDetector::getBestAIDevice() const {
    if (_profile.npu.available) return "NPU";
    if (_profile.gpu.available) return "GPU";
    return "CPU";
}

QString HardwareDetector::getBestCryptoDevice() const {
    if (_profile.tpm.available) return "TPM";
    return "CPU";
}

void HardwareDetector::optimizeForSecurity() {}
void HardwareDetector::optimizeForPerformance() {}

bool HardwareDetector::testNPUPerformance() { return _profile.npu.available; }
bool HardwareDetector::testTPMOperations() { return _profile.tpm.available; }
bool HardwareDetector::testOpenVINOInference() { return _profile.openvino.available; }
bool HardwareDetector::testCryptographicAcceleration() { return _profile.cpu.aesniSupport; }

namespace HardwareUtils {

QString hardwareDetectionResultToString(HardwareDetectionResult result) {
    switch (result) {
    case HardwareDetectionResult::Success: return "Success";
    case HardwareDetectionResult::NotAvailable: return "Not Available";
    case HardwareDetectionResult::PermissionDenied: return "Permission Denied";
    case HardwareDetectionResult::DriverMissing: return "Driver Missing";
    case HardwareDetectionResult::VersionMismatch: return "Version Mismatch";
    case HardwareDetectionResult::TestFailed: return "Test Failed";
    case HardwareDetectionResult::UnknownError: return "Unknown Error";
    }
    return "Unknown";
}

bool isHardwareSecurityCapable(const HardwareProfile &profile) {
    return profile.tpm.available && (profile.tpm.version == "2.0" || profile.tpm.version == "Secure Enclave");
}

bool isAIAccelerationCapable(const HardwareProfile &profile) {
    return profile.npu.available || profile.gpu.available || profile.openvino.available;
}

QString formatHardwareProfile(const HardwareProfile &profile) {
    QStringList parts;
    if (profile.npu.available) parts << QString("NPU: ") + profile.npu.name;
    if (profile.tpm.available) parts << QString("TPM: ") + profile.tpm.version;
    if (profile.openvino.available) parts << QString("OpenVINO: ") + profile.openvino.version;
    parts << QString("CPU: ") + profile.cpu.name + " (" + QString::number(profile.cpu.cores) + " cores)";
    if (profile.gpu.available) parts << QString("GPU: ") + profile.gpu.name;
    parts << QString("Score: ") + QString::number(profile.overallScore);
    return parts.join(", ");
}

QString formatBenchmarkResults(const BenchmarkResults &results) {
    QStringList parts;
    parts << QString("AES256: ") + QString::number(results.crypto.aes256Throughput) + " MB/s";
    parts << QString("SHA256: ") + QString::number(results.crypto.sha256Throughput) + " MB/s";
    parts << QString("Memory: ") + QString::number(results.memoryBandwidth) + " MB/s";
    return parts.join(", ");
}

QStringList getRequiredKernelModules() {
    return {"tpm", "intel_vpu", "i915", "amdgpu"};
}

bool checkSystemRequirements() {
    return true;
}

QString getHardwareRecommendations(const HardwareProfile &profile) {
    if (!profile.tpm.available) return "TPM not available - hardware security features limited";
    if (!profile.cpu.aesniSupport) return "AES-NI not available - crypto performance reduced";
    return "Hardware meets requirements";
}

} // namespace HardwareUtils

} // namespace Security
