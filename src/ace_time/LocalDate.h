/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#ifndef ACE_TIME_LOCAL_DATE_H
#define ACE_TIME_LOCAL_DATE_H

#include <stdint.h>
#include "common/common.h"
#include "LocalTime.h"

class Print;

namespace ace_time {

/**
 * The date (year, month, day) representing the date without regards to time
 * zone.
 *
 * The year field is internally represented as an int8_t offset from the year
 * 2000. However, the value of kInvalidYear is used to indicate an
 * error condition. So the actual range of the year is [1873, 2127] instead of
 * [1872, 2127].
 *
 * If the year is restricted to 2000-2099 (2 digit years), these fields
 * correspond to the range supported by the DS3231 RTC chip.
 *
 * The dayOfWeek (1=Monday, 7=Sunday, per ISO 8601) is calculated from the date
 * fields.
 *
 * Parts of this class were inspired by the java.time.LocalDate class of Java
 * 11
 * (https://docs.oracle.com/en/java/javase/11/docs/api/java.base/java/time/LocalDate.html).
 */
class LocalDate {
  public:
    /** Base year of epoch. */
    static const int16_t kEpochYear = 2000;

    /**
     * Number of seconds from Unix epoch (1970-01-01 00:00:00Z) to
     * the AceTime epoch (2000-01-01 00:00:00Z).
     */
    static const int32_t kSecondsSinceUnixEpoch = 946684800;

    /**
     * Sentinel year which indicates an error condition or sometimes a year
     * that 'does not exist'.
     */
    static const int16_t kInvalidYear = INT16_MIN;

    /**
     * Sentinel year which represents the smallest year, effectively -Infinity.
     */
    static const int16_t kMinYear = 0;

    /**
     * Sentinel year which represents the largest year, effectively +Infinity.
     */
    static const int16_t kMaxYear = 10000;

    /** Sentinel epochDays which indicates an error. */
    static const int32_t kInvalidEpochDays = INT32_MIN;

    /** Sentinel epochSeconds which indicates an error. */
    static const int32_t kInvalidEpochSeconds = INT32_MIN;

    /** Sentinel unixDays which indicates an error. */
    static const int32_t kInvalidUnixDays = INT32_MIN;

    /** Sentinel unixSeconds which indicates an error. */
    static const int32_t kInvalidUnixSeconds = INT32_MIN;

    /** Sentinel 64-bit unixSeconds which indicates an error. */
    static const int64_t kInvalidUnixSeconds64 = INT64_MIN;

    /** Maximum 64-bit Unix seconds supported by acetime_t. */
    static const int64_t kMaxValidUnixSeconds64 =
        (int64_t) (INT32_MAX) + kSecondsSinceUnixEpoch;

    /** Minimum 64-bit Unix seconds supported by acetime_t. */
    static const int64_t kMinValidUnixSeconds64 =
        (int64_t) (INT32_MIN + 1) + kSecondsSinceUnixEpoch;

    /**
     * Number of days from Unix epoch (1970-01-01 00:00:00Z) to
     * the AceTime epoch (2000-01-01 00:00:00Z).
     */
    static const int32_t kDaysSinceUnixEpoch = 10957;

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
     * Factory method using separated year, month and day fields. Returns
     * LocalDate::forError() if the parameters are out of range.
     *
     * @param year [-1,10000]
     * @param month month with January=1, December=12
     * @param day day of month [1-31]
     */
    static LocalDate forComponents(int16_t year, uint8_t month, uint8_t day) {
      int16_t yearInternal = isYearValid(year) ? year : kInvalidYear;
      return LocalDate(yearInternal, month, day);
    }

    /**
     * Factory method using the number of days since AceTime epoch of
     * 2000-01-01. If epochDays is kInvalidEpochDays, isError() will return
     * true.
     *
     * @param epochDays number of days since AceTime epoch (2000-01-01)
     */
    static LocalDate forEpochDays(int32_t epochDays) {
      int16_t year;
      uint8_t month;
      uint8_t day;
      if (epochDays == kInvalidEpochDays) {
        year = month = day = 0;
      } else {
        extractYearMonthDay(epochDays, year, month, day);
      }
      return forComponents(year, month, day);
    }

    /** Factory method using the number of days since Unix epoch 1970-01-1. */
    static LocalDate forUnixDays(int32_t unixDays) {
      if (unixDays == kInvalidEpochDays) {
        return forError();
      } else {
        return forEpochDays(unixDays - kDaysSinceUnixEpoch);
      }
    }

