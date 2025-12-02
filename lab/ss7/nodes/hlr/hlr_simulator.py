#!/usr/bin/env python3
"""
SS7 HLR Simulator - Honeypot Mode
Responds to MAP queries with fake data for threat intelligence gathering
"""

import os
import sys
import json
import time
import signal
import sqlite3
from datetime import datetime
from pathlib import Path

class HLRSimulator:
    def __init__(self):
        self.node_name = os.getenv('NODE_NAME', 'hlr_unknown')
        self.pc = os.getenv('PC', '0.1.10')
        self.gt = os.getenv('GT', '441000000000')
        self.mcc = os.getenv('MCC', '234')
        self.mnc = os.getenv('MNC', '15')
        self.ssn = os.getenv('SSN', '6')
        self.db_path = os.getenv('SUBSCRIBER_DB', '/data/hlr.db')
        self.honeypot_mode = os.getenv('HONEYPOT_MODE', 'false').lower() == 'true'
        self.log_file = os.getenv('LOG_FILE', f'/logs/{self.node_name}.log')
        self.running = True

        # Setup signal handlers
        signal.signal(signal.SIGTERM, self.shutdown)
        signal.signal(signal.SIGINT, self.shutdown)

        self.log(f"HLR Simulator initialized: PC={self.pc}, GT={self.gt}, MCC/MNC={self.mcc}/{self.mnc}")
        self.log(f"Honeypot mode: {self.honeypot_mode}")

    def log(self, message):
        """Write to log file with timestamp"""
        timestamp = datetime.utcnow().isoformat()
        log_line = f"[{timestamp}] {message}\n"

        with open(self.log_file, 'a') as f:
            f.write(log_line)

        print(log_line, end='')

    def get_subscriber(self, identifier, id_type='imsi'):
        """Query subscriber from database"""
        try:
            conn = sqlite3.connect(self.db_path)
            conn.row_factory = sqlite3.Row
            cursor = conn.cursor()

            if id_type == 'imsi':
                cursor.execute('SELECT * FROM subscribers WHERE imsi = ?', (identifier,))
            elif id_type == 'msisdn':
                cursor.execute('SELECT * FROM subscribers WHERE msisdn = ?', (identifier,))
            else:
                return None

            row = cursor.fetchone()
            conn.close()

            if row:
                return dict(row)
            return None
        except Exception as e:
            self.log(f"ERROR querying subscriber: {e}")
            return None

    def generate_fake_subscriber(self, imsi):
        """Generate plausible fake subscriber data for honeypot"""
        # Generate fake but plausible data
        msisdn = f"{self.mcc}{self.mnc}{imsi[-9:]}"  # Derive from IMSI
        fake_sub = {
            'imsi': imsi,
            'msisdn': msisdn,
            'location': 'LAC:9999,CellID:65535',  # Fake location
            'status': 'active',
            'ki': '0' * 32,  # Fake key
            'opc': '0' * 32
        }

        self.log(f"HONEYPOT: Generated fake subscriber for IMSI {imsi}")
        return fake_sub

    def handle_map_ati(self, params):
        """Handle MAP AnyTimeInterrogation (location query)"""
        msisdn = params.get('msisdn', '')
        imsi = params.get('imsi', '')
        src_addr = params.get('src_addr', 'unknown')

        self.log(f"MAP-ATI request from {src_addr}: MSISDN={msisdn}, IMSI={imsi}")

        if self.honeypot_mode:
            # Always respond with fake data
            if msisdn:
                subscriber = self.get_subscriber(msisdn, 'msisdn')
                if not subscriber:
                    # Generate fake response
                    subscriber = self.generate_fake_subscriber(imsi or f"{self.mcc}{self.mnc}0000000001")
            else:
                subscriber = self.generate_fake_subscriber(imsi)

            response = {
                'result': 'success',
                'imsi': subscriber['imsi'],
                'location': subscriber['location'],
                'status': subscriber['status']
            }

            self.log(f"HONEYPOT: Returning fake ATI response: {response}")
            return response
        else:
            # Real mode: only respond if subscriber exists
            if msisdn:
                subscriber = self.get_subscriber(msisdn, 'msisdn')
            elif imsi:
                subscriber = self.get_subscriber(imsi, 'imsi')
            else:
                self.log("ERROR: No MSISDN or IMSI provided")
                return {'result': 'error', 'error': 'unknown_subscriber'}

            if subscriber:
                response = {
                    'result': 'success',
                    'imsi': subscriber['imsi'],
                    'location': subscriber['location'],
                    'status': subscriber['status']
                }
                self.log(f"Returning ATI response: {response}")
                return response
            else:
                self.log(f"Subscriber not found: MSISDN={msisdn}, IMSI={imsi}")
                return {'result': 'error', 'error': 'unknown_subscriber'}

    def handle_map_psi(self, params):
        """Handle MAP ProvideSubscriberInfo"""
        imsi = params.get('imsi', '')
        src_addr = params.get('src_addr', 'unknown')

        self.log(f"MAP-PSI request from {src_addr}: IMSI={imsi}")

        if self.honeypot_mode or True:  # Always respond in lab
            subscriber = self.get_subscriber(imsi, 'imsi')
            if not subscriber:
                subscriber = self.generate_fake_subscriber(imsi)

            response = {
                'result': 'success',
                'imsi': subscriber['imsi'],
                'msisdn': subscriber['msisdn'],
                'location': subscriber['location'],
                'status': subscriber['status']
            }

            self.log(f"Returning PSI response: {response}")
            return response

        return {'result': 'error', 'error': 'unknown_subscriber'}

    def handle_map_ul(self, params):
        """Handle MAP UpdateLocation (roaming registration)"""
        imsi = params.get('imsi', '')
        vlr_number = params.get('vlr_number', '')
        src_addr = params.get('src_addr', 'unknown')

        self.log(f"MAP-UpdateLocation request from {src_addr}: IMSI={imsi}, VLR={vlr_number}")

        subscriber = self.get_subscriber(imsi, 'imsi')
        if subscriber:
            # Update location in database
            try:
                conn = sqlite3.connect(self.db_path)
                cursor = conn.cursor()
                cursor.execute(
                    'UPDATE subscribers SET location = ?, status = ? WHERE imsi = ?',
                    (f'VLR:{vlr_number}', 'roaming', imsi)
                )
                conn.commit()
                conn.close()

                self.log(f"Updated location for IMSI {imsi} to VLR {vlr_number}")

                return {
                    'result': 'success',
                    'imsi': imsi,
                    'msisdn': subscriber['msisdn']
                }
            except Exception as e:
                self.log(f"ERROR updating location: {e}")
                return {'result': 'error', 'error': 'system_failure'}

        if self.honeypot_mode:
            # Accept and log, but return fake data
            self.log(f"HONEYPOT: Accepting UpdateLocation for unknown IMSI {imsi}")
            return {
                'result': 'success',
                'imsi': imsi,
                'msisdn': f"{self.mcc}{self.mnc}{imsi[-9:]}"
            }

        return {'result': 'error', 'error': 'unknown_subscriber'}

    def simulate_network_listener(self):
        """Simulate listening for SS7 traffic (simplified for lab)"""
        self.log("HLR Simulator ready - listening for MAP requests")

        # Create ready marker for healthcheck
        Path('/tmp/hlr_ready').touch()

        # In real implementation, would use pysctp to listen on SCTP
        # For lab MVP, just log that we're running
        while self.running:
            # Placeholder - in real version would receive and parse MAP messages
            # For now, just keep alive and log periodically
            time.sleep(30)
            self.log(f"HLR {self.node_name} alive - Honeypot mode: {self.honeypot_mode}")

    def shutdown(self, signum, frame):
        """Graceful shutdown"""
        self.log("Received shutdown signal, stopping HLR")
        self.running = False

        # Clean up
        if os.path.exists('/tmp/hlr_ready'):
            os.remove('/tmp/hlr_ready')

        sys.exit(0)

    def run(self):
        """Main run loop"""
        try:
            self.simulate_network_listener()
        except Exception as e:
            self.log(f"FATAL ERROR: {e}")
            sys.exit(1)

if __name__ == '__main__':
    hlr = HLRSimulator()
    hlr.run()
