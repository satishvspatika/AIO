#include "globals.h"

void windDirection(void *pvParameters) {
  esp_task_wdt_add(NULL);
  strcpy(windDir_str, "000");

  // In Core 3.x with HULP, MUST use legacy ADC API to avoid driver_ng conflicts
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC1_CHANNEL_3, ADC_ATTEN_DB_11); // GPIO39
  vTaskDelay(100 / portTICK_PERIOD_MS);
  // Minimal delay for power stabilization
  vTaskDelay(100 / portTICK_PERIOD_MS);

  for (;;) {
    esp_task_wdt_reset();

    // Take 10 samples, tracking min and max for stuck detection
    long sum = 0;
    int minVal = 4095, maxVal = 0;
    for (int i = 0; i < 10; i++) {
      int raw = adc1_get_raw(ADC1_CHANNEL_3);
      sum += raw;
      if (raw < minVal)
        minVal = raw;
      if (raw > maxVal)
        maxVal = raw;
      vTaskDelay(10);
    }
    int tempWindDir = sum / 10;
    int spread = maxVal - minVal;

    // v5.67 Disconnection Detection using ADC spread:
    // - A real sensor at 0° (North) always has slight noise → spread >= 2
    // - A disconnected cable stuck at GND is perfectly flat → spread == 0 AND
    // mean == 0
    // - We require 30 consecutive flat-zero readings (~30 sec) before marking
    // as fault
    static int wd_fault_count = 0;
    if (tempWindDir == 0 && spread == 0) {
      wd_fault_count++;
      if (wd_fault_count > 30)
        wd_ok = false;
    } else {
      wd_fault_count = 0;
      wd_ok = true;
    }

    // Smooth 0-4095 → 0-359 mapping (mathematical - 359 naturally rolls to 0)
    if (wd_ok) {
      windDir = (tempWindDir * 360) / 4096;
      if (windDir > 359)
        windDir = 0; // Safety clamp
    } else {
      // Disconnected: report 000 (numeric, server-safe — not NA)
      windDir = 0;
      debugf2("[WD] ADC:%d spread:%d -> Disconnected\n", tempWindDir, spread);
    }
    snprintf(windDir_str, sizeof(windDir_str), "%03d deg", windDir);

    vTaskDelay(1000);
  }
}
