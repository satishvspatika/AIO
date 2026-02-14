# ESP32 AIO v5.2 Release Notes - Feb 14, 2026

## 🚀 Version 5.2 Enhancements: "The Resilient Engine"

This release transforms the project into a lean, power-efficient, and ultra-resilient reporting engine. All legacy command-handling logic has been removed to prioritize rock-solid data transmission and maximum battery life.

### 1. Bimodal Power Discipline (Energy Harvesting)
- **Problem**: Sending health reports to Google every 15 minutes consumes significant battery due to high-power SSL handshakes.
- **Solution**: Automated Health Reports are now restricted to twice daily: **10:30 AM** (Morning Diagnostic) and **11:00 PM** (End-of-Day Summary).
- **Benefit**: Saves ~30-45 seconds of high-power GPRS usage during the other 94 intervals, extending battery life significantly.

### 2. Ultra-Resilient GPRS (BSNL-Safe)
- **High-Frequency Polling**: Reduced signal strength (`AT+CSQ`) polling delay to **500ms**. Fast networks (Airtel) now finish instantly.
- **Deep Search Patience**: Increased signal retries to **120 (60s timeout)** and registration retries to **60**. This ensures the system captures ultra-slow BSNL towers in weak signal areas.
- **IP Stabilization**: Added a **1-second pause** after network registration to ensure the modem's internal IP stack is "calm" before the first APN request, preventing "False Start" failures.

### 3. Ghost-Session Elimination (Zero Airtime Leakage)
- **Instant Sleep**: Fixed a logic hang where the device stayed awake in `eHttpStop` or `eSMSStop` states. The device now shuts down the modem and enters Deep Sleep the moment the data transmission is complete.
- **Expanded Safe Window**: Refined the power manager window to allow deep sleep even at the **12th minute**, maximizing power savings while maintaining a safe wake-up buffer for the next 15-minute slot.

### 4. Protocol Hardening (Server Acceptance)
- **Forceful Reset**: Integrated a brute-force `AT+CIPSHUT` command at the start of every HTTP session.
- **Benefit**: This hard-clears any "half-open" or residual sessions from the previous slot, ensuring the server receives a "Clean Handshake" and preventing the "Rejected" status caused by session overlap.

### 5. Dedicated Reporting Architecture
- **Lean Codebase**: Stripped all obsolete and high-risk command logic including **OTA (Over-the-Air Update)**, **Remote REBOOT**, and **Remote PING** parsing.
- **Foolproof Engine**: The system is now a dedicated reporting tool, eliminating the complexity and potential security/stability risks associated with remote command listeners.

---
## 🛠 Current System Configuration (v5.2)
- **Core**: ESP32-D0WD-V3 (v3.1) @ 80MHz
- **System Type**: `2` (TWS-RF)
- **Unit ID**: `WS1923` (Example)
- **APN Strategy**: Smart Persistence (Airtel IoT / BSNL Fallback)
- **Precision**: 2-decimal rainfall (0.25mm / 0.50mm res)

---
*Created by Antigravity AI for Spatika Information Technologies Pvt Ltd.*
