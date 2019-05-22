#ifndef ACE_TIME_UTC_OFFSET_H
#define ACE_TIME_UTC_OFFSET_H

#include <stdint.h>

class Print;

namespace ace_time {

// These functions need to set the mOffsetCode and it seemed inefficient to go
// through the factory method and assignment operator, so I expose
// setOffsetCode() to them for efficiency. If the compiler is smart enough to
// optimize away the assignment operator, then we could remove these friend
// declarations and just use the forOffsetCode() factory method instead. I
// haven't looked into this.
class UtcOffset;
namespace utc_offset_mutation {
void incrementHour(UtcOffset& offset);
void increment15Minutes(UtcOffset& offset);
}

/**
 * A thin wrapper that represents a time offset from a reference point, usually
 * 00:00 at UTC, but not always. Use one of the static factory methods to
 * create an instance:
 *
 * @code
 * UtcOffset tz = UtcOffset::forHour(-8);
 * UtcOffset tz = UtcOffset::forHourMinute(-1, -8, 0);
 * UtcOffset tz = UtcOffset::forOffsetString("-08:00");
 * @endcode
 *
 * You can use the default constructor to create a UTC UtcOffset:
 * @code
 * UtcOffset utc;
 * @endcode
 *
 * According to https://en.wikipedia.org/wiki/List_of_UTC_time_offsets, all
 * time zones currently in use occur at 15 minute boundaries, and the smallest
 * time zone is UTC-12:00 and the biggest time zone is UTC+14:00. Therefore, we
 * can encode all currently used time zones as integer multiples of 15-minute
 * offsets from UTC. This allows the UtcOffset to be stored as a single 8-bit
 * signed integer. For the most part, the internal implementation of this class
 * does not leak out to the outside world, so it should be relatively easy to
 * change its implementation to a 16-bit integer to support 1-minute
 * granularity instead of 15-minute granularity.
 *
 * This class does NOT know about the "tz database" (aka Olson database)
 * https://en.wikipedia.org/wiki/Tz_database. That functionality is implemented
 * in the TimeZone class.
 */
class UtcOffset {
  public:
    /**
     * Create UtcOffset from integer hour offset from UTC. For example,
     * UTC-08:00 is 'forHour(-8)'.
     */
    static UtcOffset forHour(int8_t hour) {
      return UtcOffset(hour * 4);
    }

    /**
     * Create UtcOffset from (sign, hour, minute) offset from UTC, where 'sign'
     * is either -1 or +1. The 'minute' must be in multiples of 15-minutes. For
     * example, UTC-07:30 is 'forHourMinute(-1, 7, 30)'.
     */
    static UtcOffset forHourMinute(int8_t sign, uint8_t hour, uint8_t minute) {
      uint8_t code = hour * 4 + minute / 15;
      return (sign < 0) ? UtcOffset(-code) : UtcOffset(code);
    }

    /**
     * Create UtcOffset from minutes from 00:00. In the current implementation,
     * the minutes is truncated to the 15-minute boundary towards 0.
     */
    static UtcOffset forMinutes(int16_t minutes) {
      return UtcOffset(minutes / 15);
    }

    /**
     * Create from UTC offset string ("-07:00" or "+01:00"). Intended mostly
     * for testing purposes.
     */
    static UtcOffset forOffsetString(const char* offsetString);

    /** Return an error indicator. */
    static UtcOffset forError() { return UtcOffset(kErrorCode); }

    /**
     * Create UtcOffset from the offset code.
     *
     * @param offsetCode the number of 15-minute offset from UTC. 0 means UTC.
     */
    static UtcOffset forOffsetCode(int8_t offsetCode) {
      return UtcOffset(offsetCode);
    }

    /** Constructor. Create a time zone corresponding to UTC with no offset. */
    explicit UtcOffset() {}

    /**
     * Returns true if offset is 00:00. If this represents a time zone, then
     * isZero means that it is UTC. If this represents a DST delta offset, then
     * isZero means that the time zone is in standard time.
     */
    bool isZero() const { return mOffsetCode == 0; }

    /** Return the UTC offset as the number of 15 minute increments. */
    int8_t toOffsetCode() const { return mOffsetCode; }

    /** Return the number of minutes offset from UTC. */
    int16_t toMinutes() const {
      return (int16_t) 15 * mOffsetCode;
    }

    /** Return the number of seconds offset from UTC. */
    int32_t toSeconds() const {
      return (int32_t) 60 * toMinutes();
    }

    /** Extract hour and minute representation of the offset. */
    void toHourMinute(int8_t& sign, uint8_t& hour, uint8_t& minute) const {
      uint8_t code;
      if (mOffsetCode < 0) {
        sign = -1;
        code = -mOffsetCode;
      } else {
        sign = 1;
        code = mOffsetCode;
      }
      hour = code / 4;
      minute = (code & 0x03) * 15;
    }

    /** Return true if this UtcOffset represents an error. */
    bool isError() const {
      return mOffsetCode == kErrorCode;
    }

    /**
     * Print the human readable representation of the time zone as offset from
     * UTC. For example, a UtcOffset for UTC-08:00 is printed as "-08:00".
     */
    void printTo(Print& printer) const;

    // Use default copy constructor and assignment operator.
    UtcOffset(const UtcOffset&) = default;
    UtcOffset& operator=(const UtcOffset&) = default;

  private:
    friend class BasicZoneSpecifier;
    friend class ManualZoneSpecifier;
    friend class TimeZone;
    friend class OffsetDateTime; // forOffsetStringChainable()
    friend class UtcOffsetMutator;
    friend bool operator==(const UtcOffset& a, const UtcOffset& b);
    friend void utc_offset_mutation::incrementHour(UtcOffset& offset);
    friend void utc_offset_mutation::increment15Minutes(UtcOffset& offset);

    /** Sentinel value that represents an error. */
    static const int8_t kErrorCode = -128;

    /** Length of UTC offset string (e.g. "-07:00", "+01:30"). */
    static const uint8_t kUtcOffsetStringLength = 6;

    /**
     * The internal version of forOffsetString() that updates the string pointer
     * to the next unprocessed character. The resulting pointer can be passed
     * to another forDateStringInternal() method to continue parsing.
     *
     * This method assumes that the offsetString is sufficiently long.
     */
    static UtcOffset forOffsetStringChainable(const char*& offsetString);

    /** Constructor. Create a time zone from the offset code. */
    explicit UtcOffset(int8_t offsetCode):
        mOffsetCode(offsetCode) {}

    /** Set the offset code. */
    void setOffsetCode(int8_t offsetCode) { mOffsetCode = offsetCode; }

    /**
     * Time zone code, offset from UTC in 15 minute increments from UTC. In
     * theory, the code can range from [-128, 127]. But the value of -128 is
     * used to represent an internal error, causing isError() to return true
     * so the valid range is [-127, 127].
     *
     * The actual range of time zones used in real life values are expected to
     * be smaller, probably smaller than the range of [-64, 63], i.e. [-16:00,
     * +15:45].
     */
    int8_t mOffsetCode = 0;
};

inline bool operator==(const UtcOffset& a, const UtcOffset& b) {
  return a.mOffsetCode == b.mOffsetCode;
}

inline bool operator!=(const UtcOffset& a, const UtcOffset& b) {
  return ! (a == b);
}

}

#endif
