#ifndef ACE_TIME_DATE_TIME_H
#define ACE_TIME_DATE_TIME_H

#include <stdint.h>
#include "common/flash.h"
#include "OffsetDateTime.h"
#include "TimeZone.h"

class Print;

namespace ace_time {

/**
 * The date (year, month, day) and time (hour, minute, second) fields
 * representing an instant in time. In an 8-bit implementation, the year field
 * is internally represented as a int8_t number from -128 to 127 representing
 * the year 1872 to 2127 inclusive. In a 16-bit implementation, the year field
 * is an int16_t, so can represent years from 0000-9999 inclusive.
 *
 * The "epoch" for this library is 2000-01-01 00:00:00Z. The dayOfWeek
 * (1=Sunday, 7=Saturday) is calculated internally from the date components.
 * Changing the timeZone does not affect the dayOfWeek.
 *
 * Some parts of this class was inspired by the org.joda.DateTime of
 * http://www.joda.org and java.time.ZonedDateTime of JDK8.
 */
class ZonedDateTime {
  public:
    static const acetime_t kInvalidEpochSeconds = LocalTime::kInvalidSeconds;

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
     * @param year [1872-2127] for 8-bit implementation, [0000-9999] for
     *    16-bit implementation
     * @param month month with January=1, December=12
     * @param day day of month (1-31)
     * @param hour hour (0-23)
     * @param minute minute (0-59)
     * @param second second (0-59), does not support leap seconds
     * @param timeZone pointer to an existing TimeZone instance. Optional,
     *        not nullable. Default is UTC TimeZone.
     */
    static ZonedDateTime forComponents(int16_t year, uint8_t month, uint8_t day,
            uint8_t hour, uint8_t minute, uint8_t second,
            const TimeZone& timeZone = TimeZone()) {
      if (timeZone.getType() == TimeZone::kTypeAuto) {
        // First guess at the UtcOffset using Jan 1 of the given year.
        acetime_t initialEpochSeconds =
            LocalDate::forComponents(year, 1, 1).toEpochSeconds();
        UtcOffset initialUtcOffset =
            timeZone.getUtcOffset(initialEpochSeconds);

        // Second guess at the UtcOffset using the first UtcOffset.
        OffsetDateTime odt = OffsetDateTime::forComponents(
            year, month, day, hour, minute, second, initialUtcOffset);
        acetime_t epochSeconds = odt.toEpochSeconds();
        UtcOffset actualUtcOffset = timeZone.getUtcOffset(epochSeconds);

        odt = OffsetDateTime::forComponents(
            year, month, day, hour, minute, second, actualUtcOffset);
        return ZonedDateTime(odt, timeZone);
      } else {
        UtcOffset utcOffset = timeZone.getUtcOffset(0);
        OffsetDateTime odt = OffsetDateTime::forComponents(
            year, month, day, hour, minute, second, utcOffset);
        return ZonedDateTime(odt, timeZone);
      }
    }

    /**
     * Factory method. Create the ZonedDateTime from epochSeconds as seen from
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
    static ZonedDateTime forEpochSeconds(acetime_t epochSeconds,
        const TimeZone& timeZone = TimeZone()) {
      ZonedDateTime dt;
      if (epochSeconds == kInvalidEpochSeconds) return forError();

      UtcOffset utcOffset = timeZone.getUtcOffset(epochSeconds);
      dt.mOffsetDateTime = OffsetDateTime::forEpochSeconds(
          epochSeconds, utcOffset);
      dt.mTimeZone = timeZone;
      return dt;
    }

    /**
     * Factory method to create a ZonedDateTime using the number of seconds from
     * Unix epoch.
     */
    static ZonedDateTime forUnixSeconds(acetime_t unixSeconds,
        const TimeZone& timeZone = TimeZone()) {
      if (unixSeconds == LocalDate::kInvalidEpochSeconds) {
        return forEpochSeconds(unixSeconds, timeZone);
      } else {
        return forEpochSeconds(unixSeconds - LocalDate::kSecondsSinceUnixEpoch,
            timeZone);
      }
    }

