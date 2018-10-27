#ifndef ACE_TIME_OFFSET_DATE_TIME_H
#define ACE_TIME_OFFSET_DATE_TIME_H

#include <stdint.h>
#include "ZoneOffset.h"
#include "LocalDate.h"
#include "LocalTime.h"

class Print;

namespace ace_time {

/**
 * The date (year, month, day) and time (hour, minute, second) fields
 * representing the time with an offset from UTC.
 *
 * The year field is internally represented as a 2 digit number from [2000,
 * 2099]. Therefore, the "epoch" for this library is 2000-01-01 00:00:00Z.
 * These fields map directly to the fields supported by the DS3231 RTC chip.
 *
 * The dayOfWeek (1=Monday, 7=Sunday, per ISO 8601) is calculated
 * internally from the date. The value is calculated lazily and cached
 * internally. If any components are changed, then the cache is invalidated and
 * the dayOfWeek is lazily recalculated.
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
     * Number of seconds from Unix epoch (1970-01-01 00:00:00Z) to
     * the AceTime epoch (2000-01-01 00:00:00Z).
     */
    static const uint32_t kSecondsSinceUnixEpoch = 946684800;

    /** Base year of epoch. */
    static const uint16_t kEpochYear = 2000;

    /**
     * Factory method using separated date, time, and time zone fields. The
     * dayOfWeek will be lazily evaluated.
     *
     * @param year last 2 digits of the year from year 2000
     * @param month month with January=1, December=12
     * @param day day of month (1-31)
     * @param hour hour (0-23)
     * @param minute minute (0-59)
     * @param second second (0-59), does not support leap seconds
     * @param zoneOffset Optional, default is UTC time zone. The time zone
     * offset in 15-minute increments from UTC. Using a ZoneOffset object here
     * in the last component allows us to add an additional constructor that
     * accepts a millisecond component in the future.
     */
    static OffsetDateTime forComponents(uint8_t year, uint8_t month,
        uint8_t day, uint8_t hour, uint8_t minute, uint8_t second,
        ZoneOffset zoneOffset = ZoneOffset()) {
      return OffsetDateTime(year, month, day, hour, minute, second, zoneOffset);
    }

    /**
     * Factory method. Create the various components of the OffsetDateTime from
     * the epochSeconds and its ZoneOffset. The dayOfWeek will be
     * calculated lazily.
     *
     * If ZoneOffset.offsetCode() is negative, then (epochSeconds >=
     * ZoneOffset::asSeconds() must be true. Otherwise, the local time will be
     * in the year 1999, which cannot be represented by a 2-digit year
     * beginning with the year 2000.
     *
     * @param epochSeconds Number of seconds from AceTime epoch
     *    (2000-01-01 00:00:00Z). A 0 value is a sentinel that is considerd to
     *    be an error, and causes isError() to return true.
     * @param zoneOffset Optional. Default is UTC time zone.
     *
     * See https://en.wikipedia.org/wiki/Julian_day.
     */
    static OffsetDateTime forEpochSeconds(uint32_t epochSeconds,
          ZoneOffset zoneOffset = ZoneOffset()) {
      OffsetDateTime dt;
      if (epochSeconds == 0) {
        return dt.setError();
      }

      dt.mZoneOffset = zoneOffset;

      epochSeconds += zoneOffset.asSeconds();
      uint32_t epochDays = epochSeconds / 86400;
      dt.mLocalDate = LocalDate::forEpochDays(epochDays);

      uint32_t seconds = epochSeconds % 86400;
      dt.mLocalTime = LocalTime::forSeconds(seconds);

      return dt;
    }

    /**
     * Factory method. Create a OffsetDateTime from the ISO 8601 date string. If
     * the string cannot be parsed, then isError() on the constructed object
     * returns true.
     *
     * The dateString is expected to be in ISO 8601 format
     * "YYYY-MM-DDThh:mm:ss+hh:mm", but currently, the parser is very lenient
     * and does not detect most errors. It cares mostly about the positional
     * placement of the various components. It does not validate the separation
     * characters like '-' or ':'. For example, both of the following will parse
     * to the exactly same OffsetDateTime object:
     * "2018-08-31T13:48:01-07:00"
     * "2018/08/31 13#48#01-07#00"
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
    uint8_t year() const { return mLocalDate.year(); }

    /** Set the 2 digit year from year 2000. */
    void year(uint8_t year) {
      mLocalDate.year(year);
      mDayOfWeek = 0;
    }

