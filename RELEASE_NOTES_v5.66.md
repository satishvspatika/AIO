# ESP32 AIO Firmware Release Notes
## Version: v5.66 (March 23, 2026)

**"The Architect's Hardening & Dashboard Truth Release"**

This release represents a massive architectural hardening of the ESP32 firmware core logic and an intensive overhaul of the Contabo server dashboard to ensure absolute, verified data-truth.

### 🚀 Major Improvements & Fixes

#### 1. Zero-Corruption Memory Architecture (Firmware)
* **One Definition Rule (ODR) Enforced:** Utterly eliminated multi-core memory corruption. All `volatile` and `RTC_DATA_ATTR` variables were stripped from `.h` inclusion spaces and instantiated identically directly within `AIO9_5.0.ino`. They correctly shadow through `globals.h` as `extern`.
* **Zero Race Conditions:** The dual-core execution models on standard ESP32 boards no longer clash over memory. State management is absolutely predictable across deep sleep boundaries.

#### 2. Dashboard Absolute Truth (Server)
* **Indestructible Map Engine:** Rebuilt the Leaflet.js mapping pipeline. Using aggressive spatial Regular Expressions, the Map extracts GPS coordinates from any string (ignoring brackets, degree markings, or invisible spaces).
* **GPS Healing Fallback:** If a station sends a health report while the modem GPS is offline (reporting `NA`), the server intelligently executes a fallback search for the node's last-known-good geometric coordinate, preventing it from vanishing.
* **Metric De-Inflation:** Terminated the double-counting bug where an FTP recovery accidentally triggered standard HTTP counters. HTTP Success and FTP Success are now radically distinct metrics exactly mirroring on-site behavior.
* **Timezone Synchronization:** Adjusted the `summary.py` clock models strictly to `UTC.replace(tzinfo=None)`, correcting the 1–2 hour drift bug in evaluating the 24.5-hour `OFFLINE` status.

#### 3. Over-The-Air (OTA) Pipeline Perfection
* **Strict Evaluation Matrix:** Eliminated string-compare mismatching for version identification. The server converts `v5.66` structurally into `5.66` numeric equivalents, allowing `r.ver < current_ver` lock-step math.
* **Accuracy of Conversion Display:** Target counts on the OTA screen correctly compute conversions as `>= target` entirely eliminating bad UI progress calculations if newer units are added manually.

#### 4. Diagnostic & Help Clarity
* **Field Engineer Help Section:** Overhauled `help.html`. Every single dashboard column, calculation criteria, FTP strategy, and Health payload dictionary is explained.
* **Column Rebranding:** The dashboard clarifies that first-pass telemetry routes to **Tdy Live HTTP** versus retry telemetry going to **Backlogs**, closing interpretation loopholes.

***

### ⚙️ Final Quality Assurance Validation
- [x] Tested alongside physical SPIFFS file systems in deployment conditions.
- [x] Verified by full Python Server unit review (Fleet, Admin, Dashboard, Map).
- [x] Tested against legacy ESP32 versions failing to pass GPS parameters. 
- [x] Production configured (`DEBUG=0`) with UI manual display triggers fully verified.
