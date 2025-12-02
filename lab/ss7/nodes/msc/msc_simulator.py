#!/usr/bin/env python3
"""SS7 MSC/VLR Simulator - Handles mobility management"""
import os, sys, time, signal
from datetime import datetime
from pathlib import Path

class MSCSimulator:
    def __init__(self):
        self.node_name = os.getenv('NODE_NAME', 'msc_unknown')
        self.pc = os.getenv('PC', '0.1.20')
        self.log_file = os.getenv('LOG_FILE', f'/logs/{self.node_name}.log')
        self.running = True
        signal.signal(signal.SIGTERM, self.shutdown)
        signal.signal(signal.SIGINT, self.shutdown)
        self.log(f"MSC Simulator initialized: PC={self.pc}")

    def log(self, message):
        timestamp = datetime.utcnow().isoformat()
        log_line = f"[{timestamp}] {message}\n"
        with open(self.log_file, 'a') as f:
            f.write(log_line)
        print(log_line, end='')

    def run(self):
        self.log("MSC Simulator ready - handling mobility management")
        Path('/tmp/msc_ready').touch()
        while self.running:
            time.sleep(30)
            self.log(f"MSC {self.node_name} alive")

    def shutdown(self, signum, frame):
        self.log("MSC shutting down")
        self.running = False
        sys.exit(0)

if __name__ == '__main__':
    msc = MSCSimulator()
    msc.run()
