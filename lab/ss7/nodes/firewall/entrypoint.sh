#!/bin/bash
set -e
echo "[$(date -Iseconds)] Starting SS7 Firewall ${NODE_NAME:-firewall}"
exec "$@"
