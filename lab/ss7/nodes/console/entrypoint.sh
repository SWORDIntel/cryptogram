#!/bin/bash
# Console Node Entrypoint

set -e

NODE_NAME="${NODE_NAME:-ss7_console}"

echo "[$(date -Iseconds)] Starting SS7 Lab Console"

# Create ready marker
touch /tmp/console_ready

# Print banner
cat << 'EOF'
╔══════════════════════════════════════════════════════════════╗
║           CRYPTOGRAM SS7 LAB CONSOLE                         ║
║                                                              ║
║  Monitoring & Analysis Hub                                   ║
║                                                              ║
║  Available Commands:                                         ║
║    tail -f /logs/*.log           - Follow all logs          ║
║    /scripts/analyze_logs.py      - Parse and analyze logs   ║
║    /scripts/threat_report.py     - Generate threat report   ║
║    tshark -r /pcaps/*.pcap       - Analyze packet captures  ║
║    wireshark (if X11 forwarded)  - GUI analysis             ║
║                                                              ║
║  Quick Stats:                                                ║
║    ls -lh /logs/                 - Check log sizes          ║
║    ls -lh /pcaps/                - Check capture files      ║
║    /scripts/lab_status.sh        - Check all nodes          ║
╚══════════════════════════════════════════════════════════════╝
EOF

echo "[$(date -Iseconds)] Console ready - all logs available in /logs/"
echo "[$(date -Iseconds)] Packet captures available in /pcaps/"

# Execute command (default: bash shell)
exec "$@"
