#include "globals.h"

#if ENABLE_ESPNOW == 1

unsigned long espnow_timer;

// Timer
hw_timer_t *My_timer = NULL;

void IRAM_ATTR onEspnowTimer() { send_espnow = 1; }

void espnow(void *pvParameters) {
  //  debugln("ESPNOW Task ... ");
  /*
   *  Enable ESP-NOW when GPIO27 is 1
   *  Normal mode : Use attachInterrupt to GPIO27 to set espnow_start to 1
   *  DeepSleep mode : Use ext0 to wakeup and use wakeup_reason() to set
   * espnow_start to 1
   *
   */
  esp_task_wdt_add(NULL);
  espnow_start = 0;
  for (;;) {
    esp_task_wdt_reset(); // #TRUEFIX

    if ((wakeup_reason_is == ext0) && (espnow_start == 0)) {
      espnow_start = 1;
      wakeup_reason_is = 0;
    }

    if (espnow_start == 1) {
      initiate_espnow();
      My_timer = timerBegin(1000000); // 1MHz timer frequency
      timerAttachInterrupt(My_timer, &onEspnowTimer);
      timerAlarm(My_timer, 1000000, true, 0); // 1 second interval, auto-reload
      espnow_start = 0;
    }
    if (send_espnow == 1) {
      if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(I2C_MUTEX_WAIT_TIME)) ==
          pdTRUE) { // NEW I2C RTC
        snprintf(date_now, sizeof(date_now), "%02d-%02d-%04d", current_day,
                 current_month, current_year);
        snprintf(time_now, sizeof(time_now), "%02d:%02d", current_hour,
                 current_min);
        snprintf(rf_str, sizeof(rf_str), "%02d", rf_count.val);
        xSemaphoreGive(i2cMutex);
      }

      // Prepare the data for sending through ESPNOW
      strcpy(ui_data[FLD_STATION].bottomRow, station_name);
      strcpy(ui_data[FLD_DATE].bottomRow, date_now);
      strcpy(ui_data[FLD_TIME].bottomRow, time_now);
      strcpy(ui_data[FLD_VERSION].bottomRow, UNIT_VER);
      strcpy(ui_data[FLD_RF_ML].bottomRow, rf_str);

      for (int i = 0; i < FLD_COUNT; i++) {
        esp_err_t result = esp_now_send(
            destinationAddress, (uint8_t *)&ui_data[i], sizeof(ui_data[i]));
        vTaskDelay(200);
      }
      send_espnow = 0;
      vTaskDelay(200);
    }

    vTaskDelay(50);
  }
}

void OnDataSent(const wifi_tx_info_t *tx_info, esp_now_send_status_t status) {
  //  debug("\r\nLast Packet Send Status:\t");
  //  debugln(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery
  //  Fail"); if (status ==0){
  //    success = "SUCCESS";
  //  }
  //  else{
  //    success = "FAIL";
  //  }
}

// Callback function when data is received through ESPNOW
ui_data_t ReceiverReadings; // Create a struct_message to hold incoming sensor
                            // readings

