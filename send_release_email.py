#!/usr/bin/env python3
"""
Email Release Package Script via Gmail SMTP
Sends release ZIP and notes to production team
"""

import os
import sys
import json
import smtplib
import getpass
from pathlib import Path
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText
from email.mime.base import MIMEBase
from email import encoders
from datetime import datetime

def build_settings_table(release_dir):
    """Scan release_dir for per-config metadata.json files and return
    a formatted plain-text settings table for inclusion in the email."""
    release_path = Path(release_dir) if release_dir else None
    if not release_path or not release_path.exists():
        return ""

    def yn(v):
        if v is None: return "--"
        return "YES" if v else "NO"

    lines = []
    lines.append("=" * 66)
    lines.append("  COMPILE-TIME SETTINGS PER CONFIGURATION")
    lines.append("=" * 66)
    lines.append(f"  {'Config':<22} {'Debug':<6} {'WebSrv':<7} {'Nuvoton':<8} {'HealthRpt':<10} {'RF Res':>7} {'Size MB':>8}")
    lines.append(f"  {'-'*22} {'-'*5} {'-'*6} {'-'*7} {'-'*9} {'-'*7} {'-'*7}")

    found = False
    for meta_file in sorted(release_path.rglob("metadata.json")):
        try:
            with open(meta_file) as f:
                m = json.load(f)
            found = True
            cfg    = m.get('config', meta_file.parent.name)
            flash  = m.get('flash_size', '?')
            label  = f"{cfg}_{flash}"
            debug  = yn(m.get('debug'))
            wsrv   = yn(m.get('enable_webserver'))
            nuv    = yn(m.get('use_nuvoton_ui'))
            hrpt   = yn(m.get('enable_health_report'))
            rf     = f"{m.get('rf_resolution_mm','--')} mm"
            sz_b   = m.get('binary_size_bytes', 0)
            sz_mb  = f"{sz_b/(1024*1024):.2f}" if sz_b else "--"
            lines.append(f"  {label:<22} {debug:<6} {wsrv:<7} {nuv:<8} {hrpt:<10} {rf:>7} {sz_mb:>7}"
            )
        except Exception:
            continue

    if not found:
        lines.append("  (No metadata.json files found in release directory)")

    lines.append("=" * 66)
    lines.append("")
    lines.append("  NOTE: These settings were FORCED by the build script regardless")
    lines.append("  of what was set in user_config.h at the time. DEBUG is always")
    lines.append("  set to 0 for production builds. WebServer is disabled on 4MB.")
    lines.append("=" * 66)
    return "\n".join(lines)

