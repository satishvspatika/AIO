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
    lines.append("=" * 115)
    lines.append("  COMPILE-TIME SETTINGS PER CONFIGURATION")
    lines.append("=" * 115)
    lines.append(f"  {'ZIP Folder Name':<26} {'Configuration':<16} {'Flash':<6} {'UI Display':<10} {'Debug':<6} {'WebSrv':<7} {'Health Report Freq':<24} {'RF Res':>7} {'Size':>7}")
    lines.append(f"  {'-'*26} {'-'*16} {'-'*6} {'-'*10} {'-'*5} {'-'*6} {'-'*24} {'-'*7} {'-'*7}")

    found = False
    for meta_file in sorted(release_path.rglob("metadata.json")):
        try:
            with open(meta_file) as f:
                m = json.load(f)
            cfg        = m.get('config', meta_file.parent.name)
            folder_name= meta_file.parent.name
            nuv        = m.get('use_nuvoton_ui')

            if pkg_filter in ["nuvoton", "nuv"] and not nuv:
                continue
            if pkg_filter in ["matrix", "mat"] and nuv:
                continue

            found = True
            flash  = m.get('flash_size', '?').upper()
            ui_lbl = "Nuvoton" if nuv else "Matrix"
            debug  = yn(m.get('debug'))
            wsrv   = yn(m.get('enable_webserver'))
            hrpt_freq = "Twice Daily (1am & 1pm)" if m.get('enable_health_report') else "Disabled"
            rf     = f"{m.get('rf_resolution_mm','--')} mm"
            sz_b   = m.get('binary_size_bytes', 0)
            sz_mb  = f"{sz_b/(1024*1024):.2f} MB" if sz_b else "--"
            lines.append(f"  {folder_name:<26} {cfg:<16} {flash:<6} {ui_lbl:<10} {debug:<6} {wsrv:<7} {hrpt_freq:<24} {rf:>7} {sz_mb:>7}")
        except Exception:
            continue

    if not found:
        lines.append("  (No metadata.json files found in release directory matching filter)")

    lines.append("=" * 115)
    lines.append("")
    lines.append("  NOTE: Health Report frequency is set to Twice Daily (1:00 AM & 1:00 PM)")
    lines.append("  for all production builds to save battery. Switchable to 15-min pulse")
    lines.append("  remotely anytime via server command INTERVAL?param=15.")
    lines.append("=" * 115)
    return "\n".join(lines)

