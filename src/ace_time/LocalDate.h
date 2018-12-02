#ifndef ACE_TIME_LOCAL_DATE_H
#define ACE_TIME_LOCAL_DATE_H

#include <stdint.h>
#include "common/Util.h"

namespace ace_time {

/**
 * The date (year, month, day) representing the date without regards to time
 * zone. The "epoch" for this library is 2000-01-01. All fields are internally
 * represented as a uint8_t, with the year representing an offset from the year
 * 2000. The range of dates of this class is therefore 2000-01-01 Saturday to
 * 2255-12-31 Monday (inclusive).
 *
 * If the year is restricted to 2000-2099 (2 digit years), these fields
 * correspond to the range supported by the DS3231 RTC chip.
 *
 * The dayOfWeek (1=Monday, 7=Sunday, per ISO 8601) is calculated from the date
 * fields.
 *
 * Parts of this class were inspired by the java.time.LocalDate class of Java 8
 * (https://docs.oracle.com/javase/8/docs/api/java/time/LocalDate.html).
 */
class LocalDate {
  public:
    /** Sentinel epochDays which indicates an error. */
    static const uint32_t kInvalidEpochDays = UINT32_MAX;

    /** Sentinel epochSeconds which indicates an error. */
    static const uint32_t kInvalidEpochSeconds = UINT32_MAX;

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
     * @param year year offset from 2000 (0-255)
     * @param month month with January=1, December=12
     * @param day day of month (1-31)
     */
    static LocalDate forComponents(uint8_t year, uint8_t month, uint8_t day) {
      return LocalDate(year, month, day);
    }

    /**
     * Factory method using the number of days since AceTime epoch of
     * 2000-01-01. If epochDays is kInvalidEpochDays, isError() will return
     * true.
     *
     * If epochDays is greater than 93501 (i.e. 2255-12-31), the year is
     * greater than 255 and cannot be represented by a uint8_t. The behavior of
     * this method is undefined. Most likely, a truncation of the correct year
     * to uint8_t will occur.
     *
     * @param epochDays number of days since AceTime epoch (2000-01-01)
     */
    static LocalDate forEpochDays(uint32_t epochDays) {
      uint8_t year, month, day;
      if (epochDays == kInvalidEpochDays) {
        year = month = day = 0;
      } else {
        extractYearMonthDay(epochDays, year, month, day);
      }
      return LocalDate(year, month, day);
    }

    /**
     * Factory method using the number of seconds since AceTime epoch of
     * 2000-01-01. The number of seconds from midnight of the given day is
     * thrown away. If epochSeconds is kInvalidEpochSeconds, isError() will
     * return true.
     *
     * The largest date supported by this method is 2136-02-07, corresponding
     * to 4,294,944,000 seconds. (Actually, up to 4,294,967,294 will also
     * return 2136-02-07 because the partial day is thrown away.)
     *
     * @param epochSeconds number of seconds since AceTime epoch (2000-01-01)
     */
    static LocalDate forEpochSeconds(uint32_t epochSeconds) {
      if (epochSeconds == kInvalidEpochSeconds) {
        return forEpochDays(kInvalidEpochDays);
      } else {
        return forEpochDays(epochSeconds / 86400);
      }
    }

    /**
     * Factory method. Create a LocalDate from the ISO 8601 date string. If
     * the string cannot be parsed, then isError() on the constructed object
     * returns true, but the data validation is very weak. If the date string
     * is greater than 2255-12-31, the result is undefined.
     *
     * @param dateString the date in ISO 8601 format (yyyy-mm-dd)
     */
    static LocalDate forDateString(const char* dateString) {
      return LocalDate().initFromDateString(dateString);
    }

