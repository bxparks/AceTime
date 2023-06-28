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

/**
 * The epochSecond and the expected UTC offset and dateTime components.
 * This is the C++ representation of the 'TestItem' entry in
 * validation_data.json file which is defined in
 * AceTimeTools/datatypes/valtyping.py. The 'type' contains a single
 * character with the following meanings:
 *
 *    * 'A': pre-transition where the UTC offset is different
 *    * 'B': post-transition where the UTC offset is different
 *    * 'a': pre-transition where only the DST offset is different
 *    * 'b': post-transition where only the DST offset is different
 *    * 'S': a monthly test sample, on the 2nd day of the month
 *    * 'T': an alternate monthly test sample, if the 1st was invalid
 *    * 'Y': end of year test sample
 */
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
  char const type;
};

/**
 * Collection of ValidationItems (usually 300-500 samples, over 50-100 years,
 * for example, from year 2000 until 2100) for a particular timezone (e.g.
 * America/Los_Angeles) generated from a third party date/time library (e.g.
 * Python pytz). The AceTime classes will be tested against this dataset using
 * the BasicTransitionTest or ExtendedTransitionTest classes.
 */
struct ValidationData {
  int const startYear;
  int const untilYear;
  int const epochYear;
  uint16_t const numTransitions;
  const ValidationItem* const transitions;
  uint16_t const numSamples;
  const ValidationItem* const samples;
};

}
}

#endif
