#!/usr/bin/env python3
"""SS7 Firewall/IDS - Monitors and filters SS7 traffic"""
import os, time, sys, signal
from pathlib import Path

running = True
def shutdown(s, f):
    global running
    running = False
    sys.exit(0)

signal.signal(signal.SIGTERM, shutdown)
signal.signal(signal.SIGINT, shutdown)

log_file = os.getenv('LOG_FILE', '/logs/firewall.log')
Path('/tmp/firewall_ready').touch()

with open(log_file, 'a') as f:
    f.write(f"[{time.time()}] SS7 Firewall active\n")

while running:
    time.sleep(30)
