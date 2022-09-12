/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#ifndef ACE_TIME_LOCAL_DATE_H
#define ACE_TIME_LOCAL_DATE_H

#include <stdint.h>
#include <string.h> // strlen()
#include <Arduino.h>
#include <AceCommon.h> // printPad2To()
#include "common/common.h"
#include "common/DateStrings.h" // DateStrings
#include "internal/EpochConverterJulian.h"
#include "internal/EpochConverterHinnant.h"

class Print;

namespace ace_time {

/**
 * The date (year, month, day) representing the date without regards to time
 * zone.
 *
 * Normally, the range of the year field is [1,9999]. Occasionally, the year 0
 * is used to indicate -Infinity, with the month and day fields ignored. And the
 * year 10000 is used to indicate +Infinity, with the month and day fields
 * ignored. The value of INT16_MIN (-32768) is used to indicate an error
 * condition.
 *
 * The dayOfWeek (1=Monday, 7=Sunday, per ISO 8601) is calculated from the date
 * fields.
 *
 * Parts of this class were inspired by the java.time.LocalDate class of Java
 * 11
 * (https://docs.oracle.com/en/java/javase/11/docs/api/java.base/java/time/LocalDate.html).
 *
 * @tparam T_CONVERTER class responsible for providing the
 *    T_CONVERTER::toEpochDays() and T_CONVERTER::fromEpochDays() functions
 */
template <typename T_CONVERTER>
class LocalDateTemplate {
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
     * @param year [0,10000]
     * @param month month with January=1, December=12
     * @param day day of month [1-31]
     */
    static LocalDateTemplate forComponents(
        int16_t year, uint8_t month, uint8_t day) {
      year = isYearValid(year) ? year : kInvalidYear;
      return LocalDateTemplate(year, month, day);
    }

