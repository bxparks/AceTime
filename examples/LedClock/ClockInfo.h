#ifndef LED_CLOCK_CLOCK_INFO_H
#define LED_CLOCK_CLOCK_INFO_H

#include <AceTime.h>
#include "config.h"

struct ClockInfo {
  /** 12:00:00 AM to 12:00:00 PM */
  static uint8_t const kTwelve = 0;

  /** 00:00:00 - 23:59:59 */
  static uint8_t const kTwentyFour = 1;

  /** 12/24 mode */
  uint8_t hourMode = kTwelve;

  /** ManualZoneSpecifier. */
  ace_time::ManualZoneSpecifier zoneSpecifier;

  /** DateTime from the TimeKeeper. */
  ace_time::ZonedDateTime dateTime;
};

#endif
