#ifndef WORLD_CLOCK_STORED_INFO_H
#define WORLD_CLOCK_STORED_INFO_H

#include "ClockInfo.h"

/**
 * Data that is saved to and retrieved from EEPROM.
 * These settings apply to all clocks.
 */
struct StoredInfo {
#if TIME_ZONE_TYPE == TIME_ZONE_TYPE_MANUAL
  /** DST or not. */
  bool isDst0;
  bool isDst1;
  bool isDst2;
#endif

  /** Hour mode, 12H or 24H. */
  uint8_t hourMode;

  /** Blink the colon in HH:MM. */
  bool blinkingColon;
};

#endif
