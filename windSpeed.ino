#define INST_DURATION_SEC 5
#define BUFFER_SIZE 6

void windSpeed(void *pvParameters) {
  esp_task_wdt_add(NULL);
  String temp1;
  uint16_t pulseBuffer[BUFFER_SIZE];
  int bufferIndex = 0;

  // READ WIND SPEED TWS & TWS-RF
#if (SYSTEM == 1) || (SYSTEM == 2)
  if (SPIFFS.exists("/prevWindSpeed.txt")) {
    File fileTemp5 = SPIFFS.open("/prevWindSpeed.txt", FILE_READ);
    if (fileTemp5) {
      temp1 = fileTemp5.readStringUntil('\n');
      strcpy(prevWindSpeedAvg_str, temp1.c_str());
      fileTemp5.close();
      debugln("Loaded Prev Wind Speed Avg: " + String(prevWindSpeedAvg_str));
    }
  }
#endif

  strcpy(windSpeedInst_str, "00.00");
  if (!ws_ok) {
    strcpy(windSpeedInst_str, "NA");
    strcpy(prevWindSpeedAvg_str, "NA");
  }

  // Initialize buffer with current value
  uint16_t initial_count = wind_count.val;
  for (int i = 0; i < BUFFER_SIZE; i++) {
    pulseBuffer[i] = initial_count;
  }

  for (;;) {
    esp_task_wdt_reset();

    // Store current count in buffer
    pulseBuffer[bufferIndex] = wind_count.val;

    // Get count from X seconds ago (circularly)
    int oldestIndex = (bufferIndex + 1) % BUFFER_SIZE;
    uint16_t current_count = pulseBuffer[bufferIndex];
    uint16_t older_count = pulseBuffer[oldestIndex];

    // Calculate delta using the 5-second window
    float delta = (current_count >= older_count)
                      ? (float)(current_count - older_count)
                      : (float)(65536 + current_count - older_count);

    if (delta > 0) {
      // average pulses per second over the 5-second window
      float avgPulsesPerSec =
          (delta / (float)NUM_OF_TEETH) / (float)INST_DURATION_SEC;
      cur_wind_speed = WS_CALIBRATION_FACTOR * avgPulsesPerSec;

      // Filter spikes
      if (cur_wind_speed > 100.0)
        cur_wind_speed = 0;
      if (cur_wind_speed < 0)
        cur_wind_speed = 0;

      snprintf(windSpeedInst_str, sizeof(windSpeedInst_str), "%.2f",
               cur_wind_speed);
      latestSensorData.windSpeed = cur_wind_speed;
    } else {
      cur_wind_speed = 0;
      snprintf(windSpeedInst_str, sizeof(windSpeedInst_str), "00.00");
      latestSensorData.windSpeed = 0;
    }

    // CONSISTENCY: If the sensor is flagged as failed/disconnected, show NA
    if (!ws_ok) {
      strcpy(windSpeedInst_str, "NA");
      latestSensorData.windSpeed = 0;
    }

    // Move to next buffer slot
    bufferIndex = (bufferIndex + 1) % BUFFER_SIZE;

    // CONSISTENCY: If the sensor is flagged as failed/disconnected, show NA
    if (!ws_ok) {
      strcpy(windSpeedInst_str, "NA");
      strcpy(prevWindSpeedAvg_str, "NA");
      latestSensorData.windSpeed = 0;
    }

    vTaskDelay(1000 / portTICK_PERIOD_MS); // Sample every 1 second
  }
}