    /**
     * Factory method using the number of days since AceTime epoch of
     * 2000-01-01. If epochDays is kInvalidEpochDays, isError() will return
     * true.
     *
     * @param epochDays number of days since AceTime epoch (2000-01-01)
     */
    static LocalDateTemplate forEpochDays(int32_t epochDays) {
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
    static LocalDateTemplate forUnixDays(int32_t unixDays) {
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
    static LocalDateTemplate forEpochSeconds(acetime_t epochSeconds) {
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
    static LocalDateTemplate forUnixSeconds(int32_t unixSeconds) {
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
    static LocalDateTemplate forUnixSeconds64(int64_t unixSeconds) {
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
     * string cannot be parsed, then isError() on the constructed object returns
     * true, but the data validation is very weak. Year should be between 0001
     * and 9999. Created for mostly for debugging purposes not for production
     * use.
     *
     * @param dateString the date in ISO 8601 format (yyyy-mm-dd)
     */
    static LocalDateTemplate forDateString(const char* dateString) {
      if (strlen(dateString) < kDateStringLength) {
        return forError();
      }
      return forDateStringChainable(dateString);
    }

    /**
     * Variant of forDateString() that updates the pointer to the next
     * unprocessed character. This allows chaining to another
     * forXxxStringChainable() method.
     *
     * This method assumes that the dateString is sufficiently long.
     */
    static LocalDateTemplate forDateStringChainable(const char*& dateString) {
      const char* s = dateString;

      // year (assumes 4 digit year)
      int16_t year = (*s++ - '0');
      year = 10 * year + (*s++ - '0');
      year = 10 * year + (*s++ - '0');
      year = 10 * year + (*s++ - '0');

      // '-'
      s++;

      // month
      uint8_t month = (*s++ - '0');
      month = 10 * month + (*s++ - '0');

      // '-'
      s++;

      // day
      uint8_t day = (*s++ - '0');
      day = 10 * day + (*s++ - '0');

      dateString = s;
      return forComponents(year, month, day);
    }

    /**
     * Factory method that returns a LocalDate which represents an error
     * condition. The isError() method will return true.
     */
    static LocalDateTemplate forError() {
      return LocalDateTemplate(kInvalidYear, 0, 0);
    }

    /** True if year is a leap year. */
    static bool isLeapYear(int16_t year) {
      return ((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0);
    }

    /** Return true if year is within valid range of [0,10000]. */
    static bool isYearValid(int16_t year) {
      return year >= kMinYear && year <= kMaxYear;
    }

    /** Return the number of days in the current month. */
    static uint8_t daysInMonth(int16_t year, uint8_t month) {
      uint8_t days = sDaysInMonth[month - 1];
      return (month == 2 && isLeapYear(year)) ? days + 1 : days;
    }

    /** Default constructor does nothing. */
    explicit LocalDateTemplate() = default;

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
     */
    int32_t toEpochDays() const {
      if (isError()) return kInvalidEpochDays;
      return T_CONVERTER::toEpochDays(mYear, mMonth, mDay);
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
    int8_t compareTo(const LocalDateTemplate& that) const {
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
    void printTo(Print& printer) const {
      if (isError()) {
        printer.print(F("<Invalid LocalDate>"));
        return;
      }

      // Date
      using ace_common::printPad2To;
      printer.print(year());
      printer.print('-');
      printPad2To(printer, mMonth, '0');
      printer.print('-');
      printPad2To(printer, mDay, '0');
      printer.print(' ');

      // Week day
      DateStrings ds;
      printer.print(ds.dayOfWeekLongString(dayOfWeek()));
    }

    // Use default copy constructor and assignment operator.
    LocalDateTemplate(const LocalDateTemplate&) = default;
    LocalDateTemplate& operator=(const LocalDateTemplate&) = default;

  private:
    template <typename T>
    friend bool operator==(
        const LocalDateTemplate<T>& a, const LocalDateTemplate<T>& b);

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
    explicit LocalDateTemplate(int16_t year, uint8_t month, uint8_t day):
        mYear(year),
        mMonth(month),
        mDay(day) {}

    /**
     * Extract the (year, month, day, dayOfWeek) fields from epochDays.
     */
    static void extractYearMonthDay(int32_t epochDays, int16_t& year,
        uint8_t& month, uint8_t& day) {
      T_CONVERTER::fromEpochDays(epochDays, year, month, day);
    }

    int16_t mYear; // [0,10000], INT16_MIN indicates error
    uint8_t mMonth; // [1, 12], 0 indicates error
    uint8_t mDay; // [1, 31], 0 indicates error
};

/** Return true if two LocalDate objects are equal in all components. */
template <typename T>
bool operator==(const LocalDateTemplate<T>& a, const LocalDateTemplate<T>& b) {
  return a.mDay == b.mDay
      && a.mMonth == b.mMonth
      && a.mYear == b.mYear;
}

/** Return true if two LocalDate objects are not equal. */
template <typename T>
bool operator!=(const LocalDateTemplate<T>& a, const LocalDateTemplate<T>& b) {
  return ! (a == b);
}

// Using 0=Jan offset.
template <typename T>
const uint8_t LocalDateTemplate<T>::sDayOfWeek[12] = {
  5 /*Jan=31*/,
  1 /*Feb=28*/,
  0 /*Mar=31, start of "year"*/,
  3 /*Apr=30*/,
  5 /*May=31*/,
  1 /*Jun=30*/,
  3 /*Jul=31*/,
  6 /*Aug=31*/,
  2 /*Sep=30*/,
  4 /*Oct=31*/,
  0 /*Nov=30*/,
  2 /*Dec=31*/,
};

// Using 0=Jan offset.
template <typename T>
const uint8_t LocalDateTemplate<T>::sDaysInMonth[12] = {
  31 /*Jan=31*/,
  28 /*Feb=28*/,
  31 /*Mar=31*/,
  30 /*Apr=30*/,
  31 /*May=31*/,
  30 /*Jun=30*/,
  31 /*Jul=31*/,
  31 /*Aug=31*/,
  30 /*Sep=30*/,
  31 /*Oct=31*/,
  30 /*Nov=30*/,
  31 /*Dec=31*/,
};

// Use the original EpochConverterJulian until the other converters are
// validated.
using LocalDate = LocalDateTemplate<internal::EpochConverterHinnant>;

}

#endif
