# SS7 Lab Attack Scenarios

Pre-built attack scenarios for testing SS7 defenses in the lab.

## Scenario Index

| # | Name | Profile | Complexity | Description |
|---|------|---------|------------|-------------|
| 01 | Basic Location Tracking | `base` | ⭐ | Simple MAP-ATI query to HLR |
| 02 | Roaming IMSI Catcher | `roam` | ⭐⭐ | Cross-PLMN subscriber info gathering |
| 03 | SMS Intercept MITM | `roam` | ⭐⭐⭐ | SMS forwarding via hub compromise |
| 04 | STP Failover Exploit | `full` | ⭐⭐⭐ | Abuse config drift in redundant STPs |
| 05 | Rogue Operator Fraud | `threat` | ⭐⭐⭐⭐ | Fake PLMN attacks real operators |
| 06 | Multi-Hop Path Confusion | `full` | ⭐⭐⭐⭐ | Routing ambiguity for detection evasion |
| 07 | Diameter-SS7 Bridge | `full` | ⭐⭐⭐⭐⭐ | Attack 4G via legacy SS7 gateway |

## Running Scenarios

### Prerequisites

```bash
# Start lab with appropriate profile
cd /home/user/CRYPTOGRAM/lab/ss7

# For scenarios 01-03
docker compose --profile base up -d    # Scenario 01
docker compose --profile roam up -d    # Scenarios 02-03

# For advanced scenarios
docker compose --profile full up -d    # Scenarios 04, 06-07
docker compose --profile threat up -d  # Scenario 05
```

### Execute a Scenario

```bash
./scenarios/01-basic-location-tracking.sh
```

Each scenario script:
- Checks required nodes are running
- Explains the attack flow
- Executes the attack commands
- Shows expected logs/results
- Provides detection/mitigation guidance

## Scenario Details

### 01: Basic Location Tracking
**Profile**: `base`
**Attack**: MAP-AnyTimeInterrogation
**Goal**: Obtain subscriber location (LAC/CellID)

**Defense Test Points**:
- STP firewall blocks suspicious OPCs
- HLR honeypot returns fake location
- Anomaly detection flags location query rate

---

### 02: Roaming IMSI Catcher
**Profile**: `roam`
**Attack**: MAP-ProvideSubscriberInfo
**Goal**: Obtain IMSI/MSISDN/location for roaming subscribers

**Challenge**: Attacker spoofs as legitimate foreign operator
**Defense Test Points**:
- Roaming partner whitelist validation
- Rate limiting per OPC
- Correlation with real roaming traffic patterns

---

### 04: STP Failover Exploit
**Profile**: `full`
**Attack**: Configuration drift exploitation
**Goal**: Bypass security by forcing failover to weaker STP

**Simulates**: Real-world incident where backup STP had outdated firewall rules
**Defense Test Points**:
- Config synchronization across redundant nodes
- Automated rule parity validation
- Continuous failover testing

---

## Creating Custom Scenarios

Template for new scenarios:

```bash
#!/bin/bash
# Scenario XX: Your Attack Name
# Profile: base|roam|threat|full
# Description: One-line summary

set -e

echo "========================================="
echo "SCENARIO: Your Attack Name"
echo "========================================="
echo ""

# Check prerequisites
if ! docker compose ps | grep -q required_node; then
    echo "ERROR: Start with: docker compose --profile XXX up -d"
    exit 1
fi

# Phase 1: Setup
echo "[Phase 1] Setup attack environment"
# ... commands ...

# Phase 2: Execute attack
echo "[Phase 2] Execute attack"
docker compose exec -T ss7_attacker_1 bash << 'EOF'
# Attack commands here
EOF

# Phase 3: Analysis
echo ""
echo "========================================="
echo "Attack Analysis"
echo "========================================="
echo "Verify in logs: ..."
echo "Expected behavior: ..."
echo "Detection indicators: ..."
echo "Mitigation: ..."
```

## Scenario Development Workflow

1. **Design attack flow**: Map out nodes and message path
2. **Identify complexity**: Which profile provides needed infrastructure?
3. **Write scenario script**: Use template above
4. **Test in lab**: Verify attack executes correctly
5. **Document defenses**: Add detection/mitigation guidance
6. **Add to CI/CD**: Automate scenario as regression test

## Integration with Defense Testing

Scenarios can be used for:

- **Red Team Exercises**: Simulate real attacks
- **Blue Team Training**: Practice detection and response
- **Firewall Validation**: Verify rules block attacks
- **Honeypot Tuning**: Collect attacker behavior data
- **Anomaly Detection**: Generate training data for ML models

## Logging and Analysis

All scenarios log to:
- `/logs/<node>.log` - Per-node activity logs
- `/logs/firewall_*_alerts.json` - Security alerts
- `/pcaps/<node>.pcap` - Full packet captures (if enabled)

Analyze with console tools:
```bash
docker compose exec ss7_console bash

# Parse all logs
/scripts/analyze_logs.py /logs/*.log

# Generate threat report
/scripts/threat_report.py --scenario 01 --output /reports/

# View SCTP traffic
tshark -r /pcaps/stp_home_1.pcap -Y "sctp"
```

## Contributing

To add new scenarios:
1. Create script in `scenarios/XX-name.sh`
2. Update this README with scenario details
3. Test with all relevant profiles
4. Document detection methods
5. Submit PR to CRYPTOGRAM repo

---

**OPSEC**: All scenarios are for authorized lab use only. Never execute against production networks without explicit written authorization.
