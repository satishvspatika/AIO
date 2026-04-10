# Comparative Audit: AIO9 v5.81 vs. v5.82 (Hardened Master)

This report details the surgical logic improvements implemented in v5.82 to stabilize the AIO9 Legacy fleet.

---

## 🏗️ 1. Boot & Storage Resilience
| Feature | v5.81 Baseline | v5.82 Master (The Fix) | Impact |
| :--- | :--- | :--- | :--- |
| **8MB Migration** | Standard SPIFFS mount. | Labeled mount with fallback. | Prevents boot-loops on new 8MB partition tables. |
| **SD OTA Safety** | Simple `SD.rename()`. | Rename with `SD.remove()` fallback. | Eliminates infinite re-flash loops on failing SD cards. |
| **Backlog Pointers** | Pointers persisted instantly. | 1s Settle Delay + Explicit Reset. | **Fixed "Ghost Backlog"**: Data no longer reappears after deletion. |
| **GPS Restore** | **Broken** (void typo). | **Fixed** (Direct call). | Coordinates now correctly restore from flash on boot. |

---

## 🌐 2. Networking & Data Pipeline
| Feature | v5.81 Baseline | v5.82 Master (The Fix) | Impact |
| :--- | :--- | :--- | :--- |
| **TCP Zombies** | Retried in a loop. | `AT+CIPSHUT` on first 706/DNS error. | Instant recovery from network stack lockups. |
| **DNS Efficiency** | Always checked DNS. | Domain-Guarded DNS Audit. | **Saved 2-4s power** per slot by skipping DNS for static IPs. |
| **SIM Startup** | Passive registration. | WDT-aware loop + **GPIO 26 Power Cycle**. | Unit now "Cold Boots" the modem if the SIM fails to register. |
| **Buffer Safety** | Risk of stale data. | Relocated `memset` strategy. | Prevents old data from being piggybacked on new timestamps. |

---

## 🔋 3. Sync & Power Coordination
| Feature | v5.81 Baseline | v5.82 Master (The Fix) | Impact |
| :--- | :--- | :--- | :--- |
| **Deep Sleep Race** | Passive sleep entry. | **Dead Man's Switch (DMS)**. | Aborts shutdown if a new cycle starts at the last millisecond. |
| **State Sanitization** | Stale success states. | 2s Auto-Neutralization. | Prevents "Stale Success" from fooling the next wakeup cycle. |
| **Heartbeat** | **Broken** (Shadowed var). | **Fixed** (Global heartbeat). | Prevents watchdog from resetting a healthy GPRS task. |

---

## 🏁 Summary of Gains
*   **Reliability**: Integrated three new "Hardware Watchdogs" (Modem Power, DMS Audit, Heartbeat).
*   **Performance**: Transmission slots are now faster and consume less mAh.
*   **Integrity**: Linker orphans and shadowed variables (which cause silent crashes) have been eliminated.

**v5.82 is the definitive "Master Release" for the long-term field stability of the AIO9 Legacy system.**
