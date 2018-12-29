#ifndef ACE_TIME_OFFSET_DATE_TIME_H
#define ACE_TIME_OFFSET_DATE_TIME_H

#include <stdint.h>
#include "UtcOffset.h"
#include "LocalDate.h"
#include "LocalTime.h"

class Print;

namespace ace_time {

/**
 * The date (year, month, day) and time (hour, minute, second) fields
 * representing the time with an offset from UTC.
 * The dayOfWeek (1=Monday, 7=Sunday, per ISO 8601) is calculated
 * internally from the date fields.
 *
 * The year field is internally represented as int8_t offset from the year
 * 2000, so in theory it is valid from [1872, 2127]. If the year is restricted
 * to the range 00-99, these fields map directly to the fields supported by the
 * DS3231 RTC chip. The "epoch" for this library is 2000-01-01T00:00:00Z and
 * toEpochSeconds() returns a int32_t number of seconds offset from that
 * epoch. The largest possible int32_t value is INT32_MAX, but this value is
 * used by kInvalidEpochSeconds to indicate an invalid value.
 *
 * The incrementXxx() methods are convenience methods to allow the user to
 * change the date and time using just two buttons. The user is expected to
 * select a specific OffsetDateTime component using one of the buttons, then
 * press the other button to increment it.
 *
 * Parts of this class were inspired by the java.time.OffsetDateTime class of
 * Java 8
 * (https://docs.oracle.com/javase/8/docs/api/java/time/OffsetDateTime.html).
 */
class OffsetDateTime {
  public:
    /**
     * Factory method using separated date, time, and time zone fields.
     *
     * @param year [1872-2127] for 8-bit implementation, [0000-9999] for
     *    16-bit implementation
     * @param month month with January=1, December=12
     * @param day day of month (1-31)
     * @param hour hour (0-23)
     * @param minute minute (0-59)
     * @param second second (0-59), does not support leap seconds
     * @param utcOffset Optional, default is UTC time zone. The time zone
     * offset in 15-minute increments from UTC. Using a UtcOffset object here
     * in the last component allows us to add an additional constructor that
     * accepts a millisecond component in the future.
     */
    static OffsetDateTime forComponents(int16_t year, uint8_t month,
        uint8_t day, uint8_t hour, uint8_t minute, uint8_t second,
        UtcOffset utcOffset = UtcOffset()) {
      return OffsetDateTime(year, month, day, hour, minute, second, utcOffset);
    }

    /**
     * Factory method. Create the various components of the OffsetDateTime from
     * the epochSeconds and its UtcOffset.
     *
     * @param epochSeconds Number of seconds from AceTime epoch
     *    (2000-01-01 00:00:00). Use kInvalidEpochSeconds to define an
     *    invalid instance whose isError() returns true.
     * @param utcOffset Optional. Default is UTC time zone.
     *
     * See https://en.wikipedia.org/wiki/Julian_day.
     */
    static OffsetDateTime forEpochSeconds(acetime_t epochSeconds,
          UtcOffset utcOffset = UtcOffset()) {
      OffsetDateTime dt;
      if (epochSeconds == LocalDate::kInvalidEpochSeconds) return dt.setError();

      // Get the real epochSeconds
      dt.mUtcOffset = utcOffset;
      epochSeconds += utcOffset.toSeconds();

      // Integer floor-division towards -infinity
      acetime_t days = (epochSeconds < 0)
          ? (epochSeconds + 1) / 86400 - 1
          : epochSeconds / 86400;

      // Avoid % operator, because it's slow on an 8-bit process and because
      // epochSeconds could be negative.
      acetime_t seconds = epochSeconds - 86400 * days;

      dt.mLocalDate = LocalDate::forEpochDays(days);
      dt.mLocalTime = LocalTime::forSeconds(seconds);

      return dt;
    }

    /**
     * Factory method that takes the number of seconds since Unix Epoch of
     * 1970-01-01. Similar to forEpochSeconds(), the seconds corresponding to
     * the partial day are truncated down towards the smallest whole day.
     */
    static OffsetDateTime forUnixSeconds(acetime_t unixSeconds,
          UtcOffset utcOffset = UtcOffset()) {
      if (unixSeconds == LocalDate::kInvalidEpochSeconds) {
        return forEpochSeconds(unixSeconds, utcOffset);
      } else {
        return forEpochSeconds(unixSeconds - LocalDate::kSecondsSinceUnixEpoch,
            utcOffset);
      }
    }

