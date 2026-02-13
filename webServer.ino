#include "globals.h"
#include <ESPmDNS.h>

#if ENABLE_WEBSERVER == 1
const char *ssid = "Spatika Web Server";
const char *password = "12345678";
WebServer server(80);

// Helper to update activity time
void updateActivity() { last_wifi_activity_time = millis(); }

void handleRoot();
void handleData();
void handleFileList();
void handleFileDownload();
void handleFileView();
void handleViewLog();
void handleDisconnect();

void webServer(void *pvParameters) {
  esp_task_wdt_add(NULL);

  // Configure ESP32 as an Access Point
  debugln("Configuring access point...");
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  debug("AP IP address: ");
  debugln(IP);

  if (MDNS.begin("spatika")) {
    debugln("MDNS responder started");
  }

  wifi_active = true;
  last_wifi_activity_time = millis();

  // Set up the web server routes
  server.on("/", []() {
    updateActivity();
    handleRoot();
  });
  server.on("/data", []() {
    updateActivity();
    handleData();
  });
  server.on("/files", []() {
    updateActivity();
    handleFileList();
  });
  server.on("/download", []() {
    updateActivity();
    handleFileDownload();
  });
  server.on("/view", []() {
    updateActivity();
    handleViewLog();
  });
  server.on("/viewfile", []() {
    updateActivity();
    handleFileView();
  });
  server.on("/extend", []() {
    // Add 60s extension (shift start time forward), max cap at now (full
    // duration)
    unsigned long now = millis();
    if (now > last_wifi_activity_time) { // Safety check
      unsigned long elapsed = now - last_wifi_activity_time;
      if (elapsed > 60000) {
        last_wifi_activity_time += 60000;
      } else {
        last_wifi_activity_time = now;
      }
    } else {
      last_wifi_activity_time = now;
    }
    server.send(200, "text/plain", "Extended 1m");
  });
  server.on("/disconnect", []() {
    updateActivity();
    handleDisconnect();
  });

  // Start the server
  server.begin();
  debugln("************** WEB SERVER STARTED **************");

  for (;;) {
    esp_task_wdt_reset();
    server.handleClient();

    // Auto-disconnect if idle for 3 minutes (180000 ms) OR if disabled
    // externally
    if ((millis() - last_wifi_activity_time > 180000) || !wifi_active) {
      if (wifi_active) {
        debugln("WiFi Idle Timeout. Disconnecting...");
      } else {
        debugln("WiFi Disabled externally. Stopping WebServer task...");
      }
      WiFi.softAPdisconnect(true);
      wifi_active = false;
      webServerStarted = false; // Allow re-creation if logic permits (though
                                // this kills the task)
      vTaskDelete(NULL);        // Kill this task
    }

    vTaskDelay(100);
  }
}

