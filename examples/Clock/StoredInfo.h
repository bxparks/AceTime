#ifndef CLOCK_STORED_INFO_H
#define CLOCK_STORED_INFO_H

#include <AceTime.h>

/** Data that is saved to and retrieved from EEPROM. */
struct StoredInfo {
  int8_t tzCode;
};

#endif
