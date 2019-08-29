/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#ifndef ACE_TIME_HW_DS3231_H
#define ACE_TIME_HW_DS3231_H

#if ! defined(UNIX_HOST_DUINO)

#include <stdint.h>

namespace ace_time {
namespace hw {

class HardwareDateTime;
class HardwareTemperature;

/**
 * A class that reads and writes HardwareDateTime and HardwareTemperature from a
 * DS3231 RTC chip. This class is designed to access just enough features of the
 * DS3231 chip to implement the ace_time::DS3231Clock class. It is not
 * meant to provide access to all the features of the DS3231 chip. There are
 * other libraries which are far better for that purpose.
 *
 * UNTESTED ON REAL HARDWARE: As DS1307 and DS3232 RTC chips have exactly same
 * interface as DS3231 for used features except temperature on DS1307 (as this
 * one lacks this functionality), this class can be be used to control all of them.
 * Any problem with these RTCs please, contact @Naguissa (https://github.com/Naguissa/AceTime)
 *
 * According to https://learn.adafruit.com/i2c-addresses/the-list, the DS3231
 * is always on I2C address 0x68, so let's hardcode that.
 */
class DS3231 {
  public:
    /** Constructor. */
    explicit DS3231() {}

    /** Read the time into the HardwareDateTime object. */
    void readDateTime(HardwareDateTime* dateTime) const;

    /** Set the DS3231 with the HardwareDateTime values. */
    void setDateTime(const HardwareDateTime& dateTime) const;

    /** Read the temperature into the HardwareTemperature object. */
    void readTemperature(HardwareTemperature* temperature) const;

  private:
    static const uint8_t kAddress = 0x68;
};

}
}

#endif

#endif
