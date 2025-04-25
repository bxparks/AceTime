/*
 * MIT License
 * Copyright (c) 2022 Brian T. Park
 */

#ifndef ACE_TIME_EPOCH_CONVERTER_JULIAN_H
#define ACE_TIME_EPOCH_CONVERTER_JULIAN_H

#include <stdint.h>

namespace ace_time {

/**
 * Utility class that converts AceTime epoch days to (year, month, day) in the
 * Gregorian calendar and vise versa. Uses the algorithm described in
 * https://en.wikipedia.org/wiki/Julian_day which converts into Julian days as
 * an intermediate step.
 */
class EpochConverterJulian {
  public:
    /**
     * Epoch year used by this epoch converter. Must be a multiple of 400. Other
     * parts of the AceTime library will probably use a different epoch year.
     */
    static const int16_t kInternalEpochYear = 2000;

    /**
     * Number of days from Unix epoch (1970-01-01 00:00:00 UTC) to
     * the internal epoch (2000-01-01 00:00:00 UTC).
     */
    static const int32_t kDaysToInternalEpochFromUnixEpoch = 10957;

    /**
     * Number of days from the modified proleptic Julian calendar epoch (4713
     * BC 01-01, modified to start at 00:00:00 instead of 12:00:00) to the
     * internal epoch (2000-01-01). There are 1721060 days from the modified
     * Julian epoch to 0000-01-01 of the proleptic Gregorian calendar. We then
     * need to add 2000 years (5 x 400 years) to get to 2000-01-01.
     */
    static const int32_t kDaysToInternalEpochFromJulianEpoch = 1721060
        + (kInternalEpochYear / 400) * 146097; // 2451545

    /**
     * Convert (year, month, day) in the Gregorian calendar to days since the
     * internal epoch (2000-01-01). The (year, month, day) is converted into
     * Julian days, then converted to epoch days since AceTime Epoch. The Julian
     * day normally start at 12:00:00, but we use a modified Julian day number
     * starting at 00:00:00 to make it easier to convert to the Gregorian
     * calendar day.
     *
     * @verbatim
     * JDN = (1461 x (Y + 4800 + (M - 14)/12))/4
     *     + (367 x (M - 2 - 12 x ((M - 14)/12)))/12
     *     - (3 x ((Y + 4900 + (M - 14)/12)/100))/4
     *     + D - 32075
     * JDN2000 = JDN - 2451545
     * @endverbatim
     *
     * It looks like the formula needs to be done using signed integers
     * because it depends on the modulo operation (%) to truncate towards 0
     * for negative numbers.
     *
     * No input validation is performed. The behavior is undefined if the
     * parameters are outside their expected range.
     *
     * @param year year [1,9999]
     * @param month month integer [1, 12]
     * @param day day of month integer[1, 31]
     */
    static int32_t toEpochDays(int16_t year, uint8_t month, uint8_t day) {
      int8_t mm = (month - 14)/12;
      int32_t jdn = ((int32_t) 1461 * (year + 4800 + mm))/4
          + (367 * (month - 2 - 12 * mm))/12
          - (3 * ((year + 4900 + mm)/100))/4
          + day - 32075;
      return jdn - kDaysToInternalEpochFromJulianEpoch;
    }

    /**
     * Extract the (year, month, day) fields from AceTime epochDays.
     * See https://en.wikipedia.org/wiki/Julian_day for formula.
     *
     * No input validation is performed. The behavior is undefined if the
     * parameters are outside their expected range.
     */
    static void fromEpochDays(int32_t epochDays,
        int16_t& year, uint8_t& month, uint8_t& day) {

      uint32_t J = epochDays + kDaysToInternalEpochFromJulianEpoch;
      uint32_t f = J + 1401 + (((4 * J + 274277 ) / 146097) * 3) / 4 - 38;
      uint32_t e = 4 * f + 3;
      uint32_t g = e % 1461 / 4;
      uint32_t h = 5 * g + 2;
      day = (h % 153) / 5 + 1;
      month = (h / 153 + 2) % 12 + 1;
      year = (e / 1461) - 4716 + (12 + 2 - month) / 12;

      // 2000-01-01 is Saturday (7)
      //dayOfWeek = (epochDays + 6) % 7 + 1;
    }
};

}

#endif
