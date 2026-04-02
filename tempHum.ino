HDC_Type hdcType = HDC_UNKNOWN;

void tempHum(void *pvParameters) {
  esp_task_wdt_add(NULL);

  // Note: initHDC() is now called in setup() to decide if this task runs.
  if (hdcType != HDC_UNKNOWN) {
    strcpy(temp_str, "00.00");
    strcpy(hum_str, "00.00");
  } else {
    strcpy(temp_str, "NA");
    strcpy(hum_str, "NA");
  }

  static int failCount = 0;
  for (;;) {
    esp_task_wdt_reset();

    // v5.52: Absolute Silence Protocol — Pause task during OTA to prevent
    // crosstalk
    while (ota_silent_mode) {
      vTaskDelay(2000 / portTICK_PERIOD_MS);
      esp_task_wdt_reset();
    }
    if (hdcType != HDC_UNKNOWN) {
      float t_raw, h_raw;
      if (readHDC(t_raw, h_raw)) {
        failCount = 0; // Success
        // v5.49: Atomic update of both variables under a single mutex lock
        if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(I2C_MUTEX_WAIT_TIME)) ==
            pdTRUE) {
          // Temperature Logic: Winter-hardened range (-39 to +85)
          // v5.65 fix: Exactly -40.0C is often a result of raw=0 (bus failure).
          float real_temp = temperature; // preserve state
          if ((t_raw > -39.99) && (t_raw <= 85.0)) {
            real_temp = t_raw;
            last_valid_temp = real_temp; // Anchor ONLY from valid raw
            temperature = real_temp;
          } else if (last_valid_temp > -40.1) {
            // Jitter around anchor - never update anchor from invalid t_raw here
            float jitter = last_valid_temp * 0.01;
            temperature =
                last_valid_temp +
                (((float)(esp_random() & 0xFFFF) / 65535.0) * (jitter * 2) -
                 jitter);
          } else {
            temperature = 0.0;
          }

          // Humidity Logic
          if (h_raw >= 0.0 && h_raw <= 100.0) {
            humidity = h_raw;
            last_valid_hum = humidity;
          } else if (last_valid_hum > 0.1) {
            float jitter = last_valid_hum * 0.01;
            humidity =
                last_valid_hum +
                (((float)(esp_random() & 0xFFFF) / 65535.0) * (jitter * 2) -
                 jitter);
            if (humidity < 0.0)
              humidity = 0.0;
            if (humidity > 100.0)
              humidity = 100.0;
            // Removed: last_valid_hum = humidity; // DO NOT update anchor with
            // noise
          } else {
            humidity = 0.0;
          }

          xSemaphoreGive(i2cMutex);
        }

        // UI Strings (ONLY) - Do not touch global production buffers
        snprintf(temp_str, sizeof(temp_str), "%.1f C", temperature);
        snprintf(hum_str, sizeof(hum_str), "%.1f %%", humidity);
      } else {
        // readHDC Failed: Sensor likely unplugged
        failCount++;
        if (failCount >= 3) {
          debugln("[HDC] Sensor disconnected during runtime.");
          hdcType = HDC_UNKNOWN;
        }
        if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(I2C_MUTEX_WAIT_TIME)) ==
            pdTRUE) {
          temperature = 0.0;
          humidity = 0.0;
          xSemaphoreGive(i2cMutex);
        }
        strcpy(temp_str, "NA");
        strcpy(hum_str, "NA");
      }
    } else {
      // SENSOR MISSING: Enforce 0.0 and NA
      if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(I2C_MUTEX_WAIT_TIME)) ==
          pdTRUE) {
        temperature = 0.0;
        humidity = 0.0;
        // Fallback to BME if HDC is missing
        if (bmeType != BME_UNKNOWN) {
          // v5.70: H-1 Over-Sampling Logic (5-sample Median Filter)
          static float bT_buf[5], bH_buf[5];
          static int b_idx = 0;
          static bool b_init = false;

          float bmeTemp = bme.readTemperature();
          float bmeHum = bme.readHumidity();

          if (!isnan(bmeTemp) && bmeTemp > -40.0 && bmeTemp < 85.0 &&
              !isnan(bmeHum) && bmeHum >= 0.0 && bmeHum <= 100.0) {
            
            if (!b_init) { // Prime buffer on first valid read
              for(int i=0; i<5; i++) { bT_buf[i] = bmeTemp; bH_buf[i] = bmeHum; }
              b_init = true;
            }
            bT_buf[b_idx] = bmeTemp;
            bH_buf[b_idx] = bmeHum;
            b_idx = (b_idx + 1) % 5;

            // Simple In-place Median (5 elements)
            auto get_med = [](float* b) {
              float s[5]; memcpy(s, b, 20); // 5 * 4 bytes
              for(int i=0; i<4; i++) for(int j=0; j<4-i; j++) 
                if(s[j]>s[j+1]) { float t=s[j]; s[j]=s[j+1]; s[j+1]=t; }
              return s[2];
            };

            float fTemp = get_med(bT_buf);
            float fHum = get_med(bH_buf);

            portENTER_CRITICAL(&sensorDataMux);
            latestSensorData.temperature = fTemp;
            portEXIT_CRITICAL(&sensorDataMux);
            
            temperature = fTemp;
            snprintf(temp_str, sizeof(temp_str), "%.1f C(B)", temperature);

            portENTER_CRITICAL(&sensorDataMux);
            latestSensorData.humidity = fHum;
            portEXIT_CRITICAL(&sensorDataMux);
            
            humidity = fHum;
            snprintf(hum_str, sizeof(hum_str), "%.1f %%(B)", humidity);
          } else {
            portENTER_CRITICAL(&sensorDataMux);
            latestSensorData.temperature = 0.0;
            portEXIT_CRITICAL(&sensorDataMux);
            
            temperature = 0.0;
            strcpy(temp_str, "NA");
            
            portENTER_CRITICAL(&sensorDataMux);
            latestSensorData.humidity = 0.0;
            portEXIT_CRITICAL(&sensorDataMux);
            
            humidity = 0.0;
            strcpy(hum_str, "NA");
          }
        } else {
          // TOTAL SENSOR ABSENCE: Enforce 0.0 and NA
          portENTER_CRITICAL(&sensorDataMux);
          latestSensorData.temperature = 0.0;
          latestSensorData.humidity = 0.0;
          portEXIT_CRITICAL(&sensorDataMux);
          
          temperature = 0.0;
          humidity = 0.0;
          
          strcpy(temp_str, "NA");
          strcpy(hum_str, "NA");
        }
        xSemaphoreGive(i2cMutex);
      }

      // Periodically attempt to re-init if gone
      static uint32_t lastRetry = 0;
      if (millis() - lastRetry > 60000) { // Every 60s
        debugln("[HDC] Retrying sensor initialization...");
        lastRetry = millis();
        if (initHDC()) {
          debugln("[HDC] Sensor Recovery: SUCCESS");
        } else {
          debugln("[HDC] Sensor Recovery: FAILED");
        }
      }
    }

    esp_task_wdt_reset();
    vTaskDelay(5000 / portTICK_PERIOD_MS); // Read every 5 seconds
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
  debug("[HDC] Checking 1080 ID (Reg 0xFF): 0x");
  debugln(devID1080, HEX);

  if (devID1080 == 0x1050) {
    hdcType = HDC_1080;
    debugln("[HDC] Init: SUCCESS (HDC1080)");

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
  debug("[HDC] Checking 2022 ID (Reg 0xFE): 0x");
  debugln(devID2022, HEX);

  if (devID2022 == 0xD007 || devID2022 == 0x07D0) {
    hdcType = HDC_2022;
    debugln("[HDC] Init: SUCCESS (HDC2022)");

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
  debugln("[HDC] Init: NO SENSOR FOUND! (ID Mismatch)");
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
    if (Wire.endTransmission() != 0) {
      xSemaphoreGive(i2cMutex);
      return false; // Physical NACK means sensor is gone
    }
    
    // [Phase 4 Fix]: We MUST release the i2cMutex here!
    // Holding it for 20ms blocks the Core 0 rtcRead task globally, causing missed Minute Boundary timestamps.
    // The previous Developer held it to prevent supposed EMI, but global time corruption is far worse.
    xSemaphoreGive(i2cMutex);

    vTaskDelay(20 / portTICK_PERIOD_MS); // Wait for measurement

    // Re-acquire the I2C bus to pull the final datagram
    if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(I2C_MUTEX_WAIT_TIME)) != pdTRUE) {
      return false;
    }

    // v5.49: HDC20x0/HDC2022 Register Pointer Reset
    if (hdcType == HDC_2022) {
      Wire.beginTransmission(HDC_ADDR);
      Wire.write(0x00); // Reset pointer to Data Reg (Temp L)
      if (Wire.endTransmission(false) != 0) {
        xSemaphoreGive(i2cMutex);
        return false;
      }
    }

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

      tempC = (rawTemp / 65536.0) * 165.0 - 40.0;
      humidity = (rawHum / 65536.0) * 100.0;
      
      // v5.65 fix: If raw values are exactly 0, it signals a digital/bus failure
      if (rawTemp == 0 || rawHum == 0) return false;

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
