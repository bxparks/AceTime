#ifndef COMMAND_LINE_CLOCK_STORED_INFO_H
#define COMMAND_LINE_CLOCK_STORED_INFO_H

#include <AceTime.h>
#include "config.h"

/** Data that is saved to and retrieved from EEPROM. */
struct StoredInfo {

  /** Time zone of the displayed time */
  uint8_t timeZoneType;

  /** The STD offset for kTypeManual. */
  int16_t stdMinutes;

  /** The DST offset for kTypeManual. */
  int16_t dstMinutes;

  /**
  * Stable zone ID for kTypeBasic, kTypeExtended, kTypeBasicManaged,
   * kTypeExtendedManaged.
   */
  uint32_t zoneId;

#if TIME_SOURCE_TYPE == TIME_SOURCE_TYPE_NTP
  static const uint8_t kSsidMaxLength = 33; // 32 + NUL terminator
  static const uint8_t kPasswordMaxLength = 64; // 63 + NUL terminator

  char ssid[kSsidMaxLength];
  char password[kPasswordMaxLength];
#endif

};

#endif
