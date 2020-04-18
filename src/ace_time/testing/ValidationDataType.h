/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#ifndef ACE_TIME_VALIDATION_DATA_TYPE_H
#define ACE_TIME_VALIDATION_DATA_TYPE_H

#include <stdint.h>
#include "../common/common.h"

namespace ace_time {
namespace testing {

/** The epochSecond and the expected UTC offset and dateTime components. */
struct ValidationItem {
  acetime_t const epochSeconds;
  int16_t const timeOffsetMinutes;
  int16_t const deltaOffsetMinutes;
  int16_t const year;
  uint8_t const month;
  uint8_t const day;
  uint8_t const hour;
  uint8_t const minute;
  uint8_t const second;
  const char* const abbrev;
};

/**
 * Collection of ValidationItems (usually 300-500 samples over 30-50 years,
 * 2000 to 2050 for example) for a particular timezone (e.g.
 * America/Los_Angeles) generated from a third party date/time library (e.g.
 * Python pytz). The AceTime classes will be tested against this dataset using
 * the BasicTransitionTest or ExtendedTransitionTest classes.
 */
struct ValidationData {
  uint16_t const numItems;
  const ValidationItem* const items;
};

}
}

#endif
