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

    // Take 10 samples and use average for stability
    long sum = 0;
    for (int i = 0; i < 10; i++) {
      sum += adc1_get_raw(ADC1_CHANNEL_3);
      vTaskDelay(10);
    }

    int tempWindDir = sum / 10;
    static int wd_fault_count = 0;

    // Fault Detection: If railing at extremes (0-25 or 4090-4095) for 3s,
    // marking as missing
    if (tempWindDir <= 25 || tempWindDir >= 4090) {
      wd_fault_count++;
      if (wd_fault_count > 3)
        wd_ok = false;
    } else {
      wd_fault_count = 0;
      wd_ok = true;
    }

    // Logic: Map 0-4095 to 0-359 only if sensor is present
    if (wd_ok) {
      windDir = (tempWindDir * 360) / 4096;
      snprintf(windDir_str, sizeof(windDir_str), "%03d deg", windDir);
    } else {
      windDir = 0;
      strcpy(windDir_str, "NA");
    }

    latestSensorData.windDirection = windDir;
    vTaskDelay(1000);
  }
}
