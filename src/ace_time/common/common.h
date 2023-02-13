/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#ifndef ACE_TIME_COMMON_COMMON_H
#define ACE_TIME_COMMON_COMMON_H

#include <stdint.h>

/**
 * @file common.h
 *
 * Identifiers used by implementation code which need to be publically exported.
 */

namespace ace_time {

/**
 * Type for the number of seconds from epoch. The AceTime epoch is 2050-01-01
 * 00:00:00 UTC by default but can be changed using `Epoch::currentEpochYear()`.
 * Unix epoch is 1970-01-01 00:00:00 UTC.
 */
typedef int32_t acetime_t;

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

/** Swap 2 parameters. */
template <typename T>
void swap(T& a, T& b) {
  T tmp = a;
  a = b;
  b = tmp;
}

}
}

#endif
