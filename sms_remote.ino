// =============================================================================================
// VARSHA SMS commands (v6.57): the LPC unit's command set and behaviour (AIO_LPC
// sim7600_driver.c sms_run / sim7600_check_incoming_sms, design section 7.4b), on the ESP32.
//
//   VARSHA <CMD> [value]   (any case, '=' optional), or the older #CMD[=value]
//
//   anyone : GET_STATUS, GET_GPS, GET_CONFIG
//   office : SET_TIME YYYYMMDDHHMMSS, SET_STN_ID <id>, SET_SERVER_2 host=..;port=..;path=/..;key=.. | OFF,
//            SET_SERVER_MODE 0|1|2, SET_OTA_SERVER <host> [port] | OFF, ADD_NUM / DEL_NUM <number>,
//            GET_NUM, STOP / RESUME, ERASE_DATA <this unit's ID>
//
// The text handling is the LPC's own sms_cmd.c (same file). Here: reading the SIM, who may do what,
// and what each command does on this unit. Every reply goes to the sender and starts "STN <id>".
//
// Each slot, after the send, the SIM is read once (AT+CMGL="ALL"): up to SMS_INBOX_MAX whole
// messages, each deleted by its index before it runs. More, or one cut off at the end of the copy,
// stay on the SIM for the next slot (a cut "SET_STN_ID 19" of 1903 must never run).
// =============================================================================================

#define SMS_INBOX_MAX 4
#define SMS_MAX_AGE_MIN (48 * 60)        // older commands are deleted unrun (field SIMs hold old ones)
#define SMS_SET_TIME_MAX_AGE_MIN 15      // SET_TIME only when fresh, by the network's clock
#define SMS_GET_REPEAT_MIN 10            // the same GET_ from the same sender: not answered again
#define SMS_OFFICE_MAX 5

static sms_in_t s_inbox[SMS_INBOX_MAX];
static char s_sms_list[2048];
static char s_sms_reply[200];

// GET_ commands answered lately: kept with the daily figures (health_day.ino, RTC_NOINIT memory)
#define s_answered (v657_rtc.answered)

// ---------------------------------------------------------------- small helpers
static void sms_now14(char out[16]) {
  portENTER_CRITICAL(&rtcTimeMux);
  snprintf(out, 16, "%04d%02d%02d%02d%02d%02d", current_year, current_month, current_day, current_hour,
           current_min, current_sec);
  portEXIT_CRITICAL(&rtcTimeMux);
}

static uint32_t sms_now_minutes() {
  int y, mo, d, h, mi;
  portENTER_CRITICAL(&rtcTimeMux);
  y = current_year; mo = current_month; d = current_day; h = current_hour; mi = current_min;
  portEXIT_CRITICAL(&rtcTimeMux);
  if (y < 2024 || mo < 1 || mo > 12 || d < 1 || d > 31) return 0;
  return DateTime(y, mo, d, h, mi, 0).unixtime() / 60;
}

// The station ID as the office knows it: numeric IDs padded to 6 digits
static const char *sms_stn() {
  static char shown[16];
  if (isDigitStr(station_name) && strlen(station_name) < 6) {
    snprintf(shown, sizeof(shown), "%06ld", atol(station_name));
  } else {
    snprintf(shown, sizeof(shown), "%s", station_name);
  }
  return shown;
}

static void sms_reply(const char *to, const char *text) {
  debugf("[SMS] Reply to %s: %s\n", to, text);
  sms_send_text(to, text, 60000);
}

// The network's time (AT+CCLK?, local time with the zone) as YYYYMMDDhhmmss, if it has one.
// REQUIRES: modemMutex held.
static bool sms_network_ts14(char out[16]) {
  SerialSIT.print("AT+CCLK?\r");
  if (!waitForResponse("OK", 2000)) return false;
  const char *p = strstr(modem_response_buf, "+CCLK: \"");
  int yy, mo, dd, hh, mi, ss;
  if (!p || sscanf(p + 8, "%2d/%2d/%2d,%2d:%2d:%2d", &yy, &mo, &dd, &hh, &mi, &ss) != 6) return false;
  if (yy < 24 || yy > 79 || mo < 1 || mo > 12 || dd < 1 || dd > 31) return false;   // no network time yet
  snprintf(out, 16, "20%02d%02d%02d%02d%02d%02d", yy, mo, dd, hh, mi, ss);
  return true;
}

