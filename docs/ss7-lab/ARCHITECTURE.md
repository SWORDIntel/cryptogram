# SS7 Lab Architecture – "Mini Telco Universe"

## Mission Brief

100% lab-contained, multi-PLMN SS7 topology for MILTOP-grade threat research, honeypot operations, and defense testing. Provides real-world complexity (roaming, multi-hop routing, redundant STPs, multiple SMSCs) without touching production networks.

**Classification**: UNCLASSIFIED // LAB USE ONLY
**Last Updated**: 2025-12-02

---

## Design Philosophy

### Core Principles

1. **Graduated Complexity** – Scale from simple (3 nodes) to full telco mesh (14+ nodes) via Docker Compose profiles
2. **Multi-Domain Isolation** – Separate Docker networks mimic real PLMN boundaries (Home, Visited, IPX, Threat)
3. **Realistic Routing** – Multi-hop SS7 message flows, redundant paths, configurable misrouting
4. **Zero External Dependency** – Nothing talks to real carriers unless explicitly exposed and authorized
5. **Observable & Controllable** – Central console with full visibility, per-node config knobs

### Use Cases

- **Threat Research**: Test SS7 attacks (location tracking, SMS interception, fraud) in controlled environment
- **Defense Development**: Build and validate SS7 firewalls, anomaly detection, honeypot deception
- **Training**: Realistic scenarios for SOC/IR teams without real-world risk
- **CTF/Red Team**: Complex, multi-hop attack surfaces for wargaming

---

## Topology Overview

### Logical Domains

```text
┌─────────────────────────────────────────────────────────────────┐
│                     MINI TELCO UNIVERSE                         │
│                                                                 │
│  ┌──────────────┐      ┌──────────────┐      ┌──────────────┐ │
│  │  HOME PLMN   │      │   IPX/HUB    │      │ VISITED PLMN │ │
│  │              │      │              │      │              │ │
│  │ ┌──────────┐ │      │ ┌──────────┐ │      │ ┌──────────┐ │ │
│  │ │STP-H1/H2 │←─────→│ │STP-IPX1/2│←─────→│ │ STP-V    │ │ │
│  │ └────┬─────┘ │      │ └────┬─────┘ │      │ └────┬─────┘ │ │
│  │      │       │      │      │       │      │      │       │ │
│  │ ┌────┴──────┐│      │ ┌────┴──────┐│      │ ┌────┴──────┐│ │
│  │ │HLR MSC    ││      │ │ Firewall  ││      │ │HLR MSC    ││ │
│  │ │SMSC GMSC  ││      │ │ Honeypot  ││      │ │SMSC       ││ │
│  │ └───────────┘│      │ └───────────┘│      │ └───────────┘│ │
│  └──────────────┘      └──────┬───────┘      └──────────────┘ │
│                               │                                │
│                        ┌──────┴───────┐                        │
│                        │ THREAT LAB   │                        │
│                        │              │                        │
│                        │ ┌──────────┐ │                        │
│                        │ │Attacker1 │ │                        │
│                        │ │Attacker2 │ │                        │
│                        │ │Rogue Ops │ │                        │
│                        │ └──────────┘ │                        │
│                        └──────────────┘                        │
│                                                                 │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │             CONSOLE (Logs, Metrics, Control)            │   │
│  └─────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────┘
```

### Node Inventory

