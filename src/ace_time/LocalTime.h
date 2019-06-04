#ifndef ACE_TIME_LOCAL_TIME_H
#define ACE_TIME_LOCAL_TIME_H

#include <stdint.h>
#include "common/common.h"

class Print;

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
    static const acetime_t kInvalidSeconds = INT32_MIN;

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
     * the string cannot be parsed, then returns LocalTime::forError().
     * However, the data validation on parsing is very weak and the behavior is
     * undefined for most invalid time strings.
     *
     * @param @timeString time in the form of "hh:mm:ss" (e.g. 12:34:56)
     */
    static LocalTime forTimeString(const char* timeString);

    /**
     * Factory method that returns an instance which indicates an error
     * condition. The isError() method will return true.
     */
    static LocalTime forError() {
      return LocalTime(kInvalidValue, kInvalidValue, kInvalidValue);
    }

    /** Default constructor does nothing. */
    explicit LocalTime() {}

    /**
     * Return true if any component is outside the normal time range of
     * 00:00:00 to 23:59:59. We add the exception that 24:00:00 is also
     * considered valid to allow AutoZoneSpecififier to support midnight
     * transitions from the TZ Database.
     */
    bool isError() const {
      if (mSecond >= 60) return true;
      if (mMinute >= 60) return true;
      if (mHour == 24) {
        return mSecond != 0 || mMinute != 0;
      }
      return mHour > 24;
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
     * Print LocalTime to 'printer' in ISO 8601 format.
     * This class does not implement the Printable interface to avoid
     * increasing the size of the object from the additional virtual function.
     */
    void printTo(Print& printer) const;

    // Use default copy constructor and assignment operator.
    LocalTime(const LocalTime&) = default;
    LocalTime& operator=(const LocalTime&) = default;

  private:
    friend class LocalDateTime; // constructor
    friend bool operator==(const LocalTime& a, const LocalTime& b);

    /** Expected length of an ISO 8601 time string "hh:mm:ss" */
    static const uint8_t kTimeStringLength = 8;

    /** A value that is invalid for all components. */
    static const uint8_t kInvalidValue = UINT8_MAX;

    /**
     * The internal version of forTimeString() that updates the reference to
     * the pointer to the string to the next unprocessed character. This allows
     * chaining to another forDateStringChainable() method.
     *
     * This method assumes that the dateString is sufficiently long.
     */
    static LocalTime forTimeStringChainable(const char*& timeString);

    explicit LocalTime(uint8_t hour, uint8_t minute, uint8_t second):
        mHour(hour),
        mMinute(minute),
        mSecond(second) {}

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