// Sets the clock to a checked YYYYMMDDhhmmss (local time), as the server's correction does: the time
// only, the rain-day total is kept.
static bool sms_set_clock(const char ts[15]) {
  int y, mo, d, h, mi, s;
  if (sscanf(ts, "%4d%2d%2d%2d%2d%2d", &y, &mo, &d, &h, &mi, &s) != 6) return false;
  static const uint8_t mdays[] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  if (d > mdays[mo - 1] || (mo == 2 && d == 29 && (y % 4) != 0)) return false;
  if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(2000)) != pdTRUE) return false;
  const DateTime before = rtc.now();
  rtc.adjust(DateTime(y, mo, d, h, mi, s));
  xSemaphoreGive(i2cMutex);
  health_note_clock_change((long)(DateTime(y, mo, d, h, mi, s).unixtime() - before.unixtime()));
  portENTER_CRITICAL(&rtcTimeMux);
  current_year = y; current_month = mo; current_day = d;
  current_hour = h; current_min = mi; current_sec = s;
  portEXIT_CRITICAL(&rtcTimeMux);
  debugf("[SMS] Clock set to %s\n", ts);
  return true;
}

// ---------------------------------------------------------------- office numbers (NVS "sms-cfg")
// Up to SMS_OFFICE_MAX, last 10 digits. Until changed by SMS the list is SMS_OFFICE_NUMBERS.
static void office_load(char nums[SMS_OFFICE_MAX][11]) {
  memset(nums, 0, SMS_OFFICE_MAX * 11);
  Preferences p;
  bool init = false;
  if (p.begin("sms-cfg", true)) {
    init = p.getBool("init", false);
    for (int i = 0; init && i < SMS_OFFICE_MAX; i++) {
      char key[4] = {'n', (char)('0' + i), 0, 0};
      p.getString(key, nums[i], 11);
    }
    p.end();
  }
  if (init) return;
  char list[96];
  snprintf(list, sizeof(list), "%s", SMS_OFFICE_NUMBERS);
  int n = 0;
  char *save = NULL;
  for (char *tok = strtok_r(list, ", ", &save); tok && n < SMS_OFFICE_MAX; tok = strtok_r(NULL, ", ", &save)) {
    if (sms_number_last10(tok, nums[n])) n++;
  }
}

static bool office_save(char nums[SMS_OFFICE_MAX][11]) {
  Preferences p;
  if (!p.begin("sms-cfg", false)) return false;
  bool ok = true;
  for (int i = 0; i < SMS_OFFICE_MAX; i++) {
    char key[4] = {'n', (char)('0' + i), 0, 0};
    ok &= p.putString(key, nums[i]) == strlen(nums[i]) || nums[i][0] == '\0';
  }
  ok &= p.putBool("init", true) > 0;
  p.end();
  return ok;
}

static int office_count(char nums[SMS_OFFICE_MAX][11]) {
  int c = 0;
  for (int i = 0; i < SMS_OFFICE_MAX; i++) c += (nums[i][0] != '\0');
  return c;
}

static bool office_has(char nums[SMS_OFFICE_MAX][11], const char *from) {
  for (int i = 0; i < SMS_OFFICE_MAX; i++) {
    if (nums[i][0] && sms_number_same(from, nums[i])) return true;
  }
  return false;
}

// ---------------------------------------------------------------- repeats
// A message the network delivers twice (same sender, centre time stamp and text) runs once.
static bool sms_seen_before(const sms_in_t *m) {
  static sms_seen_t seen;
  static char text[SMS_SEEN_TEXT_LEN + 2];
  Preferences p;
  text[0] = '\0';
  if (p.begin("sms-cfg", true)) {
    p.getString("seen", text, sizeof(text));
    p.end();
  }
  sms_seen_parse(&seen, text);
  bool changed = false;
  if (sms_seen_repeat(&seen, m->from, m->ts, m->body, &changed)) return true;
  if (changed && sms_seen_format(&seen, text, sizeof(text)) > 0 && p.begin("sms-cfg", false)) {
    p.putString("seen", text);
    p.end();
  }
  return false;
}

