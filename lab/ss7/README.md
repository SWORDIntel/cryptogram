# CRYPTOGRAM SS7 Lab – Mini Telco Universe

<p align="center">
  <strong>Multi-PLMN SS7 Security Testing Environment</strong><br>
  100% Lab-Contained • Graduated Complexity • Real-World Attack Scenarios
</p>

---

## 🎯 Mission

Build a **realistic, scalable SS7 testing lab** that goes beyond simple 3-node setups to provide:
- **Multiple PLMNs** (Home, Visited, Rogue operators)
- **IPX/Hub infrastructure** (multi-hop routing, redundancy)
- **Threat simulation** (multiple attackers, compromised nodes)
- **Graduated complexity** (scale from 5 to 15+ nodes via Docker profiles)

Perfect for:
- 🔴 **Red Team**: SS7 penetration testing and attack development
- 🔵 **Blue Team**: Defense testing, firewall validation, honeypot operations
- 🎓 **Training**: Realistic CTF scenarios and SOC exercises
- 🔬 **Research**: SS7 vulnerability analysis and threat intelligence

**Classification**: UNCLASSIFIED // LAB USE ONLY

---

## 🚀 Quick Start

### Prerequisites

```bash
# Docker Engine 24.0+ with Compose V2
docker --version  # Requires 24.0.0+
docker compose version  # Requires v2.20.0+

# Recommended: 16GB RAM for full profile (8GB for base)
```

### 30-Second Lab

```bash
# Clone and navigate
git clone https://github.com/SWORDOps/CRYPTOGRAM.git
cd CRYPTOGRAM/lab/ss7

# Start minimal lab (5 nodes)
docker compose --profile base up -d

# Access console
docker compose exec ss7_console bash

# Run test attack
./scenarios/01-basic-location-tracking.sh

# View results
docker compose logs hlr_home | tail -20

# Cleanup
docker compose --profile base down -v
```

---

## 📐 Architecture

### Complexity Profiles

Scale lab complexity with Docker Compose profiles:

| Profile | Nodes | Use Case | Complexity |
|---------|-------|----------|------------|
| **base** | 5 | Single-hop attacks, basic testing | ⭐ |
| **roam** | 9 | Multi-PLMN roaming attacks | ⭐⭐ |
| **threat** | 12 | Multi-attacker, rogue operators | ⭐⭐⭐ |
| **full** | 15+ | STP failover, redundancy, all features | ⭐⭐⭐⭐ |

### Topology Overview

```text
         ┌─────────────────────────────────────────────┐
         │        MINI TELCO UNIVERSE                  │
         │                                             │
         │  ┌──────────┐     ┌──────────┐             │
         │  │ HOME     │◄───►│   IPX    │             │
         │  │ PLMN     │     │   HUB    │             │
         │  │          │     │          │             │
         │  │ STP×2    │     │ STP×2    │◄───┐        │
         │  │ HLR MSC  │     │ Firewall │    │        │
         │  │ SMSC     │     └──────────┘    │        │
         │  └──────────┘            ▲        │        │
         │                          │        │        │
         │  ┌──────────┐            │   ┌────┴─────┐  │
         │  │ VISITED  │◄───────────┘   │ THREAT   │  │
         │  │ PLMN     │                │ LAB      │  │
         │  │          │                │          │  │
         │  │ STP      │                │Attacker×2│  │
         │  │ HLR MSC  │                │Rogue Ops │  │
         │  │ SMSC     │                └──────────┘  │
         │  └──────────┘                              │
         │                                            │
         │  [ CONSOLE - Logs, Metrics, Wireshark ]   │
         └─────────────────────────────────────────────┘
```

**Isolated Docker Networks**:
- `net_home` – Home PLMN internal
- `net_visit` – Visited PLMN internal
- `net_ipx` – IPX/Hub interconnect
- `net_threat` – Threat lab cluster
- `net_mgmt` – Management overlay

---

## 🔧 Usage

### Start Lab Environment

```bash
# Minimal (base profile)
docker compose --profile base up -d

# With roaming support
docker compose --profile roam up -d

# Full complexity (all nodes)
docker compose --profile full up -d

# Check status
docker compose ps
```

### Access Console

```bash
# Interactive shell
docker compose exec ss7_console bash

# View all logs
tail -f /logs/*.log

# Analyze with Wireshark
tshark -i eth0 -f "sctp"

# Generate threat report
/scripts/threat_report.py
```

### Run Attack Scenarios

```bash
# Basic location tracking
./scenarios/01-basic-location-tracking.sh

# Cross-PLMN IMSI catcher
./scenarios/02-roaming-imsi-catch.sh

# STP failover exploit (requires 'full' profile)
./scenarios/04-stp-failover-exploit.sh

# See all scenarios
ls scenarios/*.sh
cat scenarios/README.md
```

### Direct Node Access

```bash
# Access attacker node
docker compose exec ss7_attacker_1 bash

# Access HLR
docker compose exec hlr_home bash

# View HLR subscriber database
docker compose exec hlr_home sqlite3 /data/hlr.db "SELECT * FROM subscribers;"

# Check STP routing
docker compose exec stp_home_1 cat /config/routes_home.cfg
```

