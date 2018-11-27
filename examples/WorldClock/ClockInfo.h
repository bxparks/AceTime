#ifndef WORLD_CLOCK_CLOCK_INFO_H
#define WORLD_CLOCK_CLOCK_INFO_H

#include <AceTime.h>

/** Data that describes the clock of a single time zone. */
struct ClockInfo {
  /** Size of the clock name buffer, including '\0'. */
  static uint8_t const kNameSize = 5;

  /** 12:00:00 AM to 12:00:00 PM */
  static uint8_t const kTwelve = 0;

  /** 00:00:00 - 23:59:59 */
  static uint8_t const kTwentyFour = 1;

  /** Name of this clock, e.g. City or Time Zone ID */
  char name[kNameSize];

  /** Time zone of the clock. */
#if TIME_ZONE_TYPE == TIME_ZONE_TYPE_MANUAL
  ace_time::ManualTimeZone timeZone;
#else
  ace_time::AutoTimeZone timeZone;
#endif

  /** Hour mode, 12H or 24H. */
  uint8_t hourMode = kTwelve;

  /** Blink the colon in HH:MM. */
  bool blinkingColon = false;
};

#endif