// --- Handle Root (/) ---
void handleRoot() {
  if (!wifi_active) {
    server.send(200, "text/html",
                "<html><body><h1>WiFi is OFF</h1></body></html>");
    return;
  }

  // --- Parse Last Recorded Log Data ---
  String rec_rf = "--", rec_temp = "--", rec_hum = "--", rec_ws = "--",
         rec_wd = "--";
  // Determine the most likely active file (the one that closes at next 8:45 AM)
  int rDay = current_day, rMonth = current_month, rYear = current_year;
  int sNo = (current_hour * 4 + current_min / 15 + 61) % 96;
  if (sNo <= 60) {
    next_date(&rDay, &rMonth, &rYear);
  }

  String filesToTry[2];
  char dateStr[20];
  snprintf(dateStr, sizeof(dateStr), "%04d%02d%02d", rYear, rMonth, rDay);
  filesToTry[0] = "/" + String(station_name) + "_" + String(dateStr) + ".txt";

  // Previous file as fallback
  previous_date(&rDay, &rMonth, &rYear);
  snprintf(dateStr, sizeof(dateStr), "%04d%02d%02d", rYear, rMonth, rDay);
  filesToTry[1] = "/" + String(station_name) + "_" + String(dateStr) + ".txt";

  char timeStr[64] = "--:--";
  bool foundData = false;
  String matchLine = "";
  String matchDate = "--", matchTime = "--";

  if (current_year > 2020) {
    for (int fIdx = 0; fIdx < 2; fIdx++) {
      if (SPIFFS.exists(filesToTry[fIdx])) {
        File f = SPIFFS.open(filesToTry[fIdx], FILE_READ);
        String lastValidTime = "";
        matchLine = ""; // Reset for each file check

        while (f.available()) {
          String line = f.readStringUntil('\n');
          line.trim();
          if (line.length() < 10)
            continue;

          // Flexible parsing to find first Date and first Time in the line
          int tokenP = 0;
          String curDate = "--", curTime = "";
          for (int i = 0; i < 8; i++) {
            int nextP = line.indexOf(',', tokenP);
            String val = (nextP == -1) ? line.substring(tokenP)
                                       : line.substring(tokenP, nextP);
            val.trim();
            if (curDate == "--" && val.indexOf('-') != -1 && val.length() >= 8)
              curDate = val;
            if (curTime == "" && val.indexOf(':') != -1 && val.length() >= 4)
              curTime = val;
            if (curTime != "" && curDate != "--")
              break;
            if (nextP == -1)
              break;
            tokenP = nextP + 1;
          }

          if (curTime != "" && curTime.indexOf(':') != -1) {
            matchLine = line;
            matchDate = curDate;
            matchTime = curTime;
          }
          esp_task_wdt_reset();
        }
        f.close();

        if (matchLine != "") {
          foundData = true;
          // Dynamic CSV Parsing of the best match
          int p = 0;
          String tokens[15];
          int tokenCount = 0;
          int timeIdx = -1;

          for (int i = 0; i < 15; i++) {
            int nextP = matchLine.indexOf(',', p);
            if (nextP == -1)
              nextP = matchLine.length();
            String val = matchLine.substring(p, nextP);
            val.trim();
            tokens[i] = val;
            if (val.indexOf(':') != -1 && timeIdx == -1)
              timeIdx = i;
            tokenCount++;
            p = nextP + 1;
            if (nextP == matchLine.length())
              break;
          }

          if (timeIdx != -1) {
            int d = timeIdx + 1;
            while (d < tokenCount && tokens[d].indexOf(':') != -1) {
              d++;
            }

            bool isBihar = (strstr(station_name, "BIHAR_TRG") != NULL);

#if SYSTEM == 0
            if (isBihar) {
              rec_rf = (d < tokenCount) ? tokens[d] : "--";
            } else {
              rec_rf = (d + 1 < tokenCount) ? tokens[d + 1] : "--";
            }
#elif SYSTEM == 1
            rec_temp = (d < tokenCount) ? tokens[d] : "--";
            rec_hum = (d + 1 < tokenCount) ? tokens[d + 1] : "--";
            rec_ws = (d + 2 < tokenCount) ? tokens[d + 2] : "--";
            rec_wd = (d + 3 < tokenCount) ? tokens[d + 3] : "--";
#elif SYSTEM == 2
            rec_rf = (d < tokenCount) ? tokens[d] : "--";
            rec_temp = (d + 1 < tokenCount) ? tokens[d + 1] : "--";
            rec_hum = (d + 2 < tokenCount) ? tokens[d + 2] : "--";
            rec_ws = (d + 3 < tokenCount) ? tokens[d + 3] : "--";
            rec_wd = (d + 4 < tokenCount) ? tokens[d + 4] : "--";
#endif
          }
          snprintf(timeStr, sizeof(timeStr), "%s %s", matchDate.c_str(),
                   matchTime.c_str());
          break; // EXIT loop: We found the latest available file with data
        }
      }
    }
  }

  // Final fallback if NO data was found in either file
  if (!foundData) {
    if (last_recorded_hr == 0 && last_recorded_min == 0)
      snprintf(timeStr, sizeof(timeStr), "--:--");
    else
      snprintf(timeStr, sizeof(timeStr), "%02d:%02d", last_recorded_hr,
               last_recorded_min);
  }
  String html = "<!DOCTYPE html><html><head><meta name='viewport' "
                "content='width=device-width, initial-scale=1.0'>";
  html += "<title>SPATIKA SENSOR DATA</title>";
  html +=
      "<style>body{font-family:Helvetica,Arial,sans-serif;text-align:center;"
      "background-color:#f0f8ff;color:#333;margin:0;padding:10px;}";
  html += ".container{max-width:800px;margin:0 auto;}";
  html += ".btn{background-color:#17a2b8;color:white;padding:12px "
          "24px;text-align:center;text-decoration:none;display:inline-block;"
          "font-size:15px;margin:10px "
          "5px;cursor:pointer;border-radius:5px;border:none;transition:"
          "background 0.2s;}";
  html += ".btn:hover{opacity:0.9;}";
  html += ".btn-danger{background-color:#dc3545;}";
  html += "input[type=submit]{background-color:#007bff;color:white;border:none;"
          "padding:10px "
          "20px;border-radius:5px;cursor:pointer;font-size:15px;transition:"
          "background 0.2s;}";
  html += "input[type=submit]:hover{opacity:0.9;}";
  html += ".card{background:white;border-radius:8px;box-shadow:0 2px 4px "
          "rgba(0,0,0,0.05);padding:10px;margin:5px;display:inline-block;min-"
          "width:90px;vertical-align:top;width:30%;box-sizing:border-box;}";
  html +=
      ".value{font-size:1.2em;font-weight:700;color:#007bff;margin-top:5px;}";
  html += ".label{font-size:0.85em;color:#6c757d;text-transform:uppercase;"
          "letter-spacing:0.5px;}";
  html += ".section-title{font-size:1.1em;color:#555;margin:20px 0 "
          "10px;font-weight:bold;text-align:left;border-bottom:2px solid "
          "#ddd;padding-bottom:5px;}";
  html += "@media (max-width: 400px) { .card { width: 45%; } }";
  html += ".warning-modal "
          "{display:none;position:fixed;top:0;left:0;width:100%;height:100%;"
          "background:rgba(0,0,0,0.8);z-index:999;text-align:center;padding-"
          "top:30%;}";
  html += ".warning-box "
          "{background:white;padding:20px;border-radius:10px;display:inline-"
          "block;width:80%;max-width:300px;}";
  html += "</style>";

  html +=
      "<script>"
      "setInterval(function() {"
      "  var xhttp = new XMLHttpRequest();"
      "  xhttp.onreadystatechange = function() {"
      "    if (this.readyState == 4 && this.status == 200) {"
      "      var data = JSON.parse(this.responseText);"
      "      if(document.getElementById('live_rf')) "
      "document.getElementById('live_rf').innerHTML = data.rf_inst + ' <span "
      "style=\"font-size:0.6em\">mm</span>';"
      "      if(document.getElementById('live_temp')) "
      "document.getElementById('live_temp').innerHTML = data.temperature + ' "
      "<span style=\"font-size:0.6em\">&deg;C</span>';"
      "      if(document.getElementById('live_hum')) "
      "document.getElementById('live_hum').innerHTML = data.humidity + ' <span "
      "style=\"font-size:0.6em\">%</span>';"
      "      if(document.getElementById('live_ws')) "
      "document.getElementById('live_ws').innerHTML = data.windSpeed + ' <span "
      "style=\"font-size:0.6em\">m/s</span>';"
      "      if(document.getElementById('live_wd')) "
      "document.getElementById('live_wd').innerHTML = data.windDir + ' <span "
      "style=\"font-size:0.6em\">&deg;</span>';"
      ""
      "      if(data.wifi_left < 15) {"
      "         document.getElementById('warnModal').style.display='block';"
      "         document.getElementById('timeLeft').innerText = data.wifi_left;"
      "      } else {"
      "         document.getElementById('warnModal').style.display='none';"
      "      }"
      "    }"
      "  };"
      "  xhttp.open('GET', '/data?t=' + Date.now(), true);"
      "  xhttp.send();"
      "}, 3000);"
      "function extendInfo() {"
      "  fetch('/extend').then(()=>{ "
      "document.getElementById('warnModal').style.display='none'; });"
      "}"
      "</script>";

  html += "</head><body>";

  char stationValue[100];
  snprintf(
      stationValue, sizeof(stationValue),
      "<h1 style='font-size: 1.8em;color:#333;margin-bottom:20px;'>%s</h1>",
      station_name);

  html +=
      "<h2 style='color:#007bff;font-size:1.5em;margin-bottom:5px;'>Spatika "
      "Web Portal</h2>";
  html += stationValue;

  // Main Container
  html += "<div class='container'>";

  // --- SECTION 1: LIVE MONITOR ---
  html += "<div class='section-title'>Live Monitor <span "
          "style='font-size:0.6em;color:#28a745;vertical-align:middle;'>&#9679;"
          " updating</span></div>";
  if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(I2C_MUTEX_WAIT_TIME)) == pdTRUE) {
    html += "<div "
            "style='display:flex;flex-wrap:wrap;justify-content:center;gap:5px;"
            "'>"; // Flex container

#if SYSTEM == 0
    html += "<div class='card'><div class='label'>Instant RF</div><div "
            "id='live_rf' class='value'>" +
            String((float)rf_count.val * RF_RESOLUTION, 2) +
            " <span style='font-size:0.6em'>mm</span></div></div>";
#endif

#if SYSTEM == 1
    html += "<div class='card'><div class='label'>Temp</div><div "
            "id='live_temp' class='value'>" +
            String(temperature, 1) +
            " <span style='font-size:0.6em'>&deg;C</span></div></div>";
    html += "<div class='card'><div class='label'>Humidity</div><div "
            "id='live_hum' class='value'>" +
            String(humidity, 1) +
            " <span style='font-size:0.6em'>%</span></div></div>";
    html += "<div class='card'><div class='label'>Wind Spd</div><div "
            "id='live_ws' class='value'>" +
            String(cur_wind_speed, 2) +
            " <span style='font-size:0.6em'>m/s</span></div></div>";
    html += "<div class='card'><div class='label'>Wind Dir</div><div "
            "id='live_wd' class='value'>" +
            String(windDir) +
            " <span style='font-size:0.6em'>&deg;</span></div></div>";
#endif

#if SYSTEM == 2
    html += "<div class='card'><div class='label'>Instant RF</div><div "
            "id='live_rf' class='value'>" +
            String((float)rf_count.val * RF_RESOLUTION, 2) +
            " <span style='font-size:0.6em'>mm</span></div></div>";
    html += "<div class='card'><div class='label'>Temp</div><div "
            "id='live_temp' class='value'>" +
            String(temperature, 1) +
            " <span style='font-size:0.6em'>&deg;C</span></div></div>";
    html += "<div class='card'><div class='label'>Humidity</div><div "
            "id='live_hum' class='value'>" +
            String(humidity, 1) +
            " <span style='font-size:0.6em'>%</span></div></div>";
    html += "<div class='card'><div class='label'>Wind Spd</div><div "
            "id='live_ws' class='value'>" +
            String(cur_wind_speed, 2) +
            " <span style='font-size:0.6em'>m/s</span></div></div>";
    html += "<div class='card'><div class='label'>Wind Dir</div><div "
            "id='live_wd' class='value'>" +
            String(windDir) +
            " <span style='font-size:0.6em'>&deg;</span></div></div>";
#endif

    html += "</div>"; // End Flex
    xSemaphoreGive(i2cMutex);
  }

  // --- SECTION 2: LAST RECORDED ---
  // Note: timeStr was already populated during parsing above if a record was
  // found. If no record was found, we default to the global last_recorded
  // variables.
  if (rec_rf == "--" && rec_temp == "--") {
    if (last_recorded_hr == 0 && last_recorded_min == 0)
      snprintf(timeStr, sizeof(timeStr), "--:--");
    else
      snprintf(timeStr, sizeof(timeStr), "%02d:%02d", last_recorded_hr,
               last_recorded_min);
  }

  html +=
      "<div class='section-title'>Last Logged @ " + String(timeStr) + "</div>";
  html += "<div "
          "style='display:flex;flex-wrap:wrap;justify-content:center;gap:5px;'"
          ">"; // Flex container

