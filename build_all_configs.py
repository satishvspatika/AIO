#!/usr/bin/env python3
"""
Automated Build Script for AIO9_5.0
Builds all SYSTEM/UNIT configurations automatically
Requires: arduino-cli (install: brew install arduino-cli)
"""

import os
import sys
import re
import json
import argparse
import subprocess
import shutil
from pathlib import Path
from datetime import datetime

# --- DIRECTORY CONFIGURATION ---
SKETCH_DIR = Path(__file__).parent
USER_CONFIG_H = SKETCH_DIR / "user_config.h"
GLOBALS_H = SKETCH_DIR / "globals.h"
OUTPUT_BASE = SKETCH_DIR / "builds"
BACKUP_CONFIG = SKETCH_DIR / "user_config.h.bak"

# Build configurations: (SYSTEM, UNIT, output_name)
CONFIGS = [
    (0, "KSNDMC_TRG", "KSNDMC_TRG"),
    (0, "BIHAR_TRG", "BIHAR_TRG"),
    (2, "SPATIKA_GEN", "SPATIKA_GEN"),
    (1, "KSNDMC_TWS", "KSNDMC_TWS"),
    (2, "KSNDMC_ADDON", "KSNDMC_ADDON"),
]

# Configs that require Nuvoton UI (USE_NUVOTON_UI = 1) in addition to Matrix UI (USE_NUVOTON_UI = 0)
NUVOTON_UI_CONFIGS = {"KSNDMC_TRG", "KSNDMC_TWS"}


# Flash size variants available:
# Use --flash 4mb or --flash 16mb on CLI to include those variants.
# Default (no flag): 8mb only (current production hardware).
ALL_FLASH_VARIANTS = {
    "4mb":  ("4M",  "partitions_4mb.csv"),   # Legacy / older ESP32 units
    "8mb":  ("8M",  "partitions.csv"),        # Current production hardware
    "16mb": ("16M,FlashFreq=80", "partitions_16mb.csv"),   # 16MB units (80MHz SPI, matches compile.sh)
}

# External Release Path
EXTERNAL_RELEASE_BASE = Path("/Users/satishkripavasan/Documents/Arduino/ESP32_NEW_DESIGN/RELEASE/AIO9_5")

# Colors
class Colors:
    RED = '\033[0;31m'
    GREEN = '\033[0;32m'
    YELLOW = '\033[1;33m'
    BLUE = '\033[0;34m'
    NC = '\033[0m'

def print_header(text):
    print(f"\n{'='*50}")
    print(f"  {text}")
    print(f"{'='*50}\n")

def print_success(text):
    print(f"{Colors.GREEN}✓ {text}{Colors.NC}")

def print_error(text):
    print(f"{Colors.RED}✗ {text}{Colors.NC}")

def print_info(text):
    print(f"{Colors.YELLOW}→ {text}{Colors.NC}")

def backup_config():
    """Backup original user_config.h"""
    print_info("Backing up user_config.h...")
    shutil.copy(USER_CONFIG_H, BACKUP_CONFIG)

def restore_config():
    """Restore original user_config.h"""
    if BACKUP_CONFIG.exists():
        print_info("Restoring original user_config.h...")
        shutil.copy(BACKUP_CONFIG, USER_CONFIG_H)

