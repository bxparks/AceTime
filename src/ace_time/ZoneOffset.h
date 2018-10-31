#ifndef ACE_TIME_ZONE_OFFSET_H
#define ACE_TIME_ZONE_OFFSET_H

#include <stdint.h>

class Print;

namespace ace_time {

/**
 * A thin object wrapper around an integer (int8_t) offset code which
 * represents the offset from UTC in 15 minute increments. For example,
 * UTC-08:00 can be created using one of the following:
 *
 * @code
 * ZoneOffset tz = ZoneOffset::forOffsetCode(-32);
 * ZoneOffset tz = ZoneOffset::forHour(-8);
 * ZoneOffset tz = ZoneOffset::forHourMinute(-1, -8, 0);
 * ZoneOffset tz = ZoneOffset::forOffsetString("-08:00");
 * @endcode
 *
 * You can use the default constructor to create a UTC ZoneOffset:
 * @code
 * ZoneOffset utc;
 * @endcode
 *
 * According to https://en.wikipedia.org/wiki/List_of_UTC_time_offsets, all
 * time zones currently in use occur at 15 minute boundaries, and the smallest
 * time zone is UTC-12:00 and the biggest time zone is UTC+14:00. Therefore,
 * we can encode all currently used time zones as integer multiples of
 * 15-minute offsets from UTC. Some locations may observe daylight saving time,
 * so the actual range of the offset in practice may be UTC-12:00 to UTC+15:00.
 *
 * This class does NOT know about the "tz database" (aka Olson database)
 * https://en.wikipedia.org/wiki/Tz_database. Therefore, it does not know about
 * symbolic time zones (e.g. "America/Los_Angeles"). It also does not know
 * about when daylight saving time (DST) starts and ends for specific time
 * zones.
 */
class ZoneOffset {
  public:
    /**
     * Create ZoneOffset from the offset code.
     *
     * @param offsetCode the number of 15-minute offset from UTC. 0 means UTC.
     */
    static ZoneOffset forOffsetCode(int8_t offsetCode) {
      return ZoneOffset(offsetCode);
    }

    /**
     * Create ZoneOffset from integer hour offset from UTC. For example,
     * UTC-08:00 is 'forHour(-8)'.
     */
    static ZoneOffset forHour(int8_t hour) {
      return ZoneOffset(hour * 4);
    }

    /**
     * Create ZoneOffset from (sign, hour, minute) offset from UTC, where 'sign'
     * is either -1 or +1. The 'minute' must be in multiples of 15-minutes. For
     * example, UTC-07:30 is 'forHourMinute(-1, 7, 30)'.
     */
    static ZoneOffset forHourMinute(int8_t sign, uint8_t hour, uint8_t minute) {
      uint8_t code = hour * 4 + minute / 15;
      return (sign < 0) ? ZoneOffset(-code) : ZoneOffset(code);
    }

    /**
     * Create from UTC offset string ("-07:00" or "+01:00"). Intended mostly
     * for testing purposes.
     */
    static ZoneOffset forOffsetString(const char* offsetString) {
      return ZoneOffset().initFromOffsetString(offsetString);
    }

    /** Constructor. Create a time zone corresponding to UTC with no offset. */
    explicit ZoneOffset() {}

    /**
     * Return the UTC offset as the number of 15 minute increments, excluding
     * DST shift.
     */
    int8_t toOffsetCode() const { return mOffsetCode; }

    /** Return the number of minutes offset from UTC. */
    int16_t toMinutes() const {
      return (int16_t) 15 * mOffsetCode;
    }

    /** Return the number of minutes offset from UTC. */
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

    /**
     * Increment the time zone by one hour (+4 in offsetCode). For usability,
     * incrementing a time zone code of +63 (UTC+15:45) by one wraps to -64
     * (UTC-16:00).
     */
    void incrementHour() {
      mOffsetCode += 4;
      if (mOffsetCode >= 64) {
        mOffsetCode = mOffsetCode - 128;
      }
    }

    /**
     * Increment time zone by one zone (i.e. 15 minutes) keeping the hour
     * component unchanged. If the offsetCode is negative, the cycle looks like:
     * (-01:00, -01:15, -01:30, -01:45, -01:00, ...).
     */
    void increment15Minutes() {
      uint8_t offsetCode = (mOffsetCode < 0) ? -mOffsetCode : mOffsetCode;
      offsetCode = (offsetCode & 0xFC) | (((offsetCode & 0x03) + 1) & 0x03);
      mOffsetCode = (mOffsetCode < 0) ? -offsetCode : offsetCode;
    }

    /**
     * Mark the ZoneOffset so that isError() returns true. An invalid
     * ZoneOffset can be returned using 'return ZoneOffset().setError()'. The
     * compiler will optimize away all the apparent method calls.
     */
    ZoneOffset& setError() {
      mOffsetCode = kErrorCode;
      return *this;
    }

    /** Return true if this ZoneOffset represents an error. */
    bool isError() const {
      return mOffsetCode == kErrorCode;
    }

    /**
     * Print the human readable representation of the time zone as offset from
     * UTC. For example, a ZoneOffset for UTC-08:00 is printed as "-08:00".
     */
    void printTo(Print& printer) const;

  private:
    /** Sentinel value that represents an error. */
    static const int8_t kErrorCode = -128;

    /** Length of UTC offset string (e.g. "-07:00", "+01:30"). */
    static const uint8_t kTimeZoneLength = 6;

    friend bool operator==(const ZoneOffset& a, const ZoneOffset& b);
    friend bool operator!=(const ZoneOffset& a, const ZoneOffset& b);

    /** Constructor. Create a time zone from the offset code. */
    explicit ZoneOffset(int8_t offsetCode):
        mOffsetCode(offsetCode) {}

    /** Set time zone from the given UTC offset string. */
    ZoneOffset& initFromOffsetString(const char* offsetString);

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

inline bool operator==(const ZoneOffset& a, const ZoneOffset& b) {
  return a.mOffsetCode == b.mOffsetCode;
}

inline bool operator!=(const ZoneOffset& a, const ZoneOffset& b) {
  return ! (a == b);
}

}

#endif
