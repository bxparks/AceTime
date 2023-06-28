/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#ifndef ACE_TIME_TIME_OFFSET_H
#define ACE_TIME_TIME_OFFSET_H

#include <stdint.h>

class Print;

namespace ace_time {

// These functions need to set the mSeconds and it seemed inefficient to
// go through the factory method and assignment operator, so I expose
// setMinutes() to them for efficiency.
class TimeOffset;
namespace time_offset_mutation {
void incrementHour(TimeOffset& offset);
void increment15Minutes(TimeOffset& offset);
}

/**
 * A thin wrapper that represents a time offset from a reference point, usually
 * 00:00 at UTC, but not always. Use one of the static factory methods to
 * create an instance. For example, each of the following creates a TimeOffset
 * of -08:00:
 *
 * @code{.cpp}
 * TimeOffset tz = TimeOffset::forHours(-8);
 * TimeOffset tz = TimeOffset::forHourMinute(-8, 0);
 * TimeOffset tz = TimeOffset::forMinutes(-480);
 * TimeOffset tz = TimeOffset::forOffsetString("-08:00");
 * @endcode
 *
 * You can use the default constructor to create a +00:00 TimeOffset:
 * @code{.cpp}
 * TimeOffset offset;
 * @endcode
 *
 * The current implementation has a resolution of 1-minute (using an internal
 * int16_t type). The previous implementation (< v0.7) had a resolution of
 * 15-minutes (using an internal int8_t type) because that was sufficient to
 * handle all current timezones for years >= 2018 (determined by looking at
 * https://en.wikipedia.org/wiki/List_of_UTC_time_offsets, and the TZ Database
 * zonefiles itself through the tzcompiler.py script). However, 15-minute
 * resolution is not sufficient to handle a handful of timezones in the past
 * (years 2000 to 2011 or so). So I changed the implementation to use 2 bytes
 * to handle 1-minute resolution.
 *
 * This class does NOT know about the TZ Database (aka Olson database)
 * https://en.wikipedia.org/wiki/Tz_database. That functionality is implemented
 * in the TimeZone class.
 */
class TimeOffset {
  public:
    /**
     * Create TimeOffset with the corresponding hour offset. For example,
     * -08:00 is 'forHours(-8)'.
     */
    static TimeOffset forHours(int8_t hours) {
      return TimeOffset::forMinutes(hours * int16_t(60));
    }

    /**
     * Create TimeOffset from (hour, minute) offset. If the offset is negative,
     * then the negative sign must be added to both the hour and minute
     * components. This allows a negative offset of less than -01:00 to be
     * created. For example, -07:30 is created by 'forHourMinute(-7, -30)' (not
     * 'forHourMinute(-7, 30), and -00:15 is created by 'forHourMinute(0,
     * -15)'.
     */
    static TimeOffset forHourMinute(int8_t hour, int8_t minute) {
      int32_t minutes = (hour * int32_t(60) + minute) * 60;
      return TimeOffset(minutes);
    }

    /**
     * Create a TimeOffset fro (hour, minute, second) offset. If the offset is
     * is negative, the negative sign must be added to all fields. For example,
     * -01:02:03 is created by `forHourMinuteSecond(-1, -2, -3)`.
     */
    static TimeOffset forHourMinuteSecond(
      int8_t hour, int8_t minute, int8_t second) {
      int32_t seconds = (hour * int32_t(60) + minute) * 60 + second;
      return TimeOffset(seconds);
    }

    /** Create TimeOffset from minutes from 00:00. */
    static TimeOffset forMinutes(int16_t minutes) {
      return TimeOffset(minutes * int32_t(60));
    }

    /** Create TimeOffset from seconds from 00:00. */
    static TimeOffset forSeconds(int32_t seconds) {
      return TimeOffset(seconds);
    }

