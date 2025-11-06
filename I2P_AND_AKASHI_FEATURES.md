# I2P Integration & Akashi Network - CRYPTOGRAM Community Features

## Overview
This document describes two new community-focused features added to CRYPTOGRAM:
1. **I2P (Invisible Internet Project) Integration** - Advanced anonymity network
2. **Akashi Network Contribution** - Optional CPU/resource donation system

**Date**: November 5, 2025
**Status**: ✅ Complete - Ready for implementation

---

## 🌐 Feature 1: I2P Integration

### What is I2P?
The **Invisible Internet Project (I2P)** is a fully distributed anonymity network that provides:
- **Garlic Routing**: Multiple messages bundled together and encrypted in layers
- **Distributed Network**: No central servers or points of failure
- **End-to-End Encryption**: All communication is encrypted
- **.i2p Addresses**: Hidden services similar to Tor's .onion
- **Censorship Resistance**: Nearly impossible to block
- **Network Database**: Distributed peer discovery

### I2P vs Tor

| Feature | Tor | I2P | CRYPTOGRAM Support |
|---------|-----|-----|-------------------|
| **Anonymity** | Circuit-based | Garlic routing | Both |
| **Use Case** | Web browsing | P2P & messaging | Both |
| **Censorship Resistance** | Good | Excellent | Both |
| **Speed** | Faster | Moderate | Both |
| **Hidden Services** | .onion | .i2p | Both |
| **Network Size** | Large | Medium | Both |

### Key Features

#### 1. Multiple Tunnel Types
```cpp
// Client tunnel - Outbound connections
I2PTunnelConfig clientTunnel;
clientTunnel.tunnelType = I2PTunnelType::Client;
clientTunnel.localPort = 7657;
clientTunnel.tunnelLength = 3;  // 3 hops for anonymity

// Server tunnel - Receive connections
I2PTunnelConfig serverTunnel;
serverTunnel.tunnelType = I2PTunnelType::Server;
serverTunnel.destination = "generated-i2p-address.b32.i2p";

// HTTP/SOCKS proxy tunnels
I2PTunnelConfig proxyTunnel;
proxyTunnel.tunnelType = I2PTunnelType::SOCKS;
proxyTunnel.localPort = 4444;
```

#### 2. SAM Protocol Support
- **SAM (Simple Anonymous Messaging)**: Standard I2P protocol
- Easy integration with existing applications
- Supports streaming and datagram communication
- Session management and persistence

#### 3. Automatic Tunnel Management
- **Auto-start**: Tunnels start automatically with CRYPTOGRAM
- **Persistence**: Keeps tunnels alive even if idle
- **Health monitoring**: Checks tunnel status every 30 seconds
- **Auto-recovery**: Recreates failed tunnels automatically

#### 4. Hidden Service (.i2p) Support
```cpp
// Generate a new I2P destination
QString destination = i2p->createDestination();
// Result: "alongbase64string.b32.i2p"

// Use for CRYPTOGRAM messaging
// Users can connect via: cryptogram://destination.i2p
```

### Technical Specifications

**Protocol**: SAM (Simple Anonymous Messaging) v3.1
**Default Router**: 127.0.0.1:7656 (SAM bridge)
**Tunnel Configuration**:
- **Tunnel Length**: 3 hops (configurable 1-7)
- **Tunnel Quantity**: 2 tunnels for redundancy
- **Tunnel Lifetime**: 10 minutes (auto-refresh)

**Encryption**:
- **Garlic Encryption**: AES-256
- **End-to-End**: ElGamal + AES
- **Session**: ECDSA-SHA256

### Use Cases

#### 1. Censorship Bypass
- Access CRYPTOGRAM in countries where it's blocked
- Circumvent ISP-level blocking
- Bypass DPI (Deep Packet Inspection)
- Use when Tor is blocked

#### 2. Enhanced Anonymity
- Hide connection to CRYPTOGRAM servers
- Protect against traffic analysis
- Additional layer beyond encryption
- Prevent IP address leakage

#### 3. Hidden Services
- Run CRYPTOGRAM relay node on .i2p
- Create anonymous chat rooms
- Host encrypted file sharing
- P2P messaging without central server

#### 4. Combined with Tor
- **Tor over I2P**: Use both for maximum anonymity
- **Failover**: Switch to I2P if Tor fails
- **Load balancing**: Distribute traffic across both

