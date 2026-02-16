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
    
    # Add sender to CC so they get a copy in their inbox
    if sender_email not in CC:
        CC.append(sender_email)
    
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
                f'attachment; filename=AIO9_v{version}.zip'
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
    
    # Send email - Create draft in Mail.app for user to send
    print(f"\n📤 Creating email draft in Mail.app...")
    
    try:
        # Create mailto URL with all parameters
        import urllib.parse
        
        all_recipients = TO + CC
        recipients_str = ",".join(all_recipients)
        
        # Create email body file
        body_file = f"/tmp/release_email_v{version}_body.txt"
        with open(body_file, 'w') as f:
            f.write(body)
        
        # Save attachments info
        attachments_file = f"/tmp/release_email_v{version}_attachments.txt"
        with open(attachments_file, 'w') as f:
            f.write(f"ZIP: {zip_file}\n")
            f.write(f"Notes: {release_notes_file}\n")
        
        print(f"✅ Email draft prepared")
        print(f"\n{'='*60}")
        print(f"📧 MANUAL STEP REQUIRED")
        print(f"{'='*60}")
        print(f"\nPlease send the email manually:")
        print(f"\n1. Open Mail.app")
        print(f"2. Create new email with:")
        print(f"   To: {', '.join(TO)}")
        print(f"   CC: {', '.join(CC)}")
        print(f"   Subject: {SUBJECT}")
        print(f"\n3. Attach files:")
        print(f"   - {zip_file}")
        print(f"   - {release_notes_file}")
        print(f"\n4. Copy email body from:")
        print(f"   {body_file}")
        print(f"\n5. Send the email")
        print(f"\n{'='*60}")
        
        # Try to open Mail.app with pre-filled data
        import subprocess
        
        # Encode subject and body for mailto URL
        subject_encoded = urllib.parse.quote(SUBJECT)
        body_preview = "Release package attached. See full details in email body file."
        body_encoded = urllib.parse.quote(body_preview)
        
        mailto_url = f"mailto:{recipients_str}?subject={subject_encoded}&body={body_encoded}"
        
        # Open mailto URL (will open Mail.app)
        subprocess.run(['open', mailto_url], check=False)
        
        print(f"\n✅ Mail.app opened with pre-filled email")
        print(f"⚠️  Remember to:")
        print(f"   1. Attach the ZIP file: {os.path.basename(zip_file)}")
        print(f"   2. Attach release notes: {os.path.basename(release_notes_file)}")
        print(f"   3. Replace body with content from: {body_file}")
        print(f"   4. Add CC: {', '.join(CC)}")
        
        return True
        
    except Exception as e:
        print(f"⚠️  Could not open Mail.app: {e}")
        print(f"\n📋 Email details saved to:")
        print(f"   Body: {body_file}")
        print(f"   Attachments: {attachments_file}")
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
