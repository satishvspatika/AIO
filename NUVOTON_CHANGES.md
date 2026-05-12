# Nuvoton UI Integration Changelog

This document tracks all changes made to the AIO9_5.0 codebase to support the Nuvoton MG51-based UART UI module. It serves as a reference for what was modified, how to revert changes, and what further actions might be required.

## Phase 1: Initial Implementation (Completed)

We implemented a compile-time toggle (`USE_NUVOTON_UI`) to safely switch between the legacy I2C LCD/Matrix Keypad and the new UART Nuvoton UI without breaking existing functionality.

### 1. `user_config.h`
- **Added:** `#define USE_NUVOTON_UI 0` in the "Compile Options" section.
- **Purpose:** Acts as the master switch. `0` compiles the legacy hardware logic. `1` compiles the Nuvoton UART logic.

### 2. `globals.h`
- **Modified:** Conditionally included `<LiquidCrystal_I2C.h>` and `<Keypad.h>` based on the `USE_NUVOTON_UI` switch.
- **Added:** Defined the `NuvotonLCD` wrapper class (with methods like `clear()`, `print()`, `setCursor()`) which abstracts UART commands to act like the legacy LCD library.
- **Modified:** Conditionally defined the `extern` declaration for the global `lcd` object to point to either `NuvotonLCD` or `LiquidCrystal_I2C`.

### 3. `AIO9_5.0.ino`
- **Modified:** Conditionally instantiated the global `lcd` object as either `NuvotonLCD` or `LiquidCrystal_I2C` around line 168.

### 4. `lcdkeypad.ino`
- **Added:** Implemented all methods for the `NuvotonLCD` class. Methods like `lcd.print()` translate ASCII chars into `Serial1.write()` commands, while `lcd.clear()` sends the `0x01` byte.
- **Added:** Implemented `translate_nuvoton_key(char n_key)` to remap Nuvoton key outputs to AIO9's internal logic (`1`->`2` (UP), `2`->`5` (DOWN), etc.).
- **Modified:** Conditionally suppressed the instantiation of the legacy `keypad` object and its `pinMode` initializations.
- **Modified:** In the `lcdkeypad` task startup logic, added initialization for `Serial1` on GPIO 14 (RX) and GPIO 4 (TX) when Nuvoton is enabled, bypassing the standard `lcd.init()` I2C routines.
- **Modified:** In the main key scanning loop (`key = keypad.getKey()`), added logic to read characters from `Serial1` and debounce them instead of querying the matrix GPIO pins.

---

## How to Revert or Remove
Because the entire integration is gated behind the `#define USE_NUVOTON_UI` macro, the code effectively acts as if it was never modified when the macro is set to `0`. 

If you wish to *permanently remove* the code:
1. Delete the `NuvotonLCD` class and `translate_nuvoton_key` functions from `globals.h` and `lcdkeypad.ino`.
2. Remove the `#if USE_NUVOTON_UI` conditional blocks around the `lcd` and `keypad` object instantiations.
3. Remove the `#if USE_NUVOTON_UI` conditional blocks around the hardware initialization inside the `lcdkeypad()` task.

## Next Steps / Pending Items
- **Hardware Fix:** The Nuvoton module currently fails to transmit characters for keys 4 (RIGHT), 5 (SET), and 6 (CLR). The colleague working on the Nuvoton firmware must fix the key matrix scanning and UART transmission logic on the MG51 chip.

## Phase 2: Bug Fixes and Power Handling (Completed)

During testing, several compilation and hardware logic flaws were discovered and fixed to make `USE_NUVOTON_UI 1` fully functional.

### 1. `lcdkeypad.ino`
- **Fixed:** Conditionally compiled out the legacy `keypad` object references (`keypad.getKey()` and `keypad.getState()`) in the LCD wake-up routine when `USE_NUVOTON_UI == 1`. 
- **Fixed:** Added the missing `NO_KEY` definition when compiling for Nuvoton UI.
- **Fixed:** Added `void print(float val, int decimals);` implementation to the `NuvotonLCD` class to fix `print` method signature mismatch.
- **Modified:** Implemented UART key-polling via `Serial1` in the wake-up loop, allowing edge-triggered Nuvoton keys to successfully wake the LCD.
- **Modified:** Disabled the legacy I2C "Alive" ping (`Wire.beginTransmission(0x27)`) which is irrelevant for UART modules.

### 2. `globals.h`
- **Fixed:** Added the missing `void print(float val, int decimals = 2);` signature to the `NuvotonLCD` class definition.

