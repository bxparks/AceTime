#ifndef ACE_TIME_LOCAL_DATE_H
#define ACE_TIME_LOCAL_DATE_H

#include <stdint.h>

namespace ace_time {

class LocalDate {
  public:
    /** Base year of epoch. */
    static const uint16_t kEpochYear = 2000;

    /**
     * Number of days between the Julian calendar epoch (4713 BC 01-01) and the
     * AceTime epoch (2000-01-01).
     */
    static const uint32_t kDaysSinceJulianEpoch = 2451545;

    /** Monday ISO 8601 number. */
    static const uint8_t kMonday = 1;

    /** Tuesday ISO 8601 number. */
    static const uint8_t kTuesday = 2;

    /** Wednesday ISO 8601 number. */
    static const uint8_t kWednesday = 3;

    /** Thursday ISO 8601 number. */
    static const uint8_t kThursday = 4;

    /** Friday ISO 8601 number. */
    static const uint8_t kFriday = 5;

    /** Saturday ISO 8601 number. */
    static const uint8_t kSaturday = 6;

    /** Sunday ISO 8601 number. */
    static const uint8_t kSunday = 7;

    /**
     * Factory method using separated year, month and day fields.
     *
     * @param year last 2 digits of the year from year 2000
     * @param month month with January=1, December=12
     * @param day day of month (1-31)
     */
    static LocalDate forComponents(uint8_t year, uint8_t month, uint8_t day) {
      return LocalDate(year, month, day);
    }

    /**
     * Factory method using the number of days from AceTime epoch of 2000-01-01.
     */
    static LocalDate forEpochDays(uint32_t epochDays) {
      uint8_t year, month, day;
      extractYearMonthDay(epochDays, year, month, day);
      return LocalDate(year, month, day);
    }

    /**
     * Factory method. Create a LocalDate from the ISO 8601 date string. If
     * the string cannot be parsed, then isError() on the constructed object
     * returns true.
     */
    static LocalDate forDateString(const char* dateString) {
      return LocalDate().initFromDateString(dateString);
    }

    explicit LocalDate() {}

    /**
     * Mark the LocalDate so that isError() returns true. Returns a reference
     * to (*this) so that an invalid LocalDate can be returned in a single
     * statement like this: 'return LocalDate().setError()'.
     */
    LocalDate& setError() {
      mYear = mMonth = mDay = 0;
      return *this;
    }

    /** Return the 2 digit year from year 2000. */
    uint8_t year() const { return mYear; }

    /** Return the full year instead of just the last 2 digits. */
    uint16_t yearFull() const { return mYear + kEpochYear; }

    /** Return the month with January=1, December=12. */
    uint8_t month() const { return mMonth; }

    /** Return the day of the month. */
    uint8_t day() const { return mDay; }

    /**
     * Calculate the day of week given the (year, month, day). Idea borrowed
     * from https://github.com/evq/utz. No validation of year, month or day is
     * performed. If this is found to be too slow, then consider caching the
     * results.
     */
    uint8_t dayOfWeek() const {
      // The "year" starts in March to shift leap year calculation to end.
      uint16_t y = kEpochYear + mYear - (mMonth < 3);
      uint16_t d = y + y/4 - y/100 + y/400 + sDayOfWeek[mMonth-1] + mDay;
      // 2000-1-1 was a Saturday=6
      return (d + 1) % 7 + 1;
    }

    /** Return true if any component indicates an error condition. */
    bool isError() const {
      return mDay < 1 || mDay > 31
          || mMonth < 1 || mMonth > 12;
    }

