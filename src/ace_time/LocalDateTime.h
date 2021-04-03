/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#ifndef ACE_TIME_LOCAL_DATE_TIME_H
#define ACE_TIME_LOCAL_DATE_TIME_H

#include <stddef.h> // size_t
#include <stdint.h> // uint8_t, etc
#include <string.h> // strlen()
#include <Arduino.h> // strncpy_P()
#include "LocalDate.h"
#include "LocalTime.h"

class Print;
class __FlashStringHelper;

namespace ace_time {

/**
 * Class that holds the date-time as the components (year, month, day, hour,
 * minute, second) without regards to the time zone. It is an aggregation of
 * the LocalDate and LocalTime classes.
 *
 * Parts of this class were inspired by the java.time.LocalDateTime class of
 * Java 11
 * (https://docs.oracle.com/en/java/javase/11/docs/api/java.base/java/time/LocalDateTime.html).
 */
class LocalDateTime {
  public:

    /**
     * Factory method using separated date and time components.
     *
     * @param year [1873-2127]
     * @param month month with January=1, December=12
     * @param day day of month [1-31]
     * @param hour hour [0-23]
     * @param minute minute [0-59]
     * @param second second [0-59], does not support leap seconds
     */
    static LocalDateTime forComponents(int16_t year, uint8_t month,
        uint8_t day, uint8_t hour, uint8_t minute, uint8_t second) {
      int8_t yearTiny = LocalDate::isYearValid(year)
          ? year - LocalDate::kEpochYear
          : LocalDate::kInvalidYearTiny;
      return forTinyComponents(yearTiny, month, day, hour, minute, second);
    }

    /** Factory method using components with an int8_t yearTiny. */
    static LocalDateTime forTinyComponents(int8_t yearTiny, uint8_t month,
        uint8_t day, uint8_t hour, uint8_t minute, uint8_t second) {
      return LocalDateTime(
          LocalDate::forTinyComponents(yearTiny, month, day),
          LocalTime::forComponents(hour, minute, second));
    }

    /**
     * Factory method. Create the various components of the LocalDateTime from
     * the epochSeconds.
     *
     * Returns LocalDateTime::forError() if epochSeconds is equal to
     * LocalDate::kInvalidEpochSeconds.
     *
     * @param epochSeconds Number of seconds from AceTime epoch
     *    (2000-01-01 00:00:00). Use LocalDate::kInvalidEpochSeconds to define
     *    an invalid instance whose isError() returns true.
     */
    static LocalDateTime forEpochSeconds(acetime_t epochSeconds) {
      LocalDate ld;
      LocalTime lt;
      if (epochSeconds == LocalDate::kInvalidEpochSeconds) {
        ld = LocalDate::forError();
        lt = LocalTime::forError();
      } else {
        // Integer floor-division towards -infinity
        acetime_t days = (epochSeconds < 0)
            ? (epochSeconds + 1) / 86400 - 1
            : epochSeconds / 86400;

        // Avoid % operator, because it's slow on an 8-bit process and because
        // epochSeconds could be negative.
        acetime_t seconds = epochSeconds - 86400 * days;
        ld = LocalDate::forEpochDays(days);
        lt = LocalTime::forSeconds(seconds);
      }

      return LocalDateTime(ld, lt);
    }

    /**
     * Factory method that takes the number of seconds since Unix Epoch of
     * 1970-01-01.
     *
     * Returns LocalDateTime::forError() if epochSeconds is equal to
     * LocalDate::kInvalidEpochSeconds.
     */
    static LocalDateTime forUnixSeconds(acetime_t unixSeconds) {
      acetime_t epochSeconds = (unixSeconds == LocalDate::kInvalidEpochSeconds)
          ? unixSeconds
          : unixSeconds - LocalDate::kSecondsSinceUnixEpoch;
      return forEpochSeconds(epochSeconds);
    }

