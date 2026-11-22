#ifndef REMOTE_HEALTH_TYPES_H
#define REMOTE_HEALTH_TYPES_H
// v6.57: types used by sms_remote.ino and health_day.ino. In a header so the Arduino build's
// generated prototypes (placed before the .ino code) can see them.
#include <stdint.h>
#include "sms_cmd.h"

// One SMS from the SIM (sms_remote.ino)
struct sms_in_t {
  int index;
  char from[24];
  char ts[24];
  char body[200];
};

// One MET day's figures for the daily health report (health_day.ino)
struct hd_day_t {
  uint32_t day;            // closing date YYYYMMDD, 0 = none
  uint16_t sent, bk, ne;
  uint16_t bd_n, bn_n;
  float bd_sum, bn_sum, bmin;
  uint32_t awake_s;
  uint16_t awake_n;
  char err[24];
};

// A GET_ command answered lately (sms_remote.ino): sender, command, minute stamp
struct sms_answered_t {
  char num[11];
  uint8_t cmd;
  uint32_t at_min;
};

// v6.57 state kept over deep sleep AND restarts. RTC_DATA_ATTR variables are set back to their
// starting values by the bootloader on every reset except a wake from deep sleep (software, panic,
// watchdog, OTA restart); RTC_NOINIT memory is not, so fault and watchdog restarts can be counted.
// It holds anything after a power-on and the old firmware's bytes after an update: valid only with
// the magic and this firmware's image ID (health_note_boot()).
struct v657_rtc_t {
  uint32_t magic;
  uint32_t image;
  hd_day_t cur, prev;
  uint32_t reported_day;
  uint16_t rst, wdt;
  long clk;
  sms_answered_t answered[6];
  uint16_t pon;            // v6.61: power-ons (battery changed or ran flat, power cut) since the last report
};

#endif