    /**
     * Return number of days since AceTime epoch (2000-01-01 00:00:00Z).
     *
     * Uses Julian days which normally start at 12:00:00. But this method
     * returns the delta number of days since 00:00:00, so we can interpret the
     * Gregorian calendar day to start at 00:00:00.
     *
     * See https://en.wikipedia.org/wiki/Julian_day
     */
    uint32_t toEpochDays() const {
      // From wiki article:
      //
      // JDN = (1461 x (Y + 4800 + (M - 14)/12))/4
      //     + (367 x (M - 2 - 12 x ((M - 14)/12)))/12
      //     - (3 x ((Y + 4900 + (M - 14)/12)/100))/4
      //     + D - 32075
      // JDN2000 = JDN - 2451545
      //
      // It looks like the formula needs to be done using signed integers
      // because it depends on the modulo operation (%) to truncate towards 0
      // for negative numbers.

      int8_t mm = (mMonth - 14)/12;
      int16_t yy = mYear + kEpochYear;
      int32_t jdn = ((int32_t) 1461 * (yy + 4800 + mm))/4
          + (367 * (mMonth - 2 - 12 * mm))/12
          - (3 * ((yy + 4900 + mm)/100))/4
          + mDay - 32075;
      return jdn - kDaysSinceJulianEpoch;
    }

    /**
     * Compare this LocalDate to that LocalDate, returning (<0, 0, >0)
     * according to whether the epochSeconds is (this<that, this==this,
     * this>that).
     */
    int8_t compareTo(const LocalDate& that) const {
      if (mYear < that.mYear) return -1;
      if (mYear > that.mYear) return 1;
      if (mMonth < that.mMonth) return -1;
      if (mMonth > that.mMonth) return 1;
      if (mDay < that.mDay) return -1;
      if (mDay > that.mDay) return 1;
      return 0;
    }

    /**
     * Print LocalDate to 'printer' in ISO 8601 format, along with the
     * day of week. Does not implement Printable to avoid memory cost of a
     * vtable pointer.
     */
    void printTo(Print& printer) const;

    /**
     * Extract the (year, month, day, dayOfWeek) fields from epochDays.
     *
     * See https://en.wikipedia.org/wiki/Julian_day.
     */
    static void extractYearMonthDay(uint32_t epochDays, uint8_t& year,
        uint8_t& month, uint8_t& day) {
      uint32_t J = epochDays + kDaysSinceJulianEpoch;
      uint32_t f = J + 1401 + (((4 * J + 274277 ) / 146097) * 3) / 4 - 38;
      uint32_t e = 4 * f + 3;
      uint32_t g = e % 1461 / 4;
      uint32_t h = 5 * g + 2;
      day = (h % 153) / 5 + 1;
      month = (h / 153 + 2) % 12 + 1;
      year = (e / 1461) - 4716 + (12 + 2 - month) / 12 - kEpochYear;

      // 2000-01-01 is Saturday (7)
      //dayOfWeek = (epochDays + 6) % 7 + 1;
    }

  private:
    friend bool operator==(const LocalDate& a, const LocalDate& b);
    friend bool operator!=(const LocalDate& a, const LocalDate& b);

    /** Minimum length of the date string. yyyy-mm-dd. */
    static const uint8_t kDateStringLength = 10;

    /**
     * Day of week table for each month, with 0=Jan to 11=Dec. The table
     * offsets actualy start with March to make the leap year calculation
     * easier.
     */
    static const uint8_t sDayOfWeek[12];

    explicit LocalDate(uint8_t year, uint8_t month, uint8_t day):
        mYear(year),
        mMonth(month),
        mDay(day) {}

    /** Extract the date components from the given dateString. */
    LocalDate& initFromDateString(const char* dateString);

    uint8_t mYear; // [00, 99], year - 2000
    uint8_t mMonth; // [1, 12], 0 indicates error
    uint8_t mDay; // [1, 31], 0 indicates error
};

/**
 * Return true if two LocalDate objects are equal in all components.
 */
inline bool operator==(const LocalDate& a, const LocalDate& b) {
  return a.mDay == b.mDay
      && a.mMonth == b.mMonth
      && a.mYear == b.mYear;
}

/** Return true if two LocalDate objects are not equal. */
inline bool operator!=(const LocalDate& a, const LocalDate& b) {
  return ! (a == b);
}

}

#endif
