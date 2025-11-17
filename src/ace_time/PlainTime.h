/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#ifndef ACE_TIME_PLAIN_TIME_H
#define ACE_TIME_PLAIN_TIME_H

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
 * Parts of this class were inspired by the java.time.PlainTime class of Java
 * 11
 * (https://docs.oracle.com/en/java/javase/11/docs/api/java.base/java/time/PlainTime.html).
 *
 * The 'resolved' parameter was originally called 'fold' as inspired by the
 * datetime package in Python 3.6, but renamed to 'resolved' because the Python
 * mechanism turned out to be too confusing and not useful enough. This
 * parameter is arguably better suited to be in OffsetDateTime or ZonedDateTime,
 * but placing this field in this class reduces the memory size of
 * OffsetDateTime or ZoneDateTime due to struct alignment.
 */
class PlainTime {
  public:
    /** An invalid seconds marker that indicates isError() true. */
    static const int32_t kInvalidSeconds = INT32_MIN;

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
    static PlainTime forComponents(
        uint8_t hour, uint8_t minute, uint8_t second) {
      return PlainTime(hour, minute, second);
    }

    /**
     * Factory method. Create the various components of the PlainTime from
     * the number of seconds from midnight. If kInvalidSeconds is given,
     * the isError() condition is set to be true. The behavior is undefined
     * if seconds is greater than 86399.
     *
     * @param seconds number of seconds from midnight, (0-86399)
     */
    static PlainTime forSeconds(acetime_t seconds) {
      uint8_t second, minute, hour;
      Resolved resolved;

      if (seconds == kInvalidSeconds) {
        second = minute = hour = kInvalidValue; // causes isError() to be true
        resolved = Resolved::kError;
      } else {
        second = seconds % 60;
        uint16_t minutes = seconds / 60;
        minute = minutes % 60;
        hour = minutes / 60;
        resolved = Resolved::kUnique;
      }

      // Return a single object to allow return value optimization.
      return PlainTime(hour, minute, second, resolved);
    }

    /**
     * Factory method. Create a PlainTime from the ISO 8601 time string. If
     * the string cannot be parsed, then returns PlainTime::forError().
     * However, the data validation on parsing is very weak and the behavior is
     * undefined for most invalid time strings.
     *
     * @param @timeString time in the form of "hh:mm:ss" (e.g. 12:34:56)
     */
    static PlainTime forTimeString(const char* timeString);

    /**
     * Variant of forTimeString() that updates the pointer to the next
     * unprocessed character. This allows chaining to another
     * forXxxStringChainable() method.
     *
     * This method assumes that the dateString is sufficiently long.
     */
    static PlainTime forTimeStringChainable(const char*& timeString);

    /**
     * Factory method that returns an instance which indicates an error
     * condition. The isError() method will return true.
     */
    static PlainTime forError() {
      return PlainTime(
        kInvalidValue, kInvalidValue, kInvalidValue, Resolved::kError
      );
    }

    /** Default constructor does nothing. */
    explicit PlainTime() {}

    /**
     * Return true if any component is outside the normal time range of 00:00:00
     * to 23:59:59. We add the exception that 24:00:00 is also considered valid
     * to allow ExtendedZoneProcessor (and maybe BasicZoneProcessor) to support
     * midnight transitions from the TZ Database.
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
    void minute(uint8_t minute) { mMinute = minute; }

    /** Return the second. */
    uint8_t second() const { return mSecond; }

    /** Set the second. */
    void second(uint8_t second) { mSecond = second; }

    /** Return the resolved. */
    Resolved resolved() const { return mResolved; }

    /** Set the resolved. */
    void resolved(Resolved resolved) { mResolved = resolved; }

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
     * Compare 'this' PlainTime with 'that' PlainTime, and return (<0, 0, >0)
     * according to whether 'this' occurs (before, same as, after) 'that'.
     * The 'resolved' parameter is ignored.
     *
     * If either this->isError() or that.isError() is true, the behavior is
     * undefined.
     */
    int8_t compareTo(const PlainTime& that) const {
      if (mHour < that.mHour) return -1;
      if (mHour > that.mHour) return 1;
      if (mMinute < that.mMinute) return -1;
      if (mMinute > that.mMinute) return 1;
      if (mSecond < that.mSecond) return -1;
      if (mSecond > that.mSecond) return 1;
      return 0;
    }

    /**
     * Print PlainTime to 'printer' in ISO 8601 format.
     * This class does not implement the Printable interface to avoid
     * increasing the size of the object from the additional virtual function.
     */
    void printTo(Print& printer) const;

    // Use default copy constructor and assignment operator.
    PlainTime(const PlainTime&) = default;
    PlainTime& operator=(const PlainTime&) = default;

  private:
    friend bool operator==(const PlainTime& a, const PlainTime& b);

    /** Expected length of an ISO 8601 time string "hh:mm:ss" */
    static const uint8_t kTimeStringLength = 8;

    /** A value that is invalid for all components. */
    static const uint8_t kInvalidValue = UINT8_MAX;

    /** Constructor that sets the components. */
    explicit PlainTime(
        uint8_t hour,
        uint8_t minute,
        uint8_t second,
        Resolved resolved = Resolved::kUnique
    ):
        mHour(hour),
        mMinute(minute),
        mSecond(second),
        mResolved(resolved)
    {}

  private:
    uint8_t mHour; // [0, 23]
    uint8_t mMinute; // [0, 59]
    uint8_t mSecond; // [0, 59]
    Resolved mResolved;
};

/** Return true if two PlainTime objects are equal. The resolved is ignored. */
inline bool operator==(const PlainTime& a, const PlainTime& b) {
  return a.mSecond == b.mSecond
      && a.mMinute == b.mMinute
      && a.mHour == b.mHour;
}

/** Return true if two PlainTime objects are not equal. The resolved is ignored. */
inline bool operator!=(const PlainTime& a, const PlainTime& b) {
  return ! (a == b);
}

}

#endif