    /** True if year is a leap year. */
    static bool isLeapYear(uint8_t year) {
      // NOTE: The compiler will optimize the (year % 400) expression into
      // (year == 0) since it knows that the max value of yYear is 255.
      return ((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0);
    }

    /** Return the number of days in the current month. */
    static uint8_t daysInMonth(uint8_t year, uint8_t month) {
      uint8_t days = sDaysInMonth[month - 1];
      return (month == 2 && isLeapYear(year)) ? days + 1 : days;
    }

    /** Default constructor does nothing. */
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

    /** Return the full year instead of just the last 2 digits. */
    uint16_t yearFull() const { return mYear + kEpochYear; }

    /** Set the year given the full year. */
    void yearFull(uint16_t yearFull) { mYear = yearFull - kEpochYear; }

    /** Return the 2 digit year from year 2000. */
    uint8_t year() const { return mYear; }

    /** Set the 2 digit year from year 2000. */
    void year(uint8_t year) { mYear = year; }

    /** Return the month with January=1, December=12. */
    uint8_t month() const { return mMonth; }

    /** Set the month. */
    void month(uint8_t month) { mMonth = month; }

    /** Return the day of the month. */
    uint8_t day() const { return mDay; }

    /** Set the day of the month. */
    void day(uint8_t day) { mDay = day; }

    /** Increment the year by one, wrapping from 99 to 0. */
    void incrementYear() {
      common::incrementMod(mYear, (uint8_t) 100);
    }

    /** Increment the month by one, wrapping from 12 to 1. */
    void incrementMonth() {
      common::incrementMod(mMonth, (uint8_t) 12, (uint8_t) 1);
    }

    /** Increment the day by one, wrapping from 31 to 1. */
    void incrementDay() {
      common::incrementMod(mDay, (uint8_t) 31, (uint8_t) 1);
    }

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
     * Returns kInvalidEpochDays if isError() is true, which allows round trip
     * conversions of forEpochDays() and toEpochDays() even when isError() is
     * true.
     *
     * Uses Julian days which normally start at 12:00:00. But this method
     * returns the delta number of days since 00:00:00, so we can interpret the
     * Gregorian calendar day to start at 00:00:00.
     *
     * See https://en.wikipedia.org/wiki/Julian_day
     */
    uint32_t toEpochDays() const {
      if (isError()) return kInvalidEpochDays;

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
     * Return the number of seconds since AceTime epoch (2000-01-01 00:00:00).
     * Returns kInvalidEpochSeconds if isError() is true.
     *
     * The result is undefined if the date is greater than 2136-02-07 because
     * the epoch seconds will overflow the uint32_t.
     */
    uint32_t toEpochSeconds() const {
      if (isError()) return kInvalidEpochSeconds;
      return 86400 * toEpochDays();
    }

    /**
     * Compare this LocalDate to that LocalDate, returning (<0, 0, >0)
     * according to whether the epochSeconds is (this<that, this==this,
     * this>that). If isError() is true, the behavior is undefined.
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

  private:
    friend class OffsetDateTime;
    friend bool operator==(const LocalDate& a, const LocalDate& b);
    friend bool operator!=(const LocalDate& a, const LocalDate& b);

    /** Minimum length of the date string. yyyy-mm-dd. */
    static const uint8_t kDateStringLength = 10;

    /**
     * Day of week table for each month, with 0=Jan to 11=Dec. The table
     * offsets actually start with March, causing the leap year to happen at
     * the end of the "year", which makes the leap year calculation easier.
     */
    static const uint8_t sDayOfWeek[12];

    /** Number of days in each month in a non-leap year. 0=Jan, 11=Dec. */
    static const uint8_t sDaysInMonth[12];

    /** Constructor. */
    explicit LocalDate(uint8_t year, uint8_t month, uint8_t day):
        mYear(year),
        mMonth(month),
        mDay(day) {}

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

    /** Extract the date components from the given dateString. */
    LocalDate& initFromDateString(const char* dateString);

    uint8_t mYear; // [00, 255], year offset from 2000
    uint8_t mMonth; // [1, 12], 0 indicates error
    uint8_t mDay; // [1, 31], 0 indicates error
};

/** Return true if two LocalDate objects are equal in all components. */
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
