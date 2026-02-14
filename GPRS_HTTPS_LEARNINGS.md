# Research & Development: SIMCOM A7672 + Google HTTPS Redirection

This document tracks the iterative attempts, failures, and architectural leanings discovered during the development of the Remote Command & Health Reporting system (v5.2 - v7.0).

## 1. Core Problem Statement
The device must communicate with Google Apps Script (HTTPS) to:
1. Update a health spreadsheet.
2. Fetch remote commands (e.g., REBOOT, OTA).
**Complexity:** Google uses multi-stage 302 redirects across different domains (`script.google.com` -> `script.googleusercontent.com`), which often crashes the SIMCOM SSL stack or causes certificate rejections.

## 2. Technical Learnings (The "Gotchas")

### SSL Error Codes
*   **+HTTPACTION: 0,715**: "SSL Handshake Failure" / Stack Crash.
    *   *Cause:* Usually happens when the modem's SSL engine state is corrupted by previous attempts or when memory buffer overflows during a redirect.
    *   *Solution:* Use a "Clean Slate" approach (Full `AT+HTTPTERM` then `AT+HTTPINIT`) for every hop.
*   **+HTTPACTION: 0,706**: "Peer Alert".
    *   *Cause:* Google's servers rejecting the device's "Hello." Usually due to an incorrect CMOS/RTC clock on the modem (Certificate validation fails).
    *   *Solution:* Force-sync the ESP32 RTC time to the modem using `AT+CCLK` before the handshake.

### The SNI (Server Name Identification) Trap
SIMCOM modules do not automatically update the SNI header when a redirect moves the request to a new host (e.g., from `script.google.com` to `script.googleusercontent.com`).
*   **The Failure:** If the host changes but the SNI stays on the old domain, Google drops the connection (SSL 706).
*   **The Fix:** Manual redirection logic must parse the `Location` header, extract the new domain, and manually update the SNI (`AT+CSSLCFG="sni",...`) before the next jump.

### Redirection Strategies
*   **Built-in (`AT+HTTPPARA="REDIR",1`)**: 
    *   *Pros:* Minimal code, handles jumps internally.
    *   *Cons:* Fails on domain changes because it doesn't update SNI. Hard to debug.
*   **Manual Monitoring**: 
    *   *Pros:* Visibility into headers, allows SNI surgical updates.
    *   *Cons:* Significant overhead in AT command processing and timing.

---

## 3. Iteration History

### v6.0 - v6.3: "The Clean Slate"
*   **Attempt:** Resetting the HTTP engine for every hop.
*   **Result:** Partial success, but 715 errors persisted due to rapid re-initialization.

### v6.4 - v6.6: "The Clock & Cipher Sync"
*   **Attempt:** Synced RTC to Modem and forced TLS 1.2 with specific ciphers.
*   **Result:** Resolved 706 (Peer Alert) errors. The first request started succeeding reliably, but jumps to `googleusercontent` still failed.

### v6.7 - v6.8: "The Built-in Redir Pivot"
*   **Attempt:** Letting the modem handle jumps automatically.
*   **Result:** Sheets updated (Success!), but commands failed to fetch because the internal redirection didn't update SNI for the command-hosting domain.

### v7.0: "The Unyielding Retry"
*   **Attempt:** Guaranteed re-init with 5 retries and official SNI sequences.
*   **Result:** Functional but high latency and complexity.

---

## 4. Current Architecture (Low Power / Production)
*   **Objective:** Robust health reporting with zero command overhead.
*   **Logic:**
    1. Sync Clock (for SSL stability).
    2. Set SNI for `script.google.com`.
    3. Execute request (Sheets update happens on the first hit).
    4. Ignore redirects and terminate immediately to save power.
