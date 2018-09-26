#ifndef ACE_TIME_TIME_ZONE_H
#define ACE_TIME_TIME_ZONE_H

#include <stdint.h>

class Print;

namespace ace_time {

/**
 * A thin object wrapper around an integer (int8_t) time zone code which
 * represents the time offset from UTC in 15 minute increments. For example,
 * Pacific Daylight Time is UTC-07:00, which is encoded as -28. According to
 * https://en.wikipedia.org/wiki/List_of_UTC_time_offsets, all time zones
 * currently in use occur at 15 minute boundaries, and the smallest time zone is
 * UTC-12:00 and the biggest time zone is UTC+14:00. Some locations may observe
 * daylight saving time, so this class supports a range that may be larger than
 * that.
 */
class TimeZone {
  public:
    /**
     * Create TimeZone from integer hour offset from UTC. For example,
     * UTC-07:00 is 'forHour(-7)'.
     */
    static TimeZone forHour(int8_t hour) {
      return TimeZone(hour * 4);
    }

    /**
     * Create TimeZone from (sign, hour, minute) offset from UTC, where 'sign'
     * is either -1 or +1. The 'minute' must be in multiples of 15-minutes. For
     * example, UTC-07:30 is 'forHourMinute(-1, 7, 30)'.
     */
    static TimeZone forHourMinute(int8_t sign, uint8_t hour, uint8_t minute) {
      uint8_t code = hour * 4 + minute / 15;
      return (sign < 0) ? TimeZone(-code) : TimeZone(code);
    }

    /** Create from UTC offset string ("-07:00" or "+01:00"). */
    static TimeZone forOffsetString(const char* tzString) {
      TimeZone tz(0);
      tz.initFromOffsetString(tzString);
      return tz;
    }

    /**
     * Constructor. Create from time zone code, the number of 15-minute
     * offset from UTC. TimeZone(0) is UTC.
     */
    explicit TimeZone(int8_t tzCode):
        mTzCode(tzCode) {}

    int8_t tzCode() const { return mTzCode; }

    void tzCode(int8_t tzCode) { mTzCode = tzCode; }

    /** Return the number of minutes offset from UTC. */
    int16_t toMinutes() const {
      return (int16_t) 15 * mTzCode;
    }

    /** Return the number of seconds offset from UTC. */
    int32_t toSeconds() const {
      return (int32_t) 900 * mTzCode;
    }

    /**
     * Increment the time zone by one hour (+4 in tzCode). For usability,
     * incrementing a time zone code of +63 (UTC+15:45) by one wraps to -64
     * (UTC-16:00).
     */
    void incrementHour() {
      mTzCode += 4;
      if (mTzCode >= 64) {
        mTzCode = mTzCode - 128;
      }
    }

    /**
     * Increment time zone by one zone, 15 minutes, keeping the hour component
     * unchanged. If the tzCode is negative, the cycle looks like:
     * (-01:00, -01:15, -01:30, -01:45, -01:00).
     */
    void increment15Minutes() {
      uint8_t tzCode = (mTzCode < 0) ? -mTzCode : mTzCode;
      tzCode = (tzCode & 0xFC) | (((tzCode & 0x03) + 1) & 0x03);
      mTzCode = (mTzCode < 0) ? -tzCode : tzCode;
    }

    /** Extract the (hour, minute) components of the time zone. */
    void extractHourMinute(uint8_t& hour, uint8_t& minute) const {
      uint8_t tzCode = (mTzCode < 0) ? -mTzCode : mTzCode;
      hour = tzCode / 4;
      minute = (tzCode & 0x03) * 15;
    }

    /**
     * Mark the TimeZone so that isError() returns true. An invalid TimeZone can
     * be returned using 'return TimeZone().setError()'. The compiler will
     * optimize away all the apparent method calls.
     */
    TimeZone& setError() {
      mTzCode = kTimeZoneErrorCode;
      return *this;
    }

    /** Return true if this TimeZone represents an error. */
    bool isError() const {
      return mTzCode == kTimeZoneErrorCode;
    }

    /**
     * Print to the given printer as an offset from UTC. A '+' or '-' sign is
     * always printed (e.g. "+01:00"). This can be used to print a DateTime in
     * ISO8601 format with a time zone specifier at the end (e.g.
     * "2018-08-29T11:32:00-07:00"). Does not implement Printable to avoid
     * memory cost of vtable pointer.
     */
    void printTo(Print& printer) const;

  private:
    /** Sential value that represents an error. */
    static const int8_t kTimeZoneErrorCode = -128;

    /** Length of UTC offset string (e.g. "-07:00", "+01:30"). */
    static const uint8_t kTimeZoneLength = 6;

    friend bool operator==(const TimeZone& a, const TimeZone& b);
    friend bool operator!=(const TimeZone& a, const TimeZone& b);

    /** Set time zone code from the given UTC offset string. */
    void initFromOffsetString(const char* offsetString);

    /**
     * Time zone code, offset from UTC in 15 minute increments from UTC. In
     * theory, the code can range from [-128, 127]. But the value of -128 is
     * used to represent an internal error, causing isError() to return true.
     * The actual range of time zones used in real life values are expected to
     * be smaller, probably smaller than the range of [-64, 63], i.e. [-16:00,
     * +15:45].
     */
    int8_t mTzCode;
};

inline bool operator==(const TimeZone& a, const TimeZone& b) {
  return a.mTzCode == b.mTzCode;
}

inline bool operator!=(const TimeZone& a, const TimeZone& b) {
  return ! (a == b);
}

}

#endif
