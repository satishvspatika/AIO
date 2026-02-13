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

## 🛠 Development Workflow
If you want to make changes:
1. **Pull**: `git pull origin main`
2. **Edit**: Modify code and update `FIRMWARE_VERSION` in `globals.h`.
3. **Commit**: `git add .` and `git commit -m "Your description"`
4. **Tag**: `git tag -a v5.0.1 -m "Short summary of changes"`
5. **Push**: `git push origin main --tags`

## ⚡ Flashing Guide for Members
Members do not need to install the Arduino IDE or compile code.
1. Download the binary assets from the latest Release.
2. Connect your ESP32.
3. Run the flash script:
   ```bash
   ./flash_firmware.sh /dev/cu.usbserial-A50285BI
   ```

## 🔍 Recent Fixes (v5.0)
- **ADC Stability**: Fixed the `driver_ng` vs `Legacy` conflict causing crashes.
- **UI Performance**: Removed flicker from LCD by using space-padding.
- **Persistence**: Implemented NVS/SD fallback for Station ID.
- **Input**: Restored Alphanumeric keypad input for Station ID.

---
*Created by Spatika Information Technologies*
