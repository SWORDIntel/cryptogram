#!/bin/bash
# HLR Node Entrypoint

set -e

NODE_NAME="${NODE_NAME:-hlr_unknown}"
PC="${PC:-0.1.10}"
GT="${GT:-441000000000}"
LOG_FILE="${LOG_FILE:-/logs/${NODE_NAME}.log}"

echo "[$(date -Iseconds)] Starting $NODE_NAME (PC: $PC, GT: $GT)" | tee -a "$LOG_FILE"

# Initialize subscriber database if it doesn't exist
if [ ! -f "$SUBSCRIBER_DB" ]; then
    echo "[$(date -Iseconds)] Initializing subscriber database: $SUBSCRIBER_DB" | tee -a "$LOG_FILE"
    sqlite3 "$SUBSCRIBER_DB" << 'EOF'
CREATE TABLE subscribers (
    imsi TEXT PRIMARY KEY,
    msisdn TEXT UNIQUE NOT NULL,
    ki TEXT,
    opc TEXT,
    location TEXT,
    status TEXT DEFAULT 'active',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Add test subscribers
INSERT INTO subscribers (imsi, msisdn, ki, opc, location, status) VALUES
('234150000000001', '441234567890', '00112233445566778899AABBCCDDEEFF', 'FEDCBA9876543210FEDCBA9876543210', 'LAC:1000,CellID:12345', 'active'),
('234150000000002', '441234567891', '11223344556677889900AABBCCDDEEFF', 'EEDDCCBBAA998877665544332211FFEE', 'LAC:1000,CellID:12346', 'active'),
('234150000000003', '441234567892', '22334455667788990011AABBCCDDEEFF', 'DDCCBBAA99887766554433221100FFEE', 'LAC:1001,CellID:12347', 'active');

-- For visited PLMN subscribers visiting home
INSERT INTO subscribers (imsi, msisdn, ki, opc, location, status) VALUES
('338010000000001', '338123456789', '33445566778899001122AABBCCDDEEFF', 'CCBBAA99887766554433221100FFDDEE', 'ROAMING:LAC:1000', 'roaming');
EOF
    echo "[$(date -Iseconds)] Database initialized with test subscribers" | tee -a "$LOG_FILE"
fi

# Export config for Python script
export NODE_NAME PC GT MCC MNC SSN SUBSCRIBER_DB HONEYPOT_MODE LOG_LEVEL LOG_FILE

echo "[$(date -Iseconds)] Configuration complete. Starting HLR simulator..." | tee -a "$LOG_FILE"

# Execute the command
exec "$@"