| Domain | Node | Role | PC | GT Range | Notes |
|--------|------|------|----|----|-------|
| **Home PLMN** | `stp_home_1` | Primary STP | 0.1.1 | - | Redundant pair |
| | `stp_home_2` | Secondary STP | 0.1.2 | - | Failover + misconfig testing |
| | `hlr_home` | HLR/HSS | 0.1.10 | +441... | Subscriber database |
| | `msc_home` | MSC/VLR | 0.1.20 | +441... | Handles MAP ops |
| | `smsc_home` | SMSC | 0.1.30 | +441... | SMS center |
| | `gmsc_home` | GMSC | 0.1.40 | +441... | Gateway MSC (optional) |
| **Visited PLMN** | `stp_visit` | STP | 0.2.1 | - | Foreign network |
| | `hlr_visit` | HLR/HSS | 0.2.10 | +338... | French dummy MCC |
| | `msc_visit` | MSC/VLR | 0.2.20 | +338... | |
| | `smsc_visit` | SMSC | 0.2.30 | +338... | |
| **IPX Hub** | `stp_ipx_1` | Hub STP | 0.9.1 | - | Primary interconnect |
| | `stp_ipx_2` | Hub STP | 0.9.2 | - | Redundant |
| | `ss7_firewall_ipx` | Firewall/IDS | 0.9.50 | - | Hub-level screening |
| **Threat Lab** | `ss7_attacker_1` | Attacker A | 0.8.1 | +99701... | SigPloit, ss7MAPer |
| | `ss7_attacker_2` | Attacker B | 0.8.2 | +99702... | Compromised SMSC sim |
| | `rogue_operator_core` | Rogue PLMN | 0.8.10 | +99700... | Fake MVNO (optional) |
| **Management** | `ss7_console` | Console | - | - | Logs, metrics, Wireshark |

---

## Docker Networks

Each domain gets isolated Docker network(s), with STPs acting as bridges:

```yaml
networks:
  net_home:     # Home PLMN internal
  net_visit:    # Visited PLMN internal
  net_ipx:      # IPX/Hub interconnect
  net_threat:   # Threat Lab cluster
  net_mgmt:     # Management/console overlay
```

**Interconnect Model:**

- `stp_home_1/2`: Connected to `net_home` + `net_ipx`
- `stp_visit`: Connected to `net_visit` + `net_ipx`
- `stp_ipx_1/2`: Connected to `net_ipx` only (hub)
- Attackers: Connected to `net_threat` + `net_ipx`
- Console: Connected to `net_mgmt` + volume mounts for logs

**Optional Misconfig Links:**

- Direct `net_home` ↔ `net_visit` bypass (simulates bilateral link that skips IPX screening)
- Enable via environment variable: `ENABLE_DIRECT_BYPASS=true`

---

## Complexity Profiles

Use Docker Compose `profiles` to dial up/down complexity:

### `base` Profile (Minimal Lab)

**Nodes**: 5 total
- 1× Home STP (`stp_home_1`)
- 1× Home HLR (`hlr_home`)
- 1× IPX STP (`stp_ipx_1`)
- 1× Attacker (`ss7_attacker_1`)
- 1× Console (`ss7_console`)

**Use**: Basic SS7 attack/defense testing, single-hop routing

```bash
docker compose --profile base up
```

### `roam` Profile (Roaming Scenarios)

**Adds**: Visited PLMN stack
- `stp_visit`, `hlr_visit`, `msc_visit`, `smsc_visit`

**Use**: Multi-PLMN roaming attacks (location tracking across borders, SMS home routing)

```bash
docker compose --profile roam up
```

### `threat` Profile (Advanced Threat Modeling)

**Adds**: Additional attackers and rogue infrastructure
- `ss7_attacker_2` (compromised SMSC)
- `rogue_operator_core` (fake PLMN)

**Use**: Multi-vector attacks, attacker coordination, rogue operator scenarios

```bash
docker compose --profile threat up
```

### `full` Profile (Complete Telco Universe)

**Adds**: All redundancy and advanced features
- `stp_home_2` (redundant STP)
- `stp_ipx_2` (redundant hub)
- `gmsc_home`, `ss7_firewall_ipx`
- Direct bypass links (configurable)

**Use**: Full complexity – STP failover, multi-hop routing, path ambiguity, hub screening bypass

```bash
docker compose --profile full up
```

---

## Message Flows

### 1. Single-Hop Attack (Base Profile)

```text
[Attacker] → [STP-IPX] → [STP-Home] → [HLR-Home]
   MAP-ATI request for +441234567890

Routing: GT translation at each STP
Screening: STP-Home firewall (if configured)
```