    /**
     * Factory method. Create a LocalDateTime from the ISO 8601 date string. If
     * the string cannot be parsed, then returns LocalDateTime::forError().
     *
     * The dateString is expected to be in ISO 8601 format
     * "YYYY-MM-DDThh:mm:ss", but currently, the parser is very lenient.
     * It cares mostly about the positional placement of the various
     * components. It does not validate the separation characters like '-' or
     * ':'. For example, both of the following will parse to the exactly same
     * LocalDateTime object: "2018-08-31T13:48:01-07:00" "2018/08/31
     * 13#48#01-07#00"
     *
     * The parsing validation is so weak that the behavior is undefined for
     * most invalid date/time strings. The range of valid dates is roughly from
     * 1872-01-01T00:00:00 to 2127-12-31T23:59:59.
     */
    static LocalDateTime forDateString(const char* dateString);

    /**
     * Variant of forDateString() that updates the pointer to the next
     * unprocessed character. This allows chaining to another
     * forXxxStringChainable() method.
     *
     * This method assumes that the dateString is sufficiently long.
     */
    static LocalDateTime forDateStringChainable(const char*& dateString);

    /**
     * Factory method. Create a LocalDateTime from date string in flash memory
     * F() strings. Mostly for unit testing.
     */
    static LocalDateTime forDateString(const __FlashStringHelper* dateString) {
      // Copy the F() string into a buffer. Use strncpy_P() because ESP32 and
      // ESP8266 do not have strlcpy_P(). We need +1 for the '\0' character and
      // another +1 to determine if the dateString is too long to fit.
      char buffer[kDateTimeStringLength + 2];
      strncpy_P(buffer, (const char*) dateString, sizeof(buffer));
      buffer[kDateTimeStringLength + 1] = 0;

      // check if the original F() was too long
      size_t len = strlen(buffer);
      if (len > kDateTimeStringLength) {
        return forError();
      }

      return forDateString(buffer);
    }

    /** Factory method that returns an instance where isError() returns true. */
    static LocalDateTime forError() {
      return LocalDateTime(LocalDate::forError(), LocalTime::forError());
    }

    /** Constructor. All internal fields are left in an undefined state. */
    explicit LocalDateTime() {}

    /** Return true if any component indicates an error condition. */
    bool isError() const {
      return mLocalDate.isError() || mLocalTime.isError();
    }

    /** Return the year. */
    int16_t year() const { return mLocalDate.year(); }

    /** Set the year. */
    void year(int16_t year) { mLocalDate.year(year); }

    /**
     * Return the single-byte year offset from year 2000. Intended for memory
     * constrained or performance critical code. May be deprecated in the
     * future.
     */
    int8_t yearTiny() const { return mLocalDate.yearTiny(); }

    /**
     * Set the single-byte year offset from year 2000. Intended for memory
     * constrained or performance critical code. May be deprecated in the
     * future.
     */
    void yearTiny(int8_t yearTiny) { mLocalDate.yearTiny(yearTiny); }

    /** Return the month with January=1, December=12. */
    uint8_t month() const { return mLocalDate.month(); }

    /** Set the month. */
    void month(uint8_t month) { mLocalDate.month(month); }

    /** Return the day of the month. */
    uint8_t day() const { return mLocalDate.day(); }

    /** Set the day of the month. */
    void day(uint8_t day) { mLocalDate.day(day); }

    /** Return the hour. */
    uint8_t hour() const { return mLocalTime.hour(); }

    /** Set the hour. */
    void hour(uint8_t hour) { mLocalTime.hour(hour); }

    /** Return the minute. */
    uint8_t minute() const { return mLocalTime.minute(); }

    /** Set the minute. */
    void minute(uint8_t minute) { mLocalTime.minute(minute); }

    /** Return the second. */
    uint8_t second() const { return mLocalTime.second(); }

    /** Set the second. */
    void second(uint8_t second) { mLocalTime.second(second); }

