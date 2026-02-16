#!/usr/bin/env python3
"""
Automated Build Script for AIO9_5.0
Builds all SYSTEM/UNIT configurations automatically
Requires: arduino-cli (install: brew install arduino-cli)
"""

import os
import sys
import subprocess
import shutil
from pathlib import Path
from datetime import datetime

# Configuration
SKETCH_DIR = Path(__file__).parent
GLOBALS_H = SKETCH_DIR / "globals.h"
OUTPUT_BASE = SKETCH_DIR / "builds"
BACKUP_GLOBALS = "/tmp/globals.h.backup"

# Build configurations: (SYSTEM, UNIT, output_name)
CONFIGS = [
    (0, "KSNDMC_TRG", "KSNDMC_TRG"),
    (0, "BIHAR_TRG", "BIHAR_TRG"),
    (0, "SPATIKA_GEN", "SPATIKA_TRG"),
    (1, "KSNDMC_TWS", "KSNDMC_TWS"),
    (2, "KSNDMC_ADDON", "KSNDMC_ADDON"),
    (2, "SPATIKA_GEN", "SPATIKA_ADDON"),
]

# External Release Path
EXTERNAL_RELEASE_BASE = Path("/Users/satishkripavasan/Documents/Arduino/ESP32_NEW_DESIGN/RELEASE/AIO9_5/AIO9_5.0")

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

def backup_globals():
    """Backup original globals.h"""
    print_info("Backing up globals.h...")
    shutil.copy(GLOBALS_H, BACKUP_GLOBALS)

def restore_globals():
    """Restore original globals.h"""
    print_info("Restoring original globals.h...")
    shutil.copy(BACKUP_GLOBALS, GLOBALS_H)

def update_globals(system, unit):
    """Update globals.h with new SYSTEM and UNIT values"""
    print_info(f"Configuring: SYSTEM={system}, UNIT={unit}")
    
    # Restore backup first
    shutil.copy(BACKUP_GLOBALS, GLOBALS_H)
    
    # Read file
    with open(GLOBALS_H, 'r') as f:
        content = f.read()
    
    # Update SYSTEM
    import re
    content = re.sub(r'#define SYSTEM \d+', f'#define SYSTEM {system}', content)
    
    # Update UNIT
    content = re.sub(r'char UNIT\[15\] = "[^"]*"', f'char UNIT[15] = "{unit}"', content)

    # Force DEBUG 0 for official builds
    content = re.sub(r'#define DEBUG \d+', '#define DEBUG 0', content)
    
    # Write back
    with open(GLOBALS_H, 'w') as f:
        f.write(content)
    
    print(f"  SYSTEM set to: {system}")
    print(f"  UNIT set to: {unit}")

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

def build_config(system, unit, output_name):
    """Build a specific configuration"""
    print_header(f"Building: {output_name}")
    
    # Update configuration
    update_globals(system, unit)
    
    # Create output directory
    output_dir = OUTPUT_BASE / output_name
    output_dir.mkdir(parents=True, exist_ok=True)
    
    # Shared Build directory to speed up compilation
    shared_build_dir = SKETCH_DIR / "build" / "all_configs_shared"
    shared_build_dir.mkdir(parents=True, exist_ok=True)
    
    # Compile
    print_info("Compiling...")
    log_file = output_dir / "build.log"
    
    # Use custom partition scheme from partitions.csv
    # 8MB Flash: 1.5MB app0 + 1.5MB app1 (OTA) + 5MB SPIFFS
    partitions_file = SKETCH_DIR / "partitions.csv"
    
    cmd = [
        'arduino-cli', 'compile',
        '--fqbn', 'esp32:esp32:esp32:PartitionScheme=custom',
        '--build-property', f'build.partitions=custom',
        '--build-property', f'build.custom_partitions={partitions_file}',
        '--build-path', str(shared_build_dir),
        '--export-binaries',
        str(SKETCH_DIR)
    ]
    
    with open(log_file, 'w') as log:
        result = subprocess.run(cmd, stdout=log, stderr=subprocess.STDOUT)
    
    if result.returncode == 0:
        print_success("Compilation successful")
        
        # Find and copy binary
        binary_files = list(shared_build_dir.glob("*.ino.bin"))
        if binary_files:
            binary = binary_files[0]
            firmware_path = output_dir / "firmware.bin"
            shutil.copy(binary, firmware_path)
            
            size = firmware_path.stat().st_size
            size_mb = size / (1024 * 1024)
            print_success(f"Binary exported: firmware.bin ({size_mb:.2f} MB)")
            
            # Get firmware version from globals.h
            import re
            with open(GLOBALS_H, 'r') as f:
                globals_content = f.read()
            version_match = re.search(r'#define FIRMWARE_VERSION "([^"]+)"', globals_content)
            firmware_version = version_match.group(1) if version_match else "UNKNOWN"
            
            # Create fw_version.txt for legacy SD card updates
            fw_version_file = output_dir / "fw_version.txt"
            with open(fw_version_file, 'w') as f:
                f.write(firmware_version)
            print_success(f"Version file created: fw_version.txt ({firmware_version})")
            
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
    print_header("AIO9_5.0 Multi-Configuration Builder")
    
    # Check arduino-cli
    if not check_arduino_cli():
        sys.exit(1)
    
    # Create output directory
    OUTPUT_BASE.mkdir(exist_ok=True)
    
    # Backup globals.h
    backup_globals()
    
    # Build all configurations
    success_count = 0
    fail_count = 0
    
    for system, unit, output_name in CONFIGS:
        if build_config(system, unit, output_name):
            success_count += 1
        else:
            fail_count += 1
    
    # Restore globals.h
    restore_globals()
    
    # Summary
    print_header("Build Summary")
    print(f"{Colors.GREEN}Successful: {success_count}{Colors.NC}")
    print(f"{Colors.RED}Failed: {fail_count}{Colors.NC}")
    print(f"\nInternal Build directory: {OUTPUT_BASE}")

    # Copy to external release directory if any builds succeeded
    if success_count > 0:
        # Get firmware version for the release directory
        import re
        with open(GLOBALS_H, 'r') as f:
            globals_content = f.read()
        version_match = re.search(r'#define FIRMWARE_VERSION "([^"]+)"', globals_content)
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
        restore_globals()
        sys.exit(1)
    except Exception as e:
        print_error(f"Unexpected error: {e}")
        restore_globals()
        sys.exit(1)