    /** Return the full year instead of just the last 2 digits. */
    uint16_t yearFull() const { return year() + kEpochYear; }

    /** Set the year given the full year. */
    void yearFull(uint16_t yearFull) {
      mLocalDate.yearFull(yearFull);
      mDayOfWeek = 0;
    }

    /** Return the month with January=1, December=12. */
    uint8_t month() const { return mLocalDate.month(); }

    /** Set the month. */
    void month(uint8_t month) {
      mLocalDate.month(month);
      mDayOfWeek = 0;
    }

    /** Return the day of the month. */
    uint8_t day() const { return mLocalDate.day(); }

    /** Set the day of the month. */
    void day(uint8_t day) {
      mLocalDate.day(day);
      mDayOfWeek = 0;
    }

    /** Return the hour. */
    uint8_t hour() const { return mLocalTime.hour(); }

    /** Set the hour. */
    void hour(uint8_t hour) {
      // Does not affect dayOfWeek.
      mLocalTime.hour(hour);
    }

    /** Return the minute. */
    uint8_t minute() const { return mLocalTime.minute(); }

    /** Set the minute. */
    void minute(uint8_t minute) {
      // Does not affect dayOfWeek.
      mLocalTime.minute(minute);
    }

    /** Return the second. */
    uint8_t second() const { return mLocalTime.second(); }

    /** Set the second. */
    void second(uint8_t second) {
      // Does not affect dayOfWeek.
      mLocalTime.second(second);
    }

    /**
     * Return the day of the week, Monday=1, Sunday=7 (per ISO 8601). The
     * dayOfWeek is calculated lazily and cached internally. Not thread-safe.
     */
    uint8_t dayOfWeek() const {
      if (mDayOfWeek == 0) {
        mDayOfWeek = calculateDayOfWeek();
      }
      return mDayOfWeek;
    }

    /** Return the offset zone of the OffsetDateTime. */
    const ZoneOffset& zoneOffset() const { return mZoneOffset; }

    /** Return the offset zone of the OffsetDateTime. */
    ZoneOffset& zoneOffset() { return mZoneOffset; }

    /** Set the offset zone. */
    void zoneOffset(const ZoneOffset& zoneOffset) {
      // Does not affect dayOfWeek.
      mZoneOffset = zoneOffset;
    }

    /**
     * Create a OffsetDateTime in a different offset zone code (with the same
     * epochSeconds).
     */
    OffsetDateTime convertToZoneOffset(const ZoneOffset& zoneOffset) const {
      uint32_t epochSeconds = toEpochSeconds();
      return OffsetDateTime::forEpochSeconds(epochSeconds, zoneOffset);
    }

    /**
     * Print OffsetDateTime to 'printer' in ISO 8601 format. Does not implement
     * Printable to avoid memory cost of a vtable pointer.
     */
    void printTo(Print& printer) const;

    /** Increment the year by one, wrapping from 99 to 0. */
    void incrementYear() {
      mLocalDate.incrementYear();
      mDayOfWeek = 0;
    }

    /** Increment the year by one, wrapping from 12 to 1. */
    void incrementMonth() {
      mLocalDate.incrementMonth();
      mDayOfWeek = 0;
    }

    /** Increment the day by one, wrapping from 31 to 1. */
    void incrementDay() {
      mLocalDate.incrementDay();
      mDayOfWeek = 0;
    }

    /** Increment the hour by one, wrapping from 23 to 0. */
    void incrementHour() {
      mLocalTime.incrementHour();
      mDayOfWeek = 0;
    }

    /** Increment the minute by one, wrapping from 59 to 0. */
    void incrementMinute() {
      mLocalTime.incrementMinute();
      mDayOfWeek = 0;
    }

    /**
     * Return number of whole days since AceTime epoch (2000-01-01 00:00:00Z),
     * taking into account the offset zone.
     */
    uint32_t toEpochDays() const {
      uint32_t epochDays = mLocalDate.toEpochDays();

      // Increment or decrement the day count depending on the offset zone.
      int32_t utcOffset = mLocalTime.toSeconds() - mZoneOffset.asSeconds();
      if (utcOffset >= 86400) return epochDays + 1;
      if (utcOffset < 0) return epochDays - 1;
      return epochDays;
    }