### 2. Roaming Location Query (Roam Profile)

```text
[Attacker posing as V-PLMN]
    → [STP-IPX]
    → [STP-Home]
    → [HLR-Home]
    ← MAP-PSI response with IMSI/location

Attacker OPC: 0.8.1 (looks like foreign operator)
Legit path: V-PLMN would query via same route for roaming subscriber
Defense challenge: Distinguish malicious vs. legit roaming query
```

### 3. Multi-Hop SMS Interception (Full Profile)

```text
[V-SMSC]
    → [STP-Visit]
    → [STP-IPX-1]
    → [STP-Home-1]
    → [GMSC-Home]
    → [HLR-Home query]
    → [SMSC-Home]

Attack vectors:
- Attacker compromises STP-IPX-1 GT routing
- Rogue operator inserts itself between IPX-1 and Home
- Direct bypass link enabled (V → Home), skips IPX screening
```

### 4. STP Failover Test (Full Profile)

```text
Normal: [Attacker] → [STP-IPX-1] → [STP-Home-1] → [HLR]

Kill STP-Home-1:
Alternative: [Attacker] → [STP-IPX-1] → [STP-Home-2] → [HLR]

Misconfiguration scenario:
- STP-Home-1 has firewall rules
- STP-Home-2 has different (or no) rules
- Attacker exploits failover to bypass screening
```

---

## Configuration Knobs

Each node has environment variables for runtime config:

### STP Nodes

```yaml
environment:
  - PC=0.1.1                    # Point code
  - SSN=8                       # Default SSN
  - SCTP_PEERS=stp_ipx_1:2905   # Comma-separated peer list
  - GT_ROUTES=/config/routes.cfg # GT routing table
  - FIREWALL_RULES=/config/fw.cfg # Optional
  - LOG_LEVEL=DEBUG
```

### HLR/MSC Nodes

```yaml
environment:
  - PC=0.1.10
  - GT=441234567890             # Base GT
  - MCC=234                     # UK
  - MNC=15                      # Dummy operator
  - SUBSCRIBER_DB=/data/hlr.db  # SQLite with test subs
  - HONEYPOT_MODE=true          # Log all requests, fake responses
```

### Attacker Nodes

```yaml
environment:
  - OPC=0.8.1                   # Own PC
  - DPC=0.1.10                  # Target (HLR-Home)
  - OGT=99701123456             # Own GT
  - TARGET_IMSI=234150000000001 # Test target
  - TOOLS=sigploit,ss7maper     # Preload attack tools
```

### Console

```yaml
volumes:
  - ./logs:/logs:ro             # All nodes log here
  - ./pcaps:/pcaps:ro           # SCTP captures
  - ./dashboards:/dashboards    # Grafana/Kibana configs
environment:
  - ENABLE_WIRESHARK=true
  - METRICS_SCRAPE_INTERVAL=5s
```

---

## Security Guardrails

### Isolation Enforcement

1. **No host network mode** – All nodes use bridged Docker networks
2. **No port exposure** – Unless explicitly needed for external test peer (with consent)
3. **Read-only mounts** – Config files mounted RO where possible
4. **Drop capabilities** – Containers run with minimal Linux caps
5. **Resource limits** – CPU/memory quotas to prevent DoS within lab

### Authorized Testing Only

- **Default config**: All nodes accept only internal Docker IPs
- **External peer testing**: Requires:
  - Explicit `EXTERNAL_PEER_ALLOWED=true` flag
  - Signed authorization document in `/docs/authorizations/`
  - Port exposure limited to single STP with strict firewall rules
  - Logging of all external traffic to immutable audit log

### Honeypot Mode

HLR/MSC nodes can run in honeypot mode:
- Accept all queries (no rejection)
- Return plausible fake data (generated IMSI, dummy locations)
- Log attacker behavior for analysis
- Never expose real subscriber data (even in lab, use synthetic data only)

