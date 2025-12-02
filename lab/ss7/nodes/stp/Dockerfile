# SS7 STP (Signal Transfer Point) Node
# Based on Osmocom stack with SIGTRAN support

FROM ubuntu:22.04

LABEL org.opencontainers.image.title="CRYPTOGRAM SS7 STP"
LABEL org.opencontainers.image.description="SS7 Signal Transfer Point for lab testing"
LABEL org.opencontainers.image.vendor="SWORD Ops"
LABEL org.opencontainers.image.source="https://github.com/SWORDOps/CRYPTOGRAM"

# Prevent interactive prompts
ENV DEBIAN_FRONTEND=noninteractive

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    git \
    autoconf \
    automake \
    libtool \
    pkg-config \
    libsctp-dev \
    libpcap-dev \
    libssl-dev \
    libtalloc-dev \
    python3 \
    python3-pip \
    tcpdump \
    netcat \
    iproute2 \
    vim \
    less \
    && rm -rf /var/lib/apt/lists/*

# Install Python dependencies for scripting
RUN pip3 install --no-cache-dir pyyaml scapy

# Clone and build libosmocore
WORKDIR /tmp
RUN git clone https://github.com/osmocom/libosmocore.git && \
    cd libosmocore && \
    git checkout 1.9.0 && \
    autoreconf -fi && \
    ./configure --prefix=/usr --disable-doxygen && \
    make -j$(nproc) && \
    make install && \
    ldconfig && \
    cd .. && rm -rf libosmocore

# Clone and build libosmo-sccp (for SIGTRAN/SCCP)
RUN git clone https://github.com/osmocom/libosmo-sccp.git && \
    cd libosmo-sccp && \
    git checkout 1.8.0 && \
    autoreconf -fi && \
    ./configure --prefix=/usr --disable-doxygen && \
    make -j$(nproc) && \
    make install && \
    ldconfig && \
    cd .. && rm -rf libosmo-sccp

# Clone and build osmo-stp
RUN git clone https://github.com/osmocom/osmo-stp.git && \
    cd osmo-stp && \
    git checkout 1.7.0 && \
    autoreconf -fi && \
    ./configure --prefix=/usr --disable-doxygen && \
    make -j$(nproc) && \
    make install && \
    cd .. && rm -rf osmo-stp

# Create directories
RUN mkdir -p /config /logs /pcaps /scripts

# Copy entrypoint and helper scripts
COPY entrypoint.sh /entrypoint.sh
COPY scripts/*.py /scripts/
RUN chmod +x /entrypoint.sh /scripts/*.py

# Expose SCTP port (M3UA)
EXPOSE 2905/sctp

# Set working directory
WORKDIR /

# Health check
HEALTHCHECK --interval=30s --timeout=10s --start-period=5s --retries=3 \
    CMD ss -ln | grep -q ':2905' || exit 1

ENTRYPOINT ["/entrypoint.sh"]
CMD ["osmo-stp", "-c", "/config/osmo-stp.cfg"]