#if SYSTEM == 0
  html += "<div class='card'><div class='label'>Logged RF</div><div "
          "class='value' style='color:#666'>" +
          rec_rf + " <span style='font-size:0.6em'>mm</span></div></div>";
#endif

#if SYSTEM == 1
  html += "<div class='card'><div class='label'>Temp</div><div class='value' "
          "style='color:#666'>" +
          rec_temp + " <span style='font-size:0.6em'>&deg;C</span></div></div>";
  html += "<div class='card'><div class='label'>Humidity</div><div "
          "class='value' style='color:#666'>" +
          rec_hum + " <span style='font-size:0.6em'>%</span></div></div>";
  html += "<div class='card'><div class='label'>Wind Spd</div><div "
          "class='value' style='color:#666'>" +
          rec_ws + " <span style='font-size:0.6em'>m/s</span></div></div>";
  html += "<div class='card'><div class='label'>Wind Dir</div><div "
          "class='value' style='color:#666'>" +
          rec_wd + " <span style='font-size:0.6em'>&deg;</span></div></div>";
#endif

#if SYSTEM == 2
  html += "<div class='card'><div class='label'>Logged RF</div><div "
          "class='value' style='color:#666'>" +
          rec_rf + " <span style='font-size:0.6em'>mm</span></div></div>";
  html += "<div class='card'><div class='label'>Temp</div><div class='value' "
          "style='color:#666'>" +
          rec_temp + " <span style='font-size:0.6em'>&deg;C</span></div></div>";
  html += "<div class='card'><div class='label'>Humidity</div><div "
          "class='value' style='color:#666'>" +
          rec_hum + " <span style='font-size:0.6em'>%</span></div></div>";
  html += "<div class='card'><div class='label'>Wind Spd</div><div "
          "class='value' style='color:#666'>" +
          rec_ws + " <span style='font-size:0.6em'>m/s</span></div></div>";
  html += "<div class='card'><div class='label'>Wind Dir</div><div "
          "class='value' style='color:#666'>" +
          rec_wd + " <span style='font-size:0.6em'>&deg;</span></div></div>";
