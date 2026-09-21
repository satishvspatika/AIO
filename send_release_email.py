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

def build_settings_table(release_dir, pkg_filter=None):
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
            cfg    = m.get('config', meta_file.parent.name)
            nuv    = m.get('use_nuvoton_ui')

            if pkg_filter in ["nuvoton", "nuv"] and not nuv:
                continue
            if pkg_filter in ["matrix", "mat"] and nuv:
                continue

            found = True
            flash  = m.get('flash_size', '?')
            label  = f"{cfg}_{flash}"
            debug  = yn(m.get('debug'))
            wsrv   = yn(m.get('enable_webserver'))
            hrpt   = yn(m.get('enable_health_report'))
            rf     = f"{m.get('rf_resolution_mm','--')} mm"
            sz_b   = m.get('binary_size_bytes', 0)
            sz_mb  = f"{sz_b/(1024*1024):.2f}" if sz_b else "--"
            lines.append(f"  {label:<22} {debug:<6} {wsrv:<7} {yn(nuv):<8} {hrpt:<10} {rf:>7} {sz_mb:>7}")
        except Exception:
            continue

    if not found:
        lines.append("  (No metadata.json files found in release directory matching filter)")

    lines.append("=" * 66)
    lines.append("")
    lines.append("  NOTE: These settings were FORCED by the build script regardless")
    lines.append("  of what was set in user_config.h at the time. DEBUG is always")
    lines.append("  set to 0 for production builds. WebServer is disabled on 4MB.")
    lines.append("=" * 66)
    return "\n".join(lines)

