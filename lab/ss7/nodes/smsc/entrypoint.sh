#!/bin/bash
set -e
NODE_NAME="${NODE_NAME:-smsc_unknown}"
LOG_FILE="${LOG_FILE:-/logs/${NODE_NAME}.log}"
echo "[$(date -Iseconds)] Starting $NODE_NAME" | tee -a "$LOG_FILE"
exec "$@"