#endif
  html += "</div>"; // End Flex

  // --- SECTION 3: ACTIONS ---
  html += "<div class='section-title'>Actions & History</div>";

  // File Mgmt
  html += "<div "
          "style='background:white;padding:20px;border-radius:8px;box-shadow:0 "
          "1px 3px rgba(0,0,0,0.1);max-width:500px;margin:0 auto;'>";

  html += "<a href='/files' class='btn' "
          "style='display:block;width:100%;box-sizing:border-box;margin:0 0 "
          "20px 0;'>Browse Log Files</a>";

  html += "<h3 style='margin:10px 0;color:#555;font-size:1em;'>Search "
          "History</h3>";
  html += "<form action='/view' method='GET' style='margin-bottom:0;'>";
  html += "<div style='text-align:left;margin-bottom:10px;'>";
  html += "<label style='font-size:0.9em;color:#777;'>Date (YYYYMMDD):</label>";
  html += "<input type='text' name='date' value='" + String(last_recorded_yy) +
          String(last_recorded_mm < 10 ? "0" : "") + String(last_recorded_mm) +
          String(last_recorded_dd < 10 ? "0" : "") + String(last_recorded_dd) +
          "' maxlength='8' "
          "style='width:100%;padding:8px;margin-top:5px;box-sizing:border-box;"
          "border:1px solid #ddd;border-radius:4px;'>";
  html += "</div>";
  html += "<div style='text-align:left;margin-bottom:15px;'>";
  html += "<label style='font-size:0.9em;color:#777;'>Time (HH:MM):</label>";
  html += "<input type='text' name='time' value='12:00' maxlength='5' "
          "style='width:100%;padding:8px;margin-top:5px;box-sizing:border-box;"
          "border:1px solid #ddd;border-radius:4px;'>";
  html += "</div>";
  html += "<input type='submit' value='Search Record' class='btn' "
          "style='width:90%;margin:0;background:#007bff;'>";
  html += "</form>";
  html += "</div>";

  // Disconnect
  html += "<br><a href='/disconnect' class='btn btn-danger' "
          "style='width:auto;padding:10px 20px;'>Disconnect WiFi</a><br><br>";

  html += "</div>"; // End Main Container
  html += "<div id='warnModal' class='warning-modal'>"
          "<div class='warning-box'>"
          "<h2 style='color:#dc3545;'>WiFi Disconnecting!</h2>"
          "<p>Closing in <b id='timeLeft'></b> seconds...</p>"
          "<button class='btn' style='background:#28a745;width:80%;' "
          "onclick='extendInfo()'>Extend Time (Keep On)</button>"
          "<br><a href='/disconnect' class='btn btn-danger' "
          "style='width:80%;'>Close Now</a>" // Explicit close option
          "</div></div>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

