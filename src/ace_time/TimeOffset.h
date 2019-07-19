/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#ifndef ACE_TIME_TIME_OFFSET_H
#define ACE_TIME_TIME_OFFSET_H

#include <stdint.h>

class Print;

namespace ace_time {

// These functions need to set the mOffsetCode and it seemed inefficient to go
// through the factory method and assignment operator, so I expose
// setOffsetCode() to them for efficiency. If the compiler is smart enough to
// optimize away the assignment operator, then we could remove these friend
// declarations and just use the forOffsetCode() factory method instead. I
// haven't looked into this.
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
 * @code
 * TimeOffset tz = TimeOffset::forHour(-8);
 * TimeOffset tz = TimeOffset::forHourMinute(-8, 0);
 * TimeOffset tz = TimeOffset::forOffsetString("-08:00");
 * @endcode
 *
 * You can use the default constructor to create a +00:00 TimeOffset:
 * @code
 * TimeOffset offset;
 * @endcode
 *
 * According to https://en.wikipedia.org/wiki/List_of_UTC_time_offsets, all
 * time zones currently in use occur at 15 minute boundaries, and the smallest
 * time zone is UTC-12:00 and the biggest time zone is UTC+14:00. Therefore, we
 * can encode all currently used time zones as integer multiples of 15-minute
 * offsets from UTC. This allows the TimeOffset to be stored as a single 8-bit
 * signed integer. For the most part, the internal implementation of this class
 * does not leak out to the outside world, so it should be relatively easy to
 * change its implementation to a int16_t type to support 1-minute granularity
 * instead of 15-minute granularity.
 *
 * This class does NOT know about the TZ Database (aka Olson database)
 * https://en.wikipedia.org/wiki/Tz_database. That functionality is implemented
 * in the TimeZone class.
 */
class TimeOffset {
  public:
    /** Sentinel value that represents an error. */
    static const int8_t kErrorCode = INT8_MIN;

    /**
     * Create TimeOffset with the corresponding hour offset. For example,
     * -08:00 is 'forHour(-8)'.
     */
    static TimeOffset forHour(int8_t hour) {
      return TimeOffset(hour * 4);
    }

    /**
     * Create TimeOffset from (hour, minute) offset. If the offset is negative,
     * then the negative sign must be added to both the hour and minute
     * components. This allows a negative offset of less than -01:00 to be
     * created. The 'minute' must be in multiples of 15-minutes. For example,
     * -07:30 is created by 'forHourMinute(-7, -30)' (not 'forHourMinute(-7,
     * 30), and -00:15 is created by 'forHourMinute(0, -15)'.
     */
    static TimeOffset forHourMinute(int8_t hour, int8_t minute) {
      int8_t code = hour * 4 + minute / 15;
      return TimeOffset(code);
    }

    /**
     * Create TimeOffset from minutes from 00:00. In the current implementation,
     * the minutes is truncated to the 15-minute boundary towards 0.
     */
    static TimeOffset forMinutes(int16_t minutes) {
      return TimeOffset(minutes / 15);
    }

    /**
     * Create from an offset string ("-07:00" or "+01:00"). Intended mostly
     * for testing purposes.
     * Returns TimeOffset::forError() if a parsing error occurs.
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
    static TimeOffset forError() { return TimeOffset(kErrorCode); }

    /**
     * Create TimeOffset from the offset code.
     *
     * @param offsetCode the amount of time offset in 15-minute increments.
     */
    static TimeOffset forOffsetCode(int8_t offsetCode) {
      return TimeOffset(offsetCode);
    }

    /** Constructor. Create a time offset of 0. */
    explicit TimeOffset() {}

    /** Return the time offset as the number of 15 minute increments. */
    int8_t toOffsetCode() const { return mOffsetCode; }

    /** Return the time offset as minutes. */
    int16_t toMinutes() const {
      return (int16_t) 15 * mOffsetCode;
    }

    /** Return the time offset as seconds. */
    int32_t toSeconds() const {
      return (int32_t) 60 * toMinutes();
    }

    /**
     * Extract hour and minute representation of the offset. This the inverse
     * of 'forHourMinute()'. If the TimeOffset is negative, then both the
     * hour and minute components will contain the negative sign.
     */
    void toHourMinute(int8_t& hour, int8_t& minute) const {
      hour = mOffsetCode / 4;
      minute = (mOffsetCode % 4) * 15;
    }

    /**
     * Returns true if offset is 00:00. If this represents a time zone, then
     * isZero means that it is UTC. If this represents a DST delta offset, then
     * isZero means that the time zone is in standard time.
     */
    bool isZero() const { return mOffsetCode == 0; }

    /** Return true if this TimeOffset represents an error. */
    bool isError() const {
      return mOffsetCode == kErrorCode;
    }

    /** Print the human readable string. For example, "-08:00". */
    void printTo(Print& printer) const;

    // Use default copy constructor and assignment operator.
    TimeOffset(const TimeOffset&) = default;
    TimeOffset& operator=(const TimeOffset&) = default;

  private:
    friend bool operator==(const TimeOffset& a, const TimeOffset& b);
    // Give access to setOffsetCode()
    friend void time_offset_mutation::incrementHour(TimeOffset& offset);
    friend void time_offset_mutation::increment15Minutes(TimeOffset& offset);

    /** Length of UTC offset string (e.g. "-07:00", "+01:30"). */
    static const uint8_t kTimeOffsetStringLength = 6;

    /** Constructor. Create a time offset from the offset code. */
    explicit TimeOffset(int8_t offsetCode):
        mOffsetCode(offsetCode) {}

    /** Set the offset code. */
    void setOffsetCode(int8_t offsetCode) { mOffsetCode = offsetCode; }

    /**
     * Time offset code, representing 15 minute increments from UTC. In theory,
     * the code can range from [-128, 127]. But the value of -128 is used to
     * represent an internal error, causing isError() to return true so the
     * valid range is [-127, 127].
     *
     * The actual range of time zones used in real life values are expected to
     * be smaller, probably smaller than the range of [-64, 63], i.e. [-16:00,
     * +15:45].
     */
    int8_t mOffsetCode = 0;
};

inline bool operator==(const TimeOffset& a, const TimeOffset& b) {
  return a.mOffsetCode == b.mOffsetCode;
}

inline bool operator!=(const TimeOffset& a, const TimeOffset& b) {
  return ! (a == b);
}

}

#endif
