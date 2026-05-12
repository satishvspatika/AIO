# Release Notes v5.91 - High-Resolution Wind Stability

## Overview
Version 5.91 focuses on resolving pulse aliasing issues observed on high-speed anemometers and hardening the ULP (Ultra Low Power) co-processor counting logic. It introduces surgical control over sampling resolution and debounce parameters.

## Key Changes

### 1. High-Resolution Wind Counting (ULP)
- **Increased Sampling Rate**: The ULP wakeup period has been reduced from **2000µs (2ms)** to **1000µs (1ms)**.
- **Rationale**: Previously, anemometers with narrow physical teeth (Unit B) would "dip" in reported speed at high RPM because the pulses were shorter than the ULP's debounce window. Doubling the sampling resolution ensures these narrow pulses are captured reliably.

### 2. Parameterized Wind Configuration
- **New Defines**: Added `WIND_SAMPLING_US` and `WIND_DEBOUNCE_CYCLES` to `user_config.h`.
- **Rationale**: This allows field-tuning of the sensor's sensitivity and timing without needing to modify the underlying assembly-level ULP code.

### 3. Rainfall Jitter Protection Scaling
- **Automatic Scaling**: The rainfall debounce cycles were increased from 5 to 10.
- **Rationale**: Since the ULP now samples twice as fast (1ms), we increased the cycle count to maintain the established **10ms** noise filter window for the rain bucket. This prevents false rain counts while allowing high-speed wind counting.

### 4. Calibration & Cleanup
- **WS_CALIBRATION_FACTOR**: Updated to **0.45** (from 0.4398) to fine-tune the velocity reporting based on physical cup radius.
- **Legacy Removal**: Deleted the unused `NUM_OF_TEETH` define from `globals.h` to consolidate configuration into `WIND_TEETH_COUNT`.

## Technical Impact
- **Sampling Frequency**: 1000Hz (was 500Hz).
- **Min Pulse Width (Wind)**: ~2ms (was ~4ms).
- **Rain Filter Window**: Maintained at 10ms.
- **Power Impact**: Negligible (ULP remains the primary counter during deep sleep).

---
*Date: May 12, 2026*
*Build: AIO9_5.0_v5.91*