def send_release_email(version, zip_file, release_notes_file, summary, release_dir=None):
    # Email configuration
    SENDER_EMAIL = "satishv.spatika@gmail.com"
    TO_EMAILS = ["production.spatika@gmail.com", "rajesh.spatika@gmail.com"]
    CC_EMAILS = ["ssraghavan.spatika@gmail.com", SENDER_EMAIL]
    SUBJECT = f"AIO9_5.0 Firmware Release v{version} - {summary}"

    print(f"\n📧 Preparing Release Email...")
    print(f"   From: {SENDER_EMAIL}")
    print(f"   To: {', '.join(TO_EMAILS)}")
    print(f"   CC: {', '.join(CC_EMAILS)}")
    print(f"   Subject: {SUBJECT}")

    # Create message container
    msg = MIMEMultipart()
    msg['From'] = SENDER_EMAIL
    msg['To'] = ", ".join(TO_EMAILS)
    msg['Cc'] = ", ".join(CC_EMAILS)
    msg['Subject'] = SUBJECT

    # Read release notes content
    try:
        with open(release_notes_file, 'r') as f:
            release_notes_md = f.read()
    except Exception as e:
        print(f"⚠️ Could not read release notes: {e}")
        release_notes_md = f"Release v{version}\n\nSummary: {summary}"

    # Build per-config settings table
    settings_table = build_settings_table(release_dir)

    # Email Body
    body = f"""Hello Team,

A new firmware release is ready for deployment.

VERSION: v{version}
DATE: {datetime.now().strftime("%B %d, %Y")}
SUMMARY: {summary}

============================================================
RELEASE NOTES
============================================================

{release_notes_md}

============================================================
COMPILE-TIME SETTINGS (What was compiled in/out)
============================================================

{settings_table}

============================================================
PACKAGE CONTENTS
============================================================

The attached ZIP file contains pre-compiled configurations:
- KSNDMC_TRG, BIHAR_TRG, SPATIKA_GEN  (SYSTEM 0 — TRG Rain Only)
- KSNDMC_TWS, KSNDMC_TWS-AP           (SYSTEM 1 — TWS, Skip RF Rain)
- KSNDMC_ADDON, SPATIKA_GEN           (SYSTEM 2 — TWS-RF All Tests)

Each config folder contains:
  firmware.bin     — pre-compiled binary (flash at offset 0x10000)
  fw_version.txt   — full version string (e.g. TRG9-DMC-6.07)
  metadata.json    — machine-readable compile settings

============================================================
DEPLOYMENT
============================================================

1. Extract ZIP.
2. Open factory_tool.html in Chrome via local HTTP server.
3. Select the device Profile (TRG / TWS / TWS-RF) and Config.
4. Select the release folder — build info will be shown automatically.
5. Connect the board and click Start Programming.

Best regards,
Spatika AIO Release Automation
"""
    msg.attach(MIMEText(body, 'plain'))

    # Attach ZIP file
    if os.path.exists(zip_file):
        print(f"   📎 Attaching ZIP: {os.path.basename(zip_file)} ({os.path.getsize(zip_file)/(1024*1024):.2f} MB)")
        with open(zip_file, "rb") as attachment:
            part = MIMEBase("application", "octet-stream")
            part.set_payload(attachment.read())
        encoders.encode_base64(part)
        part.add_header("Content-Disposition", f"attachment; filename={os.path.basename(zip_file)}")
        msg.attach(part)
    else:
        print(f"❌ Error: ZIP file not found at {zip_file}")
        return False

    # Attach Release Notes MD
    if os.path.exists(release_notes_file):
        print(f"   📎 Attaching MD: {os.path.basename(release_notes_file)}")
        with open(release_notes_file, "rb") as attachment:
            part = MIMEBase("application", "octet-stream")
            part.set_payload(attachment.read())
        encoders.encode_base64(part)
        part.add_header("Content-Disposition", f"attachment; filename={os.path.basename(release_notes_file)}")
        msg.attach(part)

    # SMTP Credentials
    print(f"\n🔑 Authenticating...")
    
    # Try to fetch from macOS Keychain first
    password = None
    try:
        import subprocess
        password = subprocess.check_output(
            ["security", "find-generic-password", "-a", SENDER_EMAIL, "-s", "AIO_RELEASE_SMTP", "-w"],
            text=True
        ).strip()
        print(f"   ✅ Authorized via macOS Keychain")
    except:
        print(f"   ℹ️ No password found in Keychain. Using manual prompt.")
        print("   Note: Use a Google 'App Password' from myaccount.google.com/apppasswords")
        password = getpass.getpass(f"   Enter App Password for {SENDER_EMAIL}: ")

    if not password:
        print("❌ Error: Password cannot be empty.")
        return False

    try:
        print(f"\n📤 Connecting to Gmail SMTP...")
        server = smtplib.SMTP("smtp.gmail.com", 587)
        server.starttls()
        server.login(SENDER_EMAIL, password)
        
        print(f"🚀 Sending email...")
        all_recipients = TO_EMAILS + CC_EMAILS
        server.sendmail(SENDER_EMAIL, all_recipients, msg.as_string())
        server.quit()
        
        print(f"\n✅ SUCCESS: Email sent successfully!")
        print(f"   Check your 'Sent' folder at {SENDER_EMAIL}")
        return True
    except Exception as e:
        print(f"\n❌ FAILED to send email: {e}")
        return False

if __name__ == "__main__":
    if len(sys.argv) < 4:
        print("Usage: python3 send_release_email.py <version> <zip_file> <release_notes> [summary] [release_dir]")
        sys.exit(1)

    version       = sys.argv[1]
    zip_file      = sys.argv[2]
    release_notes = sys.argv[3]
    summary       = sys.argv[4] if len(sys.argv) > 4 else "New Release"
    release_dir   = sys.argv[5] if len(sys.argv) > 5 else None

    success = send_release_email(version, zip_file, release_notes, summary, release_dir)
    sys.exit(0 if success else 1)