    /**
     * Factory method using the number of seconds since AceTime epoch of
     * 2000-01-01. The number of seconds from midnight of the given day is
     * thrown away. For negative values of epochSeconds, the method performs
     * a floor operation when rounding to the nearest day, in other words
     * towards negative infinity.
     *
     * If epochSeconds is kInvalidEpochSeconds, isError() will return true.
     *
     * @param epochSeconds number of seconds since AceTime epoch (2000-01-01)
     */
    static LocalDate forEpochSeconds(acetime_t epochSeconds) {
      if (epochSeconds == kInvalidEpochSeconds) {
        return forError();
      } else {
          // integer floor-division towards -infinity
          int32_t days = (epochSeconds < 0)
              ? (epochSeconds + 1) / 86400 - 1
              : epochSeconds / 86400;
        return forEpochDays(days);
      }
    }

    /**
     * Factory method that takes the number of seconds since Unix Epoch of
     * 1970-01-01. Similar to forEpochSeconds(), the seconds corresponding to
     * the partial day are truncated down towards the smallest whole day.
     * Valid until unixSeconds reaches the maximum value of `int32_t` at
     * 2038-01-19T03:14:07 UTC.
     */
    static LocalDate forUnixSeconds(int32_t unixSeconds) {
      if (unixSeconds == kInvalidUnixSeconds) {
        return forError();
      } else {
        return forEpochSeconds(unixSeconds - kSecondsSinceUnixEpoch);
      }
    }

    /**
     * Factory method that takes the 64-bit number of seconds since Unix Epoch
     * of 1970-01-01. Similar to forEpochSeconds(), the seconds corresponding to
     * the partial day are truncated down towards the smallest whole day.
     * Valid until the 64-bit unixSeconds reaches the equivalent of
     * 2068-01-19T03:14:07 UTC.
     */
    static LocalDate forUnixSeconds64(int64_t unixSeconds) {
      if (unixSeconds == kInvalidUnixSeconds64
          || unixSeconds > kMaxValidUnixSeconds64
          || unixSeconds < kMinValidUnixSeconds64) {
        return forError();
      } else {
        return forEpochSeconds(
            (acetime_t) (unixSeconds - kSecondsSinceUnixEpoch));
      }
    }

    /**
     * Factory method. Create a LocalDate from the ISO 8601 date string. If the
     * string cannot be parsed, then isError() on the constructed object
     * returns true, but the data validation is very weak. Year should
     * be between 0 and 9999. Created for debugging purposes not for
     * production use.
     *
     * @param dateString the date in ISO 8601 format (yyyy-mm-dd)
     */
    static LocalDate forDateString(const char* dateString);

    /**
     * Variant of forDateString() that updates the pointer to the next
     * unprocessed character. This allows chaining to another
     * forXxxStringChainable() method.
     *
     * This method assumes that the dateString is sufficiently long.
     */
    static LocalDate forDateStringChainable(const char*& dateString);

    /**
     * Factory method that returns a LocalDate which represents an error
     * condition. The isError() method will return true.
     */
    static LocalDate forError() {
      return LocalDate(kInvalidYear, 0, 0);
    }

