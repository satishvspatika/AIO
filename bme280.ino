#include <Adafruit_BME280.h>

Adafruit_BME280 bme;
BME_Type bmeType = BME_UNKNOWN;

bool initBME() {
  bool success = false;
  if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(2000)) == pdTRUE) {
    debugln("[BME] Testing I2C address 0x77...");
    success = bme.begin(0x77);
    if (!success) {
      debugln("[BME] 0x77 failed. Testing I2C address 0x76...");
      success = bme.begin(0x76);
    }

    if (success) {
      bmeType = BME_280;
      debugln("[BME] Init: SUCCESS! BME280 Found.");
      // Forced mode for low-power weather monitoring (sensor sleeps after
      // reading)
      bme.setSampling(
          Adafruit_BME280::MODE_FORCED,
          Adafruit_BME280::SAMPLING_X1, // Temp: 1x is enough for weather
          Adafruit_BME280::SAMPLING_X1, // Pressure: 1x is enough
          Adafruit_BME280::SAMPLING_X1, // Humidity
          Adafruit_BME280::FILTER_OFF); // Faster response, less processing
    } else {
      // Diagnostic check
      debugln("[BME] Init: SENSOR NOT FOUND!");
    }
    xSemaphoreGive(i2cMutex);
  }
  return success;
}

void bmeTask(void *pvParameters) {
  debugln("[BME] Task Started");
  esp_task_wdt_add(NULL);

  bool bme_status = (bmeType != BME_UNKNOWN);
  if (!bme_status) {
    bme_status = initBME();
  }

  for (;;) {
    while (ota_silent_mode) {
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      esp_task_wdt_reset();
    }

    if (bmeType != BME_UNKNOWN) {
      if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(I2C_MUTEX_WAIT_TIME)) ==
          pdTRUE) {

        // Trigger measurement (Forced Mode)
        bme.takeForcedMeasurement();

        float pres_pa = bme.readPressure(); // Read pressure in Pascals

        if (!isnan(pres_pa) && pres_pa > 30000.0) { // Valid range > 300 hPa
          pressure = pres_pa / 100.0; // Raw Station Pressure in hPa

          // Calculate Sea Level Pressure using configurable station altitude
          // Formula: P_sl = P_st / pow(1.0 - (h / 44330.769), 5.255)
          float factor = 1.0 - (station_altitude_m / SEALEVEL_CALC_FACTOR);
          sea_level_pressure = pressure / pow(factor, 5.255);

          // UI String: Show Station Pressure and Sea Level Pressure
          // Example: "906.52 | 1016.24"
          snprintf(pres_str, sizeof(pres_str), "%.2f|%.2f", pressure,
                   sea_level_pressure);
        }
        xSemaphoreGive(i2cMutex);
      } else {
      }
    } else {
      // SENSOR MISSING: enforce 0.0 and NA
      pressure = 0.0;
      sea_level_pressure = 0.0;
      strcpy(pres_str, "NA");

      // #9: Give up after 10 failed retries (~10 minutes)
      static int bme_retry_count = 0;
      static uint32_t lastBmeRetry = 0;
      if (bme_retry_count < 10) {
        if (millis() - lastBmeRetry > 60000) {
          lastBmeRetry = millis();
          bme_retry_count++;
          debugf1("[BME] Retry attempt %d/10...\n", bme_retry_count);
          initBME();
        }
      } else {
        // Permanent failure — only log once
        static bool bme_give_up_logged = false;
        if (!bme_give_up_logged) {
          debugln("[BME] Sensor permanently unavailable after 10 retries. "
                  "Stopping.");
          bme_give_up_logged = true;
        }
      }
    }

    esp_task_wdt_reset();
    vTaskDelay(5000 / portTICK_PERIOD_MS);
  }
}
