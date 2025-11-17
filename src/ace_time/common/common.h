/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#ifndef ACE_TIME_COMMON_COMMON_H
#define ACE_TIME_COMMON_COMMON_H

#include <stdint.h>

#define ACE_TIME_DEPRECATED __attribute__((deprecated))

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

/**
 * These are options that the calling code can use to control how to
 * disambiguate the ZonedDateTime during an overlap or a gap when converting
 * from a PlainDateTime.
 */
enum class Disambiguate : uint8_t {
  /** Select the earlier ZonedDateTime in an overlap, but the later
   * ZonedDateTime in a gap.
   */
  kCompatible = 0,

  /** Always select the earlier ZonedDateTime. */
  kEarlier = 1,

  /** Always select the later ZonedDateTime. */
  kLater = 2,

  /** The reverse of kCompatible. In other words, select the later ZonedDateTime
   * in an overlap, and the earlier ZonedDateTime in a gap.
   */
  kReversed = 3,
};

/**
 * The ways that a given PlainDateTime was resolved to a ZonedDateTime or
 * ZonedExtra through the `disambiguate` parameter, depending on whether the
 * PlainDateTime occurred in an overlap, a gap, or was a unique mapping.
 */
enum class Resolved : uint8_t {
  /** PlainDateTime could not be resolved. */
  kError = 0,

  /** PlainDateTime was resolved to a unique ZonedDateTime. */
  kUnique = 1,

  /** PlainDateTime was in an overlap, and resolved to the earlier
   * ZonedDateTime.
   */
  kOverlapEarlier = 2,

  /** PlainDateTime was in an overlap, and resolved to the later ZonedDateTime.
   */
  kOverlapLater = 3,

  /**
   * PlainDateTime was in a gap, and resolved to the earlier ZonedDateTime that
   * would have matched if we had extended the later transition rule backwards
   * in time.
   */
  kGapEarlier = 4,

  /**
   * PlainDateTime was in a gap, and resolved to the later ZonedDateTime
   * that would have matched if we had extended the earlier transition rule
   * forwards in time.
   */
  kGapLater = 5,
};

} // ace_time

#endif
