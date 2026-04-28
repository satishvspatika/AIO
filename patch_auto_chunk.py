import re

new_chunk_loop = r"""    vTaskDelay(50 / portTICK_PERIOD_MS);
    Serial.printf("[OTA] T2: Pre-HTTPREAD delay done at %lu\n", millis());
    snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+HTTPREAD=0,%d", chunk_total);
    SerialSIT.println(gprs_xmit_buf);

    int assembled = 0;
    bool read_failed = false;

    while (assembled < chunk_total) {
        // Parse the automatically injected +HTTPREAD: <len> header line
        char read_hdr[64] = {0};
        int rh_idx = 0;
        bool hdr_found = false;
        int sub_len = 0;
        unsigned long t_start = millis();
        
        while (millis() - t_start < 10000) {
            esp_task_wdt_reset();
            if (!SerialSIT.available()) { vTaskDelay(2 / portTICK_PERIOD_MS); continue; }
            char c = SerialSIT.read();
            if (rh_idx < 63) read_hdr[rh_idx++] = c;
            if (c == '\n') {
                char *ptr = strstr(read_hdr, "+HTTPREAD:");
                if (ptr != NULL) {
                    hdr_found = true;
                    sub_len = atoi(ptr + 10);
                    break;
                }
                rh_idx = 0;
                memset(read_hdr, 0, sizeof(read_hdr));
            }
            if (strstr(read_hdr, "ERROR") != NULL) break;
            if (strstr(read_hdr, "OK") != NULL && assembled > 0) break; 
        }
        
        if (!hdr_found || sub_len <= 0) {
            Serial.printf("[OTA] [ERR] Sub-header timeout/invalid at assembled=%d (hdr: %s)\n", assembled, read_hdr);
            read_failed = true;
            break;
        }
        
        // Read exactly sub_len bytes into the buffer
        int got = 0;
        unsigned long startRead = millis();
        while (got < sub_len && millis() - startRead < 15000) {
            if (got % 128 == 0) esp_task_wdt_reset();
            if (SerialSIT.available()) {
                if (assembled + got < chunk_total) {
                    buf[assembled + got] = SerialSIT.read();
                } else {
                    SerialSIT.read(); // overflow protection
                }
                got++;
            } else {
                vTaskDelay(1 / portTICK_PERIOD_MS);
            }
        }
        
        if (got != sub_len) {
            Serial.printf("[OTA] [ERR] Sub-read incomplete: got %d of %d\n", got, sub_len);
            read_failed = true;
            break;
        }
        
        assembled += got;
        Serial.printf("[OTA] Sub-chunk parsed: %d bytes (Total: %d/%d)\n", sub_len, assembled, chunk_total);

        // Consume the trailing \r\n that the A7672S injects between its automatic chunks
        if (assembled < chunk_total) {
            unsigned long crlf_start = millis();
            int crlf = 0;
            while (millis() - crlf_start < 2000 && crlf < 2) {
                if (SerialSIT.available()) {
                    char c = SerialSIT.read();
                    if (c == '\r' || c == '\n') crlf++;
                } else {
                    vTaskDelay(2 / portTICK_PERIOD_MS);
                }
                esp_task_wdt_reset();
            }
        }
    }

    if (read_failed) {
        flushSerialSIT();
        chunk_retries++; consecutive_fails++;
        esp_task_wdt_reset();
        continue;
    }

    int got = assembled;
    Serial.printf("[OTA] T4: Assembly complete at %lu. Bytes: %d\n", millis(), got);

    // Consume the final \r\nOK\r\n tail for the entire HTTPREAD command
    unsigned long tail_start = millis();
    int ok_ti = 0;
    bool tail_ok = false;
    while (millis() - tail_start < 3000) {
        if (SerialSIT.available()) {
            char tc = SerialSIT.read();
            if (tc == 'O') {
                ok_ti = 1;
            } else if (tc == 'K' && ok_ti == 1) {
                tail_ok = true;
                break;
            } else {
                ok_ti = 0; 
            }
        } else {
            vTaskDelay(5 / portTICK_PERIOD_MS);
        }
        esp_task_wdt_reset();
    }

    Serial.printf("[OTA] T5: Final Tail-Guard check at %lu, UART_avail=%d\n", millis(), SerialSIT.available());

    if (!tail_ok) {
        Serial.println("[OTA] [ERR] Final Tail OK not found/timeout. Retrying chunk.");
        flushSerialSIT();
        chunk_retries++; consecutive_fails++;
        continue;
    }
"""

with open('gprs_ftp.ino', 'r') as f:
    content = f.read()

start_marker = r"    vTaskDelay\(50 / portTICK_PERIOD_MS\);\n    Serial\.printf\(\"\[OTA\] T2: Pre-HTTPREAD delay done at \%lu\\n\", millis\(\)\);"
end_marker = r"    if \(!tail_ok\) \{\n        Serial\.println\(\"\[OTA\] \[ERR\] Tail OK not found/timeout\. Retrying chunk\.\"\);\n        flushSerialSIT\(\);\n        chunk_retries\+\+; consecutive_fails\+\+;\n        continue;\n    \}"

# Find indices
start_match = re.search(start_marker, content)
end_match = re.search(end_marker, content)

if start_match and end_match:
    final_content = content[:start_match.start()] + new_chunk_loop + content[end_match.end():]
    with open('gprs_ftp.ino', 'w') as f:
        f.write(final_content)
    print("Replaced successfully!")
else:
    print("Failed to find markers.")
