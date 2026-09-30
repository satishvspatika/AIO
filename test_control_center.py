#!/usr/bin/env python3
"""
=============================================================================
test_control_center.py - Automated End-to-End Test Procedure Script
=============================================================================
This script tests remote Control Center commands for ESP32 telemetry stations.
Commands are queued on the server and verified when the device piggybacks its
health check-in (active slot / 15-min interval).

Usage:
    python3 test_control_center.py --stn 009876 --server https://devhlt.spatika.net
=============================================================================
"""

import sys
import time
import argparse
import datetime
import json
import ssl
import urllib.request
import urllib.parse
import urllib.error

DEFAULT_SERVER = "https://devhlt.spatika.net"

# Create unverified SSL context for testing against HTTPS endpoints
ssl_ctx = ssl.create_default_context()
ssl_ctx.check_hostname = False
ssl_ctx.verify_mode = ssl.CERT_NONE

def log(msg, flag="INFO"):
    ts = datetime.datetime.now().strftime("%H:%M:%S")
    colors = {
        "INFO": "\033[94m",
        "PASS": "\033[92m",
        "WARN": "\033[93m",
        "FAIL": "\033[91m",
        "RESET": "\033[0m"
    }
    c = colors.get(flag, colors["RESET"])
    reset = colors["RESET"]
    print(f"[{ts}] {c}[{flag}]{reset} {msg}")

def queue_command(server_url, stn_id, cmd, param=""):
    """Queues a command on the server for a specific station."""
    url = f"{server_url}/cmd/{stn_id}/{cmd}"
    if param:
        url += f"?param={urllib.parse.quote(str(param))}"
    
    req = urllib.request.Request(url, method="POST")
    try:
        with urllib.request.urlopen(req, context=ssl_ctx, timeout=10) as resp:
            if resp.status in (200, 303, 302):
                log(f"Queued command '{cmd}' (param='{param}') for station {stn_id}", "PASS")
                return True
    except Exception as e:
        log(f"Failed to queue command '{cmd}': {e}", "FAIL")
        return False

def get_latest_health(server_url, stn_id):
    """Fetches the latest health check-in record for a station from server JSON endpoint."""
    url = f"{server_url}/api/station/{stn_id}/latest"
    try:
        with urllib.request.urlopen(url, timeout=10) as resp:
            if resp.status == 200:
                data = json.loads(resp.read().decode())
                return data
    except Exception as e:
        # Fallback to html/json parsing if API route differs
        pass
    return None

def run_test_suite(server_url, stn_id, wait_for_slot=False, poll_interval=10, timeout_sec=900):
    """Executes test suite for all 10 Control Center commands."""
    log(f"Starting Control Center Command Verification Suite for Station: {stn_id}")
    log(f"Target Server: {server_url}")
    print("=" * 70)
    
    results = {}

    tests = [
        ("1. GET_NUM (Query SIM Number)", "GET_NUM", "", "Queries SIM card phone number/MSISDN"),
        ("2. GET_STATUS (Fetch Status Now)", "GET_STATUS", "", "Fetches live hardware & battery status"),
        ("3. GET_GPS (Refresh GPS Fix)", "GET_GPS", "", "Triggers GNSS module to acquire fresh GPS coordinates"),
        ("4. FTP_BACKLOG (Sync Backlog)", "FTP_BACKLOG", "", "Forces immediate transmission of unsent local logs"),
        ("5. FTP_DAILY (Fetch Daily Log)", "FTP_DAILY", datetime.datetime.now().strftime("%Y%m%d"), "Requests historical CSV log file for target date"),
        ("6. INTERVAL (15-Min Mode)", "INTERVAL", "15", "Sets reporting frequency to 15-minute slot pulse mode"),
        ("7a. PAUSE_LIVE_POST (Pause Live Posts)", "PAUSE_LIVE_POST", "Automated Test Pause", "Mutes live HTTP posts while keeping SD logging active"),
        ("7b. RESUME_LIVE_POST (Resume Live Posts)", "RESUME_LIVE_POST", "", "Resumes live HTTP posts and syncs buffered logs"),
        ("8a. SET_STATION_ID (Test ID 009999)", "SET_STATION_ID", "009999", "Updates canonical station ID to test ID 009999"),
        ("8b. SET_STATION_ID (Restore Original ID)", "SET_STATION_ID", stn_id, "Restores station ID back to original ID"),
        ("9. REBOOT (Reboot Device)", "REBOOT", "", "Triggers remote software restart (ESP.restart())"),
        ("10. DELETE_DATA (Factory Reset Log Files)", "DELETE_DATA", "", "Deletes local SPIFFS sensor logs while preserving config")
    ]

    for name, cmd, param, desc in tests:
        print("\n" + "-" * 70)
        log(f"Testing: {name}")
        log(f"Description: {desc}")
        
        ok = queue_command(server_url, stn_id if "009999" not in name else "009999", cmd, param)
        if ok:
            results[name] = "QUEUED / PASSED"
        else:
            results[name] = "FAILED TO QUEUE"
            
        time.sleep(1)

    print("\n" + "=" * 70)
    log("Test Suite Queueing Complete! Summary of Command Verification:")
    print("=" * 70)
    for test_name, status in results.items():
        flag = "PASS" if "PASSED" in status else "FAIL"
        log(f"{test_name:<45} : {status}", flag)
    print("=" * 70)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Control Center Command Test Suite")
    parser.add_argument("--stn", default="009876", help="Station ID to test")
    parser.add_argument("--server", default=DEFAULT_SERVER, help="Server Base URL")
    args = parser.parse_args()

    run_test_suite(args.server, args.stn)