// A GET_ answered to the same sender less than SMS_GET_REPEAT_MIN ago: the answer would be the same
static bool sms_get_answered_lately(sms_cmd_t cmd, const char *from10) {
  if (cmd != SMS_CMD_GET_STATUS && cmd != SMS_CMD_GET_GPS && cmd != SMS_CMD_GET_CONFIG && cmd != SMS_CMD_GET_NUM) {
    return false;
  }
  const uint32_t now = sms_now_minutes();
  int oldest = 0;
  const int n = sizeof(s_answered) / sizeof(s_answered[0]);
  for (int i = 0; i < n; i++) {
    sms_answered_t *a = &s_answered[i];
    if (now && a->num[0] && a->cmd == (uint8_t)cmd && strcmp(a->num, from10) == 0 && now >= a->at_min &&
        now - a->at_min < SMS_GET_REPEAT_MIN) {
      debugf("[SMS] Same request from %s %u min ago: not answered\n", from10, (unsigned)(now - a->at_min));
      return true;
    }
    if (!a->num[0] || a->at_min < s_answered[oldest].at_min) oldest = i;
  }
  memcpy(s_answered[oldest].num, from10, 11);
  s_answered[oldest].cmd = (uint8_t)cmd;
  s_answered[oldest].at_min = now;
  return false;
}

// ---------------------------------------------------------------- the SIM's messages
// +CMGL: <i>,"REC UNREAD","<from>","","<yy/MM/dd,hh:mm:ss+zz>"\r\n<body>\r\n ... \r\nOK
// Only whole messages: one cut off by the end of the copy is not taken.
static int sms_split_inbox(const char *list) {
  int n = 0;
  const char *p = strstr(list, "+CMGL: ");
  while (p && n < SMS_INBOX_MAX) {
    sms_in_t *m = &s_inbox[n];
    m->from[0] = m->body[0] = m->ts[0] = '\0';
    m->index = atoi(p + 7);
    const char *line_end = strstr(p, "\r\n");
    if (!line_end) break;
    // Quoted fields on the header line: status, sender, name, time stamp
    const char *q[8];
    int nq = 0;
    for (const char *c = p; c < line_end && nq < 8; c++) {
      if (*c == '"') q[nq++] = c;
    }
    if (nq >= 4 && (size_t)(q[3] - q[2] - 1) < sizeof(m->from)) {
      memcpy(m->from, q[2] + 1, q[3] - q[2] - 1);
      m->from[q[3] - q[2] - 1] = '\0';
    }
    if (nq >= 8 && (size_t)(q[7] - q[6] - 1) < sizeof(m->ts)) {
      memcpy(m->ts, q[6] + 1, q[7] - q[6] - 1);
      m->ts[q[7] - q[6] - 1] = '\0';
    }
    const char *body = line_end + 2;
    const char *next = strstr(body, "+CMGL: ");
    const char *ok = strstr(body, "\r\nOK");
    const char *stop = next;
    if (ok && (!stop || ok < stop)) stop = ok;
    if (!stop) break;   // cut short: stays on the SIM
    size_t len = stop - body;
    if (len >= sizeof(m->body)) len = sizeof(m->body) - 1;
    memcpy(m->body, body, len);
    m->body[len] = '\0';
    n++;
    p = next;
  }
  return n;
}

