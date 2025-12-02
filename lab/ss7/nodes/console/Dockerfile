# SS7 Lab Console - Monitoring and Analysis
# Central management node with Wireshark, log analysis, and dashboards

FROM ubuntu:22.04

LABEL org.opencontainers.image.title="CRYPTOGRAM SS7 Console"
LABEL org.opencontainers.image.description="Central monitoring console for SS7 lab"
LABEL org.opencontainers.image.vendor="SWORD Ops"

ENV DEBIAN_FRONTEND=noninteractive

# Install core tools
RUN apt-get update && apt-get install -y \
    wireshark-common \
    tshark \
    tcpdump \
    python3 \
    python3-pip \
    sqlite3 \
    vim \
    tmux \
    htop \
    netcat \
    curl \
    wget \
    jq \
    less \
    && rm -rf /var/lib/apt/lists/*

# Install Python packages for log analysis
RUN pip3 install --no-cache-dir \
    pyyaml \
    pandas \
    matplotlib \
    scapy \
    colorama \
    tabulate \
    rich

# Create directories
RUN mkdir -p /logs /pcaps /dashboards /scripts /reports

# Copy helper scripts
COPY entrypoint.sh /entrypoint.sh
COPY scripts/*.py /scripts/ 2>/dev/null || true
RUN chmod +x /entrypoint.sh

# Default user setup (for Wireshark permissions)
RUN groupadd -r ss7user && useradd -r -g ss7user ss7user && \
    usermod -aG wireshark ss7user 2>/dev/null || true

WORKDIR /

HEALTHCHECK --interval=30s --timeout=10s --start-period=5s --retries=3 \
    CMD test -f /tmp/console_ready || exit 1

ENTRYPOINT ["/entrypoint.sh"]
CMD ["/bin/bash"]
