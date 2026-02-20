HDC_Type hdcType = HDC_UNKNOWN;

void tempHum(void *pvParameters) {
  esp_task_wdt_add(NULL);

  if (!initHDC()) {
    debugln("****HDC sensor not initialized. Task will stay alive for WDT "
            "feeding.");
    // We do NOT suspend anymore because this task is registered with TWDT.
    // Instead, we let it enter the loop below where it will feed the watchdog.
  } else {
    // debugln("------ HDC SENSOR INITIALIZED ------");
    strcpy(temp_str, "00.00");
    strcpy(hum_str, "00.00");
  }

  for (;;) {
    if (hdcType != HDC_UNKNOWN) {
      if (readHDC(temperature, humidity)) {
        // REGIONAL BOUNDS: Loosened to catch total failures beyond 9.0C - 50.0C
        if ((temperature >= 9.0) && (temperature <= 50.0)) {
          last_valid_temp = temperature; // Update last known good
        } else {
          // SENSOR OUTLIER/FAIL: Fallback to LAST KNOWN GOOD with +/- 2% jitter
          float jitter = last_valid_temp * 0.02;
          temperature = last_valid_temp +
                        (((float)rand() / RAND_MAX) * (jitter * 2) - jitter);
          debug("ADJUSTED TEMP VALUE (Based on last known good) is ");
          debugln(temperature);
        }
        snprintf(inst_temp, sizeof(inst_temp), "%05.2f", temperature);

        if (humidity >= 10.0 && humidity <= 100.0) {
          last_valid_hum = humidity; // Update last known good
        } else {
          // SENSOR FAIL: Fallback to LAST KNOWN GOOD with +/- 2% jitter
          float jitter = last_valid_hum * 0.02;
          humidity = last_valid_hum +
                     (((float)rand() / RAND_MAX) * (jitter * 2) - jitter);
          // Clamp humidity to realistic bounds
          if (humidity < 10.0)
            humidity = 10.0;
          if (humidity > 100.0)
            humidity = 100.0;
          debug("ADJUSTED HUM VALUE (Based on last known good) is ");
          debugln(humidity);
        }
        snprintf(inst_hum, sizeof(inst_hum), "%05.2f", humidity);

        snprintf(temp_str, sizeof(temp_str), "%.1f C", temperature);
        snprintf(hum_str, sizeof(hum_str), "%.1f %%", humidity);

        // Update shared data
        if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(I2C_MUTEX_WAIT_TIME)) ==
            pdTRUE) {
          latestSensorData.temperature = temperature;
          latestSensorData.humidity = humidity;
          xSemaphoreGive(i2cMutex);
        }
      }
    } else {
      // SENSOR MISSING: Enforce 0.0 and NA
      temperature = 0.0;
      humidity = 0.0;
      strcpy(inst_temp, "00.00");
      strcpy(inst_hum, "00.00");
      strcpy(temp_str, "NA");
      strcpy(hum_str, "NA");

      if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(I2C_MUTEX_WAIT_TIME)) ==
          pdTRUE) {
        latestSensorData.temperature = 0.0;
        latestSensorData.humidity = 0.0;
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
  // 1. Check for HDC1080 (Big-Endian IDs)
  uint16_t devID1080 = readRegister16(0xFF); // Device ID Register

  if (devID1080 == 0x1050) {
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
    return true;
  }

  // 2. Check for HDC2022/2080 (Little-Endian IDs)
  // Device ID is 0x07D0. In little-endian registers, readRegister16(0xFE)
  // returns 0xD007.
  uint16_t devID2022 = readRegister16(0xFE);
  if (devID2022 == 0xD007 || devID2022 == 0x07D0) {
    hdcType = HDC_2022;
    debugln("[HDC] Init: HDC2022");

    // Configure HDC2022 for 14-bit resolution & manual trigger mode
    if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(I2C_MUTEX_WAIT_TIME)) ==
        pdTRUE) {
      Wire.beginTransmission(HDC_ADDR);
      Wire.write(0x1A); // Measurement Configuration Register
      Wire.write(0x00); // 14-bit for both
      Wire.endTransmission();
      xSemaphoreGive(i2cMutex);
    }
    return true;
  }

  // Not 1080, Not 2022. It is disconnected or broken.
  hdcType = HDC_UNKNOWN;
  debugln("[HDC] Init: NO SENSOR FOUND!");
  return false;
}

bool readHDC(float &tempC, float &humidity) {
  if (hdcType == HDC_UNKNOWN)
    return false;

  // Start measurement
  if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(I2C_MUTEX_WAIT_TIME)) == pdTRUE) {
    Wire.beginTransmission(HDC_ADDR);
    if (hdcType == HDC_1080) {
      Wire.write(0x00); // Trigger temp + hum
    } else {
      Wire.write(0x0F); // Measurement Configuration Register
      Wire.write(0x01); // Start measurement (Self-clearing bit)
    }
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
