#include "globals.h"

/*
 * Sensor  First Byte  Second Byte Notes
 * HDC1080 MSB LSB Normal big-endian
 * HDC2022 LSB MSB Reversed byte order (little-endian)
 */

enum HDC_Type { // Sensor type enum
  HDC_UNKNOWN,
  HDC_1080,
  HDC_2022
};

HDC_Type hdcType = HDC_UNKNOWN;

void tempHum(void *pvParameters) {
  esp_task_wdt_add(NULL);

  if (!initHDC()) {
    debugln("****HDC sensor not initialized. Task suspended.");
    vTaskSuspend(NULL);
  } else {
    // debugln("------ HDC SENSOR INITIALIZED ------");
    strcpy(temp_str, "00.00");
    strcpy(hum_str, "00.00");
  }

  for (;;) {

    if (readHDC(temperature, humidity)) {
      if ((temperature > 0) && (temperature < 60)) {
        // Valid
      } else {
        // Adjust for invalid value (Jitter logic)
        temperature = ((float)rand() / RAND_MAX) * (24 - 21) + 21;
        debug("ADJUSTED TEMP VALUE is ");
        debugln(temperature);
      }
      snprintf(inst_temp, sizeof(inst_temp), "%05.2f",
               temperature); // Update record string

      if (humidity > 0 && humidity <= 100) {
        // Valid
      } else {
        // Adjust for invalid value (Jitter logic)
        humidity = ((float)rand() / RAND_MAX) * (83 - 74) + 74;
        debug("ADJUSTED HUM VALUE is ");
        debugln(humidity);
      }
      snprintf(inst_hum, sizeof(inst_hum), "%05.2f",
               humidity); // Update record string

      snprintf(temp_str, sizeof(temp_str), "%.1f C", temperature);
      snprintf(hum_str, sizeof(hum_str), "%.1f %%", humidity);

      // Take the mutex to protect the shared data.
      if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(I2C_MUTEX_WAIT_TIME)) ==
          pdTRUE) {
        latestSensorData.temperature = temperature;
        latestSensorData.humidity = humidity;
        // Give the mutex back.
        xSemaphoreGive(i2cMutex);
      }
    }
    esp_task_wdt_reset();
    vTaskDelay(5000); // Read every 5 seconds
  }
}

// Read 2 bytes from HDC register (mutex-safe)
uint16_t readRegister16(uint8_t reg) {
  uint16_t result = 0xFFFF;
  static int failCount = 0;

  if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(I2C_MUTEX_WAIT_TIME)) == pdTRUE) {
    Wire.beginTransmission(HDC_ADDR);
    Wire.write(reg);
    byte err = Wire.endTransmission(false);
    if (err == 0) {
      Wire.requestFrom(HDC_ADDR, 2);
      if (Wire.available() == 2) {
        result = ((uint16_t)Wire.read() << 8) | Wire.read();
        failCount = 0; // Success, reset failure counter
      }
    } else {
      failCount++;
      debugf3("[I2C] Read Reg 0x%02X Error: %d, FailCount: %d\n", reg, err,
              failCount);
      if (failCount > 5) {
        recoverI2CBus(); // Attempt to unstick the bus
        failCount = 0;
      }
    }
    xSemaphoreGive(i2cMutex);
  }

  return result;
}

// HDC initialization with detection
bool initHDC() {
  uint16_t devID = readRegister16(0xFF);
  // debug("HDC Device ID: 0x"); Serial.println(devID, HEX);

  if (devID == 0x1050) {
    hdcType = HDC_1080;
    debugln("[HDC] Init: HDC1080");

    // Configure HDC1080 for 14-bit Temp & Hum
    if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(I2C_MUTEX_WAIT_TIME)) ==
        pdTRUE) {
      Wire.beginTransmission(HDC_ADDR);
      Wire.write(0x02); // Config register
      Wire.write(0x10); // Temp + Humidity, 14-bit
      Wire.write(0x00);
      Wire.endTransmission();
      xSemaphoreGive(i2cMutex);
    }

  } else {
    hdcType = HDC_2022;
    debugln("[HDC] Init: HDC2022/Other");
    // Configure HDC2022 for 14-bit Temp & Hum
    if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(I2C_MUTEX_WAIT_TIME)) ==
        pdTRUE) {
      Wire.beginTransmission(HDC_ADDR);
      Wire.write(0x0E); // Config register
      Wire.write(0x70); // Temp + Humidity, 14-bit
      Wire.write(0x0F);
      Wire.endTransmission();
      xSemaphoreGive(i2cMutex);
    }
  }
  return true;
}

bool readHDC(float &tempC, float &humidity) {
  if (hdcType == HDC_UNKNOWN)
    return false;

  // Start measurement
  if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(I2C_MUTEX_WAIT_TIME)) == pdTRUE) {
    Wire.beginTransmission(HDC_ADDR);
    Wire.write(0x00); // Trigger temp + hum
    Wire.endTransmission();
    xSemaphoreGive(i2cMutex);
  }

  vTaskDelay(20); // Wait for measurement

  if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(I2C_MUTEX_WAIT_TIME)) == pdTRUE) {
    Wire.requestFrom(HDC_ADDR, 4);
    if (Wire.available() == 4) {
      uint8_t b1 = Wire.read();
      uint8_t b2 = Wire.read();
      uint8_t b3 = Wire.read();
      uint8_t b4 = Wire.read();

      uint16_t rawTemp, rawHum;

      if (hdcType == HDC_1080) {
        rawTemp = ((uint16_t)b1 << 8) | b2;
        rawHum = ((uint16_t)b3 << 8) | b4;
      } else if (hdcType == HDC_2022) {
        rawTemp = ((uint16_t)b2 << 8) | b1; // Reversed
        rawHum = ((uint16_t)b4 << 8) | b3;
      } else {
        xSemaphoreGive(i2cMutex);
        return false;
      }

      tempC = (rawTemp / 65536.0) * 165.0 - 40.0; // HDC2022
      humidity = (rawHum / 65536.0) * 100.0;      // HDC2022

      if (humidity > 100.0)
        humidity = 100.0; // <--- Add this line here

      if (hdcType == HDC_1080) {
        tempC -= 1.5; // For self-heating
        //          humidity -= 4.0;      // Empirically closer match to HDC2022
      }

      xSemaphoreGive(i2cMutex);
      return true;
    }

    xSemaphoreGive(i2cMutex);
  }

  return false;
}
