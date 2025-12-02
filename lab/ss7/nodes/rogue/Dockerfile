# Rogue Operator Core - Fake PLMN for threat simulation
FROM python:3.11-slim
RUN apt-get update && apt-get install -y libsctp-dev && rm -rf /var/lib/apt/lists/*
RUN pip install --no-cache-dir pysctp
RUN mkdir -p /data /logs /scripts
COPY rogue.py /scripts/rogue.py
COPY entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh /scripts/rogue.py
EXPOSE 2905/sctp
ENTRYPOINT ["/entrypoint.sh"]
CMD ["python3", "/scripts/rogue.py"]
