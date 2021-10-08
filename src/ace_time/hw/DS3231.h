/*
 * MIT License
 * Copyright (c) 2021 Brian T. Park
 */

#ifndef ACE_TIME_HW_DS3231_MODULE_H
#define ACE_TIME_HW_DS3231_MODULE_H

#include <stdint.h>
#include <AceCommon.h> // bcdToDec(), decToBcd()
#include "HardwareDateTime.h"
#include "HardwareTemperature.h"

namespace ace_time {
namespace hw {

class HardwareDateTime;
class HardwareTemperature;

/**
 * New version of the DS3231 class templatized so that any of the AceWire
 * classes can be used to access the I2C bus. The previous version was hardcoded
 * to use the Wire object from <Wire.h>. Using AceWire allows different software
 * and hardware I2C libraries to be selected at compile time.
 *
 * Making this a templatized class instead of using a virtual interface saves
 * about 400 bytes of flash on AVR processors.
 *
 * @tparam T_WIREI type of the AceWire implementation to communicate over I2C
 */
template <typename T_WIREI>
class DS3231 {
  private:
    static const uint8_t kAddress = 0x68;

  public:
    /** Constructor. */
    explicit DS3231(const T_WIREI& wireInterface) :
        mWireInterface(wireInterface)
    {}

    /** Read the time into the HardwareDateTime object. */
    void readDateTime(HardwareDateTime* dateTime) const {
      using ace_common::bcdToDec;

      mWireInterface.beginTransmission(kAddress);
      mWireInterface.write(0); // set DS3231 register pointer to 00h
      mWireInterface.endTransmission();

      // request seven bytes from DS3231 starting from register 00h
      mWireInterface.requestFrom(kAddress, (uint8_t) 7);
      dateTime->second = bcdToDec(mWireInterface.read() & 0x7F);
      dateTime->minute = bcdToDec(mWireInterface.read());
      dateTime->hour = bcdToDec(mWireInterface.read() & 0x3F);
      dateTime->dayOfWeek = bcdToDec(mWireInterface.read());
      dateTime->day = bcdToDec(mWireInterface.read());
      dateTime->month = bcdToDec(mWireInterface.read());
      dateTime->year = bcdToDec(mWireInterface.read());
    }

    /** Set the DS3231 with the HardwareDateTime values. */
    void setDateTime(const HardwareDateTime& dateTime) const {
      using ace_common::decToBcd;

      mWireInterface.beginTransmission(kAddress);
      mWireInterface.write(0); // set next input to start at 'seconds' register
      mWireInterface.write(decToBcd(dateTime.second));
      mWireInterface.write(decToBcd(dateTime.minute));
      mWireInterface.write(decToBcd(dateTime.hour));
      mWireInterface.write(decToBcd(dateTime.dayOfWeek));
      mWireInterface.write(decToBcd(dateTime.day));
      mWireInterface.write(decToBcd(dateTime.month));
      mWireInterface.write(decToBcd(dateTime.year));
      mWireInterface.endTransmission();
    }

    /** Read the temperature into the HardwareTemperature object. */
    void readTemperature(HardwareTemperature* temperature) const {
      mWireInterface.beginTransmission(kAddress);
      mWireInterface.write(0x11); // set DS3231 register pointer to 11h
      mWireInterface.endTransmission();

      mWireInterface.requestFrom(kAddress, (uint8_t) 2);
      temperature->msb = mWireInterface.read();
      temperature->lsb = mWireInterface.read();
    }

  private:
    const T_WIREI mWireInterface;
};

} // hw
} // ace_time

#endif
