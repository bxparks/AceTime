#ifndef ACE_TIME_LOCAL_TIME_H
#define ACE_TIME_LOCAL_TIME_H

#include <stdint.h>

namespace ace_time {

/**
 * The time (hour, minute, second) fields representing the time without
 * regards to the day or the time zone.
 *
 * Parts of this class were inspired by the java.time.LocalTime class of Java 8
 * (https://docs.oracle.com/javase/8/docs/api/java/time/LocalTime.html).
 */
class LocalTime {
  public:
    /**
     * Factory method using separated date, time, and time zone fields. The
     * dayOfWeek will be lazily evaluated.
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
     * the number of seconds from midnight.
     *
     * @param seconds number of seconds from midnight, (0-86399)
     */
    static LocalTime forSeconds(uint32_t seconds)  {
      uint8_t second = seconds % 60;
      uint16_t minutes = seconds / 60;
      uint8_t minute = minutes % 60;
      uint8_t hour = minutes / 60;

      return LocalTime(hour, minute, second);
    }

    /**
     * Factory method. Create a LocalTime from the ISO 8601 time string. If
     * the string cannot be parsed, then isError() on the constructed object
     * returns true.
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
     * Mark the LocalTime so that isError() returns true. Returns a
     * reference to (*this) so that an invalid OffsetDateTime can be returned in
     * a single statement like this: 'return OffsetDateTime().setError()'.
     */
    LocalTime& setError() {
      // We use the 'second' field to represent an error condition because it is
      // the first field checked by operator==(), so will provide the fastest
      // detection of the transition from isError() to a valid OffsetDateTime.
      // All other fields set to 0 to avoid compiler warnings about
      // uninitialized member variables.
      mHour = 0;
      mMinute = 0;
      mSecond = 255;
      return *this;
    }

    /** Return the hour. */
    uint8_t hour() const { return mHour; }

    /** Return the minute. */
    uint8_t minute() const { return mMinute; }

    /** Return the second. */
    uint8_t second() const { return mSecond; }

    /** Return the number of seconds since midnight. */
    uint32_t toSeconds() const {
      return ((mHour * (uint16_t) 60) + mMinute)
          * (uint32_t) 60 + mSecond;
    }

    /**
     * Compare this LocalTime with that LocalTime, and return (<0, 0, >0)
     * according to whether (this<that, this==that, this>that).
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
    friend bool operator==(const LocalTime& a, const LocalTime& b);
    friend bool operator!=(const LocalTime& a, const LocalTime& b);

    /** Expected length of an ISO 8601 time string "hh:mm:ss" */
    static const uint8_t kTimeStringLength = 8;

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