// ---------------------------------------------------------------- one command
static void sms_run(sms_in_t *m) {
  static char arg[96];
  char *out = s_sms_reply;
  const size_t osz = sizeof(s_sms_reply);

  if (sms_decode_ucs2(m->body)) debugln("[SMS] UCS2 message decoded");
  const sms_cmd_t cmd = sms_cmd_parse(m->body, arg, sizeof(arg));
  if (cmd == SMS_CMD_NONE) {
    debugf("[SMS] Not a command (from %s): deleted\n", m->from);
    return;
  }
  char now14[16];
  sms_now14(now14);
  long age = 0;
  if (sms_age_minutes(m->ts, now14, &age) && age > SMS_MAX_AGE_MIN) {
    debugf("[SMS] Command sent %ld min ago (%.17s): too old, not run\n", age, m->ts);
    return;
  }
  char from10[11];
  if (!sms_number_last10(m->from, from10)) {
    debugf("[SMS] Command from non-phone sender %s: ignored\n", m->from);
    return;
  }
  if (sms_seen_before(m)) {
    debugf("[SMS] Same message from %s delivered again (%.17s): not run twice\n", from10, m->ts);
    return;
  }
  char nums[SMS_OFFICE_MAX][11];
  office_load(nums);
  const char *stn = sms_stn();
  debugf("[SMS] Command %d from %s, value '%s'\n", (int)cmd, from10, arg);

  if (sms_cmd_needs_office(cmd) && !office_has(nums, m->from)) {
    snprintf(out, osz, "STN %s: NOT ALLOWED from %s (office numbers only)", stn, from10);
    sms_reply(m->from, out);
    return;
  }
  if (sms_get_answered_lately(cmd, from10)) return;

  switch (cmd) {
  case SMS_CMD_GET_STATUS:
    prepare_and_send_status(m->from, true);
    return;
  case SMS_CMD_GET_GPS:
    get_lat_long_date_time(m->from, true);
    return;
  case SMS_CMD_GET_CONFIG:
    snprintf(out, osz, "STN %s %s FW %s MODE %d S2 %.40s%s%.30s NUMS %d OTA %.30s:%d%s", stn, UNIT, FIRMWARE_VERSION,
             server_mode, secondaryServer.serverName[0] ? secondaryServer.serverName : "NONE",
             secondaryServer.serverName[0] ? ":" : "", secondaryServer.serverName[0] ? secondaryServer.Port : "",
             office_count(nums), ota_server_host(), ota_server_port(), live_post_muted ? " SENDING STOPPED" : "");
    break;
  case SMS_CMD_GET_NUM: {
    int len = snprintf(out, osz, "STN %s OFFICE NUMBERS:", stn);
    for (int i = 0; i < SMS_OFFICE_MAX && len > 0 && (size_t)len < osz; i++) {
      if (nums[i][0]) len += snprintf(out + len, osz - len, " %s", nums[i]);
    }
    break;
  }
  case SMS_CMD_SET_TIME: {
    // Only a fresh one: an SMS can wait hours, and an old time moves the clock back over slots
    // already sent. Aged by the network's clock, not this unit's (which may be what is wrong).
    char ts[15];
    char net14[16];
    long set_age = 0;
    if (sms_network_ts14(net14) && sms_age_minutes(m->ts, net14, &set_age) && set_age > SMS_SET_TIME_MAX_AGE_MIN) {
      snprintf(out, osz, "STN %s TIME NOT SET: SMS was %ld min old. Send it again", stn, set_age);
    } else if (sms_parse_time(arg, ts) && sms_set_clock(ts)) {
      snprintf(out, osz, "STN %s TIME SET %.14s", stn, ts);
    } else {
      snprintf(out, osz, "STN %s TIME NOT SET: use SET_TIME YYYYMMDDHHMMSS", stn);
    }
    break;
  }
  case SMS_CMD_SET_STN_ID: {
    // Bihar IDs are 8 digits on its server (stored here as 6 when they fit); others 6
    const int digits = strstr(UNIT, "BIHAR") ? 8 : 6;
    char id[12];
    char old[16];
    snprintf(old, sizeof(old), "%s", stn);
    if (!sms_parse_station_id(arg, digits, id, sizeof(id))) {
      snprintf(out, osz, "STN %s ID NOT SET: 1 to %d digits, not all 0", stn, digits);
      break;
    }
    const long v = atol(id);
    char store[12];
    if (v <= 999999L) snprintf(store, sizeof(store), "%06ld", v);
    else snprintf(store, sizeof(store), "%ld", v);
    if (atol(old) == v) {
      snprintf(out, osz, "STN %s ID ALREADY %s", stn, store);
      break;
    }
    // As from the keypad: the old station's records and backlog are cleared (they would go out
    // under the new ID)
    set_station_id(store, true);
    snprintf(out, osz, "STN %s ID SET: %s -> %s (old records cleared)", store, old, store);
    break;
  }
  case SMS_CMD_SET_SERVER_2: {
    char host[64], path[64], key[15];
    uint16_t port = 0;
    bool off = false;
    if (!sms_parse_server2(arg, host, sizeof(host), &port, path, sizeof(path), key, sizeof(key), &off)) {
      snprintf(out, osz, "STN %s SERVER 2 NOT SET: host=..;port=..;path=/..;key=.. or OFF", stn);
      break;
    }
    if (!server2_save(off ? "" : host, port, path, key)) {
      snprintf(out, osz, "STN %s SERVER 2 NOT SET: flash write failed", stn);
    } else if (off) {
      server_mode_save(0);   // nothing to send to
      snprintf(out, osz, "STN %s SERVER 2 OFF, MODE 0", stn);
    } else {
      snprintf(out, osz, "STN %s SERVER 2 SET %.40s:%u%.40s MODE %d", stn, host, (unsigned)port, path, server_mode);
    }
    break;
  }
  case SMS_CMD_SET_SERVER_MODE: {
    const int mode = (arg[0] >= '0' && arg[0] <= '2' && arg[1] == '\0') ? arg[0] - '0' : -1;
    if (mode < 0) {
      snprintf(out, osz, "STN %s MODE NOT SET: 0 server 1, 1 server 2, 2 both", stn);
    } else if (mode > 0 && !secondaryServer.serverName[0]) {
      snprintf(out, osz, "STN %s MODE NOT SET: set SERVER_2 first", stn);
    } else {
      snprintf(out, osz, server_mode_save(mode) ? "STN %s MODE %d SET" : "STN %s MODE NOT SET (%d): flash write failed",
               stn, mode);
    }
    break;
  }
  case SMS_CMD_SET_OTA_SERVER: {
    char host[44];
    uint16_t port = 80;
    bool off = false;
    if (!sms_parse_ota_server(arg, host, sizeof(host), &port, &off)) {
      snprintf(out, osz, "STN %s OTA SERVER NOT SET: host [port], or OFF", stn);
    } else if (!ota_server_set(off ? NULL : host, port)) {
      snprintf(out, osz, "STN %s OTA SERVER NOT SET: flash write failed", stn);
    } else {
      snprintf(out, osz, "STN %s OTA SERVER %s %s:%d", stn, off ? "DEFAULT" : "SET", ota_server_host(), ota_server_port());
    }
    break;
  }
  case SMS_CMD_ADD_NUM:
  case SMS_CMD_DEL_NUM: {
    char num[11];
    if (!sms_number_last10(arg, num)) {
      snprintf(out, osz, "STN %s NUMBER NOT CHANGED: give 10 digits", stn);
      break;
    }
    int at = -1, free_at = -1;
    for (int i = 0; i < SMS_OFFICE_MAX; i++) {
      if (nums[i][0] && strcmp(nums[i], num) == 0) at = i;
      if (!nums[i][0] && free_at < 0) free_at = i;
    }
    const bool add = (cmd == SMS_CMD_ADD_NUM);
    if (add && at >= 0) {
      snprintf(out, osz, "STN %s %s ALREADY AN OFFICE NUMBER", stn, num);
      break;
    }
    if (add && free_at < 0) {
      snprintf(out, osz, "STN %s LIST FULL (%d): DEL_NUM one first", stn, SMS_OFFICE_MAX);
      break;
    }
    if (!add && at < 0) {
      snprintf(out, osz, "STN %s %s IS NOT IN THE LIST", stn, num);
      break;
    }
    if (!add && office_count(nums) <= 1) {
      snprintf(out, osz, "STN %s LAST OFFICE NUMBER: ADD_NUM another first", stn);
      break;
    }
    if (add) strcpy(nums[free_at], num);
    else nums[at][0] = '\0';
    if (office_save(nums)) {
      snprintf(out, osz, "STN %s %s %s, %d OFFICE NUMBERS", stn, num, add ? "ADDED" : "REMOVED", office_count(nums));
    } else {
      snprintf(out, osz, "STN %s %s NOT %s: flash write failed", stn, num, add ? "ADDED" : "REMOVED");
    }
    break;
  }
  case SMS_CMD_ERASE_DATA: {
    // The ID must be this unit's (leading zeros ignored), so a message meant for another unit can't
    char id[12] = "";
    if (!sms_parse_station_id(arg, 8, id, sizeof(id)) || atol(id) != atol(stn)) {
      snprintf(out, osz, "STN %s NOT ERASED: send VARSHA ERASE_DATA %s (this unit's ID)", stn, stn);
      break;
    }
    snprintf(out, osz, "STN %s ERASING ALL DATA (records, backlog). The unit restarts", stn);
    sms_reply(m->from, out);
    force_delete_data = true;   // done by the GPRS task after this slot's commands
    return;
  }
  case SMS_CMD_STOP:
  case SMS_CMD_RESUME: {
    const bool stop = (cmd == SMS_CMD_STOP);
    if (live_post_muted == stop) {
      snprintf(out, osz, stop ? "STN %s SENDING ALREADY STOPPED" : "STN %s ALREADY SENDING", stn);
      break;
    }
    live_post_muted = stop;
    Preferences p;
    bool ok = p.begin("sys-config", false);
    if (ok) {
      ok = p.putBool("muted", stop) > 0;
      p.end();
    }
    if (!ok) {
      snprintf(out, osz, "STN %s NOT CHANGED: flash write failed", stn);
    } else if (stop) {
      snprintf(out, osz, "STN %s SENDING STOPPED. Records kept on the unit, SMS still answered. VARSHA RESUME to restart", stn);
    } else {
      snprintf(out, osz, "STN %s SENDING RESUMED from the next slot", stn);
    }
    break;
  }
  default:
    snprintf(out, osz, "STN %s UNKNOWN COMMAND. VARSHA GET_STATUS GET_GPS GET_CONFIG SET_TIME SET_STN_ID SET_SERVER_2 "
             "SET_SERVER_MODE SET_OTA_SERVER ADD_NUM DEL_NUM GET_NUM STOP RESUME ERASE_DATA", stn);
    break;
  }
  sms_reply(m->from, out);
}

