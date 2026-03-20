# Release Notes - v5.65 (March 20, 2026)

## 🎯 Primary Focus: Health Report Reliability & UART Protocol

This release addresses the intermittent "Health Report Failed" errors observed on both Airtel and BSNL networks. 

### 🛠 Key Fixes & Improvements:

1. **Multi-Attempt Health Cycle**:
   - Implemented a 2-step retry loop for Health Reports, mirroring the robustness of the primary data transmission.
   - If the first "Fast Push" fails, the system now automatically retries using the **Robust Handshake** (Wait for DOWNLOAD), which is significantly more stable in low-signal or congested 2G environments.

2. **UART Poisoning Fix**:
   - Changed `SerialSIT.println(payload)` to `SerialSIT.print(payload)` in the core `send_at_cmd_data` engine.
   - This prevents extra trailing `\r\n` bytes (added by `println`) from poisoning the modem's input buffer. These extra bytes were occasionally causing the subsequent `AT+HTTPACTION` command to be misinterpreted by the modem, leading to `400 Bad Request` or `713` errors.

3. **Consolidated Response Handling**:
   - Removed redundant `HTTPREAD` calls in `send_health_report` since the consolidated "Golden" engine already handles the read and validation of the server's response.

### 🚥 Expected Impact:
- **Higher Health Hit-Rate**: Stations on stubborn 2G networks (BSNL) will now benefit from the Robust mode fallback, ensuring status reports reach the dashboard even when the "Fast" method is rejected by the tower.
- **Cleaner Modem State**: By sending exactly the number of bytes specified in `AT+HTTPDATA`, the modem's state machine remains cleaner, reducing "Zombie Socket" occurrences.

---
*Status: Production Ready*