---

## 📊 Node Inventory

### Home PLMN (`net_home`)

| Node | PC | Role | Profiles |
|------|----|----|----------|
| `stp_home_1` | 0.1.1 | Primary STP | base, roam, threat, full |
| `stp_home_2` | 0.1.2 | Secondary STP (redundant) | full |
| `hlr_home` | 0.1.10 | HLR/HSS | all |
| `msc_home` | 0.1.20 | MSC/VLR | roam, full |
| `smsc_home` | 0.1.30 | SMSC | roam, full |
| `gmsc_home` | 0.1.40 | Gateway MSC | full |

### Visited PLMN (`net_visit`)

| Node | PC | Role | Profiles |
|------|----|----|----------|
| `stp_visit` | 0.2.1 | STP | roam, full |
| `hlr_visit` | 0.2.10 | HLR/HSS | roam, full |
| `msc_visit` | 0.2.20 | MSC/VLR | roam, full |
| `smsc_visit` | 0.2.30 | SMSC | roam, full |

### IPX/Hub (`net_ipx`)

| Node | PC | Role | Profiles |
|------|----|----|----------|
| `stp_ipx_1` | 0.9.1 | Hub STP | base, roam, threat, full |
| `stp_ipx_2` | 0.9.2 | Hub STP (redundant) | full |
| `ss7_firewall_ipx` | 0.9.50 | Firewall/IDS | full |

### Threat Lab (`net_threat`)

| Node | PC | Role | Profiles |
|------|----|----|----------|
| `ss7_attacker_1` | 0.8.1 | Primary attacker | all |
| `ss7_attacker_2` | 0.8.2 | Compromised SMSC sim | threat, full |
| `rogue_operator_core` | 0.8.10 | Fake PLMN | threat, full |

### Management (`net_mgmt`)

| Node | Role | Profiles |
|------|------|----------|
| `ss7_console` | Central monitoring | all |

---

## 🎭 Attack Scenarios

Pre-built scenarios in `scenarios/`:

1. **Basic Location Tracking** (`base`) – Simple MAP-ATI query
2. **Roaming IMSI Catcher** (`roam`) – Cross-PLMN subscriber tracking
3. **SMS Intercept MITM** (`roam`) – Message interception
4. **STP Failover Exploit** (`full`) – Config drift exploitation
5. **Rogue Operator Fraud** (`threat`) – Fake PLMN attacks
6. **Multi-Hop Path Confusion** (`full`) – Routing manipulation
7. **Diameter-SS7 Bridge** (`full`) – 4G/LTE attack via SS7 gateway

Each scenario includes:
- Attack flow diagram
- Detection challenges
- Mitigation recommendations
- Log analysis examples

See [`scenarios/README.md`](scenarios/README.md) for details.

---

## 🛡️ Defense Testing

### Firewall Validation

```bash
# Test STP firewall rules
docker compose exec stp_home_1 cat /config/firewall_home.cfg

# Send blocked attack
./scenarios/test_firewall_block.sh

# Verify block in logs
docker compose logs stp_home_1 | grep BLOCK
```

### Honeypot Operations

HLR/MSC nodes run in honeypot mode by default:
- Accept all queries (no rejection)
- Return plausible fake data
- Log attacker behavior for analysis

```bash
# Check honeypot responses
docker compose logs hlr_home | grep HONEYPOT

# View fake subscriber generation
docker compose logs hlr_home | grep "Generated fake subscriber"
```

### Anomaly Detection

```bash
# Analyze traffic patterns
docker compose exec ss7_console /scripts/analyze_logs.py /logs/*.log

# Detect rate anomalies
docker compose exec ss7_console /scripts/detect_anomalies.py --threshold 10

# Generate ML training data
docker compose exec ss7_console /scripts/export_features.py --format csv
```

---

## 🔍 Monitoring & Logging

### Log Files

All logs in `/logs/` (accessible from console):

```bash
docker compose exec ss7_console ls -lh /logs/

# Node logs
/logs/stp_home_1.log
/logs/hlr_home.log
/logs/attacker_1.log

# Security alerts
/logs/firewall_ipx_alerts.json
```

### Packet Captures

SCTP traffic captures in `/pcaps/` (enable with `ENABLE_PCAP=true`):

```bash
# Analyze STP traffic
docker compose exec ss7_console tshark -r /pcaps/stp_home_1.pcap -Y "m3ua"

# Filter MAP messages
tshark -r /pcaps/stp_home_1.pcap -Y "map"
```

### Real-Time Monitoring

```bash
# Follow all logs
docker compose logs -f

# Follow specific node
docker compose logs -f hlr_home

# Follow multiple nodes
docker compose logs -f stp_home_1 stp_ipx_1 hlr_home
```

---

## 🧪 Testing & Validation

### Smoke Tests

```bash
# Connectivity test
docker compose exec ss7_console /scripts/test_connectivity.sh

# Check SCTP associations
docker compose exec stp_home_1 ss -an | grep :2905

# Verify subscriber database
docker compose exec hlr_home sqlite3 /data/hlr.db "SELECT COUNT(*) FROM subscribers;"
```

