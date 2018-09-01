#ifndef ACE_TIME_DATE_TIME_H
#define ACE_TIME_DATE_TIME_H

#include <string.h> // strlcpy()
#include <stdint.h>
#include <Print.h> // Print
#include "Flash.h"
#include "Util.h"
#include "TimeZone.h"

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
 * Some of the design of this class was inspired by the DateTime class of
 * http://www.joda.org.
 */
class DateTime {
  public:
    /**
     * Number of seconds from Unix epoch (1970-01-01 00:00:00Z) to
     * the AceTime epoch (2000-01-01 00:00:00Z).
     */
    static const uint32_t kSecondsSinceUnixEpoch = 946684800;

    /**
     * Number of days between the Julian calendar epoch (4713 BC 01-01) and the
     * AceTime epoch (2000-01-01).
     */
    static const uint32_t kDaysSinceJulianEpoch = 2451545;

    /** Base year of epoch. */
    static const uint16_t kEpochYear = 2000;

    /** Constructor. */
    explicit DateTime() {}

    /**
     * Constructor using separated date, time, and time zone fields. The
     * dayOfWeek will be internally generated.
     *
     * @param timeZone Optional. The time zone offset in 15-minute increments
     * from UTC. Using a TimeZone object here in the last component allows us to
     * add an additional constructor that accepts a millisecond component in the
     * future.
     */
    explicit DateTime(uint8_t year, uint8_t month, uint8_t day, uint8_t hour,
            uint8_t minute, uint8_t second, TimeZone timeZone = TimeZone()):
        mYear(year), mMonth(month), mDay(day), mHour(hour), mMinute(minute),
        mSecond(second), mTimeZone(timeZone), mDayOfWeek(0) {
    }

    /**
     * Constructor. Create the various components of the local DateTime as seen
     * from the given time zone. If tzCode is negative, then (secondsSinceEpoch
     * >= (|tzCode| * 900)) must be true. Otherwise, the local time will be in
     * the year 1999, which cannot be represented by a 2-digit year beginning
     * with the year 2000. The dayOfWeek will be calculated internally.
     *
     * @param secondsSinceEpoch number of seconds from AceTime epoch
     *    (2000-01-01 00:00:00Z).
     * @param timeZone time zone
     *
     * See https://en.wikipedia.org/wiki/Julian_day.
     */
    explicit DateTime(uint32_t secondsSinceEpoch,
        TimeZone timeZone = TimeZone()) {
      mTimeZone = timeZone;
      secondsSinceEpoch += timeZone.toSeconds();

      uint32_t daysSinceEpoch = secondsSinceEpoch / 86400;
      fillFromDaysSinceEpoch(daysSinceEpoch);

      uint32_t seconds = secondsSinceEpoch % 86400;
      mSecond = seconds % 60;
      seconds /= 60;
      mMinute = seconds % 60;
      seconds /= 60;
      mHour = seconds;
    }

    /**
     * Constructor. Create a DateTime from the ISO8601 date string. If the
     * string cannot be parsed, then isError() on the constructed object returns
     * true.
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
    explicit DateTime(const char* dateString) {
      init(dateString);
    }

    /** Constructor from flash memory F() strings. Mostly for unit testing. */
    explicit DateTime(const __FlashStringHelper* dateString) {

      // Copy the F() string into a buffer. Use strncpy_P() because ESP32 and
      // ESP8266 do not have strlcpy_P().
      char buffer[kDateStringLength + 2];
      strncpy_P(buffer, (const char*) dateString, sizeof(buffer));
      buffer[kDateStringLength + 1] = 0;

      // check if the original F() was too long
      size_t len = strlen(buffer);
      if (len > kDateStringLength) {
        setError();
        return;
      }

      init(buffer);
    }

    /** Return true if any component indicates an error condition. */
    bool isError() const {
      return mMonth < 1 || mMonth > 12
          || mDay < 1 || mDay > 31
          || mHour >= 24
          || mMinute >= 60
          || mSecond >= 60;
    }

    uint8_t year() const { return mYear; }

    void year(uint8_t year) {
      mYear = year;
      mDayOfWeek = 0;
    }

    uint8_t month() const { return mMonth; }

    void month(uint8_t month) {
      mMonth = month;
      mDayOfWeek = 0;
    }

