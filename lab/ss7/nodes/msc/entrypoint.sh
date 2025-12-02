#!/bin/bash
set -e
NODE_NAME="${NODE_NAME:-msc_unknown}"
PC="${PC:-0.1.20}"
GT="${GT:-441000100000}"
LOG_FILE="${LOG_FILE:-/logs/${NODE_NAME}.log}"
echo "[$(date -Iseconds)] Starting $NODE_NAME (PC: $PC, GT: $GT)" | tee -a "$LOG_FILE"
export NODE_NAME PC GT MCC MNC SSN HLR_GT LOG_LEVEL LOG_FILE
echo "[$(date -Iseconds)] MSC simulator starting..." | tee -a "$LOG_FILE"
exec "$@"
