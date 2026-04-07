#define INST_DURATION_SEC 5
#define BUFFER_SIZE 6

void windSpeed(void *pvParameters) {
  esp_task_wdt_add(NULL);
  uint16_t pulseBuffer[BUFFER_SIZE];
  int bufferIndex = 0;

  // READ WIND SPEED TWS & TWS-RF
#if (SYSTEM == 1) || (SYSTEM == 2)
  // v5.70: Fix Issue 24 - Protect boot-time SPIFFS read with fsMutex
  if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(5000)) == pdTRUE) {
    if (SPIFFS.exists("/prevWindSpeed.txt")) {
      File fileTemp5 = SPIFFS.open("/prevWindSpeed.txt", FILE_READ);
      if (fileTemp5) {
        temp1 = fileTemp5.readStringUntil('\n');
        strcpy(prevWindSpeedAvg_str, temp1.c_str());
        fileTemp5.close();
        debugln("Loaded Prev Wind Speed Avg: " + String(prevWindSpeedAvg_str));
      }
    }
    xSemaphoreGive(fsMutex);
  }
#endif

  strcpy(windSpeedInst_str, "00.00");
  if (!ws_ok) {
    strcpy(windSpeedInst_str, "NA");
    strcpy(prevWindSpeedAvg_str, "NA");
  }

  // v7.78: Stabilization protocol. Ignore first 5s of boot noise.
  vTaskDelay(5000 / portTICK_PERIOD_MS);

  // v5.50 FIX: Do NOT re-anchor last_raw_wind_count here.
  // It is set correctly in setup() to either:
  //   (a) wind_count.val on fresh boot (prevents spike), or
  //   (b) the saved RTC value on deep sleep wakeup (preserves sleep pulses).
  // Re-anchoring here would erase all ULP pulses counted during deep sleep.
  // Only seed the circular buffer to the current hardware value.
  uint16_t initial_count = wind_count.val;
  for (int i = 0; i < BUFFER_SIZE; i++) {
    pulseBuffer[i] = initial_count;
  }

  for (;;) {
    esp_task_wdt_reset();

    // v5.52: Absolute Silence Protocol — Pause task during OTA
    while (ota_silent_mode) {
      vTaskDelay(2000 / portTICK_PERIOD_MS);
      esp_task_wdt_reset();
    }

    // 32-Bit Accumulation (Handles ULP 16-bit wraps safely)
    uint16_t current_count = wind_count.val;
    uint16_t raw_delta = (current_count >= last_raw_wind_count)
                             ? (current_count - last_raw_wind_count)
                             : (65536 + current_count - last_raw_wind_count);
    
    // v5.65 Fix: Atomic protection for 32-bit counter shared with scheduler task
    portENTER_CRITICAL(&windMux);
    total_wind_pulses_32 += raw_delta;
    last_raw_wind_count = current_count;
    portEXIT_CRITICAL(&windMux);

    // Store current count in buffer for INSTANTANEOUS calculation
    pulseBuffer[bufferIndex] = current_count;

    // Get count from X seconds ago (circularly)
    int oldestIndex = (bufferIndex + 1) % BUFFER_SIZE;
    uint16_t older_count = pulseBuffer[oldestIndex];

    // Calculate delta using the 5-second window
    float delta = (current_count >= older_count)
                      ? (float)(current_count - older_count)
                      : (float)(65536 + current_count - older_count);

    if (delta > 0) {
      // average pulses per second over the 5-second window
      // (Using 1 pulse per revolution logic, dividing by teeth count is incorrect)
      float avgPulsesPerSec = delta / (float)INST_DURATION_SEC;
      // v5.60: 4 Pulses per Revolution 
      cur_wind_speed = WS_CALIBRATION_FACTOR * (avgPulsesPerSec / 4.0);

      // Filter spikes
      if (cur_wind_speed > 35.0)
        cur_wind_speed = 0.0;
      if (cur_wind_speed < 0)
        cur_wind_speed = 0;

      snprintf(windSpeedInst_str, sizeof(windSpeedInst_str), "%.2f",
               cur_wind_speed);
      
      // R-4 Fix: Protect struct writes to prevent cross-core tearing
      portENTER_CRITICAL(&sensorDataMux);
      latestSensorData.windSpeed = cur_wind_speed;
      portEXIT_CRITICAL(&sensorDataMux);
    } else {
      cur_wind_speed = 0;
      snprintf(windSpeedInst_str, sizeof(windSpeedInst_str), "00.00");
      
      portENTER_CRITICAL(&sensorDataMux);
      latestSensorData.windSpeed = 0;
      portEXIT_CRITICAL(&sensorDataMux);
    }

    // #3 FIX: Single consolidated guard - if sensor is marked failed, override
    if (!ws_ok) {
      strcpy(windSpeedInst_str, "NA");
      strcpy(prevWindSpeedAvg_str, "NA");
      
      portENTER_CRITICAL(&sensorDataMux);
      latestSensorData.windSpeed = 0;
      portEXIT_CRITICAL(&sensorDataMux);
    }

    // Move to next buffer slot
    bufferIndex = (bufferIndex + 1) % BUFFER_SIZE;

    vTaskDelay(1000 / portTICK_PERIOD_MS); // Sample every 1 second
  }
}
