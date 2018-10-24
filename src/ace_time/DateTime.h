#ifndef ACE_TIME_DATE_TIME_H
#define ACE_TIME_DATE_TIME_H

#include <stdint.h>
#include "common/Flash.h"
#include "common/Util.h"
#include "OffsetDateTime.h"
#include "TimeZone.h"

class Print;

namespace ace_time {

/**
 * The date (year, month, day) and time (hour, minute, second) fields
 * representing an instant in time. The year field is internally represented as
 * a 2 digit number from [2000, 2099]. Therefore, the "epoch" for this library
 * is 2000-01-01 00:00:00Z. These fields map directly to the fields supported by
 * the DS3231 RTC chip.
 *
 * The dayOfWeek (1=Sunday, 7=Saturday) is calculated internally from the date.
 * The value is calculated lazily and cached internally. If any components are
 * changed, then the cache is invalidated and the dayOfWeek is lazily
 * recalculated. Fortunately, changing the timeZone does *not* affect the
 * dayOfWeek.
 *
 * The incrementXxx() methods are convenience methods to allow the user to
 * change the date and time using just two buttons. The user is expected to
 * select a specific DateTime component using one of the buttons, then press the
 * other button to increment it.
 *
 * Some parts of this class was inspired by the DateTime class of
 * http://www.joda.org.
 */
class DateTime {
  public:
    /**
     * Factory method using separated date, time, and time zone fields. The
     * dayOfWeek will be lazily calculated.
     *
     * @param year last 2 digits of the year from year 2000
     * @param month month with January=1, December=12
     * @param day day of month (1-31)
     * @param hour hour (0-23)
     * @param minute minute (0-59)
     * @param second second (0-59), does not support leap seconds
     * @param timeZone Optional. Default is UTC time zone.
     */
    static DateTime forComponents(uint8_t year, uint8_t month, uint8_t day,
            uint8_t hour, uint8_t minute, uint8_t second,
            TimeZone timeZone = TimeZone()) {
      ZoneOffset zoneOffset = timeZone.effectiveZoneOffset(0);
      OffsetDateTime dt(year, month, day, hour, minute, second, zoneOffset);
      return DateTime(dt, timeZone);
    }

    /**
     * Factory method. Create the DateTime from secondsSinceEpoch as seen from
     * the given time zone. If the time zone's offset is negative, then
     * (secondsSinceEpoch >= TimeZone::asEffectiveOffsetSeconds() must be true.
     * Otherwise, the local time will be in the year 1999, which cannot be
     * represented by a 2-digit year beginning with the year 2000. The
     * dayOfWeek will be calculated internally.
     *
     * @param secondsSinceEpoch Number of seconds from AceTime epoch
     *    (2000-01-01 00:00:00Z). A 0 value is a sentinel is considered to be
     *    an error and causes isError() to return true.
     * @param timeZone Optional. Default is UTC time zone.
     */
    static DateTime forSeconds(uint32_t secondsSinceEpoch,
              TimeZone timeZone = TimeZone()) {
      DateTime dt;
      if (secondsSinceEpoch == 0) {
        return dt.setError();
      }

      ZoneOffset zoneOffset = timeZone.effectiveZoneOffset(secondsSinceEpoch);
      dt.mOffsetDateTime = OffsetDateTime(secondsSinceEpoch, zoneOffset);
      dt.mTimeZone = timeZone;
      return dt;
    }

    /**
     * Factory method. Create a DateTime from the ISO8601 date string. If the
     * string cannot be parsed, then isError() on the constructed object
     * returns true.
     *
     * The dateString is expected to be in ISO8601 format
     * "YYYY-MM-DDThh:mm:ss+hh:mm", but currently, the parser is very lenient
     * and does not detect most errors. It cares mostly about the positional
     * placement of the various components. It does not validate the separation
     * characters like '-' or ':'. For example, both of the following will parse
     * to the exactly same DateTime object:
     * "2018-08-31T13:48:01-07:00"
     * "2018/08/31 13#48#01-07#00"
     */
    static DateTime forDateString(const char* dateString) {
      OffsetDateTime dt = OffsetDateTime::forDateString(dateString);
      TimeZone tz = TimeZone::forZoneOffset(dt.zoneOffset());
      return DateTime(dt, tz);
    }

    /**
     * Factory method. Create a DateTime from date string in flash memory F()
     * strings. Mostly for unit testing.
     */
    static DateTime forDateString(const __FlashStringHelper* dateString) {
      OffsetDateTime dt = OffsetDateTime::forDateString(dateString);
      TimeZone tz = TimeZone::forZoneOffset(dt.zoneOffset());
      return DateTime(dt, tz);
    }

    /** Default constructor. */
    explicit DateTime() {}

    /** Return true if any component indicates an error condition. */
    bool isError() const { return mOffsetDateTime.isError(); }

    /** Return the 2 digit year from year 2000. */
    uint8_t year() const { return mOffsetDateTime.year(); }

    /** Set the 2 digit year from year 2000. */
    void year(uint8_t year) { mOffsetDateTime.year(year); }

    /** Return the full year instead of just the last 2 digits. */
    uint16_t yearFull() const { return mOffsetDateTime.yearFull(); }

    /** Set the year given the full year. */
    void yearFull(uint16_t yearFull) { mOffsetDateTime.yearFull(yearFull); }

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

