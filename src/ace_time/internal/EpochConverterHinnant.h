/*
 * MIT License
 * Copyright (c) 2022 Brian T. Park
 */

#ifndef ACE_TIME_EPOCH_CONVERTER_HINNANT_H
#define ACE_TIME_EPOCH_CONVERTER_HINNANT_H

#include <stdint.h>

namespace ace_time {
namespace internal {

/**
 * Utility class that converts AceTime epoch days to (year, month, day) in the
 * Gregorian calendar and vise versa. Uses the algorithm described in
 * https://howardhinnant.github.io/date_algorithms.html.
 */
class EpochConverterHinnant {
  public:
    /**
     * Epoch year used by this epoch converter. Must be a multiple of 400. Other
     * parts of the AceTime library will probably use a different epoch year.
     */
    static const int16_t kConverterEpochYear = 2000;

    /**
     * Convert (year, month, day) in the Gregorian calendar to days since the
     * converter epoch (2000-01-01 instead of 2050-01-01). The `year` is
     * restricted to be greater than or equal to 0001, which allows the internal
     * 400-year era to start on 0000-03-01 with era=0, and we don't have to
     * worry about negative eras.
     *
     * No input validation is performed. The behavior is undefined if the
     * parameters are outside their expected range. The algorithm will likely
     * work for dates between 0000-03-01 and 32767-12-31 (inclusive), but has
     * been tested only for dates between 0001-01-01 and 9999-12-31 (inclusive).
     *
     * @param year, [1,9999]
     * @param month month integer, [1,12]
     * @param day day of month integer, [1,31]
     */
    static int32_t toEpochDays(int16_t year, uint8_t month, uint8_t day) {
      uint16_t yearPrime = year - ((month <= 2) ? 1 : 0);
      uint16_t era = yearPrime / 400; // [0,24]
      uint16_t yearOfEra = yearPrime - 400 * era; // [0,399]

      uint8_t monthPrime = (month <= 2) ? month + 9 : month - 3; // [0,11]
      uint16_t daysUntilMonthPrime = toDaysUntilMonthPrime(monthPrime);
      uint16_t dayOfYearPrime = daysUntilMonthPrime + day - 1; // [0,365]
      uint32_t dayOfEra = (uint32_t) 365 * yearOfEra + (yearOfEra / 4)
          - (yearOfEra / 100) + dayOfYearPrime; // [0, 146096]

      int32_t dayOfEpochPrime = dayOfEra + 146097 * era;
      return dayOfEpochPrime
          - (kConverterEpochYear / 400) * 146097 /*relative to 2000-03-01*/
          + 60 /*relative to 2000-01-01, 2000 is a leap year*/;
    }

    /**
     * Extract the (year, month, day) fields from AceTime epochDays.
     *
     * No input validation is performed. The behavior is undefined if the
     * parameters are outside their expected range.
     *
     * @param epochDays number of days from the converter epoch of 2000-01-01
     * @param year year [1,9999]
     * @param month month integer [1, 12]
     * @param day day of month integer[1, 31]
     */
    static void fromEpochDays(int32_t epochDays,
        int16_t& year, uint8_t& month, uint8_t& day) {

      int32_t dayOfEpochPrime = epochDays
          + (kConverterEpochYear / 400) * 146097 - 60;
      uint16_t era = (uint32_t) dayOfEpochPrime / 146097; // [0,24]
      uint32_t dayOfEra = dayOfEpochPrime - 146097 * era; // [0,146096]
      uint16_t yearOfEra = (dayOfEra - dayOfEra / 1460 + dayOfEra / 36524
          - dayOfEra / 146096) / 365; // [0,399]
      uint16_t yearPrime = yearOfEra + 400 * era; // [0,9999]
      uint16_t dayOfYearPrime = dayOfEra - (365 * yearOfEra + yearOfEra/4
          - yearOfEra/100);
      uint8_t monthPrime = (5 * dayOfYearPrime + 2) / 153;
      uint16_t daysUntilMonthPrime = toDaysUntilMonthPrime(monthPrime);

      day = dayOfYearPrime - daysUntilMonthPrime + 1; // [1,31]
      month = (monthPrime < 10) ? monthPrime + 3 : monthPrime - 9; // [1,12]
      year = yearPrime + ((month <= 2) ? 1 : 0); // [1,9999]
    }

    /**
     * Return the number days before the given monthPrime.
     * This uses the original formula from Hinnant's paper.
     */
    static uint16_t toDaysUntilMonthPrime(uint8_t monthPrime) {
      return (153 * monthPrime + 2) / 5;
    }
};

}
}

#endif
