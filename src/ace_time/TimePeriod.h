#ifndef ACE_TIME_TIME_PERIOD_H
#define ACE_TIME_TIME_PERIOD_H

#include <stdint.h>
#include "common/Util.h"

class Print;

namespace ace_time {

/**
 * Represents a period of time relative to some known point in time, potentially
 * represented by DateTime. Each component (hour, minute, second) is stored as
 * an unsigned byte (uint8_t). The sign bit allows forward and backward time
 * periods to be represented.
 */
class TimePeriod {
  public:
    /**
     * Constructor.
     *
     * @param hour hour (0-255)
     * @param minute minute (0-59)
     * @param second second (0-59)
     * @param sign The sign bit. Should be either +1 or -1, but in practice,
     * anything greater than equal to 0 will be considered to be +1, and
     * anything less than zero will be considered -1.
     */
    explicit TimePeriod(uint8_t hour, uint8_t minute, uint8_t second,
            int8_t sign = 1):
        mHour(hour),
        mMinute(minute),
        mSecond(second),
        mSign(sign) {}

    /**
     * Constructor from number of seconds. The largest valid 'seconds' is +/-
     * 921599 corresponding to (hour=255, minute=59, second=59). For larger
     * numbers, the 'hour' component will be truncated.
     *
     * @param seconds number of seconds (default 0)
     */
    explicit TimePeriod(int32_t seconds = 0) {
      if (seconds < 0) {
        mSign = -1;
        seconds = -seconds;
      } else {
        mSign = 1;
      }
      mSecond = seconds % 60;
      seconds /= 60;
      mMinute = seconds % 60;
      seconds /= 60;
      mHour = seconds;
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

    /** Return the sign bit. */
    int8_t sign() const { return mSign; }

    /**
     * Set the sign bit. Should be either +1 or -1, but in practice, anything
     * greater than equal to 0 will be considered to be +1, and anything less
     * than zero will be considered -1.
     */
    void sign(int8_t sign) { mSign = sign; }

    /**
     * Print to given printer. If the time period is negative, a minus sign is
     * prepended. Does not implement Printable to avoid memory cost of vtable
     * pointer.
     */
    void printTo(Print& printer) const;

    /**
     * Convert to number of seconds. The largest/smallest possible value
     * returned by this method is +/- 933555, corresponding to (hour=255,
     * minute=255, second=255).
     */
    int32_t toSeconds() const {
      int32_t seconds = ((mHour * (uint16_t) 60) + mMinute) * (uint32_t) 60
          + mSecond;
      return (mSign >= 0) ? seconds : -seconds;
    }

    /** Change the sign of the object. */
    void negate() { mSign = -mSign; }

    /** Increment the hour component by one, modulo 24. */
    void incrementHour() {
      common::incrementMod(mHour, (uint8_t) 24);
    }

    /** Increment the hour by one, modulo 'limit'. */
    void incrementHour(uint8_t limit) {
      common::incrementMod(mHour, limit);
    }

    /** Increment the minute by one, modulo 60. */
    void incrementMinute() {
      common::incrementMod(mMinute, (uint8_t) 60);
    }

    /**
     * Compare this TimePeriod with another TimePeriod and return (<0, 0, >0)
     * according to (a<b, a==b, a>b).
     */
    int8_t compareTo(const TimePeriod& that) const {
      int32_t thisSeconds = toSeconds();
      int32_t thatSeconds = that.toSeconds();
      if (thisSeconds < thatSeconds) {
        return -1;
      } else if (thisSeconds == thatSeconds) {
        return 0;
      } else {
        return 1;
      }
    }

  private:
    friend bool operator==(const TimePeriod& a, const TimePeriod& b);
    friend bool operator!=(const TimePeriod& a, const TimePeriod& b);

    uint8_t mHour; // [0, 255], normally hour < 24
    uint8_t mMinute; // [0, 59], normally minute < 60
    uint8_t mSecond; // [0, 59], normally second < 60

    /**
     * -1 or +1. In practice (>=0) is same as +1, and (<0) is same as -1. An
     * alternative design is to make the mHour field a signed int (int8_t) which
     * could hold the sign bit, saving us a byte in memory. But having some
     * fields be unsigned, and some fields signed, makes the code far more
     * complicated.
    */
    int8_t mSign;
};

/**
 * Return true if two TimePeriod objects are equal. Optimized for small changes
 * in the less signficant fields.
 */
inline bool operator==(const TimePeriod& a, const TimePeriod& b) {
  return a.mSecond == b.mSecond
      && a.mMinute == b.mMinute
      && a.mHour == b.mHour
      && a.mSign == b.mSign;
}

/** Return true if two TimePeriod objects are not equal. */
inline bool operator!=(const TimePeriod& a, const TimePeriod& b) {
  return ! (a == b);
}

}

#endif