---

## Deployment

### Prerequisites

```bash
# Docker Engine 24.0+ with Compose V2
docker --version  # Docker version 24.0.0+
docker compose version  # Docker Compose version v2.20.0+

# 16GB+ RAM recommended for full profile
# 8GB sufficient for base/roam profiles
```

### Quickstart

```bash
# Clone repo
git clone https://github.com/SWORDOps/CRYPTOGRAM.git
cd CRYPTOGRAM/lab/ss7

# Start with base profile (minimal)
docker compose --profile base up -d

# Verify nodes
docker compose ps

# Access console
docker compose exec ss7_console bash
# Inside console:
$ wireshark -i eth0 -f "sctp"  # Live capture
$ tail -f /logs/stp_home_1.log # View STP logs

# Run simple attack test
$ cd /opt/sigploit
$ python3 sigploit.py --target hlr_home --attack ATI --msisdn 441234567890

# Check if HLR logged the attack
docker compose logs hlr_home | grep ATI

# Cleanup
docker compose --profile base down -v
```

### Graduated Deployment

```bash
# 1. Base lab (single-hop)
docker compose --profile base up -d
# Test basic attack flow, validate logging

# 2. Add roaming (multi-PLMN)
docker compose --profile roam up -d
# Test cross-PLMN queries, roaming scenarios

# 3. Add threat actors (multi-attacker)
docker compose --profile threat up -d
# Test coordinated attacks, rogue operator

# 4. Full complexity (everything)
docker compose --profile full up -d
# Test STP failover, multi-hop routing, bypass paths
```

---

## Verification & Testing

### Smoke Tests

**Test 1: Connectivity**
```bash
# From console, ping all nodes
docker compose exec ss7_console bash -c '
  for node in stp_home_1 hlr_home stp_ipx_1 ss7_attacker_1; do
    echo -n "Testing $node... "
    ping -c 1 -W 1 $node > /dev/null && echo "OK" || echo "FAIL"
  done
'
```

**Test 2: SCTP Associations**
```bash
# Check STP-Home has SCTP links to IPX
docker compose exec stp_home_1 bash -c '
  ss -an | grep :2905 | grep ESTAB
'
# Should show established SCTP connections
```

**Test 3: GT Routing**
```bash
# From attacker, send test MAP message
docker compose exec ss7_attacker_1 python3 << 'EOF'
from scapy.all import *
from scapy.layers.sigtran import *

# Simple MAP-ATI to HLR-Home GT
pkt = M3UA()/MTP3()/TCAP()/MAP_ATI(msisdn="441234567890")
send(pkt, iface="eth0")
EOF

# Check HLR received it
docker compose logs hlr_home --tail 20 | grep ATI
```

**Test 4: Honeypot Response**
```bash
# HLR in honeypot mode should return fake data
# Check logs for "HONEYPOT: Returning synthetic response"
docker compose logs hlr_home | grep HONEYPOT
```

### Complexity Validation

**Test 5: Multi-Hop Tracing**
```bash
# Enable debug logging on all STPs
docker compose exec stp_ipx_1 bash -c 'echo "log-level DEBUG" > /tmp/reload'

# Send attack from Attacker → IPX → Home → HLR
# Trace in logs:
docker compose logs -f --tail 0 stp_ipx_1 stp_home_1 hlr_home | grep -E '(ROUTING|M3UA)'

# Should see message hop through multiple STPs
```

**Test 6: STP Failover** (Full profile only)
```bash
# Kill primary STP
docker compose stop stp_home_1

# Send attack (should auto-route to stp_home_2)
docker compose exec ss7_attacker_1 /opt/test_attack.sh

# Verify traffic went to backup
docker compose logs stp_home_2 | grep -c "Routed to HLR"  # Should be > 0
docker compose logs stp_home_1 | grep -c "Routed to HLR"  # Should be 0 (stopped)

# Restart and verify failback
docker compose start stp_home_1
```

