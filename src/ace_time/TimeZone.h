#ifndef ACE_TIME_TIME_ZONE_H
#define ACE_TIME_TIME_ZONE_H

#include <stdint.h>
#include "ZoneOffset.h"
#include "ZoneInfo.h"
#include "ZoneRule.h"

class Print;

namespace ace_time {

/**
 * A thin object wrapper around an integer (int8_t) time zone code which
 * represents the time offset from UTC in 15 minute increments, and a daylight
 * saving time flag (bool) that indicates whether the time zone is currently in
 * DST mode.
 *
 * For example, Pacific Standard Time is UTC-08:00, which is encoded
 * internally as an offset code of -32 and the TimeZone object can be created
 * using the constructor TimeZone(-32) or the factory method
 * TimeZone::forHour(-8). When the PST time goes into DST mode, setting the
 * isDst(true) causes the time zone object to return UTC offsets which includes
 * the DST shift. When the isDst flag is set, the "effective" UTC offset
 * returned by effectiveZoneOffset() will include the DST shift.
 *
 * Here is the TimeZone object that represents Pacific Daylight Time:
 * @code
 * TimeZone tz = TimeZone::forZoneOffset(ZoneOffset::forHour(-8)).isDst(true);
 * int16_t minutes = tz.asEffectiveOffsetMinutes();
 * @endcode
 *
 * According to https://en.wikipedia.org/wiki/List_of_UTC_time_offsets, all
 * time zones currently in use occur at 15 minute boundaries, and the smallest
 * time zone is UTC-12:00 and the biggest time zone is UTC+14:00. Therefore,
 * we can encode all currently used time zones as integer multiples of
 * 15-minute offsets from UTC. Some locations may observe daylight saving time,
 * so the actual range of the offset in practice may be UTC-12:00 to UTC+15:00.
 */
class TimeZone {
  public:
    /** Factory method. Create from ZoneOffset. */
    static TimeZone forZoneOffset(ZoneOffset zoneOffset) {
      return TimeZone(zoneOffset);
    }

    /** Factory method. Create from hour offset. */
    static TimeZone forHour(int8_t hour) {
      return TimeZone::forZoneOffset(ZoneOffset::forHour(hour));
    }

    /** Factory method. Create from hour/minute offset. */
    static TimeZone forHourMinute(int8_t sign, uint8_t hour, uint8_t minute) {
      return TimeZone::forZoneOffset(
          ZoneOffset::forHourMinute(sign, hour, minute));
    }

    /** Factory method. Create from ZoneInfo. */
    static TimeZone forZone(const ZoneInfo* zoneInfo) {
      return TimeZone(zoneInfo);
    }

    /** Constructor. Create UTC time zone with no offset. */
    explicit TimeZone():
        mTimeZoneType(kTimeZoneTypeOffset),
        mZoneOffset(),
        mZoneInfo(nullptr) {}

    /** Return the ZoneOffset. */
    const ZoneOffset& zoneOffset() const { return mZoneOffset; }

    /** Return a mutable ZoneOffset. */
    ZoneOffset& zoneOffset() { return mZoneOffset; }

    /** Return the DST mode setting. */
    bool isDst() const { return mIsDst; }

    /**
     * Set the current DST mode. Returns the reference to 'this' pointer, which
     * allows chaining to set the DST flag when creating this object. For
     * example, to create a time zone for Pacific Daylight Time, you can write:
     * @code
     * TimeZone tz = TimeZone::forZoneOffset(ZoneOffset::forHour(-8))
     *      .isDst(true);
     * @endcode
     */
    TimeZone& isDst(bool isDst) {
      mIsDst = isDst;
      return *this;
    }

    /**
     * Mark the TimeZone so that isError() returns true. An invalid TimeZone can
     * be returned using 'return TimeZone().setError()'. The compiler will
     * optimize away all the apparent method calls.
     */
    TimeZone& setError() {
      mZoneOffset.setError();
      return *this;
    }

    /** Return true if this TimeZone represents an error. */
    bool isError() const {
      return mZoneOffset.isError();
    }

    /** Return the effective zone offset. */
    ZoneOffset effectiveZoneOffset(uint32_t /*secondsSinceEpoch*/) const {
      return ZoneOffset::forOffsetCode(0);
    }

    /**
     * Print the human readable representation of the time zone as offset from
     * UTC, with an indicator of the current DST mode. The printed UTC offset
     * is the standard (i.e. base) UTC offset of the time zone, not the UTC
     * offset that includes the DST shift. In other words, a time zone of
     * UTC-08:00 whose DST is enabled (i.e. Pacific Daylight Time) is printed
     * as "UTC-08:00 DST", instead of "UTC-07:00 DST".
     *
     * In some ways, displaying the Standard UTC offset could be confusing
     * because the actual UTC offset used for DateTime calculations is not what
     * is displayed. On the other hand, displaying the effective UTC offset
     * causes confusion in other ways, for example, when the user is prompted
     * to set their current time zone, because it is not clear if the DST flag
     * describes the UTC offset that they just entered, or whether it shifts
     * the UTC offset that they just entered.
     *
     * Use effectiveZoneOffset().printTo() to print the UTC offset that includes
     * the DST shift.
     */
    void printTo(Print& printer) const;

  private:
    /** Length of UTC offset string (e.g. "-07:00", "+01:30"). */
    static const uint8_t kTimeZoneLength = 6;

    friend bool operator==(const TimeZone& a, const TimeZone& b);
    friend bool operator!=(const TimeZone& a, const TimeZone& b);

    /** Constructor. */
    explicit TimeZone(ZoneOffset zoneOffset):
        mTimeZoneType(kTimeZoneTypeOffset),
        mZoneOffset(zoneOffset),
        mZoneInfo(nullptr) {}

    /** Constructor. */
    explicit TimeZone(const ZoneInfo* zoneInfo):
        mTimeZoneType(kTimeZoneTypeInfo),
        mZoneOffset(),
        mZoneInfo(zoneInfo) {}

    /** Set time zone from the given UTC offset string. */
    TimeZone& initFromOffsetString(const char* offsetString);

    /** Time zone using ZoneOffset with manual DST. */
    static const uint8_t kTimeZoneTypeOffset = 0;

    /** Time zone using ZoneInfo from the TZ Database. */
    static const uint8_t kTimeZoneTypeInfo = 1;

    // TODO: See if the member variables can be made 'const'.

    /** Time Zone type. */
    uint8_t mTimeZoneType;

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
    ZoneOffset mZoneOffset;

    /**
     * Indicate whether Daylight Saving Time is in effect. If true, then
     * effectiveZoneOffset() will increased by 1h.
     */
    bool mIsDst = false;

    /** Zone info if the TimeZone represents a TZ Database time zone. */
    const ZoneInfo* mZoneInfo;
};

inline bool operator==(const TimeZone& a, const TimeZone& b) {
  return a.mZoneOffset == b.mZoneOffset
      && a.mIsDst == b.mIsDst;
}

inline bool operator!=(const TimeZone& a, const TimeZone& b) {
  return ! (a == b);
}

}

#endif
