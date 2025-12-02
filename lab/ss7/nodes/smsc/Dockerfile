# SS7 SMSC Node
FROM python:3.11-slim
LABEL org.opencontainers.image.title="CRYPTOGRAM SS7 SMSC"
LABEL org.opencontainers.image.vendor="SWORD Ops"
RUN apt-get update && apt-get install -y libsctp-dev tcpdump && rm -rf /var/lib/apt/lists/*
RUN pip install --no-cache-dir pysctp pyyaml scapy
RUN mkdir -p /config /data /logs /scripts
COPY smsc_simulator.py /scripts/smsc_simulator.py
COPY entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh /scripts/smsc_simulator.py
EXPOSE 2905/sctp 2775/tcp
HEALTHCHECK --interval=30s --timeout=10s CMD test -f /tmp/smsc_ready || exit 1
ENTRYPOINT ["/entrypoint.sh"]
CMD ["python3", "/scripts/smsc_simulator.py"]