### 3. Power Management Overhaul
- **Issue:** The legacy power-down logic (`digitalWrite(32, LOW)`) physically cuts the 5V power to the UI. For the passive I2C matrix, keys still work. For the active Nuvoton MG51 MCU, cutting power completely kills it, preventing it from reading keypad presses to wake the system back up.
- **Resolution:** Modified the power cut logic so that when `USE_NUVOTON_UI == 1`, `digitalWrite(32, LOW)` is skipped. Instead, only `lcd.clear()` is called to clear the screen contents. The Nuvoton module remains powered to listen for key presses over UART. *Note: This will slightly increase deep-sleep/screen-off power consumption.*

## Phase 3: Visual Shifting Bug Fixes (Completed)

During field tests, users reported that the top row text would randomly wrap and shift into the bottom row (e.g. `el.       .SIM:Airt`). 

### 1. Fixed UART Wakeup Drops
- **Issue:** The MG51 UART module enters an idle state after 1-2 seconds of silence. When the ESP32 sends the first `setCursor(0, 0)` command (`0x80`), the MG51 wakes up but drops the byte. Consequently, the next 16 characters are printed at the *old* cursor position (usually at the end of the bottom row), wrapping it improperly and shifting all text.
- **Resolution:** Updated `NuvotonLCD::setCursor()` in `lcdkeypad.ino` to transmit the cursor address byte **twice** with a 10ms delay. The first byte acts as a sacrificial wakeup ping for the MG51, while the second guarantees the cursor is positioned correctly.

### 2. Disabled LCD Heartbeat for UART UI
- **Issue:** The legacy I2C LCD has a "heartbeat" (a dot `.` flashing every 1 second) to show it hasn't frozen. Over a slow UART link without flow control, spamming UI refreshes every 1000ms increases the statistical chance of noise-induced command drops and framing errors.
- **Resolution:** Conditionally disabled the 1-second heartbeat loop inside `draw_current_page()` when `USE_NUVOTON_UI == 1`. The screen now only transmits UART data when a sensor value physically changes or a button is pressed, ensuring rock-solid visual stability.

## Phase 4: Bypassing the MG51 `0x80` Bug (Completed)

During testing, the user noticed that the top row would remain permanently frozen as `"STATION"` when scrolling, or flicker uncontrollably in Edit Mode.

- **Root Cause (The `0x80` Bug):** The Nuvoton MG51 firmware completely ignores the `0x80` command byte (which is the standard HD44780 command to move the cursor to `row 0, col 0`). This is likely due to a `byte > 0x80` instead of `byte >= 0x80` logic flaw in the custom MG51 firmware. Because `0x80` is ignored, any attempt to overwrite the top row via `lcd.setCursor(0, 0)` fails, causing the new top row text to be printed invisibly off-screen or wrapped onto the second row!
- **Resolution:** We completely stripped all reliance on `lcd.setCursor(0, 0)` for the Nuvoton UI. Instead, we now leverage `lcd.clear()` (the `0x01` byte, which works perfectly) to naturally force the cursor back to `0,0`. We introduced a "smart caching" differential drawing engine in both `draw_current_page()` and `eEditOn` modes. Now, whenever the text changes, the ESP32 clears the screen, draws the top row starting naturally from `0,0`, explicitly calls `setCursor(0, 1)` (`0xC0` - which works), and draws the bottom row. This completely bypasses the hardware bug without flickering!

## Phase 5: Edit Mode Flickering & Cursor Visibility (Completed)

During testing, the user noticed that pressing UP/DOWN while editing a field (like Time) caused the entire screen to clear and redraw, creating a frustrating flicker. Additionally, the blinking cursor was invisible.

### 1. Smart Differential Caching for Edit Mode
- **Issue:** Using `lcd.clear()` to bypass the `0x80` bug caused the screen to completely wipe every time a character was typed or changed.
- **Resolution:** Upgraded the `eEditOn` caching logic. Now, `lcd.clear()` is *only* triggered if the top row (the field name, e.g. "TIME 24hr:mm") changes—which only happens when entering/exiting edit mode. While typing or scrolling values, only the bottom row changes. The ESP32 now uses `lcd.setCursor(0, 1)` (which works perfectly on the MG51) to update the typed characters without clearing the screen at all!

### 2. Cursor Commands Implemented
- **Issue:** The `NuvotonLCD` class had empty placeholder functions for `blink()` and `cursor()`, meaning the user had no visual indicator of their cursor position while editing.
- **Resolution:** Implemented `blink()`, `noBlink()`, `cursor()`, and `noCursor()` inside the `NuvotonLCD` wrapper using standard HD44780 display control bytes (`0x0F`, `0x0C`, `0x0E`). The MG51 firmware successfully passes these bytes through, restoring full cursor visibility.
