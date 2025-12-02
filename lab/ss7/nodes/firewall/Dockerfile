# SS7 Firewall/IDS Node
FROM python:3.11-slim
RUN apt-get update && apt-get install -y libsctp-dev tcpdump && rm -rf /var/lib/apt/lists/*
RUN pip install --no-cache-dir pysctp pyyaml
RUN mkdir -p /config /logs /scripts
COPY firewall.py /scripts/firewall.py
COPY entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh /scripts/firewall.py
EXPOSE 2905/sctp
ENTRYPOINT ["/entrypoint.sh"]
CMD ["python3", "/scripts/firewall.py"]
