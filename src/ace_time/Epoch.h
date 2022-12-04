/*
 * MIT License
 * Copyright (c) 2022 Brian T. Park
 */

#ifndef ACE_TIME_EPOCH_H
#define ACE_TIME_EPOCH_H

#include <stdint.h>
#include "internal/EpochConverterHinnant.h"

/**
 * Select the epoch converter:  EpochConverterHinnant or EpochConverterJulian
 */
#define ACE_TIME_EPOCH_CONVERTER internal::EpochConverterHinnant

namespace ace_time {

/**
 * Utitliy functions for setting, retrieving, and converting the current epoch.
 * Normally, the default epoch is 2050-01-01T00:00:00 UTC. It can be changed by
 * calling the `currentEpochYear(year)` function.
 */
class Epoch {
  public:
    /** Get the current epoch year. */
    static int16_t currentEpochYear() {
      return sCurrentEpochYear;
    }

    /** Set the current epoch year. */
    static void currentEpochYear(int16_t year) {
      sCurrentEpochYear = year;
      sDaysToCurrentEpochFromConverterEpoch = daysFromConverterEpoch(year);
    }

    /**
     * Return number of days to the given {year}-01-01 from the converter
     * epoch of 2000-01-01.
     */
    static int32_t daysFromConverterEpoch(int16_t year) {
      return ACE_TIME_EPOCH_CONVERTER::toEpochDays(year, 1, 1);
    }

    /**
     * Number of days from the converter epoch (2000-01-01) to the current
     * epoch.
     */
    static int32_t daysToCurrentEpochFromConverterEpoch() {
      return sDaysToCurrentEpochFromConverterEpoch;
    }

    /**
     * Return the number of days from the Unix epoch (1970-01-01T00:00:00) to
     * the current epoch.
     */
    static int32_t daysToCurrentEpochFromUnixEpoch() {
      return ACE_TIME_EPOCH_CONVERTER::kDaysToConverterEpochFromUnixEpoch
          + sDaysToCurrentEpochFromConverterEpoch;
    }

    /**
     * Return the number of seconds from the Unix epoch (1970-01-01T00:00:00) to
     * the current epoch. The return type is a 64-bit integer because a 32-bit
     * integer would overflow if the current epoch year is set to later than
     * 2038.
     */
    static int64_t secondsToCurrentEpochFromUnixEpoch64() {
      return daysToCurrentEpochFromUnixEpoch() * (int64_t) 86400;
    }

    /**
     * The smallest year (inclusive) for which calculations involving the 32-bit
     * `epoch_seconds` and time zone transitions are guaranteed to be valid
     * without underflowing or overflowing. Valid years satisfy the condition
     * `year >= validYearLower()`. This condition is not enforced by any code
     * within the library. The limit is exposed for informational purposes for
     * downstream applications.
     *
     * A 32-bit integer has a range of about 136 years, so the half interval is
     * 68 years. But the algorithms to calculate transitions in
     * `zone_processing.h` use a 3-year window straddling the current year, so
     * the actual lower limit is probably closer to `currentEpochYear() - 66`.
     * To be conservative, this function returns `currentEpochYear() - 50`. It
     * may return a smaller value in the future if the internal calculations can
     * be verified to avoid underflow or overflow problems.
     */
    static int16_t epochValidYearLower() {
      return currentEpochYear() - 50;
    }

    /**
     * The largest year (exclusive) for which calculations involving the 32-bit
     * `epoch_seconds` and time zone transitions are guaranteed to be valid
     * without underflowing or overflowing. Valid years satisfy the condition
     * `year < validYearUpper()`. This condition is not enforced by any code
     * within the library. The limit is exposed for informational purposes for
     * downstream applications.
     *
     * A 32-bit integer has a range of about 136 years, so the half interval is
     * 68 years. But the algorithms to calculate the transitions in
     * `zone_processing.h` use a 3-year window straddling the current year, so
     * actual upper limit is probably close to `currentEpochYear() + 66`. To be
     * conservative, this function returns `currentEpochYear() + 50`. It may
     * return a larger value in the future if the internal calculations can be
     * verified to avoid underflow or overflow problems.
     */
    static int16_t epochValidYearUpper() {
      return currentEpochYear() + 50;
    }

  private:
    /** Base year `yyyy` of current epoch {yyyy}-01-01T00:00:00. */
    static int16_t sCurrentEpochYear;

    /** Number of days from kConverterEpochYear to sCurrentEpochYear. */
    static int32_t sDaysToCurrentEpochFromConverterEpoch;
};

}

#endif