// --- Handle File List (/files) ---
void handleFileList() {
  String year = server.arg("year");
  String month = server.arg("month");
  String query = server.arg("search");

  String html = "<!DOCTYPE html><html><head><meta name='viewport' "
                "content='width=device-width, initial-scale=1.0'>";
  html += "<title>LOG FILES</title>";
  html +=
      "<style>body{font-family:Helvetica,Arial,sans-serif;background:#f0f8ff;"
      "color:#333;margin:0;padding:20px;} "
      ".file-item{background:white;padding:15px;margin-bottom:12px;border-"
      "radius:8px;box-shadow:0 1px 3px "
      "rgba(0,0,0,0.1);display:flex;justify-content:space-between;align-"
      "items:"
      "center;flex-wrap:wrap;}"
      ".folder-item{background:#e9ecef;padding:20px;margin-bottom:12px;"
      "border-"
      "radius:8px;text-align:center;font-size:1.2em;font-weight:bold;cursor:"
      "pointer;color:#007bff;text-decoration:none;display:block;}"
      ".folder-item:hover{background:#dee2e6;}"
      ".btn{text-decoration:none;padding:8px "
      "16px;color:white;border-radius:5px;margin-left:5px;font-size:14px;"
      "font-"
      "weight:bold;border:none;white-space:nowrap;}"
      ".btn-view{background-color:#007bff;} "
      ".btn-dl{background-color:#28a745;}"
      "input[type=text]{padding:10px;font-size:16px;width:100%;max-width:"
      "300px;"
      "border:1px solid #ced4da;border-radius:5px;margin-right:10px;}"
      "input[type=submit]{padding:10px "
      "20px;font-size:16px;background:#007bff;color:white;border:none;border-"
      "radius:5px;cursor:pointer;}"
      "form{margin-bottom:20px;}"
      "@media (max-width: 480px) { .file-item { display:block; } .btn { "
      "display:inline-block; margin:5px 5px 0 0; } }"
      "</style></head><body>";

  html += "<h2>Log Files</h2>";

  // Search Form
  html += "<form action='/files' method='GET'>";
  html += "<input type='text' name='search' placeholder='Global Search (e.g. "
          "20260125)' value='" +
          query + "'>";
  html += "<input type='submit' value='Filter' "
          "style='margin-top:5px;background-color:#007bff;'>";
  html += "</form>";

  // --- Logic for Hierarchy vs Search ---
  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  int count = 0;

  if (query.length() > 0) {
    // --- MODE: GLOBAL SEARCH ---
    html += "<h3>Search Results for '" + query + "'</h3>";
    while (file) {
      String fileName = file.name();
      String fName = fileName;
      if (fName.startsWith("/"))
        fName = fName.substring(1);

      if (fileName.endsWith(".txt") && fileName.indexOf(station_name) != -1 &&
          fileName.indexOf(query) != -1) {
        if (!fileName.startsWith("/"))
          fileName = "/" + fileName;
        html += "<div class='file-item'>";
        html += "<strong>" + fileName + "</strong> (" + String(file.size()) +
                " B)<br><br>";
        html += "<a href='/viewfile?file=" + fileName +
                "' class='btn btn-view'>View</a>";
        html += "<a href='/download?file=" + fileName +
                "' class='btn btn-dl'>Download</a>";
        html += "</div>";
        count++;
      }
      file = root.openNextFile();
    }
    if (count == 0)
      html += "<p>No matching files found.</p>";

  } else if (year == "") {
    // --- MODE: YEAR LIST (ROOT) ---
    html += "<h3>Select Year</h3>";
    bool years[100] = {0}; // 2000-2099 map

    while (file) {
      String fileName = file.name();
      if (fileName.endsWith(".txt") && fileName.indexOf(station_name) != -1) {
        // Extract YYYY: Station_YYYYMMDD.txt. Find last '_'.
        int idx = fileName.lastIndexOf('_');
        if (idx != -1 && fileName.length() > idx + 4) {
          String yStr = fileName.substring(idx + 1, idx + 5);
          int yVal = yStr.toInt();
          if (yVal >= 2000 && yVal < 2100)
            years[yVal - 2000] = true;
        }
      }
      file = root.openNextFile();
    }

    for (int i = 0; i < 100; i++) {
      if (years[i]) {
        int yVal = 2000 + i;
        html += "<a href='/files?year=" + String(yVal) +
                "' class='folder-item'>" + String(yVal) + "</a>";
        count++;
      }
    }
    if (count == 0)
      html += "<p>No logs found.</p>";

  } else if (month == "") {
    // --- MODE: MONTH LIST (YEAR SELECTED) ---
    html += "<h3>" + year + " > Select Month</h3>";
    bool months[13] = {0}; // 1-12

    while (file) {
      String fileName = file.name();
      if (fileName.endsWith(".txt") && fileName.indexOf(station_name) != -1) {
        int idx = fileName.lastIndexOf('_'); // ..._YYYYMMDD.txt
        if (idx != -1 && fileName.length() > idx + 6) {
          String fYear = fileName.substring(idx + 1, idx + 5);
          if (fYear == year) {
            String mStr = fileName.substring(idx + 5, idx + 7);
            int mVal = mStr.toInt();
            if (mVal >= 1 && mVal <= 12)
              months[mVal] = true;
          }
        }
      }
      file = root.openNextFile();
    }

    const char *monthNames[] = {"",        "January",   "February", "March",
                                "April",   "May",       "June",     "July",
                                "August",  "September", "October",  "November",
                                "December"};
    for (int i = 1; i <= 12; i++) {
      if (months[i]) {
        String mPad = (i < 10) ? "0" + String(i) : String(i);
        html += "<a href='/files?year=" + year + "&month=" + mPad +
                "' class='folder-item'>" + String(monthNames[i]) + " (" + mPad +
                ")</a>";
        count++;
      }
    }
    if (count == 0)
      html += "<p>No logs found for " + year + ".</p>";

  } else {
    // --- MODE: FILE LIST (YEAR + MONTH SELECTED) ---
    html += "<h3>" + year + " > " + month + "</h3>";
    String filter = "_" + year + month; // Match ..._YYYYMM...

    while (file) {
      String fileName = file.name();
      if (fileName.endsWith(".txt") && fileName.indexOf(station_name) != -1 &&
          fileName.indexOf(filter) != -1) {
        if (!fileName.startsWith("/"))
          fileName = "/" + fileName;
        html += "<div class='file-item'>";
        html += "<strong>" + fileName + "</strong> (" + String(file.size()) +
                " B)<br><br>";
        html += "<a href='/viewfile?file=" + fileName +
                "' class='btn btn-view'>View</a>";
        html += "<a href='/download?file=" + fileName +
                "' class='btn btn-dl'>Download</a>";
        html += "</div>";
        count++;
      }
      file = root.openNextFile();
    }
    if (count == 0)
      html += "<p>No logs found for " + year + "-" + month + ".</p>";
  }

  html += "<br>";
  if (month != "")
    html += "<a href='/files?year=" + year +
            "' class='btn' style='background-color:#17a2b8;'>Back to " + year +
            "</a> ";
  if (year != "")
    html += "<a href='/files' class='btn' "
            "style='background-color:#007bff;'>Back to Years</a> ";
  html += "<a href='/' class='btn' style='background-color:#28a745;'>Back to "
          "Home</a>";

  html += "</body></html>";
  server.send(200, "text/html", html);
}

