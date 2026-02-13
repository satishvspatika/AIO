# AIO ESP32 (v5.0) - Project Manager

A consolidated suite of tools for developing and releasing the AIO firmware.

## 🚀 The 4 Project Workflows

### 1. Developer: Compile & Flash (High Speed)
Compile your code and flash only the app (at 0x10000) using auto-port detection.
```bash
./compile_and_flash.sh
```

### 2. Team: Online/Web Flash Assets
Gather the 4 required binaries into `WEB_FLASH_FILES/` for browser-based flash tools.
```bash
./export_web_flash.sh [CONFIG_NAME]
```

### 3. Release: Single Configuration Bundle
Generate a clean `firmware.bin` and `fw_version.txt` for one system (FTP/SD).
```bash
./export_bin.sh [CONFIG_NAME]
```

### 4. Release: Full Professional Build (All 6 Configs)
Automatically build every supported system type into the `builds/` directory.
```bash
python3 build_all_configs.py
```

---

## 📂 Configuration Names (Sync'd with UNIT)
Use these names for scripts:
`BIHAR_TRG`, `KSNDMC_TRG`, `SPATIKA_TRG`, `KSNDMC_TWS`, `KSNDMC_ADDON`, `SPATIKA_ADDON`

---
*Created by Spatika Information Technologies*