### Complexity Validation

```bash
# Test multi-hop routing (full profile)
docker compose --profile full up -d
./scenarios/06-multi-hop-path-confusion.sh

# Test STP failover
docker compose stop stp_home_1
./scenarios/test_failover.sh
docker compose start stp_home_1

# Test direct bypass link (misconfig)
echo "ENABLE_DIRECT_BYPASS=true" >> .env
docker compose up -d
./scenarios/test_bypass.sh
```

---

## 🔐 Security & Isolation

### Lab-Only Guarantees

- ✅ **No host network mode** – All nodes use bridged Docker networks
- ✅ **No port exposure** – Unless explicitly configured for external test peer
- ✅ **Read-only configs** – Mounted RO where possible
- ✅ **Dropped capabilities** – Minimal Linux caps (NET_RAW, NET_ADMIN only where needed)
- ✅ **Resource limits** – CPU/memory quotas prevent DoS

### External Peer Testing (REQUIRES AUTHORIZATION)

**⚠️ WARNING**: Only enable with documented, signed authorization

```yaml
# In docker-compose.yml, uncomment:
# ports:
#   - "2905:2905/sctp"  # Expose STP port

# Set environment flag
EXTERNAL_PEER_ALLOWED=true docker compose up -d

# Document authorization
cp authorization_template.md docs/authorizations/$(date +%Y%m%d)_authorization.md
# Fill in: Authorized by, Test peer details, Scope, Duration
```

---

## 📚 Documentation

- [`docs/ss7-lab/ARCHITECTURE.md`](../../docs/ss7-lab/ARCHITECTURE.md) – Detailed architecture and design
- [`scenarios/README.md`](scenarios/README.md) – Attack scenario catalog
- [`config/README.md`](config/README.md) – Configuration reference
- [Docker Compose Reference](docker-compose.yml) – Full service definitions

### External Resources

**Standards**:
- [ITU-T Q.700-series](https://www.itu.int/rec/T-REC-Q.700) – SS7 MTP/SCCP
- [3GPP TS 29.002](https://www.3gpp.org/DynaReport/29002.htm) – MAP Protocol
- [GSMA IR.82](https://www.gsma.com/newsroom/wp-content/uploads/IR.82-v2.0.pdf) – SS7 Security

**Tools**:
- [SigPloit](https://github.com/SigPloiter/SigPloit) – SS7 pentesting framework
- [Osmocom](https://osmocom.org/) – Open-source SS7/GSM stack
- [Wireshark](https://www.wireshark.org/) – SCTP/M3UA/MAP dissection

**Research**:
- [Positive Technologies SS7 Reports](https://www.ptsecurity.com/ww-en/analytics/ss7-vulnerability/)
- [GSMA FASG Publications](https://www.gsma.com/security/fraud-security-group/)

---

## 🛠️ Development

### Building Custom Images

```bash
# Build all node images
docker compose build

# Build specific node
docker compose build stp_home_1

# Build with no cache
docker compose build --no-cache
```

### Adding New Nodes

1. Create `nodes/<type>/Dockerfile`
2. Add service to `docker-compose.yml`
3. Set appropriate profile
4. Configure networks
5. Test with `docker compose --profile <profile> up`

### Customizing Configs

```bash
# Edit STP routes
vim config/stp/routes_home.cfg

# Edit firewall rules
vim config/stp/firewall_home.cfg

# Restart to apply
docker compose restart stp_home_1
```

---

## 🤝 Contributing

Issues and PRs welcome! See [CONTRIBUTING.md](../../CONTRIBUTING.md).

**Areas for Contribution**:
- Additional attack scenarios
- Defense mechanism implementations
- Diameter/5G integration
- ML-based anomaly detection
- CI/CD integration

---

## 📜 License

Part of the [CRYPTOGRAM](https://github.com/SWORDOps/CRYPTOGRAM) project.

License: [See main repo LICENSE](../../LICENSE)

---

## ⚠️ Legal Notice

**AUTHORIZED USE ONLY**

This lab is for:
- ✅ Authorized security research
- ✅ Defensive security development
- ✅ CTF competitions and training
- ✅ Pentesting with explicit written authorization

**DO NOT**:
- ❌ Attack production networks without authorization
- ❌ Use for malicious purposes
- ❌ Connect to real carrier infrastructure without documented consent
- ❌ Deploy honeypots on public internet without legal review

**OPSEC**: Treat all lab traffic as if it were real. Practice proper authorization workflows even in lab environments.

---

## 📞 Support

- **Issues**: https://github.com/SWORDOps/CRYPTOGRAM/issues
- **Discussions**: https://github.com/SWORDOps/CRYPTOGRAM/discussions
- **Docs**: `docs/ss7-lab/`

---

<p align="center">
  <i>"In a world where signaling is insecure by design,<br>
  the only way to learn defense is to practice offense –<br>
  but do it ethically, in a sandbox,<br>
  where the only casualties are Docker containers."</i>
</p>

<p align="center">
  <strong>Built with ⚔️ by SWORD Ops</strong>
</p>
