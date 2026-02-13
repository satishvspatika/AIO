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

### 2. Team Member Workflow (Online/Web Flash)
If a team member needs to flash a device without any local tools:
1. Run `./export_web_flash.sh` to gather the 4 binary files into the `WEB_FLASH_FILES` folder.
2. Send these files to the member.
3. They can use an **Online ESP Web Tool** (e.g., [web-install.com/esp32](https://web-install.com/esp32)) with these addresses:
   - `bootloader.bin` --> **0x1000**
   - `partitions.bin` --> **0x8000**
   - `boot_app0.bin`  --> **0xE000**
   - `application.bin` --> **0x10000**

### 3. Release Bundling (FTP / SD Card / Updates)
To generate clean `firmware.bin` and `fw_version.txt` bundles:
- **For a single configuration**: `./export_bin.sh [CONFIG_NAME]`
- **For a full release (All 6 configs)**: `python3 build_all_configs.py`

---
*Created by Spatika Information Technologies*
