# AIO ESP32 Firmware Project (v5.0)

This repository contains the firmware and build tools for the All-In-One (AIO) ESP32 Telemetry system.

## 📋 Table of Contents
1. [Release Strategy](#-release-strategy)
2. [Development Workflow](#-development-workflow)
3. [Flashing Guide for Members](#-flashing-guide-for-members)
4. [Recent Fixes (v5.0)](#-recent-fixes-v50)

---

## 🚀 Release Strategy
We maintain control by using **Git Tags** for every stable release.
- **Source Code**: Managed in this repository.
- **Compiled Binaries**: Distributed via [GitHub Releases](https://github.com/satishvspatika/AIO/releases). Members should download the `.bin` files from the "Assets" section of each release.

## 🛠 How to Manage this Project (3 Ways)

### 1. Developer Workflow (Modify, Test, & Save)
If you are making changes to the code:
- **One-Step Compile & Flash**:
  ```bash
  ./compile_and_flash.sh
  ```
  *(This automatically compiles your code and flashes ONLY the app. It will intelligently find your USB port and release any busy serial monitors.)*

- **Save Code to GitHub**: 
  ```bash
  git add .
  git commit -m "Description of fix"
  git push origin main
  ```
- **Revert/Switch versions**: 
  - To go to a specific release: `git checkout v5.0.0`
  - To go back to latest: `git checkout main`

### 2. Team Member Workflow (No Tools / Online Flash)
If a team member needs to flash a device but has no IDE/Python, they can use an **Online ESP Web Tool**. You must provide them with these 4 files and these addresses:
- `bootloader.bin` --> **0x1000**
- `partitions.bin` --> **0x8000**
- `boot_app0.bin`  --> **0xE000**
- `AIO9_5.0.ino.bin` --> **0x10000**

### 3. Bundling for a Single Configuration (Manual)
To generate the clean bundle for just the current config you are working on:
1. Export the binary from Arduino IDE (**Sketch -> Export Compiled Binary**).
2. Run the packager:
   ```bash
   ./organize_build.sh KSNDMC_TRG
   ```

### 4. Full Release (All 6 Configurations Automatically)
To generate a complete release for every supported system (TRG, TWS, TWS-RF) in one go:
```bash
python3 build_all_configs.py
```
This script will produce 6 folders in the `builds/` directory, each with its own `firmware.bin` and `fw_version.txt`.

---
*Created by Spatika Information Technologies*
