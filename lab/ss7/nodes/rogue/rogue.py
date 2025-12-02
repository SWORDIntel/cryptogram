#!/usr/bin/env python3
"""Rogue Operator Core - Simulates malicious fake PLMN"""
import os, time, signal, sys
from pathlib import Path

log_file = os.getenv('LOG_FILE', '/logs/rogue_operator.log')
capture_all = os.getenv('CAPTURE_ALL', 'false').lower() == 'true'

running = True
def shutdown(s, f):
    global running
    running = False
    sys.exit(0)

signal.signal(signal.SIGTERM, shutdown)
signal.signal(signal.SIGINT, shutdown)

with open(log_file, 'a') as f:
    f.write(f"[{time.time()}] Rogue operator started - Capture mode: {capture_all}\n")

Path('/tmp/rogue_ready').touch()

while running:
    time.sleep(30)
    with open(log_file, 'a') as f:
        f.write(f"[{time.time()}] Rogue operator alive - intercepting traffic\n")
