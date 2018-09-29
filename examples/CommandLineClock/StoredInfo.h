#ifndef COMMAND_LINE_CLOCK_STORED_INFO_H
#define COMMAND_LINE_CLOCK_STORED_INFO_H

#include <AceTime.h>
#include "config.h"

/** Data that is saved to and retrieved from EEPROM. */
struct StoredInfo {
  int8_t tzCode;

#if defined(USE_NTP)
  static const uint8_t kSsidMaxLength = 33; // 32 + NUL terminator
  static const uint8_t kPasswordMaxLength = 64; // 63 + NUL terminator

  char ssid[kSsidMaxLength];
  char password[kPasswordMaxLength];
#endif

};

#endif
