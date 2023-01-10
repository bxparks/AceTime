/*
 * MIT License
 * Copyright (c) 2023 Brian T. Park
 */

#ifndef ACE_TIME_INTERNAL_COMMON_H
#define ACE_TIME_INTERNAL_COMMON_H

#include <stdint.h>

/**
 * @file common.h
 *
 * Internal identifiers used by implementation code, not intended to be
 * publically exported.
 */

namespace ace_time {
namespace internal {

/**
  * Size of the c-string buffer needed to hold a time zone abbreviation.
  * Longest abbreviation currently seems to be 5 characters
  * (https://www.timeanddate.com/time/zones/) but the TZ database spec says
  * that abbreviations are 3 to 6 characters
  * (https://data.iana.org/time-zones/theory.html#abbreviations), so use 6 as
  * the maximum. Plus one for the terminating NUL character.
  */
const uint8_t kAbbrevSize = 6 + 1;

}
}

#endif