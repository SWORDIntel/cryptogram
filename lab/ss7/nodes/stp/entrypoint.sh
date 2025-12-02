#!/bin/bash
# STP Node Entrypoint

set -e

NODE_NAME="${NODE_NAME:-stp_unknown}"
PC="${PC:-0.0.1}"
LOG_FILE="${LOG_FILE:-/logs/${NODE_NAME}.log}"
LOG_LEVEL="${LOG_LEVEL:-INFO}"

echo "[$(date -Iseconds)] Starting $NODE_NAME (PC: $PC)" | tee -a "$LOG_FILE"

# Generate osmo-stp config from environment variables
cat > /config/osmo-stp.cfg << EOF
!
! OsmoSTP Configuration for $NODE_NAME
! Auto-generated from environment variables
!
log stderr
  logging filter all 1
  logging color 1
  logging print category-hex 0
  logging print category 1
  logging print thread-id 0
  logging timestamp 1
  logging print file basename last
  logging level set-all notice
  logging level lglobal ${LOG_LEVEL,,}

log file $LOG_FILE
  logging filter all 1
  logging color 0
  logging timestamp 1
  logging print category 1
  logging print level 1
  logging level lglobal ${LOG_LEVEL,,}

cs7 instance 0
  point-code $PC
  sccp-timer ias 300
  sccp-timer iar 660
EOF

# Add SCTP peers if specified
if [ -n "$SCTP_PEERS" ]; then
    IFS=',' read -ra PEERS <<< "$SCTP_PEERS"
    for i in "${!PEERS[@]}"; do
        peer="${PEERS[$i]}"
        IFS=':' read -r peer_host peer_port <<< "$peer"
        peer_port="${peer_port:-2905}"
        echo "  asp asp-peer-$i 2905 0 m3ua" >> /config/osmo-stp.cfg
        echo "   remote-ip $peer_host" >> /config/osmo-stp.cfg
        echo "   remote-port $peer_port" >> /config/osmo-stp.cfg
        echo "   role asp" >> /config/osmo-stp.cfg
        echo "   sctp-role client" >> /config/osmo-stp.cfg
        echo "[$(date -Iseconds)] Added peer: $peer_host:$peer_port" | tee -a "$LOG_FILE"
    done
fi

# Add GT routing if specified
if [ -f "$GT_ROUTES" ]; then
    echo "[$(date -Iseconds)] Loading GT routes from $GT_ROUTES" | tee -a "$LOG_FILE"
    cat "$GT_ROUTES" >> /config/osmo-stp.cfg
fi

# Add firewall rules if specified
if [ -f "$FIREWALL_RULES" ]; then
    echo "[$(date -Iseconds)] Loading firewall rules from $FIREWALL_RULES" | tee -a "$LOG_FILE"
    # Firewall implemented via external script monitoring traffic
    /scripts/ss7_firewall.py "$FIREWALL_RULES" &
fi

# Start packet capture if enabled
if [ "${ENABLE_PCAP:-false}" = "true" ]; then
    echo "[$(date -Iseconds)] Starting packet capture to /pcaps/${NODE_NAME}.pcap" | tee -a "$LOG_FILE"
    tcpdump -i any -w "/pcaps/${NODE_NAME}.pcap" sctp &
fi

echo "[$(date -Iseconds)] Configuration complete. Starting osmo-stp..." | tee -a "$LOG_FILE"

# Execute the command
exec "$@"
