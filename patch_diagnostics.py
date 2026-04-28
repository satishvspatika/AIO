import re

with open('gprs_ftp.ino', 'r') as f:
    content = f.read()

# Add T1
content = content.replace('SerialSIT.println("AT+HTTPACTION=0");\n    if (!waitForResponse("+HTTPACTION:", 95000)) {',
                          'Serial.printf("[OTA] T1: HTTPACTION started at %lu\\n", millis());\n    SerialSIT.println("AT+HTTPACTION=0");\n    if (!waitForResponse("+HTTPACTION:", 95000)) {')

# Add T2
content = content.replace('vTaskDelay(50 / portTICK_PERIOD_MS);\n    snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+HTTPREAD=0,%d", chunk_total);',
                          'vTaskDelay(50 / portTICK_PERIOD_MS);\n    Serial.printf("[OTA] T2: Pre-HTTPREAD delay done at %lu\\n", millis());\n    snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+HTTPREAD=0,%d", chunk_total);')

# Add HDR_PARSED and T3
hdr_block = """    if (!hdr_found) {
      Serial.println("[OTA] [WARN] HTTPREAD Header Timeout/Error. Cooling down...");
      vTaskDelay(3000 / portTICK_PERIOD_MS);
      chunk_retries++; consecutive_fails++; esp_task_wdt_reset();
      continue;
    }"""
hdr_replacement = hdr_block + """\n
    Serial.printf("[OTA] HDR_PARSED: hdr_buf='%s' len=%d UART_avail=%d\\n", read_hdr, rh_idx, SerialSIT.available());
    Serial.printf("[OTA] T3: Header parsed at %lu, UART_now=%d\\n", millis(), SerialSIT.available());
"""
content = content.replace(hdr_block, hdr_replacement)

# Add T4
t4_block = """    if (got != chunk_total) {
        Serial.printf("[OTA] [ERR] Read Timeout/Incomplete: got %d of %d\\n", got, chunk_total);
        chunk_retries++; consecutive_fails++; esp_task_wdt_reset();
        continue;
    }"""
t4_replacement = t4_block + """\n\n    Serial.printf("[OTA] T4: Binary read complete at %lu\\n", millis());"""
content = content.replace(t4_block, t4_replacement)

# Add Checksum
chk_block = """    esp_task_wdt_reset();
    if (Update.write(buf, got) == (size_t)got) {
      actual_downloaded += got;
      chunk_retries = 0;
      consecutive_fails = 0;

      Serial.printf("[OTA] Progress: %d bytes written (HEAD: %02X %02X %02X %02X)\\n",
                    actual_downloaded, buf[0], buf[1], buf[2], buf[3]);"""
chk_replacement = """    esp_task_wdt_reset();
    if (Update.write(buf, got) == (size_t)got) {
      actual_downloaded += got;
      chunk_retries = 0;
      consecutive_fails = 0;

      uint32_t chk = 0;
      for (int i = 0; i < got; i++) chk += buf[i];
      Serial.printf("[OTA] Chunk written. Offset=%d Size=%d Checksum=%lu FirstByte=0x%02X LastByte=0x%02X\\n",
                     actual_downloaded - got, got, chk, buf[0], buf[got-1]);

      Serial.printf("[OTA] Progress: %d bytes written (HEAD: %02X %02X %02X %02X)\\n",
                    actual_downloaded, buf[0], buf[1], buf[2], buf[3]);"""
content = content.replace(chk_block, chk_replacement)

# Add T5
t5_block = """    unsigned long tail_start = millis();
    while (millis() - tail_start < 3000) {
        if (SerialSIT.find("OK")) break;
        esp_task_wdt_reset();
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }

    if (SerialSIT.available() > 10) {"""
t5_replacement = """    unsigned long tail_start = millis();
    while (millis() - tail_start < 3000) {
        if (SerialSIT.find("OK")) break;
        esp_task_wdt_reset();
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }

    Serial.printf("[OTA] T5: Tail-Guard check at %lu, UART_avail=%d\\n", millis(), SerialSIT.available());
    if (SerialSIT.available() > 10) {"""
content = content.replace(t5_block, t5_replacement)

with open('gprs_ftp.ino', 'w') as f:
    f.write(content)
