import re

with open('gprs_ftp.ino', 'r') as f:
    content = f.read()

# Issue 7: Remove "+HTTPREAD:" from traps
old_traps = 'const char *traps[] = {"+HTTPREAD:", "+CREG:", "+CEREG:", "+CGEV:", "+CGREG:"};'
new_traps = 'const char *traps[] = {"+CREG:", "+CEREG:", "+CGEV:", "+CGREG:"};'
content = content.replace(old_traps, new_traps)

# Issue 5 & 6: Remove session_terminated, add retry ceiling
old_loop_start = """  int chunk_retries = 0;
  int consecutive_fails = 0;
  bool session_terminated = false;

  // === CHUNK LOOP ===
  while (true) {
    debugf("[OTA] Range Request starting at: %d\\n", actual_downloaded);
    last_activity_time = millis();"""

new_loop_start = """  int chunk_retries = 0;
  int consecutive_fails = 0;

  // === CHUNK LOOP ===
  while (true) {
    debugf("[OTA] Range Request starting at: %d\\n", actual_downloaded);
    last_activity_time = millis();

    if (chunk_retries > 40) {
      Serial.println("[OTA] [ABORT] Too many total retries. Giving up.");
      break;
    }"""
content = content.replace(old_loop_start, new_loop_start)

# Issue 4: Tail race condition
old_tail = """    unsigned long tail_start = millis();
    while (millis() - tail_start < 3000) {
        if (SerialSIT.find("OK")) break;
        esp_task_wdt_reset();
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }"""

new_tail = """    // Drain exactly 6 bytes for the expected \\r\\nOK\\r\\n tail
    char tail[8] = {0};
    int ti = 0;
    unsigned long tail_start = millis();
    while (ti < 6 && (millis() - tail_start < 3000)) {
        if (SerialSIT.available()) {
            tail[ti++] = SerialSIT.read();
        } else {
            vTaskDelay(2 / portTICK_PERIOD_MS);
        }
        esp_task_wdt_reset();
    }"""
content = content.replace(old_tail, new_tail)

# Issue 3: Remove flushSerialSIT()
old_flush = """    } else {
      Serial.printf("[OTA] Flash Write ERROR: %d - %s\\n", Update.getError(), Update.errorString());
      Update.abort(); 
      break;
    }

    flushSerialSIT(); 
    vTaskDelay(50 / portTICK_PERIOD_MS); 
    esp_task_wdt_reset();"""

new_flush = """    } else {
      Serial.printf("[OTA] Flash Write ERROR: %d - %s\\n", Update.getError(), Update.errorString());
      Update.abort(); 
      break;
    }

    vTaskDelay(50 / portTICK_PERIOD_MS); 
    esp_task_wdt_reset();"""
content = content.replace(old_flush, new_flush)

with open('gprs_ftp.ino', 'w') as f:
    f.write(content)
