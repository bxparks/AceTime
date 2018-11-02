#ifndef WORLD_CLOCK_STORED_INFO_H
#define WORLD_CLOCK_STORED_INFO_H

#include "ClockInfo.h"

/**
 * Data that is saved to and retrieved from EEPROM.
 * These settings apply to all clocks.
 */
struct StoredInfo {
  /** DST or not. */
  bool isDst;

  /** Hour mode, 12H or 24H. */
  uint8_t hourMode;

  /** Blink the colon in HH:MM. */
  bool blinkingColon;
};

#endif