    /**
     * Return the day of the week, Sunday=1, Saturday=7. This is calculated
     * lazily and cached internally. Not thread-safe.
     */
    uint8_t dayOfWeek() const { return mOffsetDateTime.dayOfWeek(); }

    /** Return the time zone of the DateTime. */
    const TimeZone& timeZone() const { return mTimeZone; }

    /** Return the time zone of the DateTime. */
    TimeZone& timeZone() { return mTimeZone; }

    /** Set the time zone. */
    void timeZone(const TimeZone& timeZone) {
      // Does not affect dayOfWeek.
      mTimeZone = timeZone;
    }

    /**
     * Create a DateTime in a different time zone code (with the same
     * secondsSinceEpoch).
     */
    DateTime convertToTimeZone(TimeZone timeZone) const {
      uint32_t secondsSinceEpoch = toSecondsSinceEpoch();
      return DateTime::forSeconds(secondsSinceEpoch, timeZone);
    }

    /**
     * Print DateTime to 'printer'. Does not implement Printable to avoid
     * memory cost of vtable pointer.
     */
    void printTo(Print& printer) const;

    /** Increment the year by one, wrapping from 99 to 0. */
    void incrementYear() { mOffsetDateTime.incrementYear(); }

    /** Increment the year by one, wrapping from 12 to 1. */
    void incrementMonth() { mOffsetDateTime.incrementMonth(); }

    /** Increment the day by one, wrapping from 31 to 1. */
    void incrementDay() { mOffsetDateTime.incrementDay(); }

    /** Increment the hour by one, wrapping from 23 to 0. */
    void incrementHour() { mOffsetDateTime.incrementHour(); }

    /** Increment the minute by one, wrapping from 59 to 0. */
    void incrementMinute() { mOffsetDateTime.incrementMinute(); }

    /**
     * Return number of whole days since AceTime epoch (2000-01-01 00:00:00Z),
     * taking into account the time zone.
     */
    uint32_t toDaysSinceEpoch() const {
      return mOffsetDateTime.toDaysSinceEpoch();
    }

    /**
     * Return seconds since AceTime epoch (2000-01-01 00:00:00Z), taking into
     * account the time zone. The return type is an unsigned 32-bit integer,
     * which can represent a range of 136 years. Since the year is stored as a 2
     * digit offset from the year 2000, the return type will be valid for all
     * DateTime values which can be stored in this class.
     *
     * Normally, Julian day starts at 12:00:00. We modify the formula given in
     * wiki page to start the Gregorian day at 00:00:00.
     *
     * See https://en.wikipedia.org/wiki/Julian_day
     */
    uint32_t toSecondsSinceEpoch() const {
      return mOffsetDateTime.toSecondsSinceEpoch();
    }

    /**
     * Return the number of seconds from Unix epoch 1970-01-01 00:00:00Z. The
     * return type is a uint32_t which can represent a range of 136 years. Since
     * the year is stored as a 2 digit year (from 2000), this method will return
     * a valid result for all dates which can be stored by this class.
     *
     * Tip: You can use the command 'date +%s -d {iso8601date}' on a Unix box to
     * print the unix seconds.
     */
    uint32_t toUnixSeconds() const {
      return mOffsetDateTime.toUnixSeconds();
    }

    /**
     * Compare this DateTime with another DateTime, and return (<0, 0, >0)
     * according to whether the secondsSinceEpoch() is (a<b, a==b, a>b). The
     * dayOfWeek field is ignored but the time zone is used.  This method
     * can return 0 (equal) even if the operator==() returns false if the
     * two DateTime objects are in different time zones.
     */
    int8_t compareTo(const DateTime& that) const {
      return mOffsetDateTime.compareTo(that.mOffsetDateTime);
    }

    /**
     * Mark the DateTime so that isError() returns true. Returns a reference to
     * (*this) so that an invalid DateTime can be returned in a single
     * statement like this: 'return DateTime().setError()'.
     */
    DateTime& setError() {
      mOffsetDateTime.setError();
      return *this;
    }

  private:
    /** Expected length of an ISO8601 date string. */
    static const uint8_t kDateStringLength = 25;

    friend bool operator==(const DateTime& a, const DateTime& b);
    friend bool operator!=(const DateTime& a, const DateTime& b);

    /** Extract the date time components from the given dateString. */
    DateTime& initFromDateString(const char* dateString);

    /** Constructor. From OffsetDateTime and TimeZone. */
    DateTime(const OffsetDateTime& offsetDateTime, TimeZone tz):
      mOffsetDateTime(offsetDateTime),
      mTimeZone(tz) {}

    OffsetDateTime mOffsetDateTime;
    TimeZone mTimeZone; // offset from UTC in 15 minute increments
};

/**
 * Return true if two DateTime objects are equal in all components. Optimized
 * for small changes in the less signficant fields, such as 'second' or
 * 'minute'. The dayOfWeek is a derived field so it is not explicitly used to
 * test equality, but it follows that if all the other fields are identical,
 * thenthe dayOfWeek must also be equal.
 */
inline bool operator==(const DateTime& a, const DateTime& b) {
  return a.mOffsetDateTime == b.mOffsetDateTime
      && a.mTimeZone == b.mTimeZone;
}

/** Return true if two DateTime objects are not equal. */
inline bool operator!=(const DateTime& a, const DateTime& b) {
  return ! (a == b);
}

}

#endif