    /**
     * Factory method. Create a ZonedDateTime from the ISO 8601 date string. If
     * the string cannot be parsed, then isError() on the constructed object
     * returns true.
     *
     * The dateString is expected to be in ISO 8601 format
     * "YYYY-MM-DDThh:mm:ss+hh:mm", but currently, the parser is very lenient
     * and does not detect most errors. It cares mostly about the positional
     * placement of the various components. It does not validate the separation
     * characters like '-' or ':'. For example, both of the following will parse
     * to the exactly same ZonedDateTime object:
     * "2018-08-31T13:48:01-07:00"
     * "2018/08/31 13#48#01-07#00"
     */
    static ZonedDateTime forDateString(const char* dateString) {
      OffsetDateTime dt = OffsetDateTime::forDateString(dateString);
      // TODO: fix time zone
      return ZonedDateTime(dt, TimeZone());
    }

    /**
     * Factory method. Create a ZonedDateTime from date string in flash memory
     * F() strings. Mostly for unit testing.
     */
    static ZonedDateTime forDateString(const __FlashStringHelper* dateString) {
      OffsetDateTime dt = OffsetDateTime::forDateString(dateString);
      // TODO: fix time zone
      return ZonedDateTime(dt, TimeZone());
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

    /** Return the single-byte year offset from year 2000. */
    int8_t yearTiny() const { return mOffsetDateTime.yearTiny(); }

    /** Set the single-byte year offset from year 2000. */
    void yearTiny(int8_t yearTiny) { mOffsetDateTime.yearTiny(yearTiny); }

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

    /** Return the time zone of the ZonedDateTime. */
    const TimeZone& timeZone() const { return mTimeZone; }

    /**
     * Set the time zone. Note that this does not convert a given ZonedDateTime
     * into a different TimeZone. Use converToTimeZone() instead.
     */
    void timeZone(const TimeZone& timeZone) { mTimeZone = timeZone; }

    /**
     * Create a ZonedDateTime in a different time zone (with the same
     * epochSeconds).
     */
    ZonedDateTime convertToTimeZone(const TimeZone& timeZone) const {
      acetime_t epochSeconds = toEpochSeconds();
      return ZonedDateTime::forEpochSeconds(epochSeconds, timeZone);
    }

    /**
     * Print ZonedDateTime to 'printer'. Does not implement Printable to avoid
     * memory cost of vtable pointer.
     */
    void printTo(Print& printer) const;

    /**
     * Return number of whole days since AceTime epoch (2000-01-01 00:00:00Z),
     * taking into account the time zone.
     */
    acetime_t toEpochDays() const {
      return mOffsetDateTime.toEpochDays();
    }

    /** Return the number of days since Unix epoch (1970-01-01 00:00:00). */
    acetime_t toUnixDays() const {
      if (isError()) return LocalDate::kInvalidEpochDays;
      return toEpochDays() + LocalDate::kDaysSinceUnixEpoch;
    }

    /**
     * Return seconds since AceTime epoch (2000-01-01 00:00:00Z), taking into
     * account the time zone.
     *
     * Normally, Julian day starts at 12:00:00. We modify the formula given in
     * wiki page to start the Gregorian day at 00:00:00.
     * See https://en.wikipedia.org/wiki/Julian_day
     */
    acetime_t toEpochSeconds() const {
      return mOffsetDateTime.toEpochSeconds();
    }

    /**
     * Return the number of seconds from Unix epoch 1970-01-01 00:00:00Z. The
     * return type is a acetime_t which can represent a range of 136 years.
     *
     * Tip: You can use the command 'date +%s -d {iso8601date}' on a Unix box to
     * print the unix seconds.
     */
    acetime_t toUnixSeconds() const {
      return mOffsetDateTime.toUnixSeconds();
    }

    /**
     * Compare this ZonedDateTime with another ZonedDateTime, and return (<0,
     * 0, >0) according to whether the epochSeconds is (a<b, a==b, a>b). The
     * dayOfWeek field is ignored but the time zone is used.  This method can
     * return 0 (equal) even if the operator==() returns false if the two
     * ZonedDateTime objects are in different time zones.
     */
    int8_t compareTo(const ZonedDateTime& that) const {
      return mOffsetDateTime.compareTo(that.mOffsetDateTime);
    }

  private:
    /** Expected length of an ISO 8601 date string. */
    static const uint8_t kDateStringLength = 25;

    friend bool operator==(const ZonedDateTime& a, const ZonedDateTime& b);
    friend bool operator!=(const ZonedDateTime& a, const ZonedDateTime& b);

    /** Constructor. From OffsetDateTime and TimeZone. */
    ZonedDateTime(const OffsetDateTime& offsetDateTime, TimeZone tz):
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
 * thenthe dayOfWeek must also be equal.
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