def update_config(system, unit, disable_webserver=False, use_nuvoton_ui=1):
    """Update user_config.h with new SYSTEM, UNIT, and USE_NUVOTON_UI values"""
    print_info(f"Configuring: SYSTEM={system}, UNIT={unit}, USE_NUVOTON_UI={use_nuvoton_ui}" + (" [WebServer OFF]" if disable_webserver else ""))
    
    # Restore backup first to get a clean slate
    shutil.copy(BACKUP_CONFIG, USER_CONFIG_H)
    
    # Read file
    with open(USER_CONFIG_H, 'r') as f:
        content = f.read()
    
    # Update SYSTEM
    import re
    content = re.sub(r'#define SYSTEM \d+', f'#define SYSTEM {system}', content)
    
    # Update UNIT
    content = re.sub(r'#define UNIT_CFG "[^"]*"', f'#define UNIT_CFG "{unit}"', content)

    # Update USE_NUVOTON_UI
    content = re.sub(r'#define USE_NUVOTON_UI \d+', f'#define USE_NUVOTON_UI {use_nuvoton_ui}', content)

    # Force DEBUG 0 for official builds
    # [BUILD-04] Optional Debug override:
    if "--enable-debug" in sys.argv:
        content = re.sub(r'#define DEBUG \d+', '#define DEBUG 1', content)
        print("  DEBUG set to: 1 (Manual override)")
    else:
        content = re.sub(r'#define DEBUG \d+', '#define DEBUG 0', content)
        print("  DEBUG set to: 0 (Official Production)")

    # Force TEST_HEALTH_DEFAULT 0 for official builds
    content = re.sub(r'#define TEST_HEALTH_DEFAULT \d+', '#define TEST_HEALTH_DEFAULT 0', content)

    # Force ENABLE_HEALTH_REPORT 0 for official builds
    content = re.sub(r'#define ENABLE_HEALTH_REPORT\s+\d+', '#define ENABLE_HEALTH_REPORT 0', content)

    # 4MB builds: disable WebServer
    if disable_webserver:
        content = re.sub(r'#define ENABLE_WEBSERVER \d+', '#define ENABLE_WEBSERVER 0', content)
    
    # Write back
    with open(USER_CONFIG_H, 'w') as f:
        f.write(content)
    
    print(f"  SYSTEM set to: {system}")
    print(f"  UNIT set to: {unit}")
    print(f"  USE_NUVOTON_UI set to: {use_nuvoton_ui}")

def extract_build_defines(config_h_path):
    """Parse key compile-time defines from user_config.h.
    Handles multi-line defines joined with backslash continuations.
    Returns a dict of human-readable settings.
    """
    with open(config_h_path, 'r') as f:
        raw = f.read()

    # Join backslash-continuation lines into single logical lines
    joined = re.sub(r'\\\s*\n\s*', ' ', raw)

    def get_int(name):
        m = re.search(rf'#define {name}\s+(\d+)', joined)
        return int(m.group(1)) if m else None

    def get_float(name):
        m = re.search(rf'#define {name}\s+([0-9.]+)', joined)
        return float(m.group(1)) if m else None

    def get_str(name):
        m = re.search(rf'#define {name}\s+"([^"]+)"', joined)
        return m.group(1) if m else None

    debug_val       = get_int('DEBUG')
    webserver_val   = get_int('ENABLE_WEBSERVER')
    nuvoton_val     = get_int('USE_NUVOTON_UI')
    health_val      = get_int('TEST_HEALTH_DEFAULT')
    health_en_val   = get_int('ENABLE_HEALTH_REPORT')
    rf_res_val      = get_float('DEFAULT_RF_RESOLUTION')
    wind_teeth_val  = get_float('WIND_TEETH_COUNT')
    pressure_val    = get_int('ENABLE_PRESSURE_SENSOR')
    calib_val       = get_int('ENABLE_CALIB_TEST')
    fw_ver_val      = get_str('FIRMWARE_VERSION')

    health_freq_map = {0: 'Daily (11am)', 1: 'Every 15 mins', 2: 'Disabled'}

    return {
        'firmware_version':    fw_ver_val or 'UNKNOWN',
        'debug':               bool(debug_val) if debug_val is not None else None,
        'enable_webserver':    bool(webserver_val) if webserver_val is not None else None,
        'use_nuvoton_ui':      bool(nuvoton_val) if nuvoton_val is not None else None,
        'enable_health_report':bool(health_en_val) if health_en_val is not None else None,
        'health_report_freq':  health_freq_map.get(health_val, 'Unknown') if health_val is not None else None,
        'rf_resolution_mm':    rf_res_val,
        'wind_teeth_count':    wind_teeth_val,
        'enable_pressure_sensor': bool(pressure_val) if pressure_val is not None else None,
        'enable_calib_test':   bool(calib_val) if calib_val is not None else None,
    }

