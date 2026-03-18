# Release Notes - v5.60 (March 18, 2026)
**"The Mathematical Consistency Update"**

This release completes the standardization of Station ID handling to match the `v5.56` industrial design, ensuring perfect compatibility between internal storage and legacy external servers.

### 🔄 Data & ID Normalization (v5.56 Standard)
*   **Intelligent Stripping**: Numeric Station IDs are now normalized to 4 digits (e.g., `001931` → `1931`) for all internal storage (NVS/SPIFFS). This prevents file-path confusion and ensures consistent sorting.
*   **Alphanumeric Preservation**: Alphanumeric IDs (e.g., `WS0094`) are recognized as non-numeric and preserved exactly as entered, maintaining support for specialized addon units.
*   **Automatic Expansion (6-Char Compliance)**: The system now automatically expands 4-digit numeric IDs back to 6 characters (`001931`) in all outbound communications:
    *   **FTP Filenames**: `.kwd` and `.swd` files now always use the 6-char prefix.
    *   **Data Records**: CSV/Semicolon payloads inside FTP files use the 6-char format.
    *   **HTTP JSON**: The `"StnNo"` or `"ID"` fields are expanded for server compatibility.

### 🛡️ Resilience & Loopholes
*   **Log Search Fallback**: The LCD "DT TM" search now includes a recursive fallback. If a search for `1931` fails, it automatically checks for `001931`. This ensures historical data on SD cards remains accessible after a firmware upgrade.
*   **Path Safety**: Fixed a critical scope issue in `copyFromSPIFFSToFS` where standard variable declarations were missing, causing compilation failures.
*   **GPRS Pushing**: Optimized the `should_push` logic to ensure data is moved to FTP immediately if the signal is better than `-95 dBm`, rather than waiting for the morning window.

### 🛠️ Improvements over v5.58/v5.59
*   **v5.59 Context**: Inherits the "Airtel TCP Zombie" fix and I2C Interrupt Isolation.
*   **v5.58 Context**: Inherits the `CIPSHUT` hard-bearer reset for Airtel 2G fallback.
*   **v5.60 Delta**: Adds the comprehensive Station ID mapping logic that was missing in the 5.58/5.59 branch, closing the loop on "Station ID Reverts" reported in the field.

---
*Created by Antigravity AI for Spatika Information Technologies Pvt Ltd.*
