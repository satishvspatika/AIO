#include "globals.h"
//const unsigned long tenMinutes = 10* 60 * 1000; // 10mins * 60secs/min *1000
//unsigned long previousMillis = 0;

void stackMonitor(void * pvParameters) {
  
  esp_task_wdt_add(NULL);
  while (1) {
    #if DEBUG == 1
        debugln("------");
        debugf1("[Monitor] RTC stack: %d\n", uxTaskGetStackHighWaterMark(rtcRead_h));
        debugf1("[Monitor] Scheduler stack: %d\n", uxTaskGetStackHighWaterMark(scheduler_h));
        debugf1("[Monitor] GPRS stack: %d\n", uxTaskGetStackHighWaterMark(gprs_h));
        debugf1("[Monitor] LcdKeypad stack: %d\n", uxTaskGetStackHighWaterMark(lcdkeypad_h));
        
        #if (SYSTEM == 1) || (SYSTEM == 2)
            debugf1("[Monitor] TempHum stack: %d\n", uxTaskGetStackHighWaterMark(tempHum_h));
            debugf1("[Monitor] WindSpeed stack: %d\n", uxTaskGetStackHighWaterMark(windSpeed_h));
            debugf1("[Monitor] WindDir stack: %d\n", uxTaskGetStackHighWaterMark(windDirection_h));
        #endif    
        debugln("------");
    #endif    
    esp_task_wdt_reset(); 
    vTaskDelay(30000); // every 10 seconds
  }
  
}
