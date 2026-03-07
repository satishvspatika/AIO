#include "globals.h"

void stackMonitor(void *pvParameters) {
  esp_task_wdt_add(NULL);
  while (1) {
    // Helper to update persistent high water marks (lowest free stack seen)
    auto update_min = [](TaskHandle_t h, int *min_val) {
      if (h != NULL) {
        int cur = uxTaskGetStackHighWaterMark(h);
        if (cur < *min_val)
          *min_val = cur;
      }
    };

    update_min(rtcRead_h, &diag_stack_min_rtc);
    update_min(scheduler_h, &diag_stack_min_sched);
    update_min(gprs_h, &diag_stack_min_gprs);
    update_min(lcdkeypad_h, &diag_stack_min_ui);

#if DEBUG == 1
    debugln("------ [Task Stack Monitor] ------");
    debugf1("RTC Min Free: %d\n", diag_stack_min_rtc);
    debugf1("Sched Min Free: %d\n", diag_stack_min_sched);
    debugf1("GPRS Min Free: %d\n", diag_stack_min_gprs);
    debugf1("UI Min Free: %d\n", diag_stack_min_ui);
    debugln("----------------------------------");
#endif

    esp_task_wdt_reset();
    vTaskDelay(30000 / portTICK_PERIOD_MS); // Monitor every 30 seconds
  }
}