    /** Return the day of the week, Monday=1, Sunday=7 (per ISO 8601). */
    uint8_t dayOfWeek() const { return mLocalDate.dayOfWeek(); }

    /** Return the LocalDate. */
    const LocalDate& localDate() const { return mLocalDate; }

    /** Return the LocalTime. */
    const LocalTime& localTime() const { return mLocalTime; }

    /**
     * Return number of whole days since AceTime epoch (2000-01-01 00:00:00Z).
     */
    acetime_t toEpochDays() const {
      if (isError()) return LocalDate::kInvalidEpochDays;
      return mLocalDate.toEpochDays();
    }

    /** Return the number of days since Unix epoch (1970-01-01 00:00:00). */
    acetime_t toUnixDays() const {
      if (isError()) return LocalDate::kInvalidEpochDays;
      return toEpochDays() + LocalDate::kDaysSinceUnixEpoch;
    }

    /**
     * Return seconds since AceTime epoch 2000-01-01 00:00:00Z, after assuming
     * that the date and time components are in UTC timezone. Returns
     * LocalDate::kInvalidEpochSeconds if isError() is true.
     */
    acetime_t toEpochSeconds() const {
      if (isError()) return LocalDate::kInvalidEpochSeconds;

      acetime_t days = mLocalDate.toEpochDays();
      acetime_t seconds = mLocalTime.toSeconds();
      return days * 86400 + seconds;
    }

    /**
     * Return seconds from Unix epoch 1970-01-01 00:00:00Z, after assuming that
     * the date and time components are in UTC timezone. Returns
     * LocalDate::kInvalidEpochSeconds if isError() is true.
     *
     * Tip: You can use the command 'date +%s -d {iso8601date}' on a Unix box
     * to print the unix seconds of a given ISO8601 date.
     */
    acetime_t toUnixSeconds() const {
      if (isError()) return LocalDate::kInvalidEpochSeconds;
      return toEpochSeconds() + LocalDate::kSecondsSinceUnixEpoch;
    }

    /**
     * Compare 'this' LocalDateTime with 'that' LocalDateTime, and return (<0,
     * 0, >0) according to whether 'this' occurs (before, same as, after)
     * 'that'. If either this->isError() or that.isError() is true, the
     * behavior is undefined.
     */
    int8_t compareTo(const LocalDateTime& that) const {
      int8_t dateCompare = localDate().compareTo(that.localDate());
      if (dateCompare != 0) return dateCompare;
      int8_t timeCompare = localTime().compareTo(that.localTime());
      if (timeCompare != 0) return timeCompare;
      return 0;
    }

    /**
     * Print LocalDateTime to 'printer' in ISO 8601 format.
     * This class does not implement the Printable interface to avoid
     * increasing the size of the object from the additional virtual function.
     */
    void printTo(Print& printer) const;

    // Use default copy constructor and assignment operator.
    LocalDateTime(const LocalDateTime&) = default;
    LocalDateTime& operator=(const LocalDateTime&) = default;

  private:
    friend bool operator==(const LocalDateTime& a, const LocalDateTime& b);

    /** Expected length of an ISO 8601 date string. */
    static const uint8_t kDateTimeStringLength = 19;

    /** Constructor from a LocalDate and LocalTime. */
    explicit LocalDateTime(const LocalDate& ld, const LocalTime& lt):
        mLocalDate(ld),
        mLocalTime(lt) {}

    LocalDate mLocalDate;
    LocalTime mLocalTime;
};

/**
 * Return true if two LocalDateTime objects are equal in all components.
 * Optimized for small changes in the less signficant fields, such as 'second'
 * or 'minute'.
 */
inline bool operator==(const LocalDateTime& a, const LocalDateTime& b) {
  return a.mLocalDate == b.mLocalDate
      && a.mLocalTime == b.mLocalTime;
}

/** Return true if two LocalDateTime objects are not equal. */
inline bool operator!=(const LocalDateTime& a, const LocalDateTime& b) {
  return ! (a == b);
}

}

#endif
