═══════════════════════════════════════════════════════════════
  SPATIKA FACTORY JIG  —  Quick Start
  AIO ESP32 Board QC & Flash Tool  |  Firmware v6.07
═══════════════════════════════════════════════════════════════

WHAT'S IN THIS FOLDER
─────────────────────
  factory_tool.html       ← Browser dashboard (DO NOT move out)
  launch_mac.command      ← macOS launcher  (double-click to start)
  launch_windows.bat      ← Windows launcher (double-click to start)
  server.ps1              ← PowerShell server used by the Windows launcher
  version.txt             ← Firmware version stamp

  ── Core firmware files (auto-loaded by the dashboard) ──
  bootloader.bin
  boot_app0.bin
  partitions_4mb.bin / partitions_8mb.bin / partitions_16mb.bin
  qc_test_4mb.bin  / qc_test_8mb.bin  / qc_test_16mb.bin
  production_4mb.bin / production_8mb.bin / production_16mb.bin

  ── Named release config folders (same structure as email ZIP) ──
  KSNDMC_TRG_8mb/         -+
    firmware.bin            |  Compiled firmware for each UNIT_CFG.
    fw_version.txt          |  Select the appropriate folder in the
    metadata.json           |  dashboard to flash and see build info.
  BIHAR_TRG_8mb/          -|
  KSNDMC_TWS_8mb/          |
  KSNDMC_ADDON_8mb/        |
  SPATIKA_TWSRF_8mb/      -+

  Each subfolder contains:
    firmware.bin     — Application binary to flash at 0x10000
    fw_version.txt   — Version string (e.g. TRG9-DMC-6.07)
    metadata.json    — Compile-time settings (Debug, WebServer, RF, etc.)

HOW TO START
────────────

  macOS
  -----
  1. Double-click  launch_mac.command
     (First time: right-click → Open to bypass Gatekeeper)
  2. Chrome / Edge opens automatically at
     http://localhost:8000/factory_tool.html
  3. Close the Terminal window that appears to stop the server.

  Windows
  -------
  1. Double-click  launch_windows.bat
  2. A console window opens — Chrome / Edge opens automatically.
  3. Close the console window to stop the server.

  ⚠  If Windows shows "Windows protected your PC", click
     "More info" → "Run anyway". This is a known SmartScreen
     warning for unsigned local scripts and is safe to bypass.

FLASHING WITH A NAMED CONFIG (from email ZIP or this folder)
──────────────────────────────────────────────────────────────
  When you receive a release ZIP by email OR copy this entire
  WEB_FLASH_FILES folder to another computer:

  1. Start the server (launch_mac.command / launch_windows.bat).
  2. Open factory_tool.html in Chrome.
  3. Select the device Profile in the top bar:
       TRG    → Rain-only boards  (KSNDMC_TRG, BIHAR_TRG, SPATIKA_GEN)
       TWS    → Wind+Rain boards  (KSNDMC_TWS, KSNDMC_TWS-AP)
       TWS-RF → All-sensors       (KSNDMC_ADDON, SPATIKA_GEN)
  4. Pick the matching Config (e.g. KSNDMC_TRG) from the Config dropdown.
  5. Click "Select Firmware Folder" and choose THIS folder (WEB_FLASH_FILES).
     The dashboard will automatically:
       • Load all core firmware files (bootloader, partitions, qc_test)
       • Load the config-specific firmware.bin from the correct subfolder
       • Display compile-time settings from metadata.json
       • Warn you if the binary is too large for the detected chip
  6. Connect the board and click "Start Programming".

REQUIREMENTS
────────────
  • macOS 12.3 or later  (Python 3 built-in — nothing else needed)
  • Windows 10 / 11      (PowerShell built-in — nothing else needed)
  • Google Chrome or Microsoft Edge  (WebSerial API required)
  • USB cable to connect the ESP32 board

IMPORTANT NOTES
───────────────
  • Keep ALL files and subfolders in the SAME root directory.
    The dashboard auto-loads everything relative to where it is served.
  • Do NOT rename or move individual files or config folders.
  • Only Google Chrome and Microsoft Edge support WebSerial.
    Safari and Firefox are NOT supported.
  • Close Arduino IDE Serial Monitor before connecting the board.
  • The metadata.json inside each config folder records the exact
    compile-time switches used (DEBUG, ENABLE_WEBSERVER, RF resolution…)
    These are displayed automatically in the dashboard.

SUPPORT
───────
  Spatika Technologies  |  AIO Board Factory Tool
  Firmware v6.07  |  © 2026
═══════════════════════════════════════════════════════════════
