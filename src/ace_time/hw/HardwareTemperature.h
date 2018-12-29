#ifndef ACE_TIME_HW_TEMPERATURE_H
#define ACE_TIME_HW_TEMPERATURE_H

#include <stdint.h>
#include <Print.h> // Print
#include "../common/util.h" // printPad2

namespace ace_time {
namespace hw {

/**
 * The temperature in Celcius as a signed (8.8) fixed-point integer. For
 * negative temperatures, convert into a positive (8.8) integer, then print with
 * a "-" in front of it. If you print just the (lsb / 256 * 100), it will be
 * incorrect for negative temperatures.
 */
struct HardwareTemperature {
  /** Return temperature in units of 1/256 degrees. */
  int16_t toTemperature256() const {
    return (int16_t) ((msb << 8) | lsb);
  }

  /** Print HardwareTemperature to 'printer'. */
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
      m = msb;
      l = lsb;
    }

    uint8_t frac = (uint16_t) l * 100 / 256;
    printer.print(m);
    printer.print('.');
    common::printPad2(printer, frac);
  }

  uint8_t msb;
  uint8_t lsb;
};

/**
* Return true if two HardwareTemperature objects are equal. Optimized for small
* changes in the less signficant fields.
*/
inline bool operator==(const HardwareTemperature& a,
    const HardwareTemperature& b) {
return a.lsb == b.lsb
    && a.msb == b.msb;
}

/** Return true if two HardwareTemperature objects are not equal. */
inline bool operator!=(const HardwareTemperature& a,
    const HardwareTemperature& b) {
return ! (a == b);
}

}
}

#endif
