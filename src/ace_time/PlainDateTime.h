/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#ifndef ACE_TIME_PLAIN_DATE_TIME_H
#define ACE_TIME_PLAIN_DATE_TIME_H

#include <stddef.h> // size_t
#include <stdint.h> // uint8_t, etc
#include <string.h> // strlen()
#include "PlainDate.h"
#include "PlainTime.h"

class Print;
class __FlashStringHelper;

namespace ace_time {

/**
 * Class that holds the date-time as the components (year, month, day, hour,
 * minute, second) without regards to the time zone. It is an aggregation of
 * the PlainDate and PlainTime classes.
 *
 * Parts of this class were inspired by the java.time.PlainDateTime class of
 * Java 11
 * (https://docs.oracle.com/en/java/javase/11/docs/api/java.base/java/time/PlainDateTime.html).
 */
class PlainDateTime {
  public:

    /**
     * Factory method using separated date and time components.
     *
     * @param year year [0,10000]
     * @param month month with January=1, December=12
     * @param day day of month [1-31]
     * @param hour hour [0-23]
     * @param minute minute [0-59]
     * @param second second [0-59], does not support leap seconds
     */
    static PlainDateTime forComponents(int16_t year, uint8_t month,
        uint8_t day, uint8_t hour, uint8_t minute, uint8_t second) {
      PlainDate pd = PlainDate::forComponents(year, month, day);
      PlainTime pt = PlainTime::forComponents(hour, minute, second);
      return PlainDateTime(pd, pt);
    }

    /**
     * Factory method. Create the various components of the PlainDateTime from
     * the epochSeconds.
     *
     * Returns PlainDateTime::forError() if epochSeconds is equal to
     * PlainDate::kInvalidEpochSeconds.
     *
     * @param epochSeconds Number of seconds from AceTime epoch
     *    (2000-01-01 00:00:00). Use PlainDate::kInvalidEpochSeconds to define
     *    an invalid instance whose isError() returns true.
     */
    static PlainDateTime forEpochSeconds(acetime_t epochSeconds) {
      if (epochSeconds == PlainDate::kInvalidEpochSeconds) {
        return forError();
      }

      // Integer floor-division towards -infinity
      int32_t days = (epochSeconds < 0)
          ? (epochSeconds + 1) / 86400 - 1
          : epochSeconds / 86400;

      // Avoid % operator, because it's slow on an 8-bit process and because
      // epochSeconds could be negative.
      int32_t seconds = epochSeconds - 86400 * days;

      PlainDate pd = PlainDate::forEpochDays(days);
      PlainTime pt = PlainTime::forSeconds(seconds);
      return PlainDateTime(pd, pt);
    }

    /**
     * Factory method that takes the 64-bit number of seconds since Unix Epoch
     * of 1970-01-01.
     * Valid until the 64-bit unixSeconds reaches the equivalent of
     * 2068-01-19T03:14:07 UTC.
     * Returns PlainDateTime::forError() if unixSeconds is invalid.
     */
    static PlainDateTime forUnixSeconds64(int64_t unixSeconds) {
      if (unixSeconds == PlainDate::kInvalidUnixSeconds64) {
        return forError();
      }

      int64_t epochSeconds64 = unixSeconds
          - Epoch::secondsToCurrentEpochFromUnixEpoch64();

      // Integer floor-division towards -infinity
      int32_t days = (epochSeconds64 < 0)
          ? (epochSeconds64 + 1) / 86400 - 1
          : epochSeconds64 / 86400;
      int32_t seconds = epochSeconds64 - (int64_t) 86400 * days;

      PlainDate pd = PlainDate::forEpochDays(days);
      PlainTime pt = PlainTime::forSeconds(seconds);
      return PlainDateTime(pd, pt);
    }

    /**
     * Factory method. Create a PlainDateTime from the ISO 8601 date string. If
     * the string cannot be parsed, then returns PlainDateTime::forError().
     *
     * The parsing validation is so weak that the behavior is undefined for most
     * invalid date/time strings. It cares mostly about the positional placement
     * of the various components. It does not validate the separation characters
     * like '-' or ':'. For example, both of the following strings will parse to
     * the exactly same PlainDateTime object: "2018-08-31T13:48:01" and
     * "2018/08/31 13.48.01"
     *
     * @param dateString the date and time in ISO 8601 format
     *        "YYYY-MM-DDThh:mm:ss". The range of valid dates is from
     *        0001-01-01T00:00:00 to 9999-12-31T23:59:59.
     */
    static PlainDateTime forDateString(const char* dateString);

    /**
     * Factory method. Create a PlainDateTime from date string in flash memory
     * F() strings. Mostly for unit testing.
     */
    static PlainDateTime forDateString(const __FlashStringHelper* dateString);

    /**
     * Variant of forDateString() that updates the pointer to the next
     * unprocessed character. This allows chaining to another
     * forXxxStringChainable() method.
     *
     * This method assumes that the dateString is sufficiently long.
     */
    static PlainDateTime forDateStringChainable(const char*& dateString);

    /** Factory method that returns an instance where isError() returns true. */
    static PlainDateTime forError() {
      return PlainDateTime(PlainDate::forError(), PlainTime::forError());
    }

    /** Constructor. All internal fields are left in an undefined state. */
    explicit PlainDateTime() {}

    /** Return true if any component indicates an error condition. */
    bool isError() const {
      return mPlainDate.isError() || mPlainTime.isError();
    }

    /** Return the year. */
    int16_t year() const { return mPlainDate.year(); }

    /** Set the year. */
    void year(int16_t year) { mPlainDate.year(year); }

    /** Return the month with January=1, December=12. */
    uint8_t month() const { return mPlainDate.month(); }

