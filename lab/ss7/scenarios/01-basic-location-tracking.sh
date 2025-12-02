#!/bin/bash
# Scenario 1: Basic Location Tracking Attack
# Profile: base
# Description: Simple MAP-ATI query to track subscriber location

set -e

SCENARIO="01-basic-location-tracking"
TARGET_MSISDN="${TARGET_MSISDN:-441234567890}"

echo "========================================="
echo "SCENARIO: Basic Location Tracking"
echo "========================================="
echo ""
echo "Profile Required: base"
echo "Target: $TARGET_MSISDN"
echo ""
echo "Attack Flow:"
echo "  Attacker → IPX STP → Home STP → Home HLR"
echo "  MAP-ATI request for location"
echo ""

# Check if attacker container is running
if ! docker compose ps | grep -q ss7_attacker_1; then
    echo "ERROR: Attacker node not running. Start with:"
    echo "  docker compose --profile base up -d"
    exit 1
fi

echo "[*] Launching MAP-ATI attack..."
echo ""

# Execute attack in attacker container
docker compose exec -T ss7_attacker_1 bash << EOF
echo "[+] Attacker node ready"
echo "[+] Target MSISDN: $TARGET_MSISDN"
echo "[+] OPC: \$OPC, DPC: \$DPC"
echo ""
echo "[*] Simulating MAP-ATI request..."
echo "    (In full implementation, would use SigPloit or ss7MAPer)"
echo ""

# Placeholder - would execute actual attack tool
# python3 /opt/sigploit/sigploit.py --attack ATI --msisdn $TARGET_MSISDN

echo "[*] Attack sent. Check logs:"
echo "    docker compose logs hlr_home | grep ATI"
echo "    docker compose logs stp_home_1 | grep MAP"
echo ""
EOF

echo ""
echo "========================================="
echo "Scenario Complete"
echo "========================================="
echo ""
echo "Verify attack in logs:"
echo "  docker compose logs hlr_home | tail -20"
echo "  docker compose logs stp_home_1 | tail -20"
echo ""
echo "Expected behavior (Honeypot mode):"
echo "  - HLR receives MAP-ATI request"
echo "  - HLR returns fake location data"
echo "  - STP logs routing decision"
echo "  - Firewall may alert on suspicious query"
echo ""