def build_settings_html_table(release_dir, pkg_filter=None):
    """Scan release_dir for per-config metadata.json files and return a clean HTML table."""
    release_path = Path(release_dir) if release_dir else None
    if not release_path or not release_path.exists():
        return ""

    def yn(v):
        if v is None: return "--"
        return "YES" if v else "NO"

    rows = []
    for meta_file in sorted(release_path.rglob("metadata.json")):
        try:
            with open(meta_file) as f:
                m = json.load(f)
            cfg        = m.get('config', meta_file.parent.name)
            folder_name= meta_file.parent.name
            nuv        = m.get('use_nuvoton_ui')

            if pkg_filter in ["nuvoton", "nuv"] and not nuv:
                continue
            if pkg_filter in ["matrix", "mat"] and nuv:
                continue

            flash  = m.get('flash_size', '?').upper()
            ui_lbl = "Nuvoton UART" if nuv else "Matrix I2C"
            debug  = yn(m.get('debug'))
            wsrv   = yn(m.get('enable_webserver'))
            hrpt_freq = "Twice Daily (1am & 1pm)" if m.get('enable_health_report') else "Disabled"
            rf     = f"{m.get('rf_resolution_mm','--')} mm"
            sz_b   = m.get('binary_size_bytes', 0)
            sz_mb  = f"{sz_b/(1024*1024):.2f} MB" if sz_b else "--"

            rows.append(f"""
            <tr style="border-bottom: 1px solid #e2e8f0; background-color: #ffffff;">
                <td style="padding: 10px; font-family: monospace; font-weight: bold; color: #1a202c;">{folder_name}</td>
                <td style="padding: 10px; color: #2d3748;">{cfg}</td>
                <td style="padding: 10px; text-align: center; color: #2d3748;">{flash}</td>
                <td style="padding: 10px; color: #2d3748;">{ui_lbl}</td>
                <td style="padding: 10px; text-align: center; color: #e53e3e;">{debug}</td>
                <td style="padding: 10px; text-align: center; color: #38a169;">{wsrv}</td>
                <td style="padding: 10px; color: #2b6cb0; font-weight: 500;">{hrpt_freq}</td>
                <td style="padding: 10px; text-align: right; color: #2d3748;">{rf}</td>
                <td style="padding: 10px; text-align: right; color: #2d3748; font-family: monospace;">{sz_mb}</td>
            </tr>""")
        except Exception:
            continue

    if not rows:
        return "<p>(No metadata files found matching filter)</p>"

    table_html = f"""
    <table style="border-collapse: collapse; width: 100%; font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif; font-size: 13px; margin: 15px 0; border: 1px solid #cbd5e0; border-radius: 6px; overflow: hidden;">
        <thead>
            <tr style="background-color: #1a365d; color: #ffffff; text-align: left; font-size: 12px; text-transform: uppercase; letter-spacing: 0.5px;">
                <th style="padding: 12px 10px;">ZIP Folder Name</th>
                <th style="padding: 12px 10px;">Configuration</th>
                <th style="padding: 12px 10px; text-align: center;">Flash</th>
                <th style="padding: 12px 10px;">UI Display</th>
                <th style="padding: 12px 10px; text-align: center;">Debug</th>
                <th style="padding: 12px 10px; text-align: center;">WebSrv</th>
                <th style="padding: 12px 10px;">Health Report Freq</th>
                <th style="padding: 12px 10px; text-align: right;">RF Res</th>
                <th style="padding: 12px 10px; text-align: right;">Size</th>
            </tr>
        </thead>
        <tbody>
            {''.join(rows)}
        </tbody>
    </table>
    """
    return table_html

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
    msg = MIMEMultipart('mixed')
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
    settings_html  = build_settings_html_table(release_dir, pkg_filter=pkg_choice)

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

    # Alternative part for plain text + HTML
    msg_alt = MIMEMultipart('alternative')

    # Plain Text Body
    plain_body = f"""Hello Team,

A new firmware release is ready for deployment.

VERSION: v{version}
DATE: {datetime.now().strftime("%B %d, %Y")}
PACKAGE TYPE: {pkg_choice.upper()}
SUMMARY: {summary}

===========================================================================================================
RELEASE NOTES
===========================================================================================================

{release_notes_md}

===========================================================================================================
COMPILE-TIME SETTINGS (What was compiled in/out)
===========================================================================================================

{settings_table}

===========================================================================================================
PACKAGE CONTENTS
===========================================================================================================

The attached ZIP file(s) ({', '.join(attached_names)}) contain the pre-compiled configurations listed in the table above.

Each config folder contains:
  firmware.bin     — pre-compiled binary (flash at offset 0x10000)
  fw_version.txt   — full version string (e.g. TRG9-DMC-6.51-N)
  metadata.json    — machine-readable compile settings

Additionally, for new board factory flashing, the standalone package (AIO9_Factory_Flash_Files.zip) contains:
  bootloader.bin, partitions.bin, boot_app0.bin, flash scripts, and FACTORY_FLASH_GUIDE.md.

Best regards,
Spatika AIO Release Automation
"""

    # HTML Body with Styled Table
    # Escape basic Markdown headings in release_notes_md for simple HTML view
    rn_html = release_notes_md.replace("\n# ", "\n<h1>").replace("\n## ", "\n<h2>").replace("\n### ", "\n<h3>").replace("\n", "<br>")

    html_body = f"""
    <!DOCTYPE html>
    <html>
    <head>
        <meta charset="utf-8">
        <style>
            body {{ font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Helvetica, Arial, sans-serif; color: #2d3748; line-height: 1.6; max-width: 900px; margin: 0 auto; padding: 20px; }}
            .header {{ background-color: #1a365d; color: white; padding: 20px; border-radius: 8px; margin-bottom: 25px; }}
            .header h1 {{ margin: 0; font-size: 24px; }}
            .meta {{ font-size: 14px; margin-top: 10px; color: #e2e8f0; }}
            .section {{ margin-bottom: 30px; background: #f7fafc; padding: 20px; border-radius: 8px; border: 1px solid #e2e8f0; }}
            .section-title {{ font-size: 18px; font-weight: bold; color: #2b6cb0; border-bottom: 2px solid #2b6cb0; padding-bottom: 8px; margin-top: 0; margin-bottom: 15px; }}
            .note-box {{ background-color: #ebf8ff; border-left: 4px solid #3182ce; padding: 12px; font-size: 13px; color: #2c5282; margin: 10px 0; border-radius: 4px; }}
            ul {{ padding-left: 20px; }}
            code {{ font-family: monospace; background: #edf2f7; padding: 2px 6px; border-radius: 4px; font-size: 13px; }}
        </style>
    </head>
    <body>
        <div class="header">
            <h1>AIO9_5.0 Firmware Release v{version}</h1>
            <div class="meta">
                <strong>Date:</strong> {datetime.now().strftime("%B %d, %Y")} | 
                <strong>Package:</strong> {pkg_choice.upper()} | 
                <strong>Summary:</strong> {summary}
            </div>
        </div>

        <div class="section">
            <h2 class="section-title">📋 Compile-Time Settings per Configuration</h2>
            {settings_html}
            <div class="note-box">
                ℹ️ <strong>Health Report Frequency:</strong> Set to <strong>Twice Daily (1:00 AM & 1:00 PM)</strong> for all production builds to maximize battery lifetime. Switchable to 15-minute pulse mode remotely anytime via server command <code>INTERVAL?param=15</code>.
            </div>
        </div>

        <div class="section">
            <h2 class="section-title">📝 Release Notes</h2>
            <div style="background: white; padding: 15px; border-radius: 6px; border: 1px solid #edf2f7;">
                {rn_html}
            </div>
        </div>

        <div class="section">
            <h2 class="section-title">📦 Package Contents & Deployment</h2>
            <p>The attached ZIP files (<code>{', '.join(attached_names)}</code>) contain the pre-compiled binaries listed above.</p>
            <ul>
                <li><code>firmware.bin</code> — Application binary (Flash offset: 0x10000)</li>
                <li><code>fw_version.txt</code> — Firmware identity string</li>
                <li><code>metadata.json</code> — Machine-readable build parameters</li>
            </ul>
            <p><strong>Factory Flashing:</strong> Use <code>AIO9_Factory_Flash_Files.zip</code> containing bootloader binaries, 16MB partition tables, and <code>flash_fresh_board*.sh</code> scripts.</p>
        </div>

        <p style="font-size: 12px; color: #718096; margin-top: 30px;">Sent automatically by Spatika AIO Release Automation System.</p>
    </body>
    </html>
    """

    msg_alt.attach(MIMEText(plain_body, 'plain'))
    msg_alt.attach(MIMEText(html_body, 'html'))
    msg.attach(msg_alt)

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
