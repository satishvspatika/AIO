# Release Notes: Firmware v6.01 (Production Hardening)

**Compared to base v5.68**

### 🚀 Key Improvements & Performance
*   **Battery Safety Patch:** Fixed an infinite loop in the GPRS task when signal is too weak for a live upload. The system now correctly enters deep sleep instead of pegging the CPU and draining power.
*   **BSNL 4G Enabling (Modern Connectivity):**
    *   Transitioned BSNL SIMs from "Always 2G" to **"4G-First/Adaptive Fallback"**. 
    *   Unified registration logic to check both **LTE (CEREG)** and **GSM (CREG)** platforms for all carriers.
    *   **20-second Safety Fallback:** If registration taking too long, the modem automatically kicks down to 2G (`CNMP=13`) for reliability.
*   **GPS Hang Resolution:**
    *   Removed a 5-second blind `vTaskDelay` that caused the UI to freeze during manual GPS send.
    *   Implemented real-time LCD status: `DIALING GPS...` → `WAITING CMGS...`.
    *   Added a 2s delay for `GPS: NO LOCK` so operators can read the error before the caller overwrites it.
*   **Secure OTA & Authentication (Phase 2):**
    *   Added **OTA Binary MD5 Verification** (`Update.setMD5()`) to prevent data corruption during flash.
    *   Added **`X-Auth-Key` Header injection** for health reports to satisfy VPS-side security requirements.

### 🛠️ Stability & Gap-Fill Fixes (08:30 AM Midnight Boundary)
*   **The "Midnight Rollover" Fix:** All gap calculations now use modulo-96 logic (`(last_sampleNo + 1 + i) % 96`). This ensures perfect data continuity through the 08:30 AM meteorological reset.
*   **Modular Chronology:** Replaced the fragile "stepping" time logic with a self-correcting modular approach to derive `temp_hr:temp_min` directly from the sample index.
*   **Anchor Quality Guard:** If the last record was a "synthetic" gap-fill, the system now prefers **live sensor values** for the next interpolation. This prevents cumulative drift during multi-day outages.
*   **Weather Realism:** 
    *   Raised afternoon temperature ceiling to **45°C** (handles Karnataka summers).
    *   Standardized Wind Speed (WS) cap at **2.44 m/s** (light breeze) for all gap records.
*   **TRG Metadata:** Fixed `fill_sig` to use the `-113` sentinel so gaps aren't miscounted as real data in diagnostics.

### 📦 Hardware Abstraction
*   **Keypad Abstraction:** Added support for **GPIO, PCF8574, and Nuvoton M51** via a unified `KEYPAD_TYPE` configuration.
*   **ADC Calibration:** Shifted to `ADC_ATTEN_DB_11` (ESP-IDF 5.x) to resolve compiler warnings and improve battery reading precision.

---
**Deployment Status:** Compiled & Flashed successfully to device (v6.01 / 8MB).
**Release Package:** `firmware_v6.01.zip` (Source + Server) now available.
