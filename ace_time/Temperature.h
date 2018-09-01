#ifndef ACE_TIME_TEMPERATURE_H
#define ACE_TIME_TEMPERATURE_H

#include <stdint.h>
#include <Print.h> // Print
#include "Util.h" // printPad2

namespace ace_time {

/**
 * The temperature in Celcius as a signed (8.8) fixed-point integer. For
 * negative temperatures, convert into a positive (8.8) integer, then print with
 * a "-" in front of it. If you print just the (lsb / 256 * 100), it will be
 * incorrect for negative temperatures.
 */
class Temperature {
  public:
    /** Empty constructor. */
    explicit Temperature() {}

    /** Constructor, from 2 bytes. */
    explicit Temperature(uint8_t msb, uint8_t lsb):
        mLsb(lsb),
        mMsb(msb) {}

    uint8_t msb() const { return mMsb; }
    void msb(uint8_t msb) { mMsb = msb; }

    uint8_t lsb() const { return mLsb; }
    void lsb(uint8_t lsb) { mLsb = lsb; }

    /** Return temperature in units of 1/256 degrees. */
    int16_t toTemperature256() const {
      return (int16_t) ((mMsb << 8) | mLsb);
    }

    /**
     * Compare this Temperature with another Temperature and return (<0, 0, >0)
     * according to (a<b, a==b, a>b).
     */
    int8_t compareTo(const Temperature& that) const {
      int16_t thisTemp = toTemperature256();
      int16_t thatTemp = that.toTemperature256();
      if (thisTemp < thatTemp) {
        return -1;
      } else if (thisTemp == thatTemp) {
        return 0;
      } else {
        return 1;
      }
    }

    /** Print Temperature to 'printer'. */
    void printTo(Print& printer) const {
      uint8_t m;
      uint8_t l;

      int16_t temp = toTemperature256();
      if (temp < 0) {
        temp = -temp;
        m = ((uint16_t) temp) >> 8;
        l = ((uint16_t) temp) & 0xFF;
        printer.print('-');
      } else {
        m = mMsb;
        l = mLsb;
      }

      uint8_t frac = (uint16_t) l * 100 / 256;
      printer.print(m);
      printer.print('.');
      printPad2(printer, frac);
    }

  private:
    friend bool operator==(const Temperature& a, const Temperature& b);
    friend bool operator!=(const Temperature& a, const Temperature& b);

    uint8_t mLsb;
    uint8_t mMsb;
};

/**
 * Return true if two Temperature objects are equal. Optimized for small changes
 * in the less signficant fields.
 */
inline bool operator==(const Temperature& a, const Temperature& b) {
  return a.mLsb == b.mLsb
      && a.mMsb == b.mMsb;
}

/** Return true if two Temperature objects are not equal. */
inline bool operator!=(const Temperature& a, const Temperature& b) {
  return ! (a == b);
}

}

#endif