def send_release_email(version, zip_file, release_notes_file, summary, release_dir=None, recipient_emails=None, factory_zip_file=None, pkg_choice="both"):
    # Email configuration
    SENDER_EMAIL = "satishv.spatika@gmail.com"
    if recipient_emails:
        if isinstance(recipient_emails, str):
            TO_EMAILS = [e.strip() for e in recipient_emails.split(",") if e.strip()]
        else:
            TO_EMAILS = recipient_emails
        CC_EMAILS = []
    else:
        TO_EMAILS = ["satishv.spatika@gmail.com"]
        CC_EMAILS = []
    
    # Discover built configurations dynamically for the subject line
    built_configs = []
    if release_dir:
        release_path = Path(release_dir)
        for meta_file in sorted(release_path.rglob("metadata.json")):
            try:
                with open(meta_file) as f:
                    m = json.load(f)
                cfg = m.get('config', meta_file.parent.name)
                nuv = m.get('use_nuvoton_ui')
                if pkg_choice in ["nuvoton", "nuv"] and not nuv:
                    continue
                if pkg_choice in ["matrix", "mat"] and nuv:
                    continue
                ui_lbl = "NUV" if nuv else "MAT"
                built_configs.append(f"{cfg}_{ui_lbl}")
            except Exception:
                continue

    pkg_label_hdr = f" [{pkg_choice.upper()} PACKAGE]" if pkg_choice and pkg_choice != "both" else ""
    if built_configs:
        config_summary = ", ".join(built_configs)
        if len(config_summary) > 50:
            config_summary = f"{len(built_configs)} configs"
        SUBJECT = f"AIO9_5.0 Firmware Release v{version}{pkg_label_hdr} ({config_summary}) - {summary}"
    else:
        SUBJECT = f"AIO9_5.0 Firmware Release v{version}{pkg_label_hdr} - {summary}"

    print(f"\n📧 Preparing Release Email...")
    print(f"   From: {SENDER_EMAIL}")
    print(f"   To: {', '.join(TO_EMAILS)}")
    if CC_EMAILS:
        print(f"   CC: {', '.join(CC_EMAILS)}")
    print(f"   Package Choice: {pkg_choice.upper()}")
    print(f"   Subject: {SUBJECT}")

    # Create message container
    msg = MIMEMultipart()
    msg['From'] = SENDER_EMAIL
    msg['To'] = ", ".join(TO_EMAILS)
    if CC_EMAILS:
        msg['Cc'] = ", ".join(CC_EMAILS)
    msg['Subject'] = SUBJECT

    # Read release notes content
    try:
        with open(release_notes_file, 'r') as f:
            release_notes_md = f.read()
    except Exception as e:
        print(f"⚠️ Could not read release notes: {e}")
        release_notes_md = f"Release v{version}\n\nSummary: {summary}"

    # Build per-config settings table with package filter
    settings_table = build_settings_table(release_dir, pkg_filter=pkg_choice)

    # Determine which zip files to attach based on pkg_choice
    pkg_choice_lower = (pkg_choice or "both").lower()
    parent_dir = Path(release_dir).parent if release_dir else Path(zip_file).parent
    
    zip_files_to_attach = []
    nuv_zip = parent_dir / f"AIO9_v{version}_NUVOTON.zip"
    mat_zip = parent_dir / f"AIO9_v{version}_MATRIX.zip"
    comb_zip = parent_dir / f"AIO9_v{version}.zip"

    if pkg_choice_lower in ["nuvoton", "nuv", "n"]:
        if nuv_zip.exists(): zip_files_to_attach.append(str(nuv_zip))
        elif os.path.exists(zip_file): zip_files_to_attach.append(zip_file)
    elif pkg_choice_lower in ["matrix", "mat", "m"]:
        if mat_zip.exists(): zip_files_to_attach.append(str(mat_zip))
        elif os.path.exists(zip_file): zip_files_to_attach.append(zip_file)
    else: # "both"
        if nuv_zip.exists() and mat_zip.exists():
            zip_files_to_attach.extend([str(nuv_zip), str(mat_zip)])
        elif comb_zip.exists():
            zip_files_to_attach.append(str(comb_zip))
        elif os.path.exists(zip_file):
            zip_files_to_attach.append(zip_file)

    attached_names = [os.path.basename(z) for z in zip_files_to_attach]

    # Email Body
    body = f"""Hello Team,

A new firmware release is ready for deployment.

VERSION: v{version}
DATE: {datetime.now().strftime("%B %d, %Y")}
PACKAGE TYPE: {pkg_choice.upper()}
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

The attached ZIP file(s) ({', '.join(attached_names)}) contain the pre-compiled configurations listed in the compile-time settings table above.

Each config folder contains:
  firmware.bin     — pre-compiled binary (flash at offset 0x10000)
  fw_version.txt   — full version string (e.g. TRG9-DMC-6.49-N)
  metadata.json    — machine-readable compile settings

Additionally, for new board factory flashing, the standalone package (AIO9_Factory_Flash_Files.zip) contains:
  bootloader.bin, partitions.bin, boot_app0.bin, flash scripts, and FACTORY_FLASH_GUIDE.md.

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

    # Attach selected Release ZIP file(s)
    for zf in zip_files_to_attach:
        if os.path.exists(zf):
            print(f"   📎 Attaching Release ZIP: {os.path.basename(zf)} ({os.path.getsize(zf)/(1024*1024):.2f} MB)")
            with open(zf, "rb") as attachment:
                part = MIMEBase("application", "octet-stream")
                part.set_payload(attachment.read())
            encoders.encode_base64(part)
            part.add_header("Content-Disposition", f"attachment; filename={os.path.basename(zf)}")
            msg.attach(part)
        else:
            print(f"❌ Error: ZIP file not found at {zf}")
            return False

    # Attach Factory Flash Files ZIP
    factory_zip = factory_zip_file
    if not factory_zip and release_dir:
        candidate = Path(release_dir).parent / "AIO9_Factory_Flash_Files.zip"
        if candidate.exists():
            factory_zip = str(candidate)

    if factory_zip and os.path.exists(factory_zip):
        print(f"   📎 Attaching Factory Flash ZIP: {os.path.basename(factory_zip)} ({os.path.getsize(factory_zip)/(1024*1024):.2f} MB)")
        with open(factory_zip, "rb") as attachment:
            part = MIMEBase("application", "octet-stream")
            part.set_payload(attachment.read())
        encoders.encode_base64(part)
        part.add_header("Content-Disposition", f"attachment; filename={os.path.basename(factory_zip)}")
        msg.attach(part)
    elif factory_zip:
        print(f"⚠️ Warning: Specified factory ZIP file not found at {factory_zip}")

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
    import argparse
    parser = argparse.ArgumentParser(description="Send AIO9 Release Email")
    parser.add_argument("version", help="Firmware version string")
    parser.add_argument("zip_file", help="Path to main release ZIP file")
    parser.add_argument("release_notes", help="Path to release notes file")
    parser.add_argument("summary", nargs="?", default="New Release", help="Release summary")
    parser.add_argument("release_dir", nargs="?", default=None, help="Release directory")
    parser.add_argument("recipients", nargs="?", default=None, help="Recipient emails")
    parser.add_argument("factory_zip", nargs="?", default=None, help="Factory ZIP file")
    parser.add_argument("--pkg", choices=["nuvoton", "matrix", "both", "nuv", "mat"], default="both", help="Package selection to attach (nuvoton, matrix, or both)")

    args = parser.parse_args()

    success = send_release_email(
        version=args.version,
        zip_file=args.zip_file,
        release_notes_file=args.release_notes,
        summary=args.summary,
        release_dir=args.release_dir,
        recipient_emails=args.recipients,
        factory_zip_file=args.factory_zip,
        pkg_choice=args.pkg
    )
    sys.exit(0 if success else 1)
