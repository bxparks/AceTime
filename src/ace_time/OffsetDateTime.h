/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#ifndef ACE_TIME_OFFSET_DATE_TIME_H
#define ACE_TIME_OFFSET_DATE_TIME_H

#include <stdint.h>
#include "TimeOffset.h"
#include "LocalDateTime.h"

class Print;

namespace ace_time {

/**
 * The date (year, month, day), time (hour, minute, second) and
 * fixed offset from UTC (timeOffset).
 *
 * The year field is valid from [0, 10000] with year 0 interpreted as -Infinity
 * and year 10000 interpreted as +Infinity. An invalid year is represented by
 * INT16_MIN (-32768). If the year is restricted to the range
 * 2000-2099, then the last 2 digits map directly to the fields supported by the
 * DS3231 RTC chip.
 *
 * The default epoch for AceTime is 2050-01-01T00:00:00 UTC, but can be changed
 * using `Epoch::currentEpochYear()`. The `toEpochSeconds()` method returns
 * a `int32_t` number of seconds offset from that epoch.
 *
 * The dayOfWeek (1=Monday, 7=Sunday, per ISO 8601) is calculated internally
 * from the date fields.
 *
 * Parts of this class were inspired by the java.time.OffsetDateTime class of
 * Java 11, and the datetime package of Python 3.
 */
class OffsetDateTime {
  public:

    /** Factory method from LocalDateTime and TimeOffset. */
    static OffsetDateTime forLocalDateTimeAndOffset(
        const LocalDateTime& localDateTime, TimeOffset timeOffset) {
      return OffsetDateTime(localDateTime, timeOffset);
    }

    /**
     * Factory method using separated date, time, and UTC offset fields.
     *
     * @param year year [0,10000]
     * @param month month with January=1, December=12
     * @param day day of month [1-31]
     * @param hour hour [0-23]
     * @param minute minute [0-59]
     * @param second second [0-59], does not support leap seconds
     * @param timeOffset the time offset from UTC. Using TimeOffset in the last
     * component (instead of an int8_t or int16_t) allows us to overload an
     * additional constructor that accepts a millisecond component in the
     * future.
     * @param fold optional disambiguation of multiple occurences [0, 1]
     */
    static OffsetDateTime forComponents(int16_t year, uint8_t month,
        uint8_t day, uint8_t hour, uint8_t minute, uint8_t second,
        TimeOffset timeOffset, uint8_t fold = 0) {
      auto ldt = LocalDateTime::forComponents(
          year, month, day, hour, minute, second, fold);
      return OffsetDateTime(ldt, timeOffset);
    }

    /**
     * Factory method. Create the various components of the OffsetDateTime from
     * the epochSeconds and its TimeOffset. Returns OffsetDateTime::forError()
     * if epochSeconds or timeOffset is an error.
     *
     * @param epochSeconds Number of seconds from AceTime epoch
     *    (2050-01-01 00:00:00 by default). Use LocalDate::kInvalidEpochSeconds
     *    to define an invalid instance whose isError() returns true.
     * @param timeOffset time offset from UTC
     */
    static OffsetDateTime forEpochSeconds(acetime_t epochSeconds,
          TimeOffset timeOffset, uint8_t fold = 0) {
      if (epochSeconds != LocalDate::kInvalidEpochSeconds) {
        epochSeconds += timeOffset.toSeconds();
      }
      auto ldt = LocalDateTime::forEpochSeconds(epochSeconds, fold);
      return OffsetDateTime(ldt, timeOffset);
    }

    /**
     * Factory method that takes the number of seconds (64-bit) since Unix Epoch
     * of 1970-01-01. Similar to forEpochSeconds(), the seconds corresponding to
     * the partial day are truncated down towards the smallest whole day.
     * Returns OffsetDateTime::forError() if unixSeconds is invalid.
     *
     * @param unixSeconds number of seconds since Unix epoch
     *    (1970-01-01T00:00:00 UTC)
     * @param timeOffset time offset from UTC
     */
    static OffsetDateTime forUnixSeconds64(
        int64_t unixSeconds, TimeOffset timeOffset, int8_t fold = 0) {
      if (unixSeconds != LocalDate::kInvalidUnixSeconds64) {
        unixSeconds += timeOffset.toSeconds();
      }
      auto ldt = LocalDateTime::forUnixSeconds64(unixSeconds, fold);
      return OffsetDateTime(ldt, timeOffset);
    }

