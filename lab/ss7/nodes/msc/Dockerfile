# SS7 MSC/VLR Node (Simplified simulator)
# Based on HLR with MSC-specific MAP operations

FROM python:3.11-slim

LABEL org.opencontainers.image.title="CRYPTOGRAM SS7 MSC"
LABEL org.opencontainers.image.description="SS7 MSC/VLR simulator for lab testing"
LABEL org.opencontainers.image.vendor="SWORD Ops"

RUN apt-get update && apt-get install -y \
    libsctp-dev \
    tcpdump \
    && rm -rf /var/lib/apt/lists/*

RUN pip install --no-cache-dir pysctp pyyaml scapy

RUN mkdir -p /config /data /logs /scripts

COPY msc_simulator.py /scripts/msc_simulator.py
COPY entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh /scripts/msc_simulator.py

EXPOSE 2905/sctp

WORKDIR /

HEALTHCHECK --interval=30s --timeout=10s --start-period=10s --retries=3 \
    CMD python3 -c "import os; exit(0 if os.path.exists('/tmp/msc_ready') else 1)"

ENTRYPOINT ["/entrypoint.sh"]
CMD ["python3", "/scripts/msc_simulator.py"]
