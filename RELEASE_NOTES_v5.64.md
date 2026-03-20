# Release Notes - v5.64 (March 20, 2026)

## 🎯 Primary Focus: Data Integrity & Mask Synchronization

This release resolves the "Off-by-One" and "Zombie Counters" reported on the fleet dashboard where "Sent" counts and "HTTP Success" counts often diverged. 

### 🛠 Key Fixes & Improvements:

1. **Sent Mask Cleansing (Met-Day Rollover)**:
   - Fixed a critical bug where the `diag_sent_mask_cur` (the bitmask tracking which samples were delivered today) was NOT being cleared during the 08:30 AM meteorological rollover.
   - This caused bits from the previous day to "leak" into the current day's totals, leading to inflated "Tdy Sent (Live)" numbers.

2. **Atomic Reconstruction Safeguard**:
   - Updated `reconstructSentMasks` to perform a mandatory zeroing of all bitmasks before scanning SPIFFS log files.
   - Prevents existing RTC RAM data from merging with file data during warm starts, ensuring the mask is a true reflection of the physical log files.

3. **Unified Diagnostic Resets**:
   - Expanded the diagnostic reset block in `setup()` to handle ALL "Fresh Start" scenarios (Power-On Reset, External Reset, and Software-triggered deletes).
   - Previously, counters were only fully zeroed on SW_CPU_RESET (Reason 12), which could lead to random starting values on manual battery swaps.

4. **Counter Consistency**:
   - Explicitly reset `diag_net_data_count` at rollover to ensure the dashboard's "Live" column starts at 0 every morning.

### 🚥 Dashboard Column Impact:
- **Tdy Sent (Live)**: Will now only count samples successfully recorded and sent *since* 08:45 AM today.
- **Tdy HTTP / Backlog**: These counters will now stay in perfect sync with the bitmask symbols.
- **Ydy Sent/Stored**: Yesterday's mask is now captured from a clean state, ensuring the "Sent" count accurately reflects the previous day's successful throughput.

---
*Status: Production Ready*
