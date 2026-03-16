# Release Notes v5.57
**"Backlog Integrity & Surgical Cleanup"**

This version resolves critical data synchronization issues where unsent records were being reported with incorrect timestamps or corrupted due to fixed-length pointer drifts in the SPIFFS flash.

### 📜 Smart Backlog Data Handling:
- **Variable-Length Pointer Logic**: Replaced the rigid "Fixed Jump" (+45 bytes) loop with a dynamic `file.position()` tracker. The firmware now reads exactly to the end of each physical line (`\n`), preventing "pointer drift" that previously caused garbage data and year-0004 timestamp errors.
- **Backlog "Time Sticking" Fix**: Corrected a variable shadowing bug in `prepare_data_and_send` where the local `charArray` pointer was not being updated for unsent records. This ensured every backlog record now carries its original historical timestamp to the server instead of being stuck at the current time (e.g., 19:30).
- **Gap & Blank Line Resilience**: Implemented a string-trimming and minimum-length check. The system now autonomously skips blank lines and record "gaps" in `unsent.txt` without bloating the server payload with empty records.

### 📟 High-Resolution Diagnostics UI:
- **Smart Backlog Counting**: The LCD "B:" counter now only increments for lines containing valid data (length > 10), providing an accurate representation of unsent records rather than a raw newline count.
- **Snappier UI Refresh**: Reduced the LCD "linger delay" from 2.0s to 0.5s. Scrolling to the *HTTP FAIL STATS* screen now provides near-instant feedback of the backlog state without sluggishness.

### 🌐 Server-Side Precision Tools:
- **Surgical Delete Feature**: Added a new route to the Spatika Fleet Server allowing for per-category station deletion. This enables administrators to remove "phantom" entries (like station 1930) from specific firmware groups (e.g., KSNDMC_TWS) without wiping the station's global history.
- **Enhanced Fleet Actions**: Integrated a "✂ Remove from Group" shortcut into the Fleet Summary dashboard for quick data hygiene.

---
*Status: Verified / Compiled / Ready for Deployment*
