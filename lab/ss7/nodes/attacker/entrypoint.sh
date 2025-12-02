#!/bin/bash
# Attacker Node Entrypoint

set -e

NODE_NAME="${NODE_NAME:-attacker_unknown}"
OPC="${OPC:-0.8.1}"
DPC="${DPC:-0.1.10}"
LOG_FILE="${LOG_FILE:-/logs/${NODE_NAME}.log}"

echo "[$(date -Iseconds)] Starting $NODE_NAME (OPC: $OPC, DPC: $DPC)" | tee -a "$LOG_FILE"
echo "[$(date -Iseconds)] Target: GT=$TARGET_GT, IMSI=$TARGET_IMSI" | tee -a "$LOG_FILE"
echo "[$(date -Iseconds)] STP Peer: $STP_PEER" | tee -a "$LOG_FILE"

# Create ready marker
touch /tmp/attacker_ready

# Export all variables for scripts
export NODE_NAME OPC DPC OGT TARGET_GT TARGET_IMSI STP_PEER LOG_FILE

# Print banner
cat << 'EOF'
╔══════════════════════════════════════════════════════════════╗
║           CRYPTOGRAM SS7 ATTACKER NODE                       ║
║                                                              ║
║  AUTHORIZED LAB USE ONLY - DO NOT ATTACK REAL NETWORKS      ║
║                                                              ║
║  Available Tools:                                            ║
║    /opt/sigploit/sigploit.py     - SigPloit framework       ║
║    /opt/ss7maper/ss7maper.py     - MAP protocol fuzzer      ║
║    /scripts/test_attacks.sh      - Common attack scenarios  ║
║                                                              ║
║  Quick Tests:                                                ║
║    ./scripts/test_connectivity.sh                           ║
║    ./scripts/map_ati.sh <msisdn>                            ║
║    ./scripts/map_psi.sh <imsi>                              ║
╚══════════════════════════════════════════════════════════════╝
EOF

echo "[$(date -Iseconds)] Attacker node ready for testing" | tee -a "$LOG_FILE"

# Execute command (default: bash shell)
exec "$@"