### Settings Location
**Path**: `Settings → CRYPTOGRAM → Network Anonymity → I2P Configuration`

**Settings Menu Structure**:
```
Settings
└── CRYPTOGRAM (near bottom)
    ├── Network Anonymity
    │   ├── Tor Configuration
    │   ├── I2P Configuration ← NEW
    │   │   ├── Enable I2P
    │   │   ├── Auto-start with CRYPTOGRAM
    │   │   ├── Router Address (default: 127.0.0.1)
    │   │   ├── Router Port (default: 7656)
    │   │   ├── Tunnel Settings
    │   │   │   ├── Tunnel Length (1-7 hops)
    │   │   │   ├── Tunnel Quantity (1-5)
    │   │   │   └── Auto-reconnect
    │   │   ├── My I2P Destination (generated)
    │   │   └── Status & Statistics
    │   └── Bridge Configuration
    └── Akashi Network (below)
```

### Integration Example
```cpp
// Initialize I2P
auto i2p = std::make_unique<I2PIntegration>(session);
i2p->initialize();

// Connect to I2P router
if (i2p->connectToRouter()) {
    // Create client tunnel for CRYPTOGRAM
    I2PTunnelConfig config;
    config.tunnelType = I2PTunnelType::Client;
    config.destination = "cryptogram-server.i2p";
    config.localPort = 8443;
    config.autoStart = true;
    config.persistent = true;

    QString tunnelId = i2p->createTunnel(config);

    // Now connect to CRYPTOGRAM through I2P tunnel
    // All traffic goes through I2P garlic routing
}
```

---

## 🤝 Feature 2: Akashi Network Contribution

### What is Akashi Network?
The **Akashi Network** is CRYPTOGRAM's distributed privacy infrastructure that allows users to optionally donate idle CPU cycles and resources to help maintain the network.

**Key Principle**: Users helping users - create a resilient, community-supported privacy network.

### Why Contribute?

#### Benefits to Community
- **Strengthen Privacy Network**: More nodes = better privacy for everyone
- **Censorship Resistance**: Distributed network is harder to block
- **Speed Improvements**: More relay nodes = faster connections
- **Resilience**: No single point of failure

#### Benefits to Contributors
- **Earn Credits**: Contribute and earn network credits (future feature)
- **Reputation Score**: Build reputation in the CRYPTOGRAM community
- **Priority Access**: Contributors get priority during high load
- **Give Back**: Help others while you're not using your computer

### Key Features

#### 1. Intelligent Idle Detection

**4 Idle Detection Levels**:

```cpp
enum class IdleDetectionLevel {
    Never,          // Never consider idle
    Conservative,   // Idle after 30 minutes of no input
    Moderate,       // Idle after 15 minutes (DEFAULT)
    Aggressive      // Idle after 5 minutes
};
```

