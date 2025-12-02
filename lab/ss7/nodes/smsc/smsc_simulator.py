#!/usr/bin/env python3
"""SS7 SMSC Simulator"""
import os, sys, time, signal
from datetime import datetime
from pathlib import Path

class SMSCSimulator:
    def __init__(self):
        self.node_name = os.getenv('NODE_NAME', 'smsc_unknown')
        self.log_file = os.getenv('LOG_FILE', f'/logs/{self.node_name}.log')
        self.running = True
        signal.signal(signal.SIGTERM, self.shutdown)
        signal.signal(signal.SIGINT, self.shutdown)

    def log(self, msg):
        ts = datetime.utcnow().isoformat()
        line = f"[{ts}] {msg}\n"
        with open(self.log_file, 'a') as f:
            f.write(line)
        print(line, end='')

    def run(self):
        self.log("SMSC Simulator ready")
        Path('/tmp/smsc_ready').touch()
        while self.running:
            time.sleep(30)
            self.log(f"SMSC {self.node_name} alive")

    def shutdown(self, signum, frame):
        self.running = False
        sys.exit(0)

if __name__ == '__main__':
    SMSCSimulator().run()
