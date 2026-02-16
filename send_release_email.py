#!/usr/bin/env python3
"""
Email Release Package Script
Sends release ZIP and notes to production team
"""

import os
import sys
import smtplib
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText
from email.mime.base import MIMEBase
from email import encoders
from pathlib import Path
from datetime import datetime

def send_release_email(version, zip_file, release_notes_file, summary):
    """
    Send release package via email
    
    Args:
        version: Version string (e.g., "5.31")
        zip_file: Path to ZIP file
        release_notes_file: Path to release notes markdown
        summary: Brief summary of release
    """
    
    # Email configuration
    TO = ["production.spatika@gmail.com", "rajesh.spatika@gmail.com"]
    CC = ["ssraghavan.spatika@gmail.com"]
    SUBJECT = f"AIO9_5.0 Firmware Release v{version} - {summary}"
    
    # Get sender email from git config
    try:
        import subprocess
        sender_email = subprocess.check_output(
            ["git", "config", "user.email"], 
            text=True
        ).strip()
        sender_name = subprocess.check_output(
            ["git", "config", "user.name"], 
            text=True
        ).strip()
    except:
        sender_email = "noreply@spatika.com"
        sender_name = "AIO Release Bot"
    
    print(f"\n📧 Preparing email...")
    print(f"   From: {sender_name} <{sender_email}>")
    print(f"   To: {', '.join(TO)}")
    print(f"   CC: {', '.join(CC)}")
    print(f"   Subject: {SUBJECT}")
    
    # Create message
    msg = MIMEMultipart()
    msg['From'] = f"{sender_name} <{sender_email}>"
    msg['To'] = ", ".join(TO)
    msg['Cc'] = ", ".join(CC)
    msg['Subject'] = SUBJECT
    msg['Date'] = datetime.now().strftime("%a, %d %b %Y %H:%M:%S %z")
    
    # Read release notes
    try:
        with open(release_notes_file, 'r') as f:
            release_notes_md = f.read()
    except:
        release_notes_md = f"Release v{version}\n\n{summary}"
    
    # Convert markdown to plain text for email body
    # Remove markdown formatting for plain text email
    release_notes_text = release_notes_md
    release_notes_text = release_notes_text.replace('#', '')
    release_notes_text = release_notes_text.replace('**', '')
    release_notes_text = release_notes_text.replace('`', '')
    
    # Email body
    body = f"""
Hello Team,

A new firmware release is ready for deployment:

Version: v{version}
Release Date: {datetime.now().strftime("%B %d, %Y")}
Summary: {summary}

{'='*60}
RELEASE NOTES
{'='*60}

{release_notes_text}

{'='*60}
PACKAGE CONTENTS
{'='*60}

The attached ZIP file contains:
- 6 pre-compiled firmware configurations:
  • KSNDMC_TRG (Rain Gauge)
  • KSNDMC_TWS (Weather Station)
  • BIHAR_TRG (Bihar Rain Gauge)
  • SPATIKA_TRG (Spatika Rain Gauge)
  • KSNDMC_ADDON (Add-on Config)
  • SPATIKA_ADDON (Spatika Add-on)

- Flash files (bootloader, partitions)
- Complete release notes (RELEASE_NOTES.md)

{'='*60}
DEPLOYMENT INSTRUCTIONS
{'='*60}

1. Extract the ZIP file
2. Choose the appropriate configuration folder
3. Flash using esptool.py or Arduino IDE
4. Verify firmware version on device

For detailed instructions, see RELEASE_NOTES.md in the ZIP.

{'='*60}

Best regards,
{sender_name}
AIO9_5.0 Release Automation

---
This is an automated release notification.
For questions, contact the development team.
"""
    
    msg.attach(MIMEText(body, 'plain'))
    
    # Attach ZIP file
    if os.path.exists(zip_file):
        print(f"   Attaching: {os.path.basename(zip_file)} ({os.path.getsize(zip_file) / (1024*1024):.2f} MB)")
        
        with open(zip_file, 'rb') as f:
            part = MIMEBase('application', 'zip')
            part.set_payload(f.read())
            encoders.encode_base64(part)
            part.add_header(
                'Content-Disposition',
                f'attachment; filename=AIO9_5.0_v{version}.zip'
            )
            msg.attach(part)
    else:
        print(f"   ⚠️  ZIP file not found: {zip_file}")
        return False
    
    # Attach release notes as separate file
    if os.path.exists(release_notes_file):
        print(f"   Attaching: {os.path.basename(release_notes_file)}")
        
        with open(release_notes_file, 'r') as f:
            part = MIMEText(f.read(), 'plain')
            part.add_header(
                'Content-Disposition',
                f'attachment; filename=RELEASE_NOTES_v{version}.md'
            )
            msg.attach(part)
    
    # Send email
    print(f"\n📤 Sending email...")
    
    try:
        # Try to use macOS 'mail' command (works with local mail setup)
        import subprocess
        
        # Save message to temp file
        temp_file = f"/tmp/release_email_v{version}.eml"
        with open(temp_file, 'w') as f:
            f.write(msg.as_string())
        
        # Use macOS mail command
        all_recipients = TO + CC
        cmd = ['mail', '-s', SUBJECT] + all_recipients
        
        with open(temp_file, 'r') as f:
            subprocess.run(cmd, stdin=f, check=True)
        
        print(f"✅ Email sent successfully via macOS mail")
        os.remove(temp_file)
        return True
        
    except Exception as e:
        print(f"⚠️  macOS mail failed: {e}")
        print(f"\n📋 Email prepared but not sent.")
        print(f"   Message saved to: /tmp/release_email_v{version}.eml")
        print(f"\n   To send manually:")
        print(f"   1. Open Mail.app")
        print(f"   2. Attach: {zip_file}")
        print(f"   3. To: {', '.join(TO)}")
        print(f"   4. CC: {', '.join(CC)}")
        print(f"   5. Subject: {SUBJECT}")
        
        # Save email content for manual sending
        with open(f"/tmp/release_email_v{version}.txt", 'w') as f:
            f.write(body)
        
        print(f"   6. Body: /tmp/release_email_v{version}.txt")
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