    /** True if year is a leap year. */
    static bool isLeapYear(int16_t year) {
      return ((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0);
    }

    /** Return true if year is within valid range of [-1, 10000]. */
    static bool isYearValid(int16_t year) {
      return year >= kMinYear && year <= kMaxYear;
    }

    /** Return the number of days in the current month. */
    static uint8_t daysInMonth(int16_t year, uint8_t month) {
      uint8_t days = sDaysInMonth[month - 1];
      return (month == 2 && isLeapYear(year)) ? days + 1 : days;
    }

    /** Default constructor does nothing. */
    explicit LocalDate() {}

    /** Return the year. */
    int16_t year() const { return mYear; }

    /** Set the year. */
    void year(int16_t year) { mYear = year; }

    /** Return the month with January=1, December=12. */
    uint8_t month() const { return mMonth; }

    /** Set the month. */
    void month(uint8_t month) { mMonth = month; }

    /** Return the day of the month. */
    uint8_t day() const { return mDay; }

    /** Set the day of the month. */
    void day(uint8_t day) { mDay = day; }

    /**
     * Calculate the day of week given the (year, month, day). Idea borrowed
     * from https://github.com/evq/utz. No validation of year, month or day is
     * performed. If this is found to be too slow, then consider caching the
     * results.
     */
    uint8_t dayOfWeek() const {
      // The "year" starts in March to shift leap year calculation to end.
      int16_t y = year() - (mMonth < 3);
      int16_t d = y + y/4 - y/100 + y/400 + sDayOfWeek[mMonth-1] + mDay;

      // 2000-1-1 was a Saturday=6, so set the offsets accordingly
      return (d < -1) ? (d + 1) % 7 + 8 : (d + 1) % 7 + 1;
    }

    /** Return true if any component indicates an error condition. */
    bool isError() const {
      return mYear == kInvalidYear
          || mDay < 1 || mDay > 31
          || mMonth < 1 || mMonth > 12;
    }

    /**
     * Return number of days since AceTime epoch (2000-01-01 00:00:00Z).
     * Returns kInvalidEpochDays if isError() is true, which allows round trip
     * conversions of forEpochDays() and toEpochDays() even when isError() is
     * true.
     *
     * In an 8-bit implementation:
     *    * the largest date 2127-12-31 returns 46751
     *    * the smallest date 1872-01-01 returns -46751
     *
     * Uses Julian days which normally start at 12:00:00. But this method
     * returns the delta number of days since 00:00:00, so we can interpret the
     * Gregorian calendar day to start at 00:00:00.
     *
     * See https://en.wikipedia.org/wiki/Julian_day
     */
    int32_t toEpochDays() const {
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
      int16_t yy = year();
      int32_t jdn = ((int32_t) 1461 * (yy + 4800 + mm))/4
          + (367 * (mMonth - 2 - 12 * mm))/12
          - (3 * ((yy + 4900 + mm)/100))/4
          + mDay - 32075;
      return jdn - kDaysSinceJulianEpoch;
    }

    /** Return the number of days since Unix epoch (1970-01-01 00:00:00). */
    int32_t toUnixDays() const {
      if (isError()) return kInvalidUnixDays;
      return toEpochDays() + kDaysSinceUnixEpoch;
    }

    /**
     * Return the number of seconds since AceTime epoch (2000-01-01 00:00:00).
     * Returns kInvalidEpochSeconds if isError() is true. This is a convenience
     * method that returns (86400 * toEpochDays()). Since acetime_t is a
     * 32-bit signed integer, the limits are different:
     *
     *    * the smallest date corresponding to INT32_MIN is 1931-12-13 20:45:52
     *      so this method supports dates as small as 1931-12-14.
     *    * the largest date corresponding to INT32_MAX is 2068-01-19 03:14:07.
     */
    acetime_t toEpochSeconds() const {
      if (isError()) return kInvalidEpochSeconds;
      return 86400 * toEpochDays();
    }

    /**
     * Return the number of seconds since Unix epoch (1970-01-01 00:00:00).
     */
    int32_t toUnixSeconds() const {
      if (isError()) return kInvalidUnixSeconds;
      return 86400 * toUnixDays();
    }

    /**
     * Return the number of seconds since Unix epoch (1970-01-01 00:00:00).
     */
    int64_t toUnixSeconds64() const {
      if (isError()) return kInvalidUnixSeconds64;
      return (int64_t) 86400 * toUnixDays();
    }

    /**
     * Compare 'this' LocalDate to 'that' LocalDate, returning (<0, 0, >0)
     * according to whether 'this' occurs (before, same as, after) 'that'. If
     * either this->isError() or that.isError() is true, the behavior is
     * undefined.
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
     * day of week.
     * This class does not implement the Printable interface to avoid
     * increasing the size of the object from the additional virtual function.
     */
    void printTo(Print& printer) const;

    // Use default copy constructor and assignment operator.
    LocalDate(const LocalDate&) = default;
    LocalDate& operator=(const LocalDate&) = default;

  private:
    friend bool operator==(const LocalDate& a, const LocalDate& b);

    /**
     * Number of days between the Julian calendar epoch (4713 BC 01-01) and the
     * AceTime epoch (2000-01-01).
     */
    static const int32_t kDaysSinceJulianEpoch = 2451545;

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

    /** Constructor that sets the components. */
    explicit LocalDate(int16_t year, uint8_t month, uint8_t day):
        mYear(year),
        mMonth(month),
        mDay(day) {}

    /**
     * Extract the (year, month, day, dayOfWeek) fields from epochDays.
     *
     * See https://en.wikipedia.org/wiki/Julian_day.
     */
    static void extractYearMonthDay(int32_t epochDays, int16_t& year,
        uint8_t& month, uint8_t& day) {
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

    int16_t mYear; // [-1, 10000], INT16_MIN indicates error
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
