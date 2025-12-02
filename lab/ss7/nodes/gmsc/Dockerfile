# SS7 GMSC Node (Gateway MSC)
FROM python:3.11-slim
RUN apt-get update && apt-get install -y libsctp-dev && rm -rf /var/lib/apt/lists/*
RUN pip install --no-cache-dir pysctp
RUN mkdir -p /logs /scripts
COPY entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh
RUN echo '#!/usr/bin/env python3\nimport time, os\nfrom pathlib import Path\nPath("/tmp/gmsc_ready").touch()\nwhile True: time.sleep(30)' > /scripts/gmsc.py
RUN chmod +x /scripts/gmsc.py
EXPOSE 2905/sctp
ENTRYPOINT ["/entrypoint.sh"]
CMD ["python3", "/scripts/gmsc.py"]
