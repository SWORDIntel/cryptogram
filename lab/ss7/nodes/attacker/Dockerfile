# SS7 Attacker Node - Pentesting Tools
# Contains SigPloit, ss7MAPer, and other SS7 security tools

FROM ubuntu:22.04

LABEL org.opencontainers.image.title="CRYPTOGRAM SS7 Attacker"
LABEL org.opencontainers.image.description="SS7 security testing toolkit for authorized lab use"
LABEL org.opencontainers.image.vendor="SWORD Ops"

ENV DEBIAN_FRONTEND=noninteractive

# Install dependencies
RUN apt-get update && apt-get install -y \
    python3 \
    python3-pip \
    python3-dev \
    git \
    build-essential \
    libsctp-dev \
    libpcap-dev \
    tcpdump \
    nmap \
    netcat \
    vim \
    tmux \
    wget \
    curl \
    && rm -rf /var/lib/apt/lists/*

# Install Python packages
RUN pip3 install --no-cache-dir \
    pysctp \
    scapy \
    pyyaml \
    requests \
    colorama \
    tabulate

# Clone and setup SigPloit
WORKDIR /opt
RUN git clone https://github.com/SigPloiter/SigPloit.git sigploit || \
    mkdir -p /opt/sigploit && \
    echo "#!/usr/bin/env python3" > /opt/sigploit/sigploit.py && \
    echo "print('SigPloit placeholder - integrate actual tool')" >> /opt/sigploit/sigploit.py && \
    chmod +x /opt/sigploit/sigploit.py

# Setup ss7MAPer (placeholder - would need actual tool)
RUN mkdir -p /opt/ss7maper && \
    cat > /opt/ss7maper/ss7maper.py << 'EOF'
#!/usr/bin/env python3
"""
ss7MAPer - MAP Protocol Testing Tool (Lab Placeholder)
"""
import sys
print("ss7MAPer - MAP Protocol Testing Tool")
print("Placeholder for lab integration")
print("In production, integrate with actual ss7MAPer toolkit")
sys.exit(0)
EOF
RUN chmod +x /opt/ss7maper/ss7maper.py

# Create attack scripts directory
RUN mkdir -p /scripts /logs /config

# Copy helper scripts
COPY entrypoint.sh /entrypoint.sh
COPY scripts/*.py /scripts/ 2>/dev/null || true
RUN chmod +x /entrypoint.sh

# Default working directory
WORKDIR /opt

# Health check
HEALTHCHECK --interval=30s --timeout=10s --start-period=5s --retries=3 \
    CMD test -f /tmp/attacker_ready || exit 1

ENTRYPOINT ["/entrypoint.sh"]
CMD ["/bin/bash"]