def check_arduino_cli():
    """Check if arduino-cli is installed"""
    try:
        result = subprocess.run(['arduino-cli', 'version'], 
                              capture_output=True, text=True, check=True)
        print_success(f"arduino-cli found: {result.stdout.strip()}")
        return True
    except (subprocess.CalledProcessError, FileNotFoundError):
        print_error("arduino-cli not found!")
        print("Install with: brew install arduino-cli")
        print("Or download from: https://arduino.github.io/arduino-cli/")
        return False

def build_config(system, unit, output_name, flash_size="8mb", flash_fqbn="8M", partition_csv="partitions.csv", use_nuvoton_ui=1):
    """Build a specific configuration for a specific flash hardware and UI variant"""
    # Tagged output name includes UI variant and flash size
    ui_suffix = "NUV" if use_nuvoton_ui == 1 else "MAT"
    tagged_name = f"{output_name}_{ui_suffix}_{flash_size}"
    print_header(f"Building: {tagged_name}")

    # Update user_config.h
    update_config(system, unit, disable_webserver=(flash_size == "4mb"), use_nuvoton_ui=use_nuvoton_ui)


    # Create output directory
    output_dir = OUTPUT_BASE / tagged_name
    output_dir.mkdir(parents=True, exist_ok=True)

    # Unique build directory for this specific config + flash size
    config_build_dir = SKETCH_DIR / "build" / f"config_{tagged_name}"
    config_build_dir.mkdir(parents=True, exist_ok=True)

    # Compile
    print_info(f"Compiling for {flash_size} flash... (Build path: {config_build_dir.name})")
    log_file = output_dir / "build.log"

    partitions_file = SKETCH_DIR / partition_csv

    cmd = [
        'arduino-cli', 'compile',
        '--clean',                                                              # Force fresh compile, no stale cache
        '--fqbn', f'esp32:esp32:esp32:FlashSize={flash_fqbn},FlashMode=dio,PartitionScheme=custom',
        '--build-property', 'build.partitions=custom',
        '--build-property', f'build.custom_partitions={partitions_file}',
        '--build-property', 'upload.maximum_size=1769472',
        '--build-path', str(config_build_dir),
        '--export-binaries',
        str(SKETCH_DIR)
    ]

    with open(log_file, 'w') as log:
        result = subprocess.run(cmd, stdout=log, stderr=subprocess.STDOUT)

    if result.returncode == 0:
        print_success("Compilation successful")
        
        # Explicit binary file selection (matches v5.74 style resilience)
        binary = config_build_dir / f"{SKETCH_DIR.name}.ino.bin"
        if not binary.exists():
            binary = SKETCH_DIR / f"{SKETCH_DIR.name}.ino.bin"
        if not binary.exists():
            cached_bins = list(config_build_dir.glob("**/*.bin")) + list(SKETCH_DIR.glob("**/*.bin"))
            for cb in cached_bins:
                if "bootloader" not in cb.name and "partitions" not in cb.name and "qc" not in cb.name and cb.stat().st_size > 500000:
                    binary = cb
                    break
        if binary.exists():
            firmware_path = output_dir / "firmware.bin"
            shutil.copy(binary, firmware_path)
            
            size = firmware_path.stat().st_size
            size_mb = size / (1024 * 1024)
            print_success(f"Binary exported: firmware.bin ({size_mb:.2f} MB)")
            
            # Get firmware version from user_config.h
            import re
            with open(USER_CONFIG_H, 'r') as f:
                config_content = f.read()
            version_match = re.search(r'#define FIRMWARE_VERSION "([^"]+)"', config_content)
            firmware_version = version_match.group(1) if version_match else "UNKNOWN"
            # Strip trailing -M or -N if present in user_config.h so UI suffix is not duplicated
            firmware_version = re.sub(r'-[MN]$', '', firmware_version)
            
            type_prefix = "TRG9" if system == 0 else ("TWS9" if system == 1 else "TWSRF9")
            
            # Map specific strings based on logic in AIO9_5.0.ino
            if unit == "KSNDMC_TRG":
                full_version = f"TRG9-DMC-{firmware_version}"
            elif unit == "BIHAR_TRG":
                full_version = f"TRG9-BIH-{firmware_version}"
            elif unit == "SPATIKA_GEN" and system == 2:
                full_version = f"TWSRF9-GEN-{firmware_version}"
            elif unit == "KSNDMC_TWS":
                full_version = f"TWS9-DMC-{firmware_version}"
            elif unit == "KSNDMC_ADDON":
                full_version = f"TWSRF9-DMC-{firmware_version}"
            else:
                client = "UNKNOWN"
                if unit.startswith("KSNDMC"):
                    client = "DMC"
                elif "_" in unit:
                    client = unit.split("_")[0]
                else:
                    client = unit
                full_version = f"{type_prefix}-{client}-{firmware_version}"
            
            # Append UI Suffix (-N for Nuvoton, -M for Matrix)
            ui_ver_suffix = "-N" if use_nuvoton_ui == 1 else "-M"
            full_version = f"{full_version}{ui_ver_suffix}"
            
            fw_version_file = output_dir / "fw_version.txt"
            with open(fw_version_file, 'w') as f:
                f.write(full_version)
            print_success(f"Version file created: fw_version.txt ({full_version})")

            # --- Generate metadata.json ---
            build_defines = extract_build_defines(USER_CONFIG_H)
            metadata = {
                'config':           output_name,
                'unit_cfg':         unit,
                'system_type':      ['TRG', 'TWS', 'TWS-RF'][system] if system in [0,1,2] else str(system),
                'flash_size':       flash_size,
                'full_version':     full_version,
                'binary_size_bytes':size,
                'build_timestamp':  datetime.now().isoformat(timespec='seconds'),
                **build_defines
            }
            meta_file = output_dir / "metadata.json"
            with open(meta_file, 'w') as f:
                json.dump(metadata, f, indent=2)
            print_success(f"Metadata file created: metadata.json")

            # ── Print compile-time settings summary ────────────────────────────────
            d = metadata
            def yn(v): return f"{Colors.GREEN}YES{Colors.NC}" if v else f"{Colors.RED}NO{Colors.NC}"
            print(f"\n  ┌{'':─<54}┐")
            print(f"  │  {Colors.YELLOW}COMPILE-TIME SETTINGS for {output_name}_{flash_size}{Colors.NC}")
            print(f"  ├{'':─<54}┤")
            print(f"  │  Debug Logs (DEBUG)          : {yn(d.get('debug'))}")
            print(f"  │  Web Server (ENABLE_WEBSERVER): {yn(d.get('enable_webserver'))}")
            print(f"  │  Nuvoton UI  (USE_NUVOTON_UI) : {yn(d.get('use_nuvoton_ui'))}")
            print(f"  │  Health Rpt  (ENABLE_HEALTH_REPORT): {yn(d.get('enable_health_report'))}")
            print(f"  │  Health Freq (TEST_HEALTH_DEFAULT) : {d.get('health_report_freq','--')}")
            print(f"  │  RF Res (DEFAULT_RF_RESOLUTION): {d.get('rf_resolution_mm','--')} mm")
            print(f"  │  Wind Teeth (WIND_TEETH_COUNT) : {d.get('wind_teeth_count','--')}")
            print(f"  │  Pressure Sensor              : {yn(d.get('enable_pressure_sensor'))}")
            print(f"  │  Calib Test                   : {yn(d.get('enable_calib_test'))}")
            print(f"  │  Binary Size                   : {size_mb:.2f} MB")
            print(f"  └{'':─<54}┘\n")

            # [BUILD-05] POST-COMPILATION IDENTITY CHECK
            # Verify that the generated version prefix matches the intended system type
            expected_prefix = "TRG9" if system == 0 else ("TWS9" if system == 1 else "TWSRF9")
            if not full_version.startswith(expected_prefix):
                print_error(f"IDENTITY MISMATCH: Config intended for {expected_prefix} but got {full_version}!")
                print_error("Build Aborted to prevent corrupted release package.")
                return False
            
            print_success(f"Identity Verified: {full_version}")
            
            # Create info file
            info_file = output_dir / "build_info.txt"
            with open(info_file, 'w') as f:
                f.write(f"Build Configuration\n")
                f.write(f"===================\n")
                f.write(f"SYSTEM: {system}\n")
                f.write(f"UNIT: {unit}\n")
                f.write(f"Output: {output_name}\n")
                f.write(f"Firmware Version: {firmware_version}\n")
                f.write(f"Build Date: {datetime.now()}\n")
                f.write(f"Binary Size: {size_mb:.2f} MB\n")
                f.write(f"Binary Path: {firmware_path}\n")
                f.write(f"Version File: {fw_version_file}\n")
            
            return True
        else:
            print_error("Binary not found")
            return False
    else:
        print_error("Compilation failed")
        print(f"See log: {log_file}")
        return False

