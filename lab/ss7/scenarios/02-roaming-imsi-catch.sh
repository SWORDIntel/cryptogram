#!/bin/bash
# Scenario 2: Cross-PLMN IMSI Catcher Simulation
# Profile: roam
# Description: Attacker poses as foreign PLMN to query roaming subscribers

set -e

SCENARIO="02-roaming-imsi-catch"
TARGET_IMSI="${TARGET_IMSI:-234150000000001}"

echo "========================================="
echo "SCENARIO: Roaming IMSI Catcher"
echo "========================================="
echo ""
echo "Profile Required: roam"
echo "Target IMSI: $TARGET_IMSI"
echo ""
echo "Attack Flow:"
echo "  Attacker (poses as V-PLMN) → IPX → Home PLMN"
echo "  MAP-PSI request for subscriber info"
echo "  Exploits legitimate roaming query path"
echo ""

# Check required nodes
if ! docker compose ps | grep -q ss7_attacker_1 || ! docker compose ps | grep -q stp_visit; then
    echo "ERROR: Required nodes not running. Start with:"
    echo "  docker compose --profile roam up -d"
    exit 1
fi

echo "[*] Attacker spoofs as Visited PLMN..."
echo "[*] Launching MAP-PSI query..."
echo ""

docker compose exec -T ss7_attacker_1 bash << EOF
echo "[+] Spoofing OPC to appear as V-PLMN: 0.2.20"
echo "[+] Target: Home HLR (0.1.10)"
echo "[+] Query: MAP-PSI for IMSI $TARGET_IMSI"
echo ""
echo "[*] Simulating cross-PLMN subscriber info query..."
echo "    (Would use: python3 /opt/ss7maper/ss7maper.py --psi $TARGET_IMSI)"
echo ""
echo "[*] Request sent via IPX hub"
EOF

echo ""
echo "========================================="
echo "Detection Challenge"
echo "========================================="
echo ""
echo "This attack is hard to detect because:"
echo "  - Uses legitimate roaming query path"
echo "  - Attacker OPC looks like foreign operator"
echo "  - Normal MAP-PSI operations are common in roaming"
echo ""
echo "Defense strategies:"
echo "  - Validate roaming agreements (whitelist OPCs)"
echo "  - Rate limiting per source operator"
echo "  - Correlation with actual roaming traffic"
echo "  - Anomaly detection (unusual query patterns)"
echo ""
echo "Check logs:"
echo "  docker compose logs hlr_home | grep PSI"
echo "  docker compose logs stp_ipx_1 | grep 0.8.1"
echo ""
