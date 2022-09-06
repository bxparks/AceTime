/*
 * MIT License
 * Copyright (c) 2022 Brian T. Park
 */

#ifndef ACE_TIME_EPOCH_CONVERTER_JULIAN_H
#define ACE_TIME_EPOCH_CONVERTER_JULIAN_H

#include <stdint.h>

namespace ace_time {
namespace internal {

/**
 * Utility class that converts AceTime epoch days to (year, month, day) in the
 * Gregorian calendar and vise versa. Uses the algorithm described in
 * https://en.wikipedia.org/wiki/Julian_day which converts into Julian days as
 * an intermediate step.
 */
class EpochConverterJulian {
  public:
    /** Base year of the AceTime epoch. */
    static const int16_t kEpochYear = 2000;

    /**
     * Number of days between the Julian calendar epoch (4713 BC 01-01) and the
     * AceTime epoch (2000-01-01).
     */
    static const int32_t kDaysSinceJulianEpoch = 2451545;

    /**
     * Convert (year, month, day) in the Gregorian calendar to days since
     * AceTime Epoch (2000-01-01). The (year, month, day) is converted into
     * Julian days, then converted to epoch days since AceTime Epoch. The Julian
     * day normally start at 12:00:00. But this method returns the delta number
     * of days since 00:00:00, so we can interpret the Gregorian calendar day to
     * start at 00:00:00.
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
      return jdn - kDaysSinceJulianEpoch;
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

      uint32_t J = epochDays + kDaysSinceJulianEpoch;
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
}

#endif