def main():
    parser = argparse.ArgumentParser(description="AIO9_5.0 Multi-Configuration Builder")
    parser.add_argument(
        "--flash", nargs="+", default=["8mb"],
        choices=["4mb", "8mb", "16mb"],
        help="Flash size variant(s) to build. Default: 8mb. Example: --flash 8mb 4mb"
    )
    parser.add_argument(
        "--enable-debug", action="store_true",
        help="Keep Serial Debug logs active in the official build (Not recommended for field units)."
    )
    parser.add_argument(
        "--configs", nargs="+", default=None,
        help="Specific configuration name(s) to build (e.g., KSNDMC_TRG or KSNDMC)."
    )
    parser.add_argument(
        "--ui", nargs="+", default=["both"],
        choices=["nuvoton", "matrix", "both", "n", "m"],
        help="UI display variant to compile: 'nuvoton' (or 'n'), 'matrix' (or 'm'), or 'both'. Default: both"
    )
    args = parser.parse_args()

    # Build active flash variant list
    FLASH_VARIANTS = []
    for size in args.flash:
        fqbn, csv = ALL_FLASH_VARIANTS[size]
        FLASH_VARIANTS.append((size, fqbn, csv))

    # Match configurations case-insensitively as substrings
    active_configs = CONFIGS
    if args.configs:
        matched = []
        for c in CONFIGS:
            for term in args.configs:
                if term.lower() in c[2].lower():
                    matched.append(c)
                    break
        active_configs = matched
        if not active_configs:
            print_error(f"No configurations matched filter: {args.configs}")
            sys.exit(1)

    # Determine UI targets
    ui_targets = []
    requested_uis = [x.lower() for x in args.ui]
    if "both" in requested_uis:
        ui_targets = [1, 0]
    else:
        for ui in requested_uis:
            if ui in ["nuvoton", "n"]:
                if 1 not in ui_targets: ui_targets.append(1)
            elif ui in ["matrix", "m"]:
                if 0 not in ui_targets: ui_targets.append(0)

    # Calculate total planned builds taking UI restrictions into account
    # Nuvoton UI (USE_NUVOTON_UI == 1) is ONLY built for KSNDMC_TRG and KSNDMC_TWS
    total_builds = 0
    for _ in FLASH_VARIANTS:
        for u_target in ui_targets:
            for _, _, output_name in active_configs:
                if u_target == 1 and output_name not in NUVOTON_UI_CONFIGS and not args.configs:
                    continue
                total_builds += 1

    print_header(f"AIO9_5.0 Multi-Configuration Builder")
    print(f"  Flash targets: {', '.join(args.flash).upper()}")
    print(f"  UI targets: {', '.join(['NUVOTON' if u==1 else 'MATRIX' for u in ui_targets])}")
    print(f"  Configurations: {len(active_configs)}")
    print(f"  Total builds: {total_builds}")
    
    # Check arduino-cli
    if not check_arduino_cli():
        sys.exit(1)
    
    # Create clean output directory
    if OUTPUT_BASE.exists():
        print_info("Cleaning previous builds...")
        shutil.rmtree(OUTPUT_BASE)
    OUTPUT_BASE.mkdir(exist_ok=True)

    # Wipe arduino-cli global build cache (~/Library/Caches/arduino/)
    # CRITICAL: arduino-cli 1.4.1 maintains a global sketch+core cache that persists
    # across builds even when --clean and per-config --build-path are used.
    # Without this wipe, stale .o files from old ESP32 core versions (e.g. 2.0.17)
    # are silently reused, making builds appear instant while producing
    # corrupted/mixed-version binaries.
    arduino_cache = Path.home() / "Library" / "Caches" / "arduino"
    for cache_sub in ("sketches", "cores"):
        cache_dir = arduino_cache / cache_sub
        if cache_dir.exists():
            shutil.rmtree(cache_dir)
            cache_dir.mkdir(parents=True, exist_ok=True)
    print_info("arduino-cli global build cache cleared (ensures clean compile from source)")

    # Backup user_config.h
    backup_config()
    
    # Build all configurations × all flash variants × all UI targets
    success_count = 0
    fail_count = 0

    for flash_size, flash_fqbn, partition_csv in FLASH_VARIANTS:
        print_header(f"=== Flash Variant: {flash_size.upper()} ===")
        for use_nuvoton_ui in ui_targets:
            ui_label = "NUVOTON UI" if use_nuvoton_ui == 1 else "MATRIX UI"
            print_info(f"Building for UI variant: {ui_label}")
            for system, unit, output_name in active_configs:
                # Nuvoton UI (USE_NUVOTON_UI=1) is only built for KSNDMC_TRG and KSNDMC_TWS unless explicitly overridden via --configs
                if use_nuvoton_ui == 1 and output_name not in NUVOTON_UI_CONFIGS and not args.configs:
                    print_info(f"Skipping Nuvoton UI for {output_name} (Nuvoton UI is restricted to KSNDMC_TRG & KSNDMC_TWS)")
                    continue
                if build_config(system, unit, output_name, flash_size, flash_fqbn, partition_csv, use_nuvoton_ui=use_nuvoton_ui):
                    success_count += 1
                else:
                    fail_count += 1
                    # [BUILD-06] FAIL FAST: Stop build process if any config fails
                    restore_config() # Ensure user_config.h is restored before exit
                    ui_suffix = "NUV" if use_nuvoton_ui == 1 else "MAT"
                    print_error(f"\nBUILD ABORTED: {output_name}_{ui_suffix}_{flash_size} failed. Check logs.")
                    sys.exit(1)
    
    # Restore user_config.h
    restore_config()
    
    # Summary
    print_header("Build Summary")
    print(f"{Colors.GREEN}Successful: {success_count}{Colors.NC}")
    print(f"{Colors.RED}Failed: {fail_count}{Colors.NC}")
    print(f"\nInternal Build directory: {OUTPUT_BASE}")

    # Copy to external release directory if any builds succeeded
    if success_count > 0:
        # Get firmware version for the release directory
        import re
        with open(USER_CONFIG_H, 'r') as f:
            config_content = f.read()
        version_match = re.search(r'#define FIRMWARE_VERSION "([^"]+)"', config_content)
        firmware_version = version_match.group(1) if version_match else "UNKNOWN"
        
        # New Dynamic Base Path
        external_base = Path("/Users/satishkripavasan/Documents/Arduino/ESP32_NEW_DESIGN/RELEASE/AIO9_5") / f"v{firmware_version}"
        
        print_info(f"Copying release-ready binaries to: {external_base}")
        try:
            external_base.mkdir(parents=True, exist_ok=True)
            
            # 1. Copy Configurations
            for config_dir in OUTPUT_BASE.iterdir():
                if config_dir.is_dir():
                    source_bin = config_dir / "firmware.bin"
                    source_ver = config_dir / "fw_version.txt"
                    if source_bin.exists():
                        target_dir = external_base / config_dir.name
                        target_dir.mkdir(parents=True, exist_ok=True)
                        shutil.copy(source_bin, target_dir / "firmware.bin")
                        if source_ver.exists():
                            shutil.copy(source_ver, target_dir / "fw_version.txt")
                        source_meta = config_dir / "metadata.json"
                        if source_meta.exists():
                            shutil.copy(source_meta, target_dir / "metadata.json")
            
            # 2. Copy flash_files (Required for ESPTool)
            source_flash = SKETCH_DIR / "flash_files"
            if source_flash.exists():
                target_flash = external_base / "flash_files"
                if target_flash.exists():
                    shutil.rmtree(target_flash)
                shutil.copytree(source_flash, target_flash)
                print_success("Flash files (bootloader/partitions) copied.")

            # 3. Copy Version-Specific Release Notes
            source_notes = SKETCH_DIR / f"RELEASE_NOTES_v{firmware_version}.md"
            if not source_notes.exists():
                source_notes = SKETCH_DIR / "RELEASE_NOTES.md" # Fallback
                
            if source_notes.exists():
                shutil.copy(source_notes, external_base / "RELEASE_NOTES.md")
                print_success(f"Release Notes copied to: {external_base / 'RELEASE_NOTES.md'}")

            print_success(f"Release v{firmware_version} package complete.")
            
            # 4. Create ZIP archive of the release bundle
            try:
                import zipfile
                zip_filename = external_base.parent / f"AIO9_v{firmware_version}.zip"
                
                print(f"→ Creating release archive: {zip_filename.name}")
                
                with zipfile.ZipFile(zip_filename, 'w', zipfile.ZIP_DEFLATED) as zipf:
                    # Walk through all files in the release directory
                    for root, dirs, files in os.walk(external_base):
                        for file in files:
                            file_path = Path(root) / file
                            # Calculate relative path from external_base parent
                            arcname = file_path.relative_to(external_base.parent)
                            zipf.write(file_path, arcname)
                
                zip_size = zip_filename.stat().st_size / (1024 * 1024)
                print_success(f"Release archive created: {zip_filename.name} ({zip_size:.2f} MB)")

                # 5. Create standalone Factory Flash Files ZIP archive
                try:
                    factory_zip_filename = external_base.parent / "AIO9_Factory_Flash_Files.zip"
                    print(f"→ Creating Factory Flash Files archive: {factory_zip_filename.name}")
                    
                    factory_files_to_pack = [
                        SKETCH_DIR / "flash_files",
                        SKETCH_DIR / "partitions.csv",
                        SKETCH_DIR / "partitions_16mb.csv",
                        SKETCH_DIR / "partitions_4mb.csv",
                        SKETCH_DIR / "flash_firmware.sh",
                        SKETCH_DIR / "quick_flash.sh",
                        SKETCH_DIR / "FACTORY_FLASH_GUIDE.md",
                    ]
                    
                    with zipfile.ZipFile(factory_zip_filename, 'w', zipfile.ZIP_DEFLATED) as fzipf:
                        for item in factory_files_to_pack:
                            if item.exists():
                                if item.is_dir():
                                    for root, dirs, files in os.walk(item):
                                        for file in files:
                                            file_path = Path(root) / file
                                            arcname = file_path.relative_to(SKETCH_DIR)
                                            fzipf.write(file_path, arcname)
                                elif item.is_file():
                                    fzipf.write(item, item.relative_to(SKETCH_DIR))
                    
                    factory_zip_size = factory_zip_filename.stat().st_size / (1024 * 1024)
                    print_success(f"Factory Flash archive created: {factory_zip_filename.name} ({factory_zip_size:.2f} MB)")
                except Exception as e:
                    print_error(f"Failed to create factory zip archive: {e}")

            except Exception as e:
                print_error(f"Failed to create zip archive: {e}")

        except Exception as e:
            print_error(f"Failed to copy to external release directory: {e}")
    
    # List all builds
    print("\nBuilt configurations:")
    for config_dir in sorted(OUTPUT_BASE.iterdir()):
        if config_dir.is_dir():
            firmware = config_dir / "firmware.bin"
            if firmware.exists():
                size = firmware.stat().st_size / (1024 * 1024)
                print(f"  {config_dir.name}: firmware.bin ({size:.2f} MB)")
    
    print("\nDone!")
    return 0 if fail_count == 0 else 1

if __name__ == "__main__":
    try:
        sys.exit(main())
    except KeyboardInterrupt:
        print("\n\nBuild cancelled by user")
        restore_config()
        sys.exit(1)
    except Exception as e:
        print_error(f"Unexpected error: {e}")
        restore_config()
        sys.exit(1)