    uint8_t day() const { return mDay; }

    void day(uint8_t day) {
      mDay = day;
      mDayOfWeek = 0;
    }

    uint8_t hour() const { return mHour; }

    void hour(uint8_t hour) {
      mHour = hour;
    }

    uint8_t minute() const { return mMinute; }

    void minute(uint8_t month) {
      mMinute = month;
    }

    uint8_t second() const { return mSecond; }

    void second(uint8_t second) {
      mSecond = second;
    }

    /** The dayOfWeek is calculated lazily and cached internally. */
    uint8_t dayOfWeek() const {
      if (mDayOfWeek == 0) {
        mDayOfWeek = calculateDayOfWeek();
      }
      return mDayOfWeek;
    }

    const TimeZone& timeZone() const { return mTimeZone; }

    TimeZone& timeZone() { return mTimeZone; }

    void timeZone(const TimeZone& timeZone) { mTimeZone = timeZone; }

    /**
     * Create a DateTime in a different time zone code (with the same
     * secondsSinceEpoch).
     */
    DateTime convertToTimeZone(TimeZone timeZone) const {
      uint32_t secondsSinceEpoch = toSecondsSinceEpoch();
      return DateTime(secondsSinceEpoch, timeZone);
    }

    /** Print DateTime to 'printer'. */
    void printTo(Print& printer) const;

    void incrementHour() {
      incrementMod(mHour, (uint8_t) 24);
      mDayOfWeek = 0;
    }

    void incrementMinute() {
      incrementMod(mMinute, (uint8_t) 60);
      mDayOfWeek = 0;
    }

    void incrementYear() {
      incrementMod(mYear, (uint8_t) 100);
      mDayOfWeek = 0;
    }

    void incrementMonth() {
      incrementMod(mMonth, (uint8_t) 12, (uint8_t) 1);
      mDayOfWeek = 0;
    }

    void incrementDay() {
      incrementMod(mDay, (uint8_t) 31, (uint8_t) 1);
      mDayOfWeek = 0;
    }

    /**
     * Return number of whole days since AceTime epoch, taking into account the
     * time zone.
     */
    uint32_t toDaysSinceEpoch() const {
      uint32_t daysSinceEpoch = toDaysSinceEpochIgnoringTimeZone();
      int32_t utcOffset = ((mHour * 60) + mMinute) * (uint32_t) 60 + mSecond;
      utcOffset -= mTimeZone.toSeconds();

      // Increment or decrement the day count depending on the time zone.
      if (utcOffset >= 86400) {
        return daysSinceEpoch + 1;
      } else if (utcOffset < 0) {
        return daysSinceEpoch - 1;
      } else {
        return daysSinceEpoch;
      }
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
      uint32_t daysSinceEpoch = toDaysSinceEpochIgnoringTimeZone();
      int32_t utcOffset = ((mHour * 60) + mMinute) * (uint32_t) 60 + mSecond;
      utcOffset -= mTimeZone.toSeconds();
      return daysSinceEpoch * (uint32_t) 86400 + utcOffset;
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
      return toSecondsSinceEpoch() + kSecondsSinceUnixEpoch;
    }

    /**
     * Compare this DateTime with another DateTime, and return (<0, 0, >0)
     * according to whether the secondsSinceEpoch() is (a<b, a==b, a>b). The
     * dayOfWeek field is ignored but the time zone is used.
     */
    int8_t compareTo(const DateTime& that) const {
      uint32_t thisSeconds = toSecondsSinceEpoch();
      uint32_t thatSeconds = that.toSecondsSinceEpoch();
      if (thisSeconds < thatSeconds) {
        return -1;
      } else if (thisSeconds == thatSeconds) {
        return 0;
      } else {
        return 1;
      }
    }

  private:
    /** Expected length of an ISO8601 date string. */
    static const uint8_t kDateStringLength = 25;

    friend bool operator==(const DateTime& a, const DateTime& b);
    friend bool operator!=(const DateTime& a, const DateTime& b);

    /** Mark the DateTime so that isError() returns true. */
    void setError() {
      // Setting mMonth = 0 causes isError() to return the quickest
      mMonth = 0;
    }

