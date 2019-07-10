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
 * offset from UTC (timeOffset). The dayOfWeek (1=Monday, 7=Sunday, per ISO
 * 8601) is calculated internally from the date fields.
 *
 * The year field is internally represented as int8_t offset from the year
 * 2000, so in theory it is valid from [1873, 2127] since -128 is used to
 * indicate an error condition. If the year is restricted to the range 00-99,
 * these fields map directly to the fields supported by the DS3231 RTC chip.
 * The "epoch" for this library is 2000-01-01T00:00:00Z and toEpochSeconds()
 * returns a int32_t number of seconds offset from that epoch.
 *
 * Parts of this class were inspired by the java.time.OffsetDateTime class of
 * Java 11
 * (https://docs.oracle.com/en/java/javase/11/docs/api/java.base/java/time/OffsetDateTime.html).
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
     * @param year [1873-2127] @param month month with January=1, December=12
     * @param day day of month [1-31] @param hour hour [0-23] @param minute
     * minute [0-59] @param second second [0-59], does not support leap seconds
     * @param timeOffset the time offset from UTC. Using TimeOffset in the last
     * component (instead of an int8_t or int16_t) allows us to overload an
     * additional constructor that accepts a millisecond component in the
     * future.
     */
    static OffsetDateTime forComponents(int16_t year, uint8_t month, uint8_t
        day, uint8_t hour, uint8_t minute, uint8_t second, TimeOffset
        timeOffset) {
      auto ldt = LocalDateTime::forComponents(year, month, day, hour, minute,
          second);
      return OffsetDateTime(ldt, timeOffset);
    }

    /**
     * Factory method. Create the various components of the OffsetDateTime from
     * the epochSeconds and its TimeOffset. Returns OffsetDateTime::forError()
     * if epochSeconds or timeOffset is an error.
     *
     * @param epochSeconds Number of seconds from AceTime epoch
     *    (2000-01-01 00:00:00). Use LocalDate::kInvalidEpochSeconds to define
     *    an invalid instance whose isError() returns true.
     * @param timeOffset time offset from UTC
     */
    static OffsetDateTime forEpochSeconds(acetime_t epochSeconds,
          TimeOffset timeOffset) {
      if (epochSeconds != LocalDate::kInvalidEpochSeconds) {
        epochSeconds += timeOffset.toSeconds();
      }
      auto ldt = LocalDateTime::forEpochSeconds(epochSeconds);
      return OffsetDateTime(ldt, timeOffset);
    }

    /**
     * Factory method that takes the number of seconds since Unix Epoch of
     * 1970-01-01. Similar to forEpochSeconds(), the seconds corresponding to
     * the partial day are truncated down towards the smallest whole day.
     */
    static OffsetDateTime forUnixSeconds(acetime_t unixSeconds,
          TimeOffset timeOffset) {
      acetime_t epochSeconds = (unixSeconds == LocalDate::kInvalidEpochSeconds)
          ? unixSeconds
          : unixSeconds - LocalDate::kSecondsSinceUnixEpoch;
      return forEpochSeconds(epochSeconds, timeOffset);
    }

    /**
     * Factory method. Create a OffsetDateTime from the ISO 8601 date string. If
     * the string cannot be parsed, then returns OffsetDateTime::forError().
     * Created for debugging purposes not for production use.
     *
     * The parsing validation is so weak that the behavior is undefined for
     * most invalid date/time strings. The range of valid dates is roughly from
     * 1872-01-01T00:00:00 to 2127-12-31T23:59:59. However, the UTC offset may
     * cause some of the dates on the two extreme ends invalid. The behavior is
     * undefined in those cases.
     *
     * @param dateString the date and time in ISO 8601 format
     *        "YYYY-MM-DDThh:mm:ss+/-hh:mm", but currently, the parser is very
     *        lenient. It cares mostly about the positional placement of the
     *        various components. It does not validate the separation
     *        characters like '-' or ':'. For example, both of the following
     *        will parse to the exactly same OffsetDateTime object:
     *        "2018-08-31T13:48:01-07:00" "2018/08/31 13#48#01-07#00"
     */
    static OffsetDateTime forDateString(const char* dateString);

    /**
     * Variant of forDateString() that updates the pointer to the next
     * unprocessed character. This allows chaining to another
     * forXxxStringChainable() method.
     *
     * This method assumes that the dateString is sufficiently long.
     */
    static OffsetDateTime forDateStringChainable(const char*& dateString);

    /**
     * Factory method. Create a OffsetDateTime from date string in flash memory
     * F() strings. Mostly for unit testing. Returns OffsetDateTime::forError()
     * if a parsing error occurs.
     */
    static OffsetDateTime forDateString(const __FlashStringHelper* dateString) {
      // Copy the F() string into a buffer. Use strncpy_P() because ESP32 and
      // ESP8266 do not have strlcpy_P(). We need +1 for the '\0' character and
      // another +1 to determine if the dateString is too long to fit.
      char buffer[kDateStringLength + 2];
      strncpy_P(buffer, (const char*) dateString, sizeof(buffer));
      buffer[kDateStringLength + 1] = 0;

      // check if the original F() was too long
      size_t len = strlen(buffer);
      if (len > kDateStringLength) {
        return forError();
      }

      return forDateString(buffer);
    }

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

    /** Return the single-byte year offset from year 2000. */
    int8_t yearTiny() const { return mLocalDateTime.yearTiny(); }

    /** Set the single-byte year offset from year 2000. */
    void yearTiny(int8_t yearTiny) { mLocalDateTime.yearTiny(yearTiny); }

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

    /** Return the day of the week, Monday=1, Sunday=7 (per ISO 8601). */
    uint8_t dayOfWeek() const { return mLocalDateTime.dayOfWeek(); }

    /** Return the offset zone of the OffsetDateTime. */
    TimeOffset timeOffset() const { return mTimeOffset; }

    /** Set the offset zone. */
    void timeOffset(TimeOffset timeOffset) { mTimeOffset = timeOffset; }

    /** Return the LocalDateTime. */
    const LocalDateTime& localDateTime() const { return mLocalDateTime; }

    /** Return the LocalDate. */
    const LocalDate& localDate() const { return mLocalDateTime.localDate(); }

    /** Return the LocalTime. */
    const LocalTime& localTime() const { return mLocalDateTime.localTime(); }

    /**
     * Create a OffsetDateTime in a different offset zone code (with the same
     * epochSeconds).
     */
    OffsetDateTime convertToTimeOffset(TimeOffset timeOffset) const {
      acetime_t epochSeconds = toEpochSeconds();
      return OffsetDateTime::forEpochSeconds(epochSeconds, timeOffset);
    }

    /**
     * Return number of whole days since AceTime epoch (2000-01-01 00:00:00Z),
     * taking into account the offset zone.
     */
    acetime_t toEpochDays() const {
      if (isError()) return LocalDate::kInvalidEpochDays;

      acetime_t epochDays = mLocalDateTime.localDate().toEpochDays();

      // Increment or decrement the day count depending on the time offset.
      acetime_t timeOffset = mLocalDateTime.localTime().toSeconds()
          - mTimeOffset.toSeconds();
      if (timeOffset >= 86400) return epochDays + 1;
      if (timeOffset < 0) return epochDays - 1;
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
      return mLocalDateTime.toEpochSeconds() - mTimeOffset.toSeconds();
    }

    /**
     * Return the number of seconds from Unix epoch 1970-01-01 00:00:00Z.
     * It returns kInvalidEpochSeconds if isError() is true.
     *
     * Tip: You can use the command 'date +%s -d {iso8601date}' on a Unix box to
     * convert an ISO8601 date to the unix seconds.
     */
    acetime_t toUnixSeconds() const {
      if (isError()) return LocalDate::kInvalidEpochSeconds;
      return toEpochSeconds() + LocalDate::kSecondsSinceUnixEpoch;
    }

    /**
     * Compare this OffsetDateTime with another OffsetDateTime, and return (<0,
     * 0, >0) according to whether the epochSeconds is (a<b, a==b, a>b). This
     * method can return 0 (equal) even if the operator==() returns false if
     * the two OffsetDateTime objects are using different time offsets.
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
