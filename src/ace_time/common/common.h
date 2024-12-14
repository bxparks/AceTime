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
  *
  *   - The longest explicit abbreviation in the database (as of 2019 or so)
  *   seems to be 5 characters (https://www.timeanddate.com/time/zones/)
  *   - The TZ database spec used to say that abbreviations are 3 to 6
  *   characters but that wording is no longer in the document
  *   (https://data.iana.org/time-zones/theory.html#abbreviations).
  *   - The zic(1) man page says "A time zone abbreviation has fewer than 3 or
  *   more than 6 characters. POSIX requires at least 3, and requires
  *   implementations to support at least 6". The first part of that wording
  *   makes no sense at all.
  *   - The %z specifier, added in TZDB 2024b, autogenerates the abbreviation
  *   using a [+/-][hh[mm[ss]]] pattern, which can be 7 characters long.
  *
  * Let's increase the max length from 6 to 7 to handle the %z. We also need one
  * extra byte for the terminating NUL character.
  */
const uint8_t kAbbrevSize = 7 + 1;

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
