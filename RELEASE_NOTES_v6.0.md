# Release Notes: v6.0.0 (Phase 17)
## 🎯 Overview
Version 6.0 introduces a major Hardware Abstraction Layer (HAL) upgrade, allowing the system to natively toggle between the legacy 5-Pin GPIO keypad matrix and a new strictly I2C-based PCF8574T I/O expander keypad matrix. This critical pivot recovers 5 native ESP32 GPIO pins (including the dangerous strapping pin GPIO 12) for ultra-low-power interrupt sensors.

---

## ✨ New Features

### 1. **Dual-Hardware Keypad Abstraction (PCF8574T I2C)** 📝
- **Description:** Implemented a unified C++ preprocessor framework (`ENABLE_I2C_KEYPAD`) that physically swaps hardware object instances at compile-time.
- **Use Case:** By toggling `ENABLE_I2C_KEYPAD 1` in `user_config.h`, the entire UI thread natively pivots to polling the Keypad through an I2C expansion chip (PCF8574T at address 0x20). By setting it to `0`, the system reverts exactly to the 5-Pin GPIO architecture with zero logical differences.

---

## 🔧 Core Architectural Changes

### 1. **Robust I2C Mutex Threading for UI Polling** 🔧
- **Problem:** Because `keypad.getKey()` natively checks physical button constraints across dynamic matrices without yielding, running it on an I2C bus would instigate literal hardware collisions with the BME280 sensor and RTC DS1307 if not fiercely guarded.
- **Solution:** Surgically enclosed all physical `keypad.getKey()` invocations within the master `i2cMutex` specifically and solely when `ENABLE_I2C_KEYPAD == 1`. 
- **Impact:** The UI layer now executes 100% collision-free I2C calls, sharing the bus organically with the environmental sensors, while the native GPIO compilation state remains un-mutexed to preserve original microseconds-level hardware response rates.

---

## 📋 Technical Details

### Modified Files
- `user_config.h` - Added strictly togglable `#define ENABLE_I2C_KEYPAD` macro.
- `globals.h` - Conditionally allocates `#include <Keypad_I2C.h>` or `<Keypad.h>`.
- `lcdkeypad.ino` - Added conditional `PCF8574T_ADDR` maps, embedded `keypad.begin()` into the LCD initialization block, and strictly applied `xSemaphoreTake(i2cMutex)` around runtime evaluation locks.

### Hardware Modification Manifest & Hardware Reconfiguration (For Technical Head)
If `ENABLE_I2C_KEYPAD` is set to `1`, the following physical board modifications are strictly required for the `PCF8574T` (0x20) expander IC:

1. **Matrix Routing:** Route the keypad ribbon cable logic lines off the core ESP32 (freeing GPIO 4, 12, 13, 14, 15) and wire them to the PCF8574T pins:
    - **Row 1:** Wire to PCF8574 Pin **P0**
    - **Row 2:** Wire to PCF8574 Pin **P1**
    - **Column 1:** Wire to PCF8574 Pin **P2**
    - **Column 2:** Wire to PCF8574 Pin **P3**
    - **Column 3:** Wire to PCF8574 Pin **P4**

2. **I2C Bus & Data Sync:**
    - Wire PCF8574 **SDA** to ESP32 GPIO **21**.
    - Wire PCF8574 **SCL** to ESP32 GPIO **22**.
    
3. **Power Supply Rails (CRITICAL):**
    - Wire PCF8574 **VCC** directly and exclusively to the **3.3V Always-On** system rail, *not* the switched 5V logic that powers the LCD. The IC draws `<10µA` in standby; if it loses power when the LCD deep-sleeps, physical UI wake interrupts cannot occur.
    - Wire PCF8574 **GND** to common ESP32 system GROUND. 

4. **UI Wake-Up Logic Interrupt (INT):**
    - Wire the PCF8574 **INT** (Interrupt) pin physically to **ESP32 GPIO 27**. Since the PCF is active-low and open-drain, any key press will natively pull GPIO 27 down to trigger the exact `ext0` UI Deep Sleep Wakeup sequence originally programmed for native keystrokes!

### Dependencies & Library Requirements
The UI layer requires a strict dependency shift away from the standard `Keypad.h` (Mark Stanley) implementation to natively poll the multiplexer.
- **Library Fork Required:** `Keypad_I2C` (by Joe Weaver / G.D. Young). 
- **Repository URI:** `https://github.com/joeyoung/arduino_keypads`
- **Compiler Note:** Arduino IDE's native Library Manager does not natively index this specific optimized library fork. Ensure the explicit `Keypad_I2C` class library is manually downloaded from the repository URI, extracted into `/Arduino/libraries/`, and successfully linked prior to `arduino-cli` compilation.
