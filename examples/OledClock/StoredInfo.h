#ifndef OLED_CLOCK_STORED_INFO_H
#define OLED_CLOCK_STORED_INFO_H

#include <AceTime.h>

/** Data that is saved to and retrieved from EEPROM. */
struct StoredInfo {
  /** 12:00:00 AM to 12:00:00 PM */
  static uint8_t const kTwelve = 0;

  /** 00:00:00 - 23:59:59 */
  static uint8_t const kTwentyFour = 1;

  uint8_t hourMode; // either kTwelve or kTwentyFour

  uint8_t timeZoneType; // kTypeManual or kTypeBasic
  int16_t offsetMinutes; // defined if timeZoneType == kTypeManual
  bool isDst; // defined if timeZoneType == kTypeManual
  // TODO: Replace with stable hash(ZoneInfo)
  uint8_t zoneIndex; // defined if timeZoneType == kTypeBasic
};

#endif