    /**
     * Factory method. Create a OffsetDateTime from the ISO 8601 date string. If
     * the string cannot be parsed, then isError() on the constructed object
     * returns true.
     *
     * The dateString is expected to be in ISO 8601 format
     * "YYYY-MM-DDThh:mm:ss+hh:mm", but currently, the parser is very lenient.
     * It cares mostly about the positional placement of the various
     * components. It does not validate the separation characters like '-' or
     * ':'. For example, both of the following will parse to the exactly same
     * OffsetDateTime object: "2018-08-31T13:48:01-07:00" "2018/08/31
     * 13#48#01-07#00"
     *
     * The parsing validation is so weak that the behavior is undefined for
     * most invalid date/time strings. The range of valid dates is roughly from
     * 1872-01-01T00:00:00 to 2127-12-31T23:59:59. However, the UTC offset may
     * cause some of the dates on the two extreme ends invalid. The behavior is
     * undefined in those cases.
     */
    static OffsetDateTime forDateString(const char* dateString) {
      return OffsetDateTime().initFromDateString(dateString);
    }

    /**
     * Factory method. Create a OffsetDateTime from date string in flash memory
     * F() strings. Mostly for unit testing.
     */
    static OffsetDateTime forDateString(const __FlashStringHelper* dateString) {
      // Copy the F() string into a buffer. Use strncpy_P() because ESP32 and
      // ESP8266 do not have strlcpy_P().
      char buffer[kDateStringLength + 2];
      strncpy_P(buffer, (const char*) dateString, sizeof(buffer));
      buffer[kDateStringLength + 1] = 0;

      // check if the original F() was too long
      size_t len = strlen(buffer);
      if (len > kDateStringLength) {
        return OffsetDateTime().setError();
      }

      return forDateString(buffer);
    }

    /** Constructor. All internal fields are left in an undefined state. */
    explicit OffsetDateTime() {}

    /** Return true if any component indicates an error condition. */
    bool isError() const {
      return mLocalDate.isError() || mLocalTime.isError();
    }

    /** Return the 2 digit year from year 2000. */
    int8_t yearShort() const { return mLocalDate.yearShort(); }

    /** Set the 2 digit year from year 2000. */
    void yearShort(int8_t yearShort) { mLocalDate.yearShort(yearShort); }

    /** Return the year. */
    int16_t year() const { return mLocalDate.year(); }

    /** Set the year. */
    void year(int16_t year) { mLocalDate.year(year); }

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

    /** Return the offset zone of the OffsetDateTime. */
    const UtcOffset& utcOffset() const { return mUtcOffset; }

    /** Return the offset zone of the OffsetDateTime. */
    UtcOffset& utcOffset() { return mUtcOffset; }

    /** Set the offset zone. */
    void utcOffset(const UtcOffset& utcOffset) { mUtcOffset = utcOffset; }

    /**
     * Create a OffsetDateTime in a different offset zone code (with the same
     * epochSeconds).
     */
    OffsetDateTime convertToUtcOffset(const UtcOffset& utcOffset) const {
      acetime_t epochSeconds = toEpochSeconds();
      return OffsetDateTime::forEpochSeconds(epochSeconds, utcOffset);
    }

    /**
     * Print OffsetDateTime to 'printer' in ISO 8601 format. Does not implement
     * Printable to avoid memory cost of a vtable pointer.
     */
    void printTo(Print& printer) const;

    /** Increment the year by one, wrapping from 99 to 0. */
    void incrementYear() {
      mLocalDate.incrementYear();
    }

    /** Increment the year by one, wrapping from 12 to 1. */
    void incrementMonth() {
      mLocalDate.incrementMonth();
    }

    /** Increment the day by one, wrapping from 31 to 1. */
    void incrementDay() {
      mLocalDate.incrementDay();
    }

    /** Increment the hour by one, wrapping from 23 to 0. */
    void incrementHour() {
      mLocalTime.incrementHour();
    }

    /** Increment the minute by one, wrapping from 59 to 0. */
    void incrementMinute() {
      mLocalTime.incrementMinute();
    }

    /**
     * Return number of whole days since AceTime epoch (2000-01-01 00:00:00Z),
     * taking into account the offset zone.
     */
    acetime_t toEpochDays() const {
      if (isError()) return LocalDate::kInvalidEpochDays;

      acetime_t epochDays = mLocalDate.toEpochDays();

      // Increment or decrement the day count depending on the offset zone.
      acetime_t utcOffset = mLocalTime.toSeconds() - mUtcOffset.toSeconds();
      if (utcOffset >= 86400) return epochDays + 1;
      if (utcOffset < 0) return epochDays - 1;
      return epochDays;
    }

