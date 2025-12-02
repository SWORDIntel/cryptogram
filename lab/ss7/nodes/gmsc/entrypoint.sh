#!/bin/bash
set -e
echo "[$(date -Iseconds)] Starting GMSC ${NODE_NAME:-gmsc}"
exec "$@"