    /**
     * Factory method. Create a OffsetDateTime from the ISO 8601 date string. If
     * the string cannot be parsed, then returns OffsetDateTime::forError().
     * Created for debugging purposes not for production use.
     *
     * The parsing validation is so weak that the behavior is undefined for most
     * invalid date/time strings. It cares mostly about the positional placement
     * of the various components. It does not validate the separation characters
     * like '-' or ':'. For example, both of the following will parse to the
     * exactly same OffsetDateTime object: "2018-08-31T13:48:01-07:00"
     * "2018/08/31 13.48.01-07.00"
     *
     * @param dateString the date and time in ISO 8601 format
     *        "YYYY-MM-DDThh:mm:ss+/-hh:mm". The range of valid dates is from
     *        0001-01-01T00:00:00 to 9999-12-31T23:59:59.
     */
    static OffsetDateTime forDateString(const char* dateString);

    /**
     * Factory method. Create a OffsetDateTime from date string in flash memory
     * F() strings. Mostly for unit testing. Returns OffsetDateTime::forError()
     * if a parsing error occurs.
     */
    static OffsetDateTime forDateString(const __FlashStringHelper* dateString);

    /**
     * Variant of forDateString() that updates the pointer to the next
     * unprocessed character. This allows chaining to another
     * forXxxStringChainable() method.
     *
     * This method assumes that the dateString is sufficiently long.
     */
    static OffsetDateTime forDateStringChainable(const char*& dateString);

    /** Factory method that returns an instance whose isError() is true. */
    static OffsetDateTime forError() {
      return OffsetDateTime(LocalDateTime::forError(), TimeOffset::forError());
    }

    /** Constructor. All internal fields are left in an undefined state. */
    explicit OffsetDateTime() {}

    /** Return true if any component indicates an error condition. */
    bool isError() const {
      // Check mTimeOffset first because it's expected to be invalid more often.
      return  mTimeOffset.isError() || mLocalDateTime.isError();
    }

    /** Return the year. */
    int16_t year() const { return mLocalDateTime.year(); }

    /** Set the year. */
    void year(int16_t year) { mLocalDateTime.year(year); }

    /** Return the month with January=1, December=12. */
    uint8_t month() const { return mLocalDateTime.month(); }

    /** Set the month. */
    void month(uint8_t month) { mLocalDateTime.month(month); }

    /** Return the day of the month. */
    uint8_t day() const { return mLocalDateTime.day(); }

    /** Set the day of the month. */
    void day(uint8_t day) { mLocalDateTime.day(day); }

    /** Return the hour. */
    uint8_t hour() const { return mLocalDateTime.hour(); }

    /** Set the hour. */
    void hour(uint8_t hour) { mLocalDateTime.hour(hour); }

    /** Return the minute. */
    uint8_t minute() const { return mLocalDateTime.minute(); }

    /** Set the minute. */
    void minute(uint8_t minute) { mLocalDateTime.minute(minute); }

    /** Return the second. */
    uint8_t second() const { return mLocalDateTime.second(); }

    /** Set the second. */
    void second(uint8_t second) { mLocalDateTime.second(second); }

    /** Return the fold. */
    uint8_t fold() const { return mLocalDateTime.fold(); }

    /** Set the fold. */
    void fold(uint8_t fold) { mLocalDateTime.fold(fold); }

    /** Return the day of the week, Monday=1, Sunday=7 (per ISO 8601). */
    uint8_t dayOfWeek() const { return mLocalDateTime.dayOfWeek(); }

    /** Return the UTC offset of the OffsetDateTime. */
    TimeOffset timeOffset() const { return mTimeOffset; }

    /** Set the UTC offset. */
    void timeOffset(TimeOffset timeOffset) { mTimeOffset = timeOffset; }

    /** Return the LocalDateTime. */
    const LocalDateTime& localDateTime() const { return mLocalDateTime; }

    /** Return the LocalDate. */
    const LocalDate& localDate() const { return mLocalDateTime.localDate(); }

    /** Return the LocalTime. */
    const LocalTime& localTime() const { return mLocalDateTime.localTime(); }

    /**
     * Create a OffsetDateTime in a different UTC offset code (with the same
     * epochSeconds).
     *
     * Calls forEpochSeconds() so subject to its overflow/underflow limits.
     */
    OffsetDateTime convertToTimeOffset(TimeOffset timeOffset) const {
      acetime_t epochSeconds = toEpochSeconds();
      return OffsetDateTime::forEpochSeconds(epochSeconds, timeOffset);
    }

