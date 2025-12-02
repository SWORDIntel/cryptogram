#!/usr/bin/env python3
"""
SS7 Firewall Script - Monitors traffic and enforces rules
Runs alongside osmo-stp to provide filtering and honeypot functionality
"""

import sys
import os
import json
import yaml
import time
import signal
from datetime import datetime
from pathlib import Path

class SS7Firewall:
    def __init__(self, rules_file):
        self.rules_file = rules_file
        self.rules = []
        self.log_file = os.getenv('LOG_FILE', '/logs/firewall.log')
        self.alert_file = os.getenv('ALERT_FILE', '/logs/firewall_alerts.json')
        self.honeypot_mode = os.getenv('HONEYPOT_MODE', 'false').lower() == 'true'
        self.running = True

        # Setup signal handlers
        signal.signal(signal.SIGTERM, self.shutdown)
        signal.signal(signal.SIGINT, self.shutdown)

        self.load_rules()

    def load_rules(self):
        """Load firewall rules from config file"""
        if not os.path.exists(self.rules_file):
            self.log(f"WARNING: Rules file not found: {self.rules_file}")
            return

        try:
            with open(self.rules_file, 'r') as f:
                if self.rules_file.endswith('.yaml') or self.rules_file.endswith('.yml'):
                    config = yaml.safe_load(f)
                else:
                    # Simple line-based format
                    config = {'rules': [line.strip() for line in f if line.strip() and not line.startswith('#')]}

            self.rules = config.get('rules', [])
            self.log(f"Loaded {len(self.rules)} firewall rules")
        except Exception as e:
            self.log(f"ERROR loading rules: {e}")

    def log(self, message):
        """Write to log file with timestamp"""
        timestamp = datetime.utcnow().isoformat()
        log_line = f"[{timestamp}] {message}\n"

        with open(self.log_file, 'a') as f:
            f.write(log_line)

        print(log_line, end='')

    def alert(self, event_type, details):
        """Write alert to JSON alert file"""
        alert = {
            'timestamp': datetime.utcnow().isoformat(),
            'type': event_type,
            'details': details,
            'honeypot_mode': self.honeypot_mode
        }

        with open(self.alert_file, 'a') as f:
            f.write(json.dumps(alert) + '\n')

        self.log(f"ALERT: {event_type} - {json.dumps(details)}")

    def check_rules(self, packet_info):
        """Check if packet matches any firewall rules"""
        # Simplified rule matching - in real implementation would parse SCTP/M3UA/SCCP
        for rule in self.rules:
            if self.rule_matches(rule, packet_info):
                return rule
        return None

    def rule_matches(self, rule, packet_info):
        """Check if a rule matches the packet"""
        # Simplified - would need full protocol parsing
        # Rule format: "BLOCK OPC=0.8.* DPC=0.1.10 OPCODE=MAP-ATI"
        # For now, just placeholder
        return False

    def process_traffic(self):
        """Monitor and process SS7 traffic"""
        self.log("SS7 Firewall started")
        self.log(f"Honeypot mode: {self.honeypot_mode}")

        # In real implementation, would sniff SCTP traffic
        # For now, just log that we're running
        while self.running:
            time.sleep(5)
            # Placeholder - would process actual packets here
            # self.check_packet(packet)

    def shutdown(self, signum, frame):
        """Graceful shutdown"""
        self.log("Received shutdown signal, stopping firewall")
        self.running = False
        sys.exit(0)

    def run(self):
        """Main run loop"""
        try:
            self.process_traffic()
        except Exception as e:
            self.log(f"FATAL ERROR: {e}")
            sys.exit(1)

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("Usage: ss7_firewall.py <rules_file>")
        sys.exit(1)

    firewall = SS7Firewall(sys.argv[1])
    firewall.run()