**Test 7: Direct Bypass Path** (Full profile + misconfig)
```bash
# Enable bypass link
echo "ENABLE_DIRECT_BYPASS=true" >> .env
docker compose up -d

# Attacker traffic should have TWO possible paths:
#   1. Attacker → IPX → Home (normal)
#   2. Attacker → Home (direct bypass)

# Misconfigure bypass to skip firewall
docker compose exec stp_home_2 bash -c '
  echo "BYPASS_FIREWALL=true" >> /config/routes.cfg
'

# Attack should succeed even if stp_home_1 firewall would block
docker compose exec ss7_attacker_1 /opt/test_blocked_attack.sh
docker compose logs hlr_home | grep "Attack succeeded via bypass"
```

---

## Scenario Library

See `/lab/ss7/scenarios/` for pre-built test scenarios:

1. **`01-basic-location-tracking.sh`** – Simple MAP-ATI attack (base profile)
2. **`02-roaming-imsi-catch.sh`** – Cross-PLMN IMSI catcher (roam profile)
3. **`03-sms-intercept-mitm.sh`** – SMS forwarding attack via hub compromise
4. **`04-stp-failover-exploit.sh`** – Abuse failover to bypass firewall (full profile)
5. **`05-rogue-operator-fraud.sh`** – Fake PLMN attacks real operators (threat profile)
6. **`06-multi-hop-path-confusion.sh`** – Ambiguous routing to evade detection
7. **`07-diameter-ss7-bridge.sh`** – Attack 4G/5G via legacy SS7 gateway (advanced)

Each scenario includes:
- Topology requirements (which profile)
- Attack steps
- Expected detection signatures
- Defense mitigations

---

## Roadmap

### Phase 1: Foundation (Current)
- [x] Multi-PLMN topology design
- [ ] Docker Compose with profiles
- [ ] Basic STP/HLR/MSC configs
- [ ] Console with Wireshark

### Phase 2: Advanced Nodes
- [ ] Real SMSC (Kannel/Jasmin integration)
- [ ] SS7 firewall with signature detection
- [ ] Honeypot with ML-based attacker profiling
- [ ] GMSC with inter-PLMN routing

### Phase 3: Automation
- [ ] Scenario orchestration engine
- [ ] Automated red team vs. blue team wargames
- [ ] CI/CD integration for defense testing
- [ ] Grafana dashboards for real-time metrics

### Phase 4: Hybrid Topologies
- [ ] Diameter/SG interfaces (4G/LTE)
- [ ] 5G SBA with N7 (SEPP) attacks
- [ ] IoT NB-IoT / LTE-M attack surfaces
- [ ] Satellite SS7 (Iridium/Thuraya sim)

---

## References

### Standards
- **ITU-T Q.700-series**: SS7 MTP/SCUP specs
- **3GPP TS 29.002**: MAP protocol
- **GSMA IR.82**: SS7 security guidelines
- **ETSI TS 129 002**: MAP for GSM/UMTS

### Tools
- **SigPloit**: SS7 pentesting framework
- **ss7MAPer**: MAP fuzzing toolkit
- **Osmocom**: Open-source SS7/GSM stack (basis for many nodes)
- **Wireshark**: SCTP/M3UA/MAP dissection

### Threat Intel
- **Positive Technologies**: SS7 vulnerability research
- **AdaptiveMobile**: Signaling security reports
- **GSMA FASG**: Fraud & Security Group publications

---

## Support & Contact

- **Issues**: https://github.com/SWORDOps/CRYPTOGRAM/issues
- **Docs**: `/docs/ss7-lab/`
- **Scenarios**: `/lab/ss7/scenarios/`

**OPSEC NOTE**: This lab is for authorized research only. Do not connect to production networks without explicit written authorization and legal review.

---

*"In a world where signaling is insecure by design, the only way to learn defense is to practice offense – but do it ethically, in a sandbox, where the only casualties are Docker containers."*

**END ARCHITECTURE BRIEF**