void OnDataRecv(const esp_now_recv_info *recv_info, const uint8_t *incomingData,
                int len) {
  // Extract MAC address from recv_info for v3.x compatibility
  const uint8_t *mac = recv_info->src_addr;

  if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(I2C_MUTEX_WAIT_TIME)) ==
      pdTRUE) { // NEW I2C
    //      DateTime now = rtc.now();
    xSemaphoreGive(i2cMutex);
  }

  espnow_timer = millis();
  memcpy(&ReceiverReadings, incomingData, sizeof(ReceiverReadings));
  debug("Bytes received: ");
  debugln(len);
  ui_data[ReceiverReadings.idx - 1].idx = ReceiverReadings.idx;
  strcpy(ui_data[ReceiverReadings.idx - 1].topRow, ReceiverReadings.topRow);
  strcpy(ui_data[ReceiverReadings.idx - 1].bottomRow,
         ReceiverReadings.bottomRow);
  ui_data[ReceiverReadings.idx - 1].fieldType = ReceiverReadings.fieldType;
  debug("Data is  ");
  debugln(ui_data[ReceiverReadings.idx - 1].idx);
  debugln(ui_data[ReceiverReadings.idx - 1].topRow);
  debugln(ui_data[ReceiverReadings.idx - 1].bottomRow);
  debugln(ui_data[ReceiverReadings.idx - 1].fieldType);

  //  {1,"STATION","SIT099",eAlphaNum},
  //  {2,"DATE(dd-mm-yyyy)","08-03-2023",eNumeric},
  //  {3,"TIME 24hr:mm","00:00",eNumeric},
  //  {4,"RF","0.0",eLive},
  //  {5,"VERSION","TRG23 0.1",eDisplayOnly}

  if (ui_data[ReceiverReadings.idx - 1].idx == 1) { // Station
    sscanf(ui_data[ReceiverReadings.idx - 1].bottomRow, "%s", station_name);
    File file = SPIFFS.open("/station.txt", FILE_WRITE);
    if (!file) {
      debugln("Failed to open station.txt for writing");
    } // #TRUEFIX
    file.print(ui_data[ReceiverReadings.idx - 1].bottomRow);
    file.close();
  }
  if (ui_data[ReceiverReadings.idx - 1].idx == 2) { // Date
    sscanf(ui_data[ReceiverReadings.idx - 1].bottomRow, "%02d-%02d-%04d",
           &current_day, &current_month, &current_year);

    if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(I2C_MUTEX_WAIT_TIME)) ==
        pdTRUE) { // NEW I2C
                  //          DateTime now = rtc.now();
      rtc.adjust(DateTime(current_year, current_month, current_day,
                          current_hour, current_min, 0));
      record_hr = current_hour;
      record_min = current_min;
      //          current_hour = now.hour(); current_min = now.minute();
      xSemaphoreGive(i2cMutex);
    }

    //     sprintf(date_now,"%02d-%02d-%04d",current_day,current_month,current_year);
    //     strcpy(ui_data[1].bottomRow,date_now);

  } else if (ui_data[ReceiverReadings.idx - 1].idx == 3) { // Time
    sscanf(ui_data[ReceiverReadings.idx - 1].bottomRow, "%02d:%02d",
           &current_hour, &current_min);
    if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(I2C_MUTEX_WAIT_TIME)) ==
        pdTRUE) { // NEW I2C
                  //          DateTime now = rtc.now();
      rtc.adjust(DateTime(current_year, current_month, current_day,
                          current_hour, current_min, 0));
      record_hr = current_hour;
      record_min = current_min;
      //          current_hour = now.hour(); current_min = now.minute();
      xSemaphoreGive(i2cMutex);
    }
  } else if (ui_data[ReceiverReadings.idx - 1].idx == 6) { // Send Field SMS
    if ((sync_mode == eHttpBegin) || (sync_mode == eSMSStart)) {
      debugln("  SYSTEM BUSY ..  ");
      debugln("  TRY LATER  ");
    } else {
      debugln("Sending SMS");
      send_status = 1;
      get_signal_strength();
      get_network();
      get_registration();
      get_a7672s();
      char msg_rcvd_number[100];
      strcpy(msg_rcvd_number, universalNumber); // Universal number
      prepare_and_send_status(msg_rcvd_number);
      debugln("    SMS SENT   ");
      vTaskDelay(1500);
      //            debugln("Turning off GPRS Module");
      //            SerialSIT.println("AT+CPOF");
      //  {6,"SEND STATUS","YES ?",eDisplayOnly},
      strcpy(ui_data[5].topRow, "SEND STATUS");
      strcpy(ui_data[5].bottomRow, "YES ?");

      esp_err_t result =
          esp_now_send(destinationAddress, (uint8_t *)&ui_data[5],
                       sizeof(ui_data[5])); // NEWTRIM : changed from i to 5

      gprs_mode = eGprsSignalOk; // V3 ITER12
    }
  }
}

void initiate_espnow() {

  if (peer_added != 1) {
    debugln("ESPNOW has started ...");
    WiFi.mode(WIFI_STA); // for ESPNOW
    esp_wifi_set_mac(WIFI_IF_STA,
                     &newMACAddress[0]); // Change it to new MAC address

    // Init ESP-NOW
    if (esp_now_init() != ESP_OK) {
      debugln("Error initializing ESP-NOW");
      return;
    }

    esp_now_register_send_cb(
        OnDataSent); // Once ESPNow is successfully Init, we will register for
                     // Send CB to get the status of Trasnmitted packet
    esp_now_register_recv_cb(
        OnDataRecv); // Register for a callback function that will be called
                     // when data is received
  }

  if (peer_added == 0) {
    // Register peer
    memcpy(peerInfo.peer_addr, destinationAddress, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    // Add peer
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
      debugln("Failed to add peer");
      return;
    } else {
      peer_added = 1;
      debugln("Added ESPNOW peer");
    }
  } else {
    debugln("Peer already added ...");
  }
}
#endif
