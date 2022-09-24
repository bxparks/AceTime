/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#ifndef ACE_TIME_ZONED_DATE_TIME_H
#define ACE_TIME_ZONED_DATE_TIME_H

#include <stdint.h>
#include "common/compat.h"
#include "OffsetDateTime.h"
#include "TimeZone.h"

class Print;

namespace ace_time {

/**
 * The date (year, month, day), time (hour, minute, second), and
 * a timeZone object that supports the zones defined by the IANA TZ database.
 *
 * The year field is internally represented as an int16_t number, with 0
 * indicating -Infinity and 10000 representing +Infinity, so the normal range is
 * [1,9999]. An invalid year is represented by INT16_MIN (-32768).
 *
 * The default epoch for AceTime is 2000-01-01 00:00:00 UTC, but can be changed
 * using `LocaDate::localEpochYear()`. The `toEpochSeconds()` method returns a
 * `int32_t` number of seconds offset from that epoch.
 *
 * The dayOfWeek (1=Sunday, 7=Saturday) is calculated internally from the date
 * components. Changing the timeZone does not affect the dayOfWeek.
 *
 * Some parts of this class were inspired by the org.joda.DateTime of
 * http://www.joda.org, the java.time.ZonedDateTime class of Java 11, and the
 * datetime package of Python 3.
 */
class ZonedDateTime {
  public:
    /**
     * Factory method using separated date, time, and time zone fields.
     * This is intended mostly for testing purposes. Most production code
     * will use the forEpochSeconds() method.
     *
     * The TimeOffset at the given date/time component is calculated using
     * TimeZone::getOffsetDateTime().
     *
     * @param year year [0,10000]
     * @param month month with January=1, December=12
     * @param day day of month [1-31]
     * @param hour hour [0-23]
     * @param minute minute [0-59]
     * @param second second [0-59], does not support leap seconds
     * @param timeZone a TimeZone instance (use TimeZone() for UTC)
     * @param fold optional disambiguation of multiple occurences [0, 1]
     */
    static ZonedDateTime forComponents(int16_t year, uint8_t month, uint8_t day,
        uint8_t hour, uint8_t minute, uint8_t second,
        const TimeZone& timeZone, uint8_t fold = 0) {
      auto ldt = LocalDateTime::forComponents(
          year, month, day, hour, minute, second, fold);
      auto odt = timeZone.getOffsetDateTime(ldt);
      return ZonedDateTime(odt, timeZone);
    }

    /**
     * Factory method. Create the ZonedDateTime from epochSeconds as seen from
     * the given time zone. The dayOfWeek will be calculated internally.
     * Returns ZonedDateTime::forError() if epochSeconds is invalid.
     *
     * @param epochSeconds Number of seconds from AceTime epoch
     *    (2000-01-01 00:00:00 UTC). A value of LocalDate::kInvalidEpochSeconds
     *    is a sentinel that is considered to be an error and causes isError()
     *    to return true.
     * @param timeZone a TimeZone instance (use TimeZone() for UTC)
     */
    static ZonedDateTime forEpochSeconds(acetime_t epochSeconds,
        const TimeZone& timeZone) {
      OffsetDateTime odt = (epochSeconds == LocalDate::kInvalidEpochSeconds)
          ? OffsetDateTime::forError()
          : timeZone.getOffsetDateTime(epochSeconds);
      return ZonedDateTime(odt, timeZone);
    }

    /**
     * Factory method to create a ZonedDateTime using the 64-bit number of
     * seconds from Unix epoch.
     * Valid until the 64-bit unixSeconds reaches the equivalent of
     * 2068-01-19T03:14:07 UTC.
     * Returns ZonedDateTime::forError() if unixSeconds is invalid.
     *
     * @param unixSeconds number of seconds since Unix epoch
     *    (1970-01-01T00:00:00 UTC)
     * @param timeZone a TimeZone instance (use TimeZone() for UTC)
     */
    static ZonedDateTime forUnixSeconds64(
        int64_t unixSeconds, const TimeZone& timeZone) {
      acetime_t epochSeconds;
      if (unixSeconds == LocalDate::kInvalidEpochSeconds64) {
        epochSeconds = LocalDate::kInvalidEpochSeconds;
      } else {
        epochSeconds = unixSeconds
            // relative to base epoch
            - LocalDate::kSecondsFromUnixEpochToBaseEpoch
            // relative to local epoch
            - LocalDate::sDaysFromBaseEpochToLocalEpoch * (int64_t) 86400;
      }
      return forEpochSeconds(epochSeconds, timeZone);
    }

