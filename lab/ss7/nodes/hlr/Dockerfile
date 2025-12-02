# SS7 HLR/HSS Node (Honeypot Mode)
# Simulates Home Location Register for lab testing

FROM python:3.11-slim

LABEL org.opencontainers.image.title="CRYPTOGRAM SS7 HLR"
LABEL org.opencontainers.image.description="SS7 HLR/HSS simulator with honeypot capabilities"
LABEL org.opencontainers.image.vendor="SWORD Ops"

# Install dependencies
RUN apt-get update && apt-get install -y \
    libsctp-dev \
    tcpdump \
    sqlite3 \
    && rm -rf /var/lib/apt/lists/*

# Install Python packages
RUN pip install --no-cache-dir \
    pysctp \
    sqlalchemy \
    pyyaml \
    scapy

# Create directories
RUN mkdir -p /config /data /logs /scripts

# Copy application files
COPY hlr_simulator.py /scripts/hlr_simulator.py
COPY entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh /scripts/hlr_simulator.py

# Expose SCTP port
EXPOSE 2905/sctp

WORKDIR /

# Health check
HEALTHCHECK --interval=30s --timeout=10s --start-period=10s --retries=3 \
    CMD python3 -c "import os; exit(0 if os.path.exists('/tmp/hlr_ready') else 1)"

ENTRYPOINT ["/entrypoint.sh"]
CMD ["python3", "/scripts/hlr_simulator.py"]