    /**
     * Return number of whole days since AceTime epoch taking into account the
     * UTC offset. The default epoch is 2050-01-01 00:00:00 UTC but can be
     * changed using `Epoch::currentEpochYear()`.
     */
    int32_t toEpochDays() const {
      if (isError()) return LocalDate::kInvalidEpochDays;

      int32_t epochDays = mLocalDateTime.localDate().toEpochDays();

      // Increment or decrement the day count depending on the time offset.
      acetime_t timeOffset = mLocalDateTime.localTime().toSeconds()
          - mTimeOffset.toSeconds();
      if (timeOffset >= 86400) {
        epochDays++;
      } else if (timeOffset < 0) {
        epochDays--;
      }

      return epochDays;
    }

    /** Return the number of days since Unix epoch (1970-01-01 00:00:00). */
    int32_t toUnixDays() const {
      if (isError()) return LocalDate::kInvalidEpochDays;
      return toEpochDays() + Epoch::daysToCurrentEpochFromUnixEpoch();
    }

    /**
     * Return seconds since AceTime epoch taking into account the UTC offset.
     * The default epoch is 2050-01-01 00:00:00 UTC but can be changed using
     * `Epoch::currentEpochYear()`.
     */
    acetime_t toEpochSeconds() const {
      if (isError()) return LocalDate::kInvalidEpochSeconds;
      acetime_t epochSeconds = mLocalDateTime.toEpochSeconds();
      if (epochSeconds == LocalDate::kInvalidEpochSeconds) {
        return epochSeconds;
      }
      return epochSeconds - mTimeOffset.toSeconds();
    }

    /**
     * Return the 64-bit number of seconds from Unix epoch 1970-01-01 00:00:00
     * UTC. Returns LocalDate::kInvalidUnixSeconds64 if isError() is true.
     *
     * Tip: You can use the command 'date +%s -d {iso8601date}' on a Unix box to
     * convert an ISO8601 date to the unix seconds.
     */
    int64_t toUnixSeconds64() const {
      if (isError()) return LocalDate::kInvalidUnixSeconds64;
      return mLocalDateTime.toUnixSeconds64() - mTimeOffset.toSeconds();
    }

    /**
     * Compare 'this' OffsetDateTime with 'that' OffsetDateTime, and return (<0,
     * 0, >0) according to whether the epochSeconds (incorporating the time
     * offset) is (a<b, a==b, a>b). This method can return 0 (equal) even if
     * the operator==() returns false if the two OffsetDateTime objects are
     * using different time offsets.
     *
     * If you want to know whether the local representatation of 'this'
     * OffsetDateTime occurs before or after the local representation of
     * 'that', use `this->localDateTime().compareTo(that.localDateTime())`
     * instead. This expression ignores the time offset which is sometimes what
     * you want.
     *
     * If either this->isError() or that.isError() is true, the result is
     * undefined.
     */
    int8_t compareTo(const OffsetDateTime& that) const {
      acetime_t thisSeconds = toEpochSeconds();
      acetime_t thatSeconds = that.toEpochSeconds();
      if (thisSeconds < thatSeconds) return -1;
      if (thisSeconds > thatSeconds) return 1;
      return 0;
    }

    /**
     * Print OffsetDateTime to 'printer' in ISO 8601 format.
     * This class does not implement the Printable interface to avoid
     * increasing the size of the object from the additional virtual function.
     */
    void printTo(Print& printer) const;

    // Use default copy constructor and assignment operator.
    OffsetDateTime(const OffsetDateTime&) = default;
    OffsetDateTime& operator=(const OffsetDateTime&) = default;

  private:
    friend bool operator==(const OffsetDateTime& a, const OffsetDateTime& b);

    /** Expected length of an ISO 8601 date string, including UTC offset. */
    static const uint8_t kDateStringLength = 25;

    /** Constructor from LocalDateTime and a TimeOffset. */
    explicit OffsetDateTime(const LocalDateTime& ldt, TimeOffset timeOffset):
        mLocalDateTime(ldt),
        mTimeOffset(timeOffset) {}

    LocalDateTime mLocalDateTime;
    TimeOffset mTimeOffset;
};

/**
 * Return true if two OffsetDateTime objects are equal in all components.
 * Optimized for small changes in the less signficant fields, such as 'second'
 * or 'minute'.
 */
inline bool operator==(const OffsetDateTime& a, const OffsetDateTime& b) {
  return a.mLocalDateTime == b.mLocalDateTime
      && a.mTimeOffset == b.mTimeOffset;
}

/** Return true if two OffsetDateTime objects are not equal. */
inline bool operator!=(const OffsetDateTime& a, const OffsetDateTime& b) {
  return ! (a == b);
}

}

#endif
