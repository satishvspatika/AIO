#!/usr/bin/env python3
"""
Email Release Package Script via Gmail SMTP
Sends release ZIP and notes to production team
"""

import os
import sys
import smtplib
import getpass
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText
from email.mime.base import MIMEBase
from email import encoders
from datetime import datetime

def send_release_email(version, zip_file, release_notes_file, summary):
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
PACKAGE CONTENTS
============================================================

The attached ZIP file contains 6 pre-compiled configurations:
- KSNDMC_TRG, BIHAR_TRG, SPATIKA_TRG (SYSTEM 0)
- KSNDMC_TWS (SYSTEM 1)
- KSNDMC_ADDON, SPATIKA_ADDON (SYSTEM 2)

============================================================
DEPLOYMENT
============================================================

1. Extract ZIP.
2. Choose config folder.
3. Flash firmware.bin using esptool or Arduino.

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
        print("Usage: python3 send_release_email.py <version> <zip_file> <release_notes> [summary]")
        sys.exit(1)
    
    version = sys.argv[1]
    zip_file = sys.argv[2]
    release_notes = sys.argv[3]
    summary = sys.argv[4] if len(sys.argv) > 4 else "New Release"
    
    success = send_release_email(version, zip_file, release_notes, summary)
    sys.exit(0 if success else 1)
