#include "globals.h"

void windDirection(void *pvParameters) {
  esp_task_wdt_add(NULL);
  strcpy(windDir_str, "000");

  // In Core 3.x with HULP, MUST use legacy ADC API to avoid driver_ng conflicts
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC1_CHANNEL_3, ADC_ATTEN_DB_11); // GPIO39
  vTaskDelay(100 / portTICK_PERIOD_MS);

  for (;;) {
    esp_task_wdt_reset();

    while (ota_silent_mode) {
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      esp_task_wdt_reset();
    }

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
      vTaskDelay(10 / portTICK_PERIOD_MS);
    }
    int tempWindDir = sum / 10;
    int spread = maxVal - minVal;

    // v5.67 Disconnection Detection using ADC spread:
    // - A real sensor at 0° (North) always has slight noise -> spread >= 2
    // - A disconnected cable stuck at GND is perfectly flat -> spread == 0 AND mean == 0
    // - We require 300 consecutive flat-zero readings before marking as fault
    static int wd_fault_count = 0;
    if (tempWindDir == 0 && spread < 2) {
      wd_fault_count++;
      if (wd_fault_count > 300)
        wd_ok = false;
    } else {
      wd_fault_count = 0;
      wd_ok = true;
    }

    static bool prev_wd_ok = true;
    if (!wd_ok) {
      windDir = 0;
      if (prev_wd_ok) {
        debugln("[WD] Sensor disconnected (ADC=0, spread=0). Suppressing further prints.");
      }
    } else {
      // Physical Potentiometer Track Calibration:
      // ADC_MIN (140 raw) = 0° (North)
      // ADC_MAX (3950 raw) = 359° (North-West)
      // Values inside the dead-gap (0..139) map cleanly to 0° North
      const int ADC_MIN = 140;
      const int ADC_MAX = 3950;
      const int ADC_SPAN = ADC_MAX - ADC_MIN; // 3810

      if (tempWindDir <= ADC_MIN) {
        windDir = 0;
      } else {
        int mapped = (int)(((float)(tempWindDir - ADC_MIN) * 360.0f) / (float)ADC_SPAN);
        if (mapped < 0) mapped = 0;
        windDir = mapped % 360;
      }

      if (!prev_wd_ok) {
        debugf2("[WD] Sensor reconnected. ADC:%d -> Dir:%d deg\n", tempWindDir, windDir);
      }
    }
    prev_wd_ok = wd_ok;
    snprintf(windDir_str, sizeof(windDir_str), "%03d deg", windDir);

    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