    /**
     * Factory method. Create a ZonedDateTime from the ISO 8601 date string. If
     * the string cannot be parsed, then isError() on the constructed object
     * returns true.
     *
     * @param dateString a string in ISO 8601 format
     *    "YYYY-MM-DDThh:mm:ss+hh:mm". The parser is very lenient and does
     *    not detect most errors. It cares mostly about the positional
     *    placement of the various components. It does not validate the
     *    separation characters like '-' or ':'. For example, both of the
     *    following will parse to the exactly same ZonedDateTime object:
     *    "2018-08-31T13:48:01-07:00" "2018/08/31 13#48#01-07#00".
     */
    static ZonedDateTime forDateString(const char* dateString) {
      OffsetDateTime dt = OffsetDateTime::forDateString(dateString);
      return ZonedDateTime(dt, TimeZone::forTimeOffset(dt.timeOffset()));
    }

    /**
     * Factory method. Create a ZonedDateTime from date string in flash memory
     * F() strings. Mostly for unit testing.
     */
    static ZonedDateTime forDateString(const __FlashStringHelper* dateString) {
      OffsetDateTime dt = OffsetDateTime::forDateString(dateString);
      return ZonedDateTime(dt, TimeZone::forTimeOffset(dt.timeOffset()));
    }

    /** Return an instance whose isError() returns true. */
    static ZonedDateTime forError() {
      return ZonedDateTime(OffsetDateTime::forError(), TimeZone());
    }

    /** Default constructor. */
    explicit ZonedDateTime() {}

    /** Return true if any component indicates an error condition. */
    bool isError() const { return mOffsetDateTime.isError(); }

    /** Return the year. */
    int16_t year() const { return mOffsetDateTime.year(); }

    /** Set the year given the full year. */
    void year(int16_t year) { mOffsetDateTime.year(year); }

    /** Return the month with January=1, December=12. */
    uint8_t month() const { return mOffsetDateTime.month(); }

    /** Set the month. */
    void month(uint8_t month) { mOffsetDateTime.month(month); }

    /** Return the day of the month. */
    uint8_t day() const { return mOffsetDateTime.day(); }

    /** Set the day of the month. */
    void day(uint8_t day) { mOffsetDateTime.day(day); }

    /** Return the hour. */
    uint8_t hour() const { return mOffsetDateTime.hour(); }

    /** Set the hour. */
    void hour(uint8_t hour) { mOffsetDateTime.hour(hour); }

    /** Return the minute. */
    uint8_t minute() const { return mOffsetDateTime.minute(); }

    /** Set the minute. */
    void minute(uint8_t minute) { mOffsetDateTime.minute(minute); }

    /** Return the second. */
    uint8_t second() const { return mOffsetDateTime.second(); }

    /** Set the second. */
    void second(uint8_t second) { mOffsetDateTime.second(second); }

    /** Return the fold. */
    uint8_t fold() const { return mOffsetDateTime.fold(); }

    /** Set the fold. */
    void fold(uint8_t fold) { mOffsetDateTime.fold(fold); }

    /**
     * Return the day of the week using ISO 8601 numbering where Monday=1 and
     * Sunday=7.
     */
    uint8_t dayOfWeek() const { return mOffsetDateTime.dayOfWeek(); }

    /** Return the time zone of the ZonedDateTime. */
    const TimeZone& timeZone() const { return mTimeZone; }

    /**
     * Set the time zone. Note that this does not convert a given ZonedDateTime
     * into a different TimeZone. Use converToTimeZone() instead.
     */
    void timeZone(const TimeZone& timeZone) { mTimeZone = timeZone; }

    /** Return the offset zone of the OffsetDateTime. */
    TimeOffset timeOffset() const { return mOffsetDateTime.timeOffset(); }

    /** Return the LocalDateTime of the components. */
    const LocalDateTime& localDateTime() const {
      return mOffsetDateTime.localDateTime();
    }

    /**
     * Normalize the ZonedDateTime after mutation. This must be called after any
     * mutation method is called (i.e. year(), month(), day(), hour(), minute(),
     * second(), timezone()) in order to obtain correct values for various
     * derivative information (e.g. toEpochSeconds()). Multiple mutations can be
     * batched together before calling this method.
     *
     * This method exists because AceTime objects are mutable instead of
     * immutable. If the objects were immutable, then each mutation would create
     * a new object that would be automatically normalized, and an unnormalized
     * object would not be visible outside of the library. Unfortunately, making
     * the AceTime classes immutable causes the library to consume too much
     * additional memory and consume too much CPU resources on 8-bit processors.
     * So we must provide this normalize() method which must be called
     * manually by the client code.
     */
    void normalize() {
      mOffsetDateTime = mTimeZone.getOffsetDateTime(localDateTime());
    }