    /** Return the number of days since Unix epoch (1970-01-01 00:00:00). */
    acetime_t toUnixDays() const {
      if (isError()) return LocalDate::kInvalidEpochDays;
      return toEpochDays() + LocalDate::kDaysSinceUnixEpoch;
    }

    /**
     * Return seconds since AceTime epoch (2000-01-01 00:00:00Z), taking into
     * account the offset zone.
     */
    acetime_t toEpochSeconds() const {
      if (isError()) return LocalDate::kInvalidEpochSeconds;

      acetime_t epochDays = mLocalDate.toEpochDays();
      acetime_t utcOffset = mLocalTime.toSeconds() - mUtcOffset.toSeconds();
      return epochDays * (acetime_t) 86400 + utcOffset;
    }

    /**
     * Return the number of seconds from Unix epoch 1970-01-01 00:00:00Z.
     * It returns kInvalidEpochSeconds if isError() is true.
     *
     * Tip: You can use the command 'date +%s -d {iso8601date}' on a Unix box to
     * print the unix seconds.
     */
    acetime_t toUnixSeconds() const {
      if (isError()) return LocalDate::kInvalidEpochSeconds;
      return toEpochSeconds() + LocalDate::kSecondsSinceUnixEpoch;
    }

    /**
     * Compare this OffsetDateTime with another OffsetDateTime, and return (<0,
     * 0, >0) according to whether the epochSeconds is (a<b, a==b, a>b). This
     * method can return 0 (equal) even if the operator==() returns false if
     * the two OffsetDateTime objects are in different offset zones.
     */
    int8_t compareTo(const OffsetDateTime& that) const {
      acetime_t thisSeconds = toEpochSeconds();
      acetime_t thatSeconds = that.toEpochSeconds();
      if (thisSeconds < thatSeconds) return -1;
      if (thisSeconds > thatSeconds) return 1;
      return 0;
    }

    /**
     * Mark the OffsetDateTime so that isError() returns true. Returns a
     * reference to (*this) so that an invalid OffsetDateTime can be returned in
     * a single statement like this: 'return OffsetDateTime().setError()'.
     */
    OffsetDateTime& setError() {
      mLocalDate.setError();
      mLocalTime.setError();
      return *this;
    }

  private:
    /** Expected length of an ISO 8601 date string. */
    static const uint8_t kDateStringLength = 25;

    friend bool operator==(const OffsetDateTime& a, const OffsetDateTime& b);
    friend bool operator!=(const OffsetDateTime& a, const OffsetDateTime& b);

    /**
     * Constructor using separated date, time, and time zone fields.
     *
     * @param year
     * @param month month with January=1, December=12
     * @param day day of month (1-31)
     * @param hour hour (0-23)
     * @param minute minute (0-59)
     * @param second second (0-59), does not support leap seconds
     * @param utcOffset Optional, default is UTC time zone. The time zone
     * offset in 15-minute increments from UTC. Using a UtcOffset object here
     * in the last component allows us to add an additional constructor that
     * accepts a millisecond component in the future.
     */
    explicit OffsetDateTime(int16_t year, uint8_t month, uint8_t day,
            uint8_t hour, uint8_t minute, uint8_t second,
            UtcOffset utcOffset = UtcOffset()):
        mLocalDate(year, month, day),
        mLocalTime(hour, minute, second),
        mUtcOffset(utcOffset) {}

    /** Extract the date time components from the given dateString. */
    OffsetDateTime& initFromDateString(const char* dateString);

    LocalDate mLocalDate;
    LocalTime mLocalTime;
    UtcOffset mUtcOffset; // offset from UTC
};

/**
 * Return true if two OffsetDateTime objects are equal in all components.
 * Optimized for small changes in the less signficant fields, such as 'second'
 * or 'minute'.
 */
inline bool operator==(const OffsetDateTime& a, const OffsetDateTime& b) {
  return a.mLocalDate == b.mLocalDate
      && a.mLocalTime == b.mLocalTime
      && a.mUtcOffset == b.mUtcOffset;
}

/** Return true if two OffsetDateTime objects are not equal. */
inline bool operator!=(const OffsetDateTime& a, const OffsetDateTime& b) {
  return ! (a == b);
}

}

#endif
