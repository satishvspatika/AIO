### 🚀 Firmware v5.67 Release Notes
**Target:** ESP32 Weather Station (`AIO9_5.0`)
**Previous Version:** v5.66

v5.67 is a structural hardening update that targets user-experience improvements on the LCD, critical I2C sensor stability during sleep states, and automated intelligence regarding data structure integrity during OTA installations.

#### 🌟 1. Core System & OTA Architecture
* **Intelligent Cross-Flash SPIFFS Protection:** 
  The bootloader now analyzes the firmware `UNIT_VER` architecture prefix (`TWS9-DMC` vs. `TWSRF9-GEN`) before initializing. 
  * If a **standard OTA patch** occurs (e.g., 5.66 → 5.67), it preserves all backlog CSV files seamlessly as usual.
  * If a **Cross-Architecture flash** occurs (e.g., TWS is flashed to TWS-RF Addon), the differing system structures will automatically trigger a surgical format of incompatible CSV backlog logs (`/unsent.txt`, etc.). This prevents immediate hard-crashes caused by differing CSV column widths, while still perfectly preserving vital files (`station.txt`, `rf_res.txt`, `firmware.doc`).

#### 🛠 2. Sensor & Diagnostic Hardening
* **Sensor Fault "Self-Healing":** Reconstructed the Golden Summary logic to remove permanent fault-flag latching. If a sensor reports a transient glitch resulting in an "Unreal Value" (ERV/Z), the flag will now strictly auto-clear when the sensor starts reporting valid data on subsequent 15-minute intervals. Eliminates scenarios where a station gets needlessly stuck in "Rescue Mode" substituting fake inputs indefinitely.
* **Ambient Thermal Upper Limit:** The temperature fault threshold has been raised from `45.0°C` to `50.0°C` to prevent extreme summer ambient peaks from falsely tripping the sensor fault logic.
* **Wind-Speed Sensitivity Tune:** Removed an inadvertent duplicate `diag_ws_same_count` increment cycle that was causing "WS_STUCK" to trigger in half the intended time (5 hours instead of 10 hours).
* **Duplicate Character Fix:** Removed a leftover duplicate `H_FAULT("WS_STUCK")` generation in the Health Report string block to save HTTP payload characters.

#### 🎛 3. LCD & User Experience (UX)
* **Blinking Activity Indicators:** The LCD will now pulse/blink the text (`SENDING...` / `PLEASE WAIT...`) during manual operations (Health Send, SMS Send, GPS Send) to provide the end user with visual proof that the station hasn't frozen during lengthy cellular AT commands.
* **"Blank Slate" Station ID Edits:** Hitting `SET` on the Station ID field now instantly clears the visual buffer, eliminating the need to clunkily use arrows/zeros to overwrite old strings. Safely guarded against empty commits (saving an empty ID reverts to the previous ID).
* **I2C Paralyzation Fix ("RTC read error 2"):** Completely stripped the experimental `protectI2CPins()` functionality that ran when the LCD powered off (either via menu or 60s inactivity). This was aggressively converting the I2C physical layer to digital `INPUT` pins, preventing the active CPU from talking to the RTC/HDC1080 and immediately throwing crippling `RTC Read Error 2` logs.

#### ☔️ 4. ULP Co-Processor
* **Rainfall Debounce Optimization:** The assembly-level debounce threshold for the tipping bucket has been cleanly adjusted from 20ms (`10` loops) down to 10ms (`5` loops). This preserves field-level mechanical noise immunity but correctly permits accelerated "manual" testing via human-triggered tipping without skipping pulse registrations.
