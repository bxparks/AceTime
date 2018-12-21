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
 * a int8_t number from -128 to 127 representing the year 1872 to 2127. The
 * "epoch" for this library is 2000-01-01 00:00:00Z. If the year is restricted
 * to 0 to 99, the DateTime components map directly to the fields supported by
 * the DS3231 RTC chip. The dayOfWeek (1=Sunday, 7=Saturday) is calculated
 * internally from the date components. Changing the timeZone does not affect
 * the dayOfWeek.
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
    static const uint32_t kInvalidEpochSeconds =
        OffsetDateTime::kInvalidEpochSeconds;

    /**
     * Factory method using separated date, time, and time zone fields.
     * This is mostly provided for testing purposes. Most production code
     * will use the forEpochSeconds() method.
     *
     * If the timeZone derived from the ZoneInfo, then the local time may be
     * ambiguous, because we need to calculate the epochSeconds of the local
     * time before we can determine the UtcOffset, but the UtcOffset is a
     * function of the epochSeconds. This function resolves the ambiguity by
     * first obtaining the UtcOffset defined on Jan 1 of the specified year.
     * (In the northern hemisphere, this will be the Standard time). Using
     * that, it calculates what it believes is the actaul epochSeconds of the
     * components given to the method. Using this epochSeconds, it obtains a
     * second guess of the UtcOffset at that instant in time. Then it uses
     * that as the actual UtcOffset of this object.
     *
     * In the northern hemisphere the Standard time is observed on Jan 1.
     * If there are 2 valid UtcOffset for a given local date/time (i.e. when
     * the clock is shifted back by an hour in the autumn), this method will
     * prefer the Standard time representation, instead of the Daylight time
     * representation. If the local date/time represents a time in the gap
     * (i.e. when the clock is shifted forward), it is interpreted as if the
     * Standard time was extended into the gap. For example, in the United
     * States, the time of 2018-03-11T02:01 does not actually exist because the
     * clock was shifted forward that day from 02:00 to 03:00. However, this
     * method will interpret the 02:01 as if it was measured using the Standard
     * time offset.
     *
     * @param year
     * @param month month with January=1, December=12
     * @param day day of month (1-31)
     * @param hour hour (0-23)
     * @param minute minute (0-59)
     * @param second second (0-59), does not support leap seconds
     * @param timeZone pointer to an existing TimeZone instance. Optional,
     *        not nullable. Default is UTC TimeZone.
     */
    static DateTime forComponents(uint16_t year, uint8_t month, uint8_t day,
            uint8_t hour, uint8_t minute, uint8_t second,
            const TimeZone& timeZone = TimeZone()) {
      if (timeZone.getType() == TimeZone::kTypeManual) {
        UtcOffset utcOffset = timeZone.getUtcOffset(0);
        OffsetDateTime odt = OffsetDateTime::forComponents(
            year, month, day, hour, minute, second, utcOffset);
        return DateTime(odt, timeZone);
      } else {
        // First guess at the UtcOffset using Jan 1 of the given year.
        uint32_t initialEpochSeconds =
            LocalDate::forComponents(year, 1, 1).toEpochSeconds();
        UtcOffset initialUtcOffset =
            timeZone.getUtcOffset(initialEpochSeconds);

        // Second guess at the UtcOffset using the first UtcOffset.
        OffsetDateTime odt = OffsetDateTime::forComponents(
            year, month, day, hour, minute, second, initialUtcOffset);
        uint32_t epochSeconds = odt.toEpochSeconds();
        UtcOffset actualUtcOffset = timeZone.getUtcOffset(epochSeconds);

        odt = OffsetDateTime::forComponents(
            year, month, day, hour, minute, second, actualUtcOffset);
        return DateTime(odt, timeZone);
      }
    }

    /**
     * Factory method. Create the DateTime from epochSeconds as seen from
     * the given time zone. If the time zone's offset is negative, then
     * (epochSeconds >= TimeZone::effectiveOffsetSeconds().toEpochSeconds())
     * must be true. Otherwise, the local time will be in the year 1999, which
     * cannot be represented by a 2-digit year beginning with the year 2000.
     * The dayOfWeek will be calculated internally.
     *
     * @param epochSeconds Number of seconds from AceTime epoch
     *    (2000-01-01 00:00:00Z). A value of kInvalidEpochSeconds is a sentinel
     *    is considered to be an error and causes isError() to return true.
     * @param timeZone Optional, not nullable. Default is UTC TimeZone.
     */
    static DateTime forEpochSeconds(uint32_t epochSeconds,
        const TimeZone& timeZone = TimeZone()) {
      DateTime dt;
      if (epochSeconds == kInvalidEpochSeconds) {
        return dt.setError();
      }

      UtcOffset utcOffset = timeZone.getUtcOffset(epochSeconds);
      dt.mOffsetDateTime = OffsetDateTime::forEpochSeconds(
          epochSeconds, utcOffset);
      dt.mTimeZone = timeZone;
      return dt;
    }

    /**
     * Factory method. Create a DateTime from the ISO 8601 date string. If the
     * string cannot be parsed, then isError() on the constructed object
     * returns true.
     *
     * The dateString is expected to be in ISO 8601 format
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
      // TODO: fix time zone
      return DateTime(dt, TimeZone());
    }

    /**
     * Factory method. Create a DateTime from date string in flash memory F()
     * strings. Mostly for unit testing.
     */
    static DateTime forDateString(const __FlashStringHelper* dateString) {
      OffsetDateTime dt = OffsetDateTime::forDateString(dateString);
      // TODO: fix time zone
      return DateTime(dt, TimeZone());
    }

    /** Default constructor. */
    explicit DateTime() {}

    /** Return true if any component indicates an error condition. */
    bool isError() const { return mOffsetDateTime.isError(); }

    /**
     * Mark the DateTime so that isError() returns true. Returns a reference to
     * (*this) so that an invalid DateTime can be returned in a single
     * statement like this: 'return DateTime().setError()'.
     */
    DateTime& setError() {
      mOffsetDateTime.setError();
      return *this;
    }

    /** Return the 2 digit year from year 2000. */
    int8_t year2() const { return mOffsetDateTime.year2(); }

    /** Set the 2 digit year from year 2000. */
    void year2(int8_t year2) { mOffsetDateTime.year2(year2); }

    /** Return the year. */
    uint16_t year() const { return mOffsetDateTime.year(); }

    /** Set the year given the full year. */
    void year(uint16_t year) { mOffsetDateTime.year(year); }

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
     * Return the day of the week using ISO 8601 numbering where Monday=1 and
     * Sunday=7.
     */
    uint8_t dayOfWeek() const { return mOffsetDateTime.dayOfWeek(); }

    /** Return the time zone of the DateTime. */
    const TimeZone& timeZone() const { return mTimeZone; }

    /**
     * Set the time zone. Note that this does not convert a given DateTime
     * into a different TimeZone. Use converToTimeZone() instead.
     */
    void timeZone(const TimeZone& timeZone) { mTimeZone = timeZone; }

    /**
     * Create a DateTime in a different time zone (with the same epochSeconds).
     */
    DateTime convertToTimeZone(const TimeZone& timeZone) const {
      uint32_t epochSeconds = toEpochSeconds();
      return DateTime::forEpochSeconds(epochSeconds, timeZone);
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
    uint32_t toEpochDays() const {
      return mOffsetDateTime.toEpochDays();
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
    uint32_t toEpochSeconds() const {
      return mOffsetDateTime.toEpochSeconds();
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
     * according to whether the epochSeconds is (a<b, a==b, a>b). The
     * dayOfWeek field is ignored but the time zone is used.  This method
     * can return 0 (equal) even if the operator==() returns false if the
     * two DateTime objects are in different time zones.
     */
    int8_t compareTo(const DateTime& that) const {
      return mOffsetDateTime.compareTo(that.mOffsetDateTime);
    }

  private:
    /** Expected length of an ISO 8601 date string. */
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
    TimeZone mTimeZone;
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
