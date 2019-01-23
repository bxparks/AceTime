#ifndef ACE_TIME_LOCAL_TIME_H
#define ACE_TIME_LOCAL_TIME_H

#include <stdint.h>
#include "common/common.h"
#include "common/util.h"

namespace ace_time {

/**
 * The time (hour, minute, second) fields representing the time without regards
 * to the day or the time zone. The valid range is 00:00:00 to 23:59:59.
 * Trying to create an instance outside of this range causes the isError()
 * method to return true, and toSeconds() returns kInvalidSeconds.
 *
 * Parts of this class were inspired by the java.time.LocalTime class of Java 8
 * (https://docs.oracle.com/javase/8/docs/api/java/time/LocalTime.html).
 */
class LocalTime {
  public:
    /** An invalid seconds marker that indicates isError() true. */
    static const acetime_t kInvalidSeconds = INT32_MAX;

    /**
     * Factory method using separated date, time, and time zone fields. The
     * dayOfWeek will be lazily evaluated. No data validation is performed on
     * the fields on construction, but if any field is out of range, then
     * isError() will return true.
     *
     * @param hour hour (0-23)
     * @param minute minute (0-59)
     * @param second second (0-59), does not support leap seconds
     */
    static LocalTime forComponents(uint8_t hour, uint8_t minute,
        uint8_t second) {
      return LocalTime(hour, minute, second);
    }

    /**
     * Factory method. Create the various components of the LocalTime from
     * the number of seconds from midnight. If kInvalidSeconds is given,
     * the isError() condition is set to be true. The behavior is undefined
     * if seconds is greater than 86399.
     *
     * @param seconds number of seconds from midnight, (0-86399)
     */
    static LocalTime forSeconds(acetime_t seconds)  {
      uint8_t second, minute, hour;

      if (seconds == kInvalidSeconds) {
        second = minute = hour = kInvalidValue; // causes isError() to be true
      } else {
        second = seconds % 60;
        uint16_t minutes = seconds / 60;
        minute = minutes % 60;
        hour = minutes / 60;
      }

      // Return a single object to allow return value optimization.
      return LocalTime(hour, minute, second);
    }

    /**
     * Factory method. Create a LocalTime from the ISO 8601 time string. If
     * the string cannot be parsed, then isError() on the constructed object
     * returns true. However, the data validation on parsing is very weak and
     * the behavior is undefined for most invalid time strings.
     */
    static LocalTime forTimeString(const char* timeString) {
      return LocalTime().initFromTimeString(timeString);
    }

    /** Default constructor does nothing. */
    explicit LocalTime() {}

    /** Return true if any component indicates an error condition. */
    bool isError() const {
      // Warning: Don't change the order of the following boolean conditionals
      // without changing setError().
      return mSecond >= 60
          || mMinute >= 60
          || mHour >= 24;
    }

    /**
     * Mark the LocalTime so that isError() returns true. Returns a reference
     * to (*this) so that an invalid OffsetDateTime can be returned in a single
     * statement like this: 'return OffsetDateTime().setError()'.
     */
    LocalTime& setError() {
      mHour = mMinute = mSecond = kInvalidValue;
      return *this;
    }

    /** Return the hour. */
    uint8_t hour() const { return mHour; }

    /** Set the hour. */
    void hour(uint8_t hour) { mHour = hour; }

    /** Return the minute. */
    uint8_t minute() const { return mMinute; }

    /** Set the minute. */
    void minute(uint8_t month) { mMinute = month; }

    /** Return the second. */
    uint8_t second() const { return mSecond; }

    /** Set the second. */
    void second(uint8_t second) { mSecond = second; }

    /**
     * Return the number of seconds since midnight.
     * Return kInvalidSeconds if isError() is true.
     */
    acetime_t toSeconds() const {
      if (isError()) {
        return kInvalidSeconds;
      } else {
        return ((mHour * (int16_t) 60) + mMinute)
            * (int32_t) 60 + mSecond;
      }
    }

    /** Increment the hour by one, wrapping from 23 to 0. */
    void incrementHour() {
      common::incrementMod(mHour, (uint8_t) 24);
    }

    /** Increment the minute by one, wrapping from 59 to 0. */
    void incrementMinute() {
      common::incrementMod(mMinute, (uint8_t) 60);
    }

    /**
     * Compare this LocalTime with that LocalTime, and return (<0, 0, >0)
     * according to whether (this<that, this==that, this>that). The behavior
     * is undefined if isError() is true.
     */
    int8_t compareTo(const LocalTime& that) const {
      if (mHour < that.mHour) return -1;
      if (mHour > that.mHour) return 1;
      if (mMinute < that.mMinute) return -1;
      if (mMinute > that.mMinute) return 1;
      if (mSecond < that.mSecond) return -1;
      if (mSecond > that.mSecond) return 1;
      return 0;
    }

    /**
     * Print LocalTime to 'printer' in ISO 8601 format. Does not implement
     * Printable to avoid memory cost of a vtable pointer.
     */
    void printTo(Print& printer) const;


  private:
    friend class LocalDateTime;
    friend class OffsetDateTime;
    friend bool operator==(const LocalTime& a, const LocalTime& b);

    /** Expected length of an ISO 8601 time string "hh:mm:ss" */
    static const uint8_t kTimeStringLength = 8;

    static const uint8_t kInvalidValue = UINT8_MAX;

    explicit LocalTime(uint8_t hour, uint8_t minute, uint8_t second):
        mHour(hour),
        mMinute(minute),
        mSecond(second) {}

    /** Extract the time components from the given timeString. */
    LocalTime& initFromTimeString(const char* timeString);

    uint8_t mHour; // [0, 23]
    uint8_t mMinute; // [0, 59]
    uint8_t mSecond; // [0, 59]
};

/** Return true if two LocalTime objects are equal. */
inline bool operator==(const LocalTime& a, const LocalTime& b) {
  return a.mSecond == b.mSecond
      && a.mMinute == b.mMinute
      && a.mHour == b.mHour;
}

/** Return true if two LocalTime objects are not equal. */
inline bool operator!=(const LocalTime& a, const LocalTime& b) {
  return ! (a == b);
}

}

#endif