    /** Set the month. */
    void month(uint8_t month) { mPlainDate.month(month); }

    /** Return the day of the month. */
    uint8_t day() const { return mPlainDate.day(); }

    /** Set the day of the month. */
    void day(uint8_t day) { mPlainDate.day(day); }

    /** Return the hour. */
    uint8_t hour() const { return mPlainTime.hour(); }

    /** Set the hour. */
    void hour(uint8_t hour) { mPlainTime.hour(hour); }

    /** Return the minute. */
    uint8_t minute() const { return mPlainTime.minute(); }

    /** Set the minute. */
    void minute(uint8_t minute) { mPlainTime.minute(minute); }

    /** Return the second. */
    uint8_t second() const { return mPlainTime.second(); }

    /** Set the second. */
    void second(uint8_t second) { mPlainTime.second(second); }

    /** Return the resolved. */
    Resolved resolved() const { return mPlainTime.resolved(); }

    /** Set the resolved. */
    void resolved(Resolved resolved) { mPlainTime.resolved(resolved); }

    /** Return the day of the week, Monday=1, Sunday=7 (per ISO 8601). */
    uint8_t dayOfWeek() const { return mPlainDate.dayOfWeek(); }

    /** Return the PlainDate. */
    const PlainDate& plainDate() const { return mPlainDate; }

    /** Return the PlainTime. */
    const PlainTime& plainTime() const { return mPlainTime; }

    /** Return the PlainDate. */
    ACE_TIME_DEPRECATED
    const PlainDate& localDate() const { return plainDate(); }

    /** Return the PlainTime. */
    ACE_TIME_DEPRECATED
    const PlainTime& localTime() const { return plainTime(); }

    /**
     * Return number of whole days since AceTime epoch. The default epoch is
     * 2000-01-01 00:00:00 UTC, but can be changed using
     * `Epoch::currentEpochYear()`.
     */
    int32_t toEpochDays() const {
      if (isError()) return PlainDate::kInvalidEpochDays;
      return mPlainDate.toEpochDays();
    }

    /** Return the number of days since Unix epoch (1970-01-01 00:00:00). */
    int32_t toUnixDays() const {
      if (isError()) return PlainDate::kInvalidEpochDays;
      return toEpochDays() + Epoch::daysToCurrentEpochFromUnixEpoch();
    }

    /**
     * Return seconds since the current AceTime epoch defined by
     * Epoch::currentEpochYear(). The default epoch is 2000-01-01 00:00:00
     * UTC, but can be changed using `Epoch::currentEpochYear()`.
     *
     * Returns PlainDate::kInvalidEpochSeconds if isError() is true, or the
     * epochSeconds is out of range.
     */
    acetime_t toEpochSeconds() const {
      if (isError()) return PlainDate::kInvalidEpochSeconds;
      int32_t days = mPlainDate.toEpochDays();
      int32_t seconds = mPlainTime.toSeconds();
      return (int32_t) 86400 * days + seconds;
    }

    /**
     * Return 64-bit seconds from Unix epoch 1970-01-01 00:00:00 UTC, after
     * assuming that the date and time components are in UTC timezone. Returns
     * PlainDate::kInvalidUnixSeconds64 if isError() is true.
     *
     * Tip: You can use the command 'date +%s -d {iso8601date}' on a Unix box
     * to print the unix seconds of a given ISO8601 date.
     */
    int64_t toUnixSeconds64() const {
      if (isError()) return PlainDate::kInvalidUnixSeconds64;
      int32_t days = toUnixDays();
      int32_t seconds = mPlainTime.toSeconds();
      return (int64_t) 86400 * days + seconds;
    }

    /**
     * Compare 'this' PlainDateTime with 'that' PlainDateTime, and return (<0,
     * 0, >0) according to whether 'this' occurs (before, same as, after)
     * 'that'. If either this->isError() or that.isError() is true, the
     * behavior is undefined.
     */
    int8_t compareTo(const PlainDateTime& that) const {
      int8_t dateCompare = plainDate().compareTo(that.plainDate());
      if (dateCompare != 0) return dateCompare;
      int8_t timeCompare = plainTime().compareTo(that.plainTime());
      if (timeCompare != 0) return timeCompare;
      return 0;
    }

    /**
     * Print PlainDateTime to 'printer' in ISO 8601 format.
     * This class does not implement the Printable interface to avoid
     * increasing the size of the object from the additional virtual function.
     */
    void printTo(Print& printer) const;

    // Use default copy constructor and assignment operator.
    PlainDateTime(const PlainDateTime&) = default;
    PlainDateTime& operator=(const PlainDateTime&) = default;

  private:
    friend bool operator==(const PlainDateTime& a, const PlainDateTime& b);

    /** Expected length of an ISO 8601 date string. */
    static const uint8_t kDateTimeStringLength = 19;

    /** Constructor from a PlainDate and PlainTime. */
    explicit PlainDateTime(const PlainDate& pd, const PlainTime& pt):
        mPlainDate(pd),
        mPlainTime(pt) {}

    PlainDate mPlainDate;
    PlainTime mPlainTime;
};

/**
 * Return true if two PlainDateTime objects are equal in all components.
 * Optimized for small changes in the less signficant fields, such as 'second'
 * or 'minute'.
 */
inline bool operator==(const PlainDateTime& a, const PlainDateTime& b) {
  return a.mPlainDate == b.mPlainDate
      && a.mPlainTime == b.mPlainTime;
}

/** Return true if two PlainDateTime objects are not equal. */
inline bool operator!=(const PlainDateTime& a, const PlainDateTime& b) {
  return ! (a == b);
}

}

#endif
