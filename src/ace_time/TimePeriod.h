/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#ifndef ACE_TIME_TIME_PERIOD_H
#define ACE_TIME_TIME_PERIOD_H

#include <stdint.h>

class Print;

namespace ace_time {

/**
 * Represents a period of time relative to some known point in time, potentially
 * represented by a DateTime. Each component (hour, minute, second) is stored
 * as an unsigned byte (uint8_t). The sign bit allows forward and backward time
 * periods to be represented.
 *
 * There are 3 conditions which return isError() true:
 *
 *    * generic error: sign == 0
 *    * overflow: sign == 1
 *    * underflow: sign == -1
 */
class TimePeriod {
  public:
    /** An invalid time period seconds. */
    static const int32_t kInvalidPeriodSeconds = INT32_MIN;

    /**
     * The largest period that can be represented by this class, in seconds,
     * corresponding to +/- 255h59m59s. The same limit applies in the positive
     * and negative direction.
     */
    static const int32_t kMaxPeriodSeconds = 921599;

    /**
     * Factory method that creates a TimePeriod representing a generic error so
     * that isError() returns true.
     *
     * @param sign optional sign value which indicates generic error (0,
     * default), overflow (1), or underflow (-1).
     */
    static TimePeriod forError(int8_t sign = 0) {
      return TimePeriod(255, 255, 255, sign);
    }

    /**
     * Constructor.
     * No input validation or normalization is performed by this constructor.
     *
     * @param hour hour (0-255)
     * @param minute minute (0-59)
     * @param second second (0-59)
     * @param sign The sign bit. Should be either +1 or -1. Any other value may
     *        cause the isError() method to return true.
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
     * number or smaller values, an error object will be returned whose
     * isError() returns true.
     *
     * @param seconds number of seconds (default 0)
     */
    explicit TimePeriod(int32_t seconds = 0) {
      if (seconds == kInvalidPeriodSeconds) {
        mHour = mMinute = mSecond = 255;
        mSign = 0;
        return;
      } else if (seconds < -kMaxPeriodSeconds) {
        mHour = mMinute = mSecond = 255;
        mSign = -1;
        return;
      } else if (seconds > kMaxPeriodSeconds) {
        mHour = mMinute = mSecond = 255;
        mSign = 1;
        return;
      }

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

    /** Set the hour. Should be between [0, 255]. No error checking is done. */
    void hour(uint8_t hour) { mHour = hour; }

    /** Return the minute. */
    uint8_t minute() const { return mMinute; }

    /** Set the minute. Should be between [0, 59]. No error checking is done. */
    void minute(uint8_t minute) { mMinute = minute; }

    /** Return the second. */
    uint8_t second() const { return mSecond; }

    /** Set the second. Should be between [0, 59]. No error checking is done. */
    void second(uint8_t second) { mSecond = second; }

    /** Return the sign bit. */
    int8_t sign() const { return mSign; }

    /**
     * Set the sign bit. Should be either +1 or -1. Setting it to 0 causes
     * isError() to return true, to indicate a generic error condition.
     * Any other value caused undefined behavior.
     */
    void sign(int8_t sign) { mSign = sign; }

    /**
     * Convert to number of seconds. For a normal TimePeriod, the maximum and
     * minimum values are +kMaxPeriodSeconds or -kMaxPeriodSeconds. If
     * isError() is true, this returns kInvalidPeriodSeconds.
     */
    int32_t toSeconds() const {
      if (isError()) {
        return kInvalidPeriodSeconds;
      }
      int32_t seconds = ((mHour * (int16_t) 60) + mMinute) * (int32_t) 60
          + mSecond;
      return (mSign > 0) ? seconds : -seconds;
    }

    /**
     * Return true if this represents an error. Check the sign() value to
     * descriminate the 3 different error conditions:
     *
     *  * sign==0: generic error
     *  * sign==1: overflow
     *  * sign==-1: underflow
     */
    bool isError() const {
      return mSign == 0 || mSecond == 255 || mMinute == 255;
    }

    /**
     * Compare 'this' TimePeriod with 'that' TimePeriod and return (<0, 0, >0)
     * according to (this<that, this==that, this>that). If either 'this' or
     * 'that' returns true for isError(), then the result is undefined.
     */
    int8_t compareTo(const TimePeriod& that) const {
      int32_t thisSeconds = toSeconds();
      int32_t thatSeconds = that.toSeconds();
      if (thisSeconds < thatSeconds) {
        return -1;
      } else if (thisSeconds > thatSeconds) {
        return 1;
      } else {
        return 0;
      }
    }

    /**
     * Print to given printer. If the time period is negative, a minus sign is
     * prepended. If the TimePeriod is an error, prints the following:
     *
     *    * sign == 0, generic error: <Error>
     *    * sign == 1, overflow: <+Inf>
     *    * sign == -1, underflow: <-Inf>
     *
     * This class does not implement the Printable interface to avoid
     * increasing the size of the object from the additional virtual function.
     */
    void printTo(Print& printer) const;

    // Use default copy constructor and assignment operator.
    TimePeriod(const TimePeriod&) = default;
    TimePeriod& operator=(const TimePeriod&) = default;

  private:
    friend bool operator==(const TimePeriod& a, const TimePeriod& b);

    uint8_t mHour; // [0, 255], normally hour < 24
    uint8_t mMinute; // [0, 59], normally minute < 60
    uint8_t mSecond; // [0, 59], normally second < 60

    /**
     * -1, 0, or +1, where 0 indicates an generic error. An alternative design
     * is to make the mHour field a signed int (int8_t) which could hold the
     * sign bit, saving us a byte in memory. But having some fields be unsigned,
     * and some fields signed, makes the code far more complicated.
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