    /**
     * Create a ZonedDateTime in a different time zone (with the same
     * epochSeconds).
     */
    ZonedDateTime convertToTimeZone(const TimeZone& timeZone) const {
      acetime_t epochSeconds = toEpochSeconds();
      return ZonedDateTime::forEpochSeconds(epochSeconds, timeZone);
    }

    /**
     * Return number of whole days since AceTime epoch taking into account the
     * time zone. The default epoch is 2000-01-01 00:00:00 UTC but can be
     * changed using `LocalDate::localEpochYear()`.
     */
    int32_t toEpochDays() const {
      return mOffsetDateTime.toEpochDays();
    }

    /** Return the number of days since Unix epoch (1970-01-01 00:00:00). */
    int32_t toUnixDays() const {
      return mOffsetDateTime.toUnixDays();
    }

    /**
     * Return seconds since AceTime epoch taking into account the time zone. The
     * default epoch is 2000-01-01 00:00:00 UTC but can be changed using
     * `LocalDate::localEpochYear()`.
     */
    acetime_t toEpochSeconds() const {
      return mOffsetDateTime.toEpochSeconds();
    }

    /**
     * Return the 64-bit number of seconds from Unix epoch 1970-01-01 00:00:00
     * UTC. Returns LocalDAte::kInvalidEpochSeconds64 if isError() is true.
     *
     * Tip: You can use the command 'date +%s -d {iso8601date}' on a Unix box to
     * print the unix seconds.
     */
    int64_t toUnixSeconds64() const {
      return mOffsetDateTime.toUnixSeconds64();
    }

    /**
     * Compare 'this' ZonedDateTime with 'that' ZonedDateTime, and return (<0,
     * 0, >0) according to whether the equivalent epochSeconds (with the
     * timezone incorporated) is (a<b, a==b, a>b). The dayOfWeek field is
     * ignored.  This method can return 0 (equal) even if the operator==()
     * returns false if the two ZonedDateTime objects are in different time
     * zones.
     *
     * If you want to know whether the local representatation of 'this'
     * ZonedDateTime occurs before or after the local representation of 'that',
     * use `this->localDateTime().compareTo(that.localDateTime())` instead.
     * This expression ignores the time zone which is sometimes what you want.
     *
     * If either this->isError() or that.isError() is true, the result is
     * undefined.
     */
    int8_t compareTo(const ZonedDateTime& that) const {
      return mOffsetDateTime.compareTo(that.mOffsetDateTime);
    }

    /**
     * Print ZonedDateTime to 'printer'.
     * This class does not implement the Printable interface to avoid
     * increasing the size of the object from the additional virtual function.
     */
    void printTo(Print& printer) const;

    // Use default copy constructor and assignment operator.
    ZonedDateTime(const ZonedDateTime&) = default;
    ZonedDateTime& operator=(const ZonedDateTime&) = default;

  private:
    /** Expected length of an ISO 8601 date string. */
    static const uint8_t kDateStringLength = 25;

    friend bool operator==(const ZonedDateTime& a, const ZonedDateTime& b);

    /** Constructor. From OffsetDateTime and TimeZone. */
    ZonedDateTime(const OffsetDateTime& offsetDateTime, const TimeZone& tz):
      mOffsetDateTime(offsetDateTime),
      mTimeZone(tz) {}

    OffsetDateTime mOffsetDateTime;
    TimeZone mTimeZone;
};

/**
 * Return true if two ZonedDateTime objects are equal in all components.
 * Optimized for small changes in the less signficant fields, such as 'second'
 * or 'minute'. The dayOfWeek is a derived field so it is not explicitly used
 * to test equality, but it follows that if all the other fields are identical,
 * then the dayOfWeek must also be equal.
 */
inline bool operator==(const ZonedDateTime& a, const ZonedDateTime& b) {
  return a.mOffsetDateTime == b.mOffsetDateTime
      && a.mTimeZone == b.mTimeZone;
}

/** Return true if two ZonedDateTime objects are not equal. */
inline bool operator!=(const ZonedDateTime& a, const ZonedDateTime& b) {
  return ! (a == b);
}

}

#endif