    /**
     * Return seconds since AceTime epoch (2000-01-01 00:00:00Z), taking into
     * account the offset zone. The return type is an unsigned 32-bit integer,
     * which can represent a range of 136 years. Since the year is stored as a 2
     * digit offset from the year 2000, the return type will be valid for all
     * OffsetDateTime values which can be stored in this class.
     *
     * Normally, Julian day starts at 12:00:00. We modify the formula given in
     * wiki page to start the Gregorian day at 00:00:00.
     *
     * See https://en.wikipedia.org/wiki/Julian_day
     */
    uint32_t toEpochSeconds() const {
      uint32_t epochDays = mLocalDate.toEpochDays();
      int32_t utcOffset = mLocalTime.toSeconds() - mZoneOffset.asSeconds();
      return epochDays * (uint32_t) 86400 + utcOffset;
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
      return toEpochSeconds() + kSecondsSinceUnixEpoch;
    }

    /**
     * Compare this OffsetDateTime with another OffsetDateTime, and return (<0,
     * 0, >0) according to whether the epochSeconds is (a<b, a==b, a>b).
     * The dayOfWeek field is ignored but the offset zone is used.  This method
     * can return 0 (equal) even if the operator==() returns false if the two
     * OffsetDateTime objects are in different offset zones.
     */
    int8_t compareTo(const OffsetDateTime& that) const {
      uint32_t thisSeconds = toEpochSeconds();
      uint32_t thatSeconds = that.toEpochSeconds();
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
      mDayOfWeek = 0;
      return *this;
    }

  private:
    /** Expected length of an ISO 8601 date string. */
    static const uint8_t kDateStringLength = 25;

    friend bool operator==(const OffsetDateTime& a, const OffsetDateTime& b);
    friend bool operator!=(const OffsetDateTime& a, const OffsetDateTime& b);

    /**
     * Constructor using separated date, time, and time zone fields. The
     * dayOfWeek will be internally generated.
     *
     * @param year last 2 digits of the year from year 2000
     * @param month month with January=1, December=12
     * @param day day of month (1-31)
     * @param hour hour (0-23)
     * @param minute minute (0-59)
     * @param second second (0-59), does not support leap seconds
     * @param zoneOffset Optional, default is UTC time zone. The time zone
     * offset in 15-minute increments from UTC. Using a ZoneOffset object here
     * in the last component allows us to add an additional constructor that
     * accepts a millisecond component in the future.
     */
    explicit OffsetDateTime(uint8_t year, uint8_t month, uint8_t day,
            uint8_t hour, uint8_t minute, uint8_t second,
            ZoneOffset zoneOffset = ZoneOffset()):
        mLocalDate(year, month, day),
        mLocalTime(hour, minute, second),
        mZoneOffset(zoneOffset),
        mDayOfWeek(0) {}

    /** Extract the date time components from the given dateString. */
    OffsetDateTime& initFromDateString(const char* dateString);

    /**
     * Calculate the correct day of week from the internal fields. If any of the
     * component fields (year, month, day, hour, minute, second) are manually
     * changed, this method must be called to update the dayOfWeek. The
     * dayOfWeek does not depend on the offset zone.
     */
    uint8_t calculateDayOfWeek() const {
      uint32_t epochDays = mLocalDate.toEpochDays();
      // 2000-01-01 is a Saturday (6)
      return (epochDays + 5) % 7 + 1;
    }

    LocalDate mLocalDate;
    LocalTime mLocalTime;
    ZoneOffset mZoneOffset; // offset from UTC
    mutable uint8_t mDayOfWeek; // (1=Monday, 7=Sunday)
};

/**
 * Return true if two OffsetDateTime objects are equal in all components.
 * Optimized for small changes in the less signficant fields, such as 'second'
 * or 'minute'. The dayOfWeek is a derived field so it is not explicitly used
 * to test equality. If all the other fields are identical, then the dayOfWeek
 * must also be equal.
 */
inline bool operator==(const OffsetDateTime& a, const OffsetDateTime& b) {
  return a.mLocalDate == b.mLocalDate
      && a.mLocalTime == b.mLocalTime
      && a.mZoneOffset == b.mZoneOffset;
}

/** Return true if two OffsetDateTime objects are not equal. */
inline bool operator!=(const OffsetDateTime& a, const OffsetDateTime& b) {
  return ! (a == b);
}

}

#endif
