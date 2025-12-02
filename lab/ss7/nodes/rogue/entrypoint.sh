#!/bin/bash
set -e
echo "[$(date -Iseconds)] Starting Rogue Operator ${NODE_NAME:-rogue} - MCC=${MCC:-997}"
exec "$@"
