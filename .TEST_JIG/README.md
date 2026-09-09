# Spatika TEST_JIG Quality Control & Flashing System

This directory houses the Quality Control (QC) firmware and the browser-based WebSerial flasher dashboard used for testing and programming assembled boards (ESP32, GPRS modem, Nuvoton UI).

## Structure
* `/qc_test/` - The dedicated ESP32 firmware that executes the hardware tests.
* `factory_tool.html` - The HTML/JS WebSerial dashboard that flashes the ESP32 and displays real-time test reports.
* `build_jig.sh` - Bash script to automate building the test firmware for 4MB/8MB/16MB targets and packaging them.
