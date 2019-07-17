#ifndef OLED_CLOCK_CLOCK_INFO_H
#define OLED_CLOCK_CLOCK_INFO_H

#include <AceTime.h>

/** Information about the clock, mostly independent of rendering. */
struct ClockInfo {
  /** 12:00:00 AM to 12:00:00 PM */
  static uint8_t const kTwelve = 0;

  /** 00:00:00 - 23:59:59 */
  static uint8_t const kTwentyFour = 1;

  static uint8_t const kTimeModeSeconds = 0;
  static uint8_t const kTimeModeComponents = 1;

  /** 12/24 mode */
  uint8_t hourMode;

  /** TimeZone data. */
  ace_time::TimeZoneData timeZoneData;

  /**
   * Zone index into the ZoneRegistry. Defined if timeZoneData.type ==
   * kTypeBasic.
   */
  uint8_t zoneIndex;

  /** Track epochSeconds or dateTime. */
  uint8_t timeMode;

  /** Current time. */
  ace_time::ZonedDateTime dateTime;
};

#endif
