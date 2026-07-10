# 📖 AIO9_5.0 Selective Release User Manual

This manual describes how to use the selective build and release automation tools to compile, package, and email firmware binaries for the **AIO9_5.0** platform. It details options for separating UI architectures (Nuvoton UART LCD vs. Matrix I2C/GPIO LCD), matching specific station configurations, and preparing official releases.

---

## 🧭 Core Release Parameters

The build script `build_all_configs.py` and the release script `create_release.sh` support the following command-line flags:

| Parameter | Options / Syntax | Default | Description |
| :--- | :--- | :--- | :--- |
| `--configs` | Space-separated name filters (e.g. `KSNDMC_TRG`, `KSNDMC`, `BIHAR`, `SPATIKA`) | `None` (Builds all configs) | Filters which configurations to compile. Matches case-insensitively as a substring. |
| `--ui` | `nuvoton` (or `n`), `matrix` (or `m`), `both` | `both` | Specifies display architectures to compile. |
| `--flash` | `8mb`, `4mb`, `16mb` | `8mb` | Specifies ESP32 target flash size. Can build multiple variants at once. |
| `--enable-debug` | Flag (no values required) | `Disabled` | Compiles with serial debugging enabled (`DEBUG 1`). If omitted, builds with `DEBUG 0` for production. |

---

## 🎨 UI Architecture Suffixes

To differentiate UI display architectures in files and reports:

1. **Firmware Version Suffix (`UNIT_VER`)**:
   * **Nuvoton UI (`USE_NUVOTON_UI 1`)**: Appends `-N` (e.g. `TRG9-DMC-6.10-N`)
   * **Matrix UI (`USE_NUVOTON_UI 0`)**: Appends `-M` (e.g. `TRG9-DMC-6.10-M`)
2. **Directory Name Suffix**:
   * **Nuvoton UI**: Folder is named with `_NUV_` (e.g. `KSNDMC_TRG_NUV_8mb`)
   * **Matrix UI**: Folder is named with `_MAT_` (e.g. `KSNDMC_TRG_MAT_8mb`)
3. **LCD Display Middle-Abbreviation (Dynamic)**:
   If the version string exceeds 16 characters on the 16x2 physical LCD (e.g. `TWSRF9-DMC-6.10-N`), the display dynamically shortens the middle network field:
   * `-DMC-` $\rightarrow$ `-D-` (renders as `TWSRF9-D-6.10-N`)
   * `-BIH-` $\rightarrow$ `-B-` (renders as `TRG9-B-6.10-N`)
   * `-GEN-` $\rightarrow$ `-G-` (renders as `TWSRF9-G-6.10-N`)

---

## 🚀 Execution Workflows

### Workflow A: Development & Local Verification
To build, package, and inspect binaries locally without creating Git commits, tags, or triggering email notifications, invoke the Python script directly:

```bash
python3 build_all_configs.py [options]
```

### Workflow B: Official Release Pipeline
To trigger the full release process (creating Git commits, tagging the release, generating release note templates, building binaries, and emailing the packaged ZIP to the production team), run the shell release script:

```bash
./create_release.sh "Your Release Message" [options]
```
> [!NOTE]
> All additional arguments (like `--configs` and `--ui`) are automatically forwarded from `create_release.sh` to the underlying build script.

---

## 📝 CLI Configuration Examples

### 1. Release only KSNDMC_TRG with both UI variants
To compile and package both Nuvoton and Matrix variants for the telemetry rain gauge (8MB partition scheme):
```bash
# Local build only
python3 build_all_configs.py --configs KSNDMC_TRG --ui both

# Official Release + Email
./create_release.sh "TRG Rain Gauge dual UI release" --configs KSNDMC_TRG --ui both
```

### 2. Release only KSNDMC_TWS with Matrix UI alone
To build only the Matrix (I2C/GPIO) version of the Telemetry Weather Station:
```bash
# Local build only
python3 build_all_configs.py --configs KSNDMC_TWS --ui matrix

# Official Release + Email
./create_release.sh "Matrix UI TWS field deployment" --configs KSNDMC_TWS --ui matrix
```

### 3. Release all KSNDMC units with Nuvoton UI alone
To compile all configurations containing `KSNDMC` (`KSNDMC_TRG`, `KSNDMC_TWS`, and `KSNDMC_ADDON`) for the Nuvoton display:
```bash
# Local build only
python3 build_all_configs.py --configs KSNDMC --ui nuvoton

# Official Release + Email
./create_release.sh "Nuvoton UI upgrade for all DMC stations" --configs KSNDMC --ui nuvoton
```

### 4. Release BIHAR units with Nuvoton UI alone
To target all Bihar configurations (`BIHAR_TRG`):
```bash
# Local build only
python3 build_all_configs.py --configs BIHAR --ui nuvoton

# Official Release + Email
./create_release.sh "Bihar Govt TRG release" --configs BIHAR --ui nuvoton
```

---

## 📂 Output Folder Structure

After a successful compilation, binaries are output to `builds/` (internal) and then copied to the external release path (`/Users/satishkripavasan/Documents/Arduino/ESP32_NEW_DESIGN/RELEASE/AIO9_5/v<VERSION>/`).

For example, running `--configs KSNDMC_TRG --ui both` generates:

```text
RELEASE/AIO9_5/v6.10/
├── AIO9_v6.10.zip                  <-- Packaged release archive attached to email
├── RELEASE_NOTES.md                <-- Release notes
├── flash_files/                    <-- Bootloaders, partitions, boot_app0
│   ├── bootloader.bin
│   └── partitions.csv
├── KSNDMC_TRG_NUV_8mb/
│   ├── firmware.bin                <-- Binary for Nuvoton UI
│   ├── fw_version.txt              <-- Contains: "TRG9-DMC-6.10-N"
│   └── metadata.json               <-- Compile parameters
└── KSNDMC_TRG_MAT_8mb/
    ├── firmware.bin                <-- Binary for Matrix UI
    ├── fw_version.txt              <-- Contains: "TRG9-DMC-6.10-M"
    └── metadata.json               <-- Compile parameters
```

---

## 📧 Dynamic Email Notification

When `send_release_email.py` runs, it scans the generated release folder dynamically:
* **Subject line**: Automatically lists the built configurations. For example:
  `AIO9_5.0 Firmware Release v6.10 (KSNDMC_TRG_NUV, KSNDMC_TRG_MAT) - Dynamic boot optimizations`
* **Plain Text Body Table**: Automatically compiles an ASCII summary of the compile settings extracted from each build's `metadata.json`:
  ```text
  ==================================================================
    COMPILE-TIME SETTINGS PER CONFIGURATION
  ==================================================================
    Config                 Debug  WebSrv  Nuvoton  HealthRpt  RF Res  Size MB
    ---------------------- -----  ------  -------  ---------  ------  -------
    KSNDMC_TRG_NUV_8mb     NO     YES     YES      NO         0.25 mm 1.15
    KSNDMC_TRG_MAT_8mb     NO     YES     NO       NO         0.25 mm 1.12
  ==================================================================
  ```