    /**
     * Create from an offset string (e.g. "-07:00", "+01:00", "-02:15:33").
     * Intended mostly for testing purposes. Returns TimeOffset::forError() if a
     * parsing error occurs.
     *
     * NOTE: Error checking is not robust, and can be corrupted easily by a
     * misformatted string, or a string with an invalid number of characters.
     * This is intended only for debugging purposes, not for production quality.
     */
    static TimeOffset forOffsetString(const char* offsetString);

    /**
     * Variant of forOffsetString() that updates the string pointer to the next
     * unprocessed character. The resulting pointer can be passed to another
     * forDateStringInternal() method to chain the parsing.
     *
     * This method assumes that the offsetString is sufficiently long.
     * Returns TimeOffset::forError() if a parsing error occurs.
     */
    static TimeOffset forOffsetStringChainable(const char*& offsetString);

    /** Return an error indicator. */
    static TimeOffset forError() { return TimeOffset(kErrorSeconds); }

    /** Constructor. Create a time offset of 0. */
    explicit TimeOffset() {}

    /** Return the time offset as minutes. */
    int16_t toMinutes() const { return mSeconds / 60; }

    /** Return the time offset as seconds. */
    int32_t toSeconds() const { return mSeconds; }

    /**
     * Extract hour and minute representation of the offset. This the inverse
     * of 'forHourMinute()'. If the TimeOffset is negative, then both the
     * hour and minute components will contain the negative sign.
     */
    void toHourMinute(int8_t& hour, int8_t& minute) const {
      int32_t minutes = mSeconds / 60;
      hour = minutes / 60;
      minute = minutes % 60;
    }

    /**
     * Extract hour, minute, second from the offset. Truncation is performed
     * towards zero, so if the offset seconds is negative, each of the hour,
     * minute, second fields will be negative.
     */
    void toHourMinuteSecond(
        int8_t& hour, int8_t& minute, int8_t& second) const {
      int32_t minutes = mSeconds / 60;
      second = mSeconds % 60;
      hour = minutes / 60;
      minute = minutes % 60;
    }

    /**
     * Returns true if offset is 00:00. If this represents a time zone, then
     * isZero means that it is UTC. If this represents a DST delta offset, then
     * isZero means that the time zone is in standard time.
     */
    bool isZero() const { return mSeconds == 0; }

    /** Return true if this TimeOffset represents an error. */
    bool isError() const {
      return mSeconds == kErrorSeconds;
    }

    /**
     * Print the human readable string, including a "-" or "+" prefix, in the
     * form of "+/-hh:mm" or "+/-hh:mm:ss". If the 'second' field is 0, then
     * only the hour and minute fields are printed (e.g. "-08:00"), instead of
     * all three fields (e.g. "+08:15:20").
     */
    void printTo(Print& printer) const;

    // Use default copy constructor and assignment operator.
    TimeOffset(const TimeOffset&) = default;
    TimeOffset& operator=(const TimeOffset&) = default;

  private:
    friend bool operator==(const TimeOffset& a, const TimeOffset& b);

    // Give access to setMinutes()
    friend void time_offset_mutation::incrementHour(TimeOffset& offset);
    friend void time_offset_mutation::increment15Minutes(TimeOffset& offset);

    /** Constructor. Create a time offset from the offset minutes. */
    explicit TimeOffset(int32_t seconds):
        mSeconds(seconds) {}

    /** Set the offset minutes. */
    void setMinutes(int16_t minutes) {
      mSeconds = minutes * int32_t(60);
    }

    /** Set the offset seconds. */
    void setSeconds(int32_t seconds) {
      mSeconds = seconds;
    }

  private:
    /** Sentinel value that represents an error. */
    static const int32_t kErrorSeconds = INT32_MIN;

    /**
     * Time offset seconds from UTC. The value INT32_MIN is used to represent an
     * internal error causing isError() to return true.
     */
    int32_t mSeconds = 0;
};

inline bool operator==(const TimeOffset& a, const TimeOffset& b) {
  return a.mSeconds == b.mSeconds;
}

inline bool operator!=(const TimeOffset& a, const TimeOffset& b) {
  return ! (a == b);
}

}

#endif