    /** Extract the date time components from the given dateString. */
    void init(const char* dateString);

    /**
     * Calculate the correct day of week from the internal fields. If any of the
     * component fields (year, month, day, hour, minute, second) are manually
     * changed, this method must be called to update the dayOfWeek. The
     * dayOfWeek does not depend on the time zone.
     */
    uint8_t calculateDayOfWeek() const {
      uint32_t daysSinceEpoch = toDaysSinceEpochIgnoringTimeZone();
      // 2000-01-01 is a Saturday (7)
      return (daysSinceEpoch + 6) % 7 + 1;
    }

    /**
     * Return number of days since AceTime epoch (2000-01-01 00:00:00Z),
     * ignoring the tzCode.
     *
     * Uses Julian days which normally start at 12:00:00. But this method returns
     * the delta number of days since 00:00:00, so we can interpret the Gregorian
     * calendar day to start at 00:00:00.
     *
     * See https://en.wikipedia.org/wiki/Julian_day
     */
    uint32_t toDaysSinceEpochIgnoringTimeZone() const {
      // From wiki article:
      //
      // JDN = (1461 x (Y + 4800 + (M - 14)/12))/4
      //     + (367 x (M - 2 - 12 x ((M - 14)/12)))/12
      //     - (3 x ((Y + 4900 + (M - 14)/12)/100))/4
      //     + D - 32075
      // JDN2000 = JDN - 2451545
      //
      // It looks like the formula needs to be done using signed integers
      // because it depends on the modulo operation (%) to truncate towards 0
      // for negative numbers.

      int8_t mm = (mMonth - 14)/12;
      int16_t yy = mYear + kEpochYear;
      int32_t jdn = ((int32_t) 1461 * (yy + 4800 + mm))/4
          + (367 * (mMonth - 2 - 12 * mm))/12
          - (3 * ((yy + 4900 + mm)/100))/4
          + mDay - 32075;
      return jdn - kDaysSinceJulianEpoch;
    }


    /**
     * Fill in the (year, month, day, dayOfWeek) fields from daysSinceEpoch.
     * The (hour, minute, second, timeZone) fields are left untouched.
     *
     * See https://en.wikipedia.org/wiki/Julian_day.
     */
    void fillFromDaysSinceEpoch(uint32_t daysSinceEpoch) {
      uint32_t J = daysSinceEpoch + kDaysSinceJulianEpoch;
      uint32_t f = J + 1401 + (((4 * J + 274277 ) / 146097) * 3) / 4 - 38;
      uint32_t e = 4 * f + 3;
      uint32_t g = e % 1461 / 4;
      uint32_t h = 5 * g + 2;
      mDay = (h % 153) / 5 + 1;
      mMonth = (h / 153 + 2) % 12 + 1;
      mYear = (e / 1461) - 4716 + (12 + 2 - mMonth) / 12 - kEpochYear;

      // 2000-01-01 is Saturday (7)
      mDayOfWeek = (daysSinceEpoch + 6) % 7 + 1;
    }

    uint8_t mYear; // [00, 99], year - 2000
    uint8_t mMonth; // [1, 12]
    uint8_t mDay; // [1, 31]
    uint8_t mHour; // [0, 23]
    uint8_t mMinute; // [0, 59]
    uint8_t mSecond; // [0, 59]
    TimeZone mTimeZone; // offset from UTC in 15 minute increments
    mutable uint8_t mDayOfWeek; // (1=Sunday, 7=Saturday)
};

/**
 * Return true if two DateTime objects are equal. Optimized for small changes in
 * the less signficant fields, such as 'second' or 'minute'. The dayOfWeek field
 * must also match, unlike compareTo() where it is ignored.
 */
inline bool operator==(const DateTime& a, const DateTime& b) {
  return a.mSecond == b.mSecond
      && a.mMinute == b.mMinute
      && a.mHour == b.mHour
      && a.dayOfWeek() == b.dayOfWeek()
      && a.mDay == b.mDay
      && a.mMonth == b.mMonth
      && a.mYear == b.mYear
      && a.mTimeZone == b.mTimeZone;
}

/** Return true if two DateTime objects are not equal. */
inline bool operator!=(const DateTime& a, const DateTime& b) {
  return ! (a == b);
}

}

#endif