// --- Handle File Download (/download?file=...) ---
// --- Handle File Download (/download?file=...) ---
void handleFileDownload() {
  if (server.hasArg("file")) {
    String fileName = server.arg("file");
    if (!fileName.startsWith("/"))
      fileName = "/" + fileName;

    if (SPIFFS.exists(fileName)) {
      File file = SPIFFS.open(fileName, "r");
      // Force filename for download
      String downloadName = fileName;
      if (downloadName.startsWith("/"))
        downloadName = downloadName.substring(1);
      server.sendHeader("Content-Disposition",
                        "attachment; filename=\"" + downloadName + "\"");
      server.streamFile(file, "text/plain"); // Send as text/plain
      file.close();
      return;
    }
  }
  server.send(404, "text/plain", "File Not Found");
}

// --- Handle File View (/viewfile?file=...) ---
void handleFileView() {
  if (server.hasArg("file")) {
    String fileName = server.arg("file");
    if (!fileName.startsWith("/"))
      fileName = "/" + fileName;

    if (SPIFFS.exists(fileName)) {
      File file = SPIFFS.open(fileName, "r");

      // Header
      server.setContentLength(CONTENT_LENGTH_UNKNOWN);
      server.send(200, "text/html", "");
      server.sendContent(
          "<!DOCTYPE html><html><head><meta name='viewport' "
          "content='width=device-width, initial-scale=1.0'>"
          "<style>"
          "body{font-family:Helvetica,Arial,sans-serif;padding:20px;"
          "background:"
          "#f4f4f9;color:#333;}"
          ".btn{text-decoration:none;padding:12px "
          "20px;color:white;border:none;border-radius:5px;margin:10px "
          "5px;display:inline-block;font-size:15px;cursor:pointer;font-"
          "weight:"
          "bold;text-align:center;}"
          ".btn-dl{background-color:#28a745;}"
          ".btn-copy{background-color:#17a2b8;}"
          ".btn-back{background-color:#138496;}"
          "pre{background:white;padding:15px;border-radius:5px;overflow-x:"
          "auto;"
          "border:1px solid #ddd;font-size:12px;line-height:1.4;}"
          ".tip{font-size:13px;color:#666;background:#e9ecef;padding:10px;"
          "border-radius:5px;margin-top:10px;border-left:4px solid #17a2b8;}"
          "</style>"
          "<script>"
          "function copyContent() {"
          "  var temp = document.createElement('textarea');"
          "  var pre = document.getElementById('fileContent');"
          "  temp.value = pre.innerText;"
          "  document.body.appendChild(temp);"
          "  temp.select();"
          "  try { document.execCommand('copy'); alert('Content Copied! "
          "Switch "
          "to WhatsApp and Paste.'); } catch(e) { alert('Copy failed.'); }"
          "  document.body.removeChild(temp);"
          "}"
          "</script>"
          "</head><body>");

      server.sendContent("<h3>" + fileName + "</h3>");

      server.sendContent("<div>");
      server.sendContent("<a href='/download?file=" + fileName +
                         "' class='btn btn-dl'>Download File (Save)</a>");
      server.sendContent("<button onclick='copyContent()' class='btn "
                         "btn-copy'>Copy Content</button>");
      server.sendContent("</div>");

      server.sendContent(
          "<div class='tip'><strong>Tip:</strong> To attach this file in "
          "WhatsApp: Download it, disconnect WiFi (to enable Internet), then "
          "share it from your Downloads folder.</div><hr>");

      server.sendContent("<pre id='fileContent'>");

      // Stream content in chunks
      char buf[1025];
      while (file.available()) {
        int len = file.readBytes(buf, 1024);
        buf[len] = 0; // Null terminate
        server.sendContent(String(buf));
      }

      server.sendContent("</pre>");
      server.sendContent(
          "<hr><a href='/files' class='btn btn-back'>Back to List</a>");
      server.sendContent(" <a href='/' class='btn btn-back'>Back to "
                         "Home</a></body></html>");
      file.close();
      return;
    }
  }
  server.send(404, "text/plain", "File Not Found");
}

