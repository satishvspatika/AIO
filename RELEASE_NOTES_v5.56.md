# Release Notes v5.56
**"Deep Diagnostics & Cellular Self-Healing"**

This version introduces aggressive self-healing mechanisms for cellular network anomalies (specifically targeting BSNL/Airtel "Ghost Contexts" and Tower Denials) and enhances remote diagnostic visibility on the Fleet Dashboard.

### 🛡️ Aggressive Cellular Self-Healing (The "BSNL 2G Trap" Fix):
- **Ghost Context Elimination (Healer)**: In strict 2G environments (like BSNL), lingering background PDP contexts produce `+CME ERROR: unknown error`. A new pre-activation cleanup routine (`AT+CGACT?`) exhaustively detects and physically kills all active "Ghost" contexts (e.g., CID 9) before attempting to activate the target APN.
- **Tower Denial (CREG 3) Immediate Recovery**: If the modem is actively rejected by the cellular tower (`+CREG: 3`), the firmware no longer waits passively. It immediately forces a network Detach and Re-attach (`AT+CGATT=0` / `AT+CGATT=1`) to negotiate a fresh path into the network.
- **Physical Modem Hard Reset**: Implemented a last-resort recovery mechanism. If registration fails 4-8 times consecutively, the ESP32 aggressively pulls the Modem VCC pin (`GPIO 26`) LOW to perform a physical power wipe of the A7672S chip.
- **Robust APN Sequences**: Enhanced fallback scanning loop through preferred arrays (default -> bsnlnet -> airtelm2msolutions -> iot -> airtelgprs) ensuring plug-and-play profile detection across varying SIM cards.

### 📊 Server Dashboard & Diagnostics Expansion:
- **Modem Mutex Tracking**: Exposed the newly developed `modem_mutex_fails` counter. The Fleet Dashboard will now visibly flag and display resource contention metrics.
- **SPIFFS Backlog Count Visibility**: The Station History table now securely retrieves and renders the exact count of unsent files residing in the embedded SPIFFS flash (`http_backlog_cnt`), displaying combinations like "M: 0 / B: 4" directly on the dashboard.
- **Export Data Enrichment**: CSV Fleet Downloads now incorporate the `HTTP_Backlog_Count` and `Modem_Mutex_Fails` metrics for bulk analytical offline parsing.

### 📟 Refined LCD UI Responsiveness:
- **Zero-Stutter Scrolling ("Linger Delay")**: Implemented a caching mechanism for exploring the `HTTP FAIL STATS` panel across the hardware keys. The system uses a cached memory reading for fast GUI toggling and only engages the expensive 1-second Flash (SPIFFS) physical file search if the user "lingers" on the screen for greater than 2 seconds, entirely eliminating frame drops and UI freezes.
- **Anti-Jump Initialization**: Realigned the LCD UI base strings to use an empty 16-character padded template, neutralizing the visual text "jump" when painting the screen.

---
*Status: Verified / Compiled / Deployed*
