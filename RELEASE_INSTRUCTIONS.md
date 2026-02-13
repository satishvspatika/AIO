# Release Management for AIO Project

To maintain this project and distribute it to team members, follow this workflow.

## 1. Development Workflow
1.  **Change Code**: Edit `.ino` or `.h` files.
2.  **Commit**: `git add .` then `git commit -m "Description of change"`
3.  **Push**: `git push origin main`

## 2. Creating a Release (For Team Members)
Instead of members downloading code and compiling themselves (which requires `arduino-cli`), we distribute the compiled binaries.

### How to bundle a Release:
1.  Run the compile command to generate fresh binaries in the `build/` folder.
2.  Go to [GitHub Releases](https://github.com/satishvspatika/AIO/releases/new).
3.  Set a Version Tag (e.g., `v5.0.1`).
4.  **Upload these files from the build folder**:
    *   `AIO9_5.0.ino.bin` (The App)
    *   `AIO9_5.0.ino.bootloader.bin`
    *   `AIO9_5.0.ino.partitions.bin`
    *   `boot_app0.bin`

## 3. Deployment (For Project Members)
Members can use the `flash_firmware.sh` script (included in this repo) to flash the device without needing any development tools except `esptool.py`.

### Flashing Command:
```bash
./flash_firmware.sh /dev/cu.usbserial-A50285BI
```

---
*Maintained by the AIO Development Team*
