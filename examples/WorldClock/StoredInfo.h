#ifndef WORLD_CLOCK_STORED_INFO_H
#define WORLD_CLOCK_STORED_INFO_H

#include "ClockInfo.h"

/** Data that is saved to and retrieved from EEPROM. */
struct StoredInfo {
  ClockInfo clock0;
  ClockInfo clock1;
  ClockInfo clock2;
};

#endif