**Smart Detection**:
- Monitors keyboard and mouse activity
- Tracks CPU usage (doesn't contribute if system is busy)
- Checks battery status (stops on low battery)
- Detects charging state (optional: only when charging)

#### 2. Flexible CPU Contribution Levels

**7 CPU Levels**:
```cpp
enum class CPUContributionLevel {
    None = 0,       // 0% - Disabled
    Minimal = 5,    // 5% CPU
    Low = 10,       // 10% CPU
    Default = 20,   // 20% CPU ← DEFAULT
    Medium = 30,    // 30% CPU
    High = 50,      // 50% CPU
    Maximum = 75    // 75% CPU (max for stability)
};
```

**Default**: 20% CPU when idle

**Why 20%?**
- Noticeable contribution without impacting performance
- Won't heat up device significantly
- Won't drain battery excessively
- Easy to override if desired

#### 3. Multiple Contribution Types

```cpp
struct AkashiConfiguration {
    bool enableRelay = true;      // Act as message relay
    bool enableCompute = true;    // Donate CPU for tasks
    bool enableStorage = false;   // Provide storage (opt-in)
    bool enableBandwidth = true;  // Share bandwidth
};
```

**Types Explained**:

**Relay** (Default: ON)
- Forward encrypted messages between peers
- Help users in censored regions
- Improve network performance
- No storage required

**Compute** (Default: ON)
- Process encryption tasks
- Help with key generation
- Assist with voice morphing
- Support quantum encryption

**Storage** (Default: OFF - opt-in)
- Store encrypted chunks of data
- Help with distributed backup
- Requires disk space (configurable: 1-100 GB)

**Bandwidth** (Default: ON)
- Share internet connection
- Configurable limit (default: 10 Mbps)
- Helps users with slow connections

#### 4. Battery-Aware Operation

```cpp
struct AkashiConfiguration {
    bool onlyWhenCharging = true;     // DEFAULT
    bool onlyWhenIdle = true;         // DEFAULT
    bool enableWhenBatteryLow = false;
    int minBatteryPercent = 50;       // Minimum battery to continue
};
```

**Smart Battery Management**:
- Stops contributing when battery < 50% (configurable)
- Auto-pauses on battery (optional)
- Resumes when plugged in
- Prevents battery drain

### Settings Location
**Path**: `Settings → CRYPTOGRAM → Akashi Network`
**Position**: Near bottom of CRYPTOGRAM menu, 2 levels deep

**Complete Menu Structure**:
```
Settings (Main Menu)
├── ...
├── Privacy and Security
├── Data and Storage
├── ...
└── CRYPTOGRAM ← Near bottom
    ├── Security Features
    ├── Network Anonymity
    │   ├── Tor Configuration
    │   ├── I2P Configuration
    │   └── Bridge Configuration
    └── Akashi Network ← HERE
        ├── 🎁 Contribute to Network
        │   ├── Enable Contribution [Toggle] (OFF by default)
        │   └── Help Support Privacy Infrastructure
        │
        ├── ⚙️ Contribution Settings
        │   ├── CPU Contribution Level
        │   │   ├── None (0%)
        │   │   ├── Minimal (5%)
        │   │   ├── Low (10%)
        │   │   ├── Default (20%) ← Selected
        │   │   ├── Medium (30%)
        │   │   ├── High (50%)
        │   │   └── Maximum (75%)
        │   │
        │   ├── When to Contribute
        │   │   ├── Idle Detection Level
        │   │   │   ├── Never (always respect CPU limit)
        │   │   │   ├── Conservative (30 min idle)
        │   │   │   ├── Moderate (15 min idle) ← Selected
        │   │   │   └── Aggressive (5 min idle)
        │   │   │
        │   │   ├── ✓ Only when charging [Checkbox] ← ON
        │   │   └── ✓ Only when idle [Checkbox] ← ON
        │   │
        │   └── Contribution Types
        │       ├── ✓ Relay encrypted messages [Checkbox] ← ON
        │       ├── ✓ Donate CPU cycles [Checkbox] ← ON
        │       ├── ☐ Provide storage [Checkbox] (OFF)
        │       │   └── Max Storage: [5] GB
        │       └── ✓ Share bandwidth [Checkbox] ← ON
        │           └── Max Bandwidth: [10] Mbps
        │
        ├── 🔋 Advanced Options
        │   ├── ☐ Continue on battery [Checkbox]
        │   └── Minimum battery to continue: [50]%
        │
        └── 📊 Statistics
            ├── Status: Idle - Not Contributing
            ├── Total Compute Time: 0h 0m
            ├── Tasks Completed: 0
            ├── Data Relayed: 0 MB
            ├── Credits Earned: 0
            ├── Reputation: 0 (New)
            └── [View Detailed Statistics]
```

### Usage Scenarios

#### Scenario 1: Office Worker (Default Settings)
```
Time: 9:00 AM - User arrives, starts CRYPTOGRAM
Status: Akashi disabled (user is active)

Time: 12:00 PM - User goes to lunch (15 min+)
Status: Akashi detects idle, starts contributing at 20% CPU

Time: 12:30 PM - User returns
Status: Akashi detects activity, stops contributing immediately

Time: 6:00 PM - User goes home, computer stays on
Status: After 15 min idle, Akashi contributes all evening
```

#### Scenario 2: Student on Laptop (Conservative)
```
Settings:
- CPU Level: Low (10%)
- Idle Detection: Conservative (30 min)
- Only when charging: ON
- Only when idle: ON

Behavior:
- Doesn't contribute during class (on battery)
- Starts contributing when plugged in at library (after 30 min idle)
- Stops immediately when student resumes work
- Minimal impact on battery and performance
```

#### Scenario 3: Privacy Enthusiast (Maximum)
```
Settings:
- CPU Level: Maximum (75%)
- Idle Detection: Aggressive (5 min)
- Only when charging: OFF
- Continue on battery: ON
- All contribution types: ON
- Storage: 50 GB

Behavior:
- Contributes aggressively whenever idle (5 min)
- Continues even on battery (unless < 50%)
- Provides maximum support to network
- Earns maximum credits/reputation
```

### Statistics & Rewards

#### Real-Time Statistics
```cpp
struct AkashiStatistics {
    qint64 totalComputeTime;      // "24h 15m contributed"
    qint64 totalTasksCompleted;   // "1,543 tasks completed"
    qint64 totalDataRelayed;      // "15.3 GB relayed"
    double averageCPUUsage;       // "Average: 18.5% CPU"

    int activeConnections;        // "Currently helping: 5 peers"
    int totalPeersHelped;         // "Total peers helped: 127"

    qint64 creditsEarned;         // "Credits: 15,430"
    int reputation;               // "Reputation: 87/100"
};
```

#### Credits System (Future Feature)
- Earn 1 credit per minute of CPU contribution
- Earn 10 credits per GB relayed
- Earn 50 credits per task completed
- Use credits for:
  - Priority message routing
  - Increased storage quota
  - Premium features
  - Unlock special themes/badges

#### Reputation System
- Reputation: 0-100 score
- Increases with consistent contribution
- Decreases if contribution is unreliable
- Benefits:
  - Higher reputation = higher credit multiplier
  - Trusted nodes get priority tasks
  - Community recognition

### Privacy & Security

#### What is Shared?
- ✅ Anonymous node ID (randomly generated)
- ✅ Available resources (CPU, bandwidth)
- ✅ Network statistics (uptime, reliability)

#### What is NOT Shared?
- ❌ Your real IP address (always uses Tor/I2P)
- ❌ Your CRYPTOGRAM identity
- ❌ Your messages or data
- ❌ Your contacts or activity
- ❌ Any personally identifiable information

#### Security Measures
- All contribution is anonymous
- Uses separate identity from main account
- Encrypted communication with Akashi network
- Can't be traced back to you
- Optional: Contribute only through Tor/I2P

### Technical Implementation

#### Idle Detection
```cpp
class AkashiNetwork {
private:
    // Platform-specific idle time detection
    qint64 getSystemIdleTime() {
        #ifdef Q_OS_WIN
        // Windows: GetLastInputInfo()
        LASTINPUTINFO lii;
        lii.cbSize = sizeof(LASTINPUTINFO);
        GetLastInputInfo(&lii);
        return (GetTickCount() - lii.dwTime) / 1000;

        #elif defined(Q_OS_LINUX)
        // Linux: X11 XScreenSaverQueryInfo
        Display *display = XOpenDisplay(NULL);
        XScreenSaverInfo *info = XScreenSaverAllocInfo();
        XScreenSaverQueryInfo(display, DefaultRootWindow(display), info);
        return info->idle / 1000;

        #elif defined(Q_OS_MAC)
        // macOS: CGEventSourceSecondsSinceLastEventType
        CFTimeInterval idleTime = CGEventSourceSecondsSinceLastEventType(
            kCGEventSourceStateHIDSystemState,
            kCGAnyInputEventType
        );
        return static_cast<qint64>(idleTime);
        #endif
    }
};
```

#### CPU Throttling
```cpp
void AkashiNetwork::adjustCPUUsage() {
    // Calculate target CPU usage based on level
    double targetCPU = static_cast<double>(_config.cpuLevel) / 100.0;

    // Measure current usage
    double currentCPU = getCurrentCPUUsage();

    // Adjust worker thread priority
    if (currentCPU > targetCPU) {
        // Reduce priority
        _workerThread->setPriority(QThread::LowPriority);
        // Add sleep delays between tasks
        _workerThread->msleep(100);
    } else if (currentCPU < targetCPU * 0.8) {
        // Increase priority (but not too high)
        _workerThread->setPriority(QThread::NormalPriority);
    }
}
```

### Example Configuration

#### Recommended Settings

**For Laptops** (battery-conscious):
```cpp
AkashiConfiguration config;
config.enabled = true;
config.cpuLevel = CPUContributionLevel::Low;      // 10%
config.idleLevel = IdleDetectionLevel::Conservative; // 30 min
config.onlyWhenCharging = true;                   // Only when plugged in
config.onlyWhenIdle = true;
config.enableRelay = true;
config.enableCompute = true;
config.enableStorage = false;                     // No storage on laptop
config.enableBandwidth = true;
config.maxBandwidthMbps = 5;                      // Lower bandwidth
```

**For Desktops** (always-on, more resources):
```cpp
AkashiConfiguration config;
config.enabled = true;
config.cpuLevel = CPUContributionLevel::Medium;   // 30%
config.idleLevel = IdleDetectionLevel::Moderate;  // 15 min
config.onlyWhenCharging = false;                  // Desktop (always plugged)
config.onlyWhenIdle = true;
config.enableRelay = true;
config.enableCompute = true;
config.enableStorage = true;                      // Enable storage
config.maxStorageGB = 20;                         // 20 GB storage
config.enableBandwidth = true;
config.maxBandwidthMbps = 50;                     // Higher bandwidth
```

**For Servers** (24/7 contribution):
```cpp
AkashiConfiguration config;
config.enabled = true;
config.cpuLevel = CPUContributionLevel::High;     // 50%
config.idleLevel = IdleDetectionLevel::Never;     // Always contribute
config.onlyWhenIdle = false;                      // Always contribute
config.enableRelay = true;
config.enableCompute = true;
config.enableStorage = true;
config.maxStorageGB = 500;                        // 500 GB storage
config.enableBandwidth = true;
config.maxBandwidthMbps = 1000;                   // 1 Gbps
```

---

## 🎯 Impact on CRYPTOGRAM

### Combined Network Strength

**With I2P + Akashi**:
- **Tor**: General anonymity, widely supported
- **I2P**: P2P anonymity, censorship-resistant
- **Akashi**: Community-powered infrastructure

**Result**: Unstoppable, censorship-proof, community-supported privacy network

### User Benefits

**Free Users**:
- Access to privacy infrastructure (Tor, I2P, bridges)
- Optional contribution (not required)
- Fair access to network resources

**Contributors**:
- Priority routing during high load
- Earn credits for future features
- Build reputation in community
- Help others fight censorship

### Community Impact

**Censorship Resistance**:
- More relay nodes = harder to block
- Distributed infrastructure = no central point
- Community resilience

**Performance**:
- More nodes = faster connections
- Better geographic distribution
- Reduced server costs

**Privacy**:
- Larger network = better anonymity
- More traffic = harder to analyze
- Collective privacy protection

---

## 📋 Files Created

### Headers
- `Telegram/SourceFiles/data/data_i2p_integration.h`
- `Telegram/SourceFiles/data/data_akashi_network.h`
- `Telegram/SourceFiles/settings/settings_cryptogram.h`

### Documentation
- `I2P_AND_AKASHI_FEATURES.md` (this file)

### To Be Implemented
- `data_i2p_integration.cpp` (I2P SAM protocol implementation)
- `data_akashi_network.cpp` (Akashi contribution system)
- `settings_cryptogram.cpp` (Settings UI implementation)

---

## 🚀 Deployment Plan

### Phase 1: I2P Integration (Week 1)
- Implement SAM protocol communication
- Add tunnel management
- Create settings UI
- Test with I2P router
- Documentation and help screens

### Phase 2: Akashi Network (Week 2)
- Implement idle detection
- Create CPU throttling system
- Build network protocol
- Add statistics tracking
- Create settings UI

### Phase 3: Integration (Week 3)
- Combine I2P + Akashi
- Add to main settings menu
- User testing
- Performance optimization
- Security audit

### Phase 4: Launch (Week 4)
- Public beta release
- Community feedback
- Documentation
- Promotional materials
- Network monitoring

---

## 📖 User Documentation

### Quick Start Guide

**To Enable I2P**:
1. Install I2P router (https://geti2p.net)
2. Open CRYPTOGRAM
3. Go to: Settings → CRYPTOGRAM → Network Anonymity → I2P
4. Toggle "Enable I2P"
5. Click "Auto-start with CRYPTOGRAM"
6. Done! Your connections now use I2P

**To Enable Akashi Contribution**:
1. Open CRYPTOGRAM
2. Go to: Settings → CRYPTOGRAM → Akashi Network
3. Toggle "Enable Contribution"
4. Adjust CPU level if desired (default: 20%)
5. Done! You're helping the community

**That's it!** Everything is automatic and privacy-preserving.

---

**CRYPTOGRAM - Community-Powered Privacy for Everyone** 🔐🤝
