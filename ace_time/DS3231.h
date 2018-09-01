#ifndef ACE_TIME_DS3231_H
#define ACE_TIME_DS3231_H

#include <stdint.h>
#include "DateTime.h"

namespace ace_time {

class DateTime;
class Temperature;

/**
 * A class that reads and writes DateTime and Temperature from a DS3231 RTC
 * chip.
 */
class DS3231 {
  public:
    static const uint8_t kI2cAddress = 0x68;

    /** Constructor. */
    explicit DS3231(uint8_t address = kI2cAddress):
        mAddress(address) {}

    /** Read the time into the DateTime object. */
    void readDateTime(DateTime* dateTime) const;

    /** Set the DS3231 with the DateTime values. */
    void setDateTime(const DateTime& dateTime) const;

    /** Read the temperatue into the Temperature object. */
    void readTemperature(Temperature* temperature) const;

  private:
    const uint8_t mAddress;
};

}

#endif