// --- Handle View Log (/view?date=...&time=...) ---
void handleViewLog() {
  String dateInput = server.arg("date"); // YYYYMMDD
  String timeInput = server.arg("time"); // HH:MM

  String searchFileName = "/" + String(station_name) + "_" + dateInput + ".txt";
  String result = "Data not found for " + dateInput + " " + timeInput;

  if (SPIFFS.exists(searchFileName)) {
    File f = SPIFFS.open(searchFileName, FILE_READ);
    String searchPattern =
        "," + timeInput +
        ","; // Format in CSV depends on log type but usually ,HH:MM,
    while (f.available()) {
      String line = f.readStringUntil('\n');
      if (line.indexOf(searchPattern) != -1) {
        result = "<b>Record Found:</b><br><br>" + line;
        // You could parse this line similar to lcdkeypad logic to confirm
        // precise match or format it nicely. For now, raw line is useful.
        break;
      }
    }
    f.close();
  }

  String html = "<!DOCTYPE html><html><head><meta name='viewport' "
                "content='width=device-width, initial-scale=1.0'>"
                "<style>"
                "body{font-family:Helvetica,Arial,sans-serif;padding:20px;"
                "background:#f8f9fa;color:#333;}"
                ".btn{background-color:#17a2b8;color:white;padding:12px "
                "24px;text-decoration:none;display:inline-block;border-radius:"
                "5px;font-size:15px;margin-top:20px;}"
                ".btn:hover{background-color:#138496;}"
                "</style></head><body>";
  html += "<h2>Search Result</h2>";
  html += "<p>" + result + "</p>";
  html += "<br><a href='/' class='btn' style='background:#17a2b8;'>Back to "
          "Home</a>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

// --- Handle Disconnect ---
void handleDisconnect() {
  server.send(
      200, "text/html",
      "<h1>WiFi Disconnecting...</h1><p>You can now close this tab.</p>");
  delay(100);
  WiFi.softAPdisconnect(true);
  wifi_active = false;
  webServerStarted = false;
  vTaskDelete(NULL);
}

void handleData() {
  String json = "{";
  if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(I2C_MUTEX_WAIT_TIME)) == pdTRUE) {
    json += "\"time\": \"" + String(current_hour) + ":" + String(current_min) +
            "\"";

#if SYSTEM == 0
    json += ", \"rf_inst\": " + String((float)rf_count.val * RF_RESOLUTION, 2);
#endif

#if SYSTEM == 1
    json += ", \"temperature\": " + String(temperature, 1) +
            ", \"humidity\": " + String(humidity, 1) +
            ", \"windSpeed\": " + String(cur_wind_speed, 2) +
            ", \"windDir\": " + String(windDir);
#endif

#if SYSTEM == 2
    json += ", \"rf_inst\": " + String((float)rf_count.val * RF_RESOLUTION, 2) +
            ", \"rf_cum\": " + String(new_current_cumRF, 2) +
            ", \"temperature\": " + String(temperature, 1) +
            ", \"humidity\": " + String(humidity, 1) +
            ", \"windSpeed\": " + String(cur_wind_speed, 2) +
            ", \"windDir\": " + String(windDir);
#endif

    unsigned long elapsed = millis() - last_wifi_activity_time;
    long left = (elapsed < 180000) ? (180000 - elapsed) / 1000 : 0;
    json += ", \"wifi_left\": " + String(left);

    xSemaphoreGive(i2cMutex);
  }
  json += "}";
  server.send(200, "application/json", json);
}
#endif