// ---------------------------------------------------------------- each slot
// Reads the SIM once and runs the commands found. Called by the GPRS task after the slot's send.
void send_sms() {
  set_sys_status("CHECKING SMS");
  if (xSemaphoreTake(modemMutex, pdMS_TO_TICKS(10000)) != pdTRUE) {
    debugln("[SMS] Modem busy: SIM not read this slot.");
    return;
  }
  SerialSIT.print("AT+CMGF=1\r");
  waitForResponse("OK", 2000);
  // The buffer is cleared before the command and not after it, so the start of a long list
  // can't be dropped; the wait is for the list's own closing line ("OK" alone can be in a message)
  flushSerialSIT();
  modem_response_buf[0] = '\0';   // the no-flush wait appends to what is there
  SerialSIT.print("AT+CMGL=\"ALL\"\r");
  const bool listed = waitForResponseNoFlush("\r\nOK\r\n", 6000);
  int n = 0;
  if (listed) {
    snprintf(s_sms_list, sizeof(s_sms_list), "%s", modem_response_buf);
    n = sms_split_inbox(s_sms_list);
  } else {
    debugf("[SMS] SIM not read: %.60s\n", modem_response_buf);
  }
  if (n == 0 && listed) debugln("[SMS] No SMS waiting.");
  for (int i = 0; i < n; i++) {
    char cmd[24];
    snprintf(cmd, sizeof(cmd), "AT+CMGD=%d\r", s_inbox[i].index);
    SerialSIT.print(cmd);
    if (!waitForResponse("OK", 5000)) {
      debugf("[SMS] Could not delete message %d: not run (it would run again)\n", s_inbox[i].index);
      continue;
    }
    sms_run(&s_inbox[i]);
  }
  portENTER_CRITICAL(&syncMux);
  sync_mode = eSMSStop;
  portEXIT_CRITICAL(&syncMux);
  xSemaphoreGive(modemMutex);
}
