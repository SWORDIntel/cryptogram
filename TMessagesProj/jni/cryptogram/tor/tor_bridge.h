#pragma once

#include <cstdint>
#include <string>
#include <functional>
#include <mutex>
#include <atomic>

namespace Cryptogram {

enum class TorStatus {
    Stopped,
    Starting,
    Running,
    Failed
};

struct TorConfig {
    std::string socksHost = "127.0.0.1";
    uint16_t socksPort = 9050;
    std::string dataDirectory;
    std::string controlHost = "127.0.0.1";
    uint16_t controlPort = 9051;
    std::string controlPassword;
    bool useBridges = false;
    std::string bridgeLine;
    bool usePluggableTransport = false;
    std::string ptExecutable;
    std::string ptOptions;
};

struct TorCircuitInfo {
    std::string circuitId;
    std::string path;
    std::string status;
    int hops = 0;
};

class TorBridge {
public:
    static TorBridge& instance();

    bool start(const TorConfig& config);
    void stop();
    TorStatus status() const;
    std::string statusString() const;

    std::string getSocksProxy() const;
    std::vector<TorCircuitInfo> getCircuits() const;
    bool newIdentity();

    std::string getBootstrapStatus() const;
    int getBootstrapProgress() const;

    std::string getLastError() const;

private:
    TorBridge() = default;
    ~TorBridge();

    TorBridge(const TorBridge&) = delete;
    TorBridge& operator=(const TorBridge&) = delete;

    mutable std::mutex _mutex;
    std::atomic<TorStatus> _status{TorStatus::Stopped};
    TorConfig _config;
    std::atomic<int> _bootstrapProgress{0};
    std::string _lastError;
    std::string _bootstrapStatus;

    bool _libtorAvailable = false;
    void* _libtorHandle = nullptr;

    bool loadLibtor();
    void unloadLibtor();
};

} // namespace Cryptogram
