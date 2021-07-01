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
 * Abstract interface to the DS3231Module to allow all template instantiations
 * of DS3231Module to be accessed through a single interface. This is a
 * convenience interface. The overhead of making these calls through the virtual
 * dispatch is expected to be negligible compared to the calls into the I2C
 * library.
 */
class DS3231Interface {
  public:
    virtual void readDateTime(HardwareDateTime* dateTime) = 0;
    virtual void setDateTime(const HardwareDateTime& dateTime) = 0;
    virtual void readTemperature(HardwareTemperature* temperature) = 0;
};

/**
 * Same as the hw/DS3231 class, but designed for use with one of the templatized
 * interface classes of AceWire to access the I2C bus, instead of hardcoding
 * the use of the <Wire.h> library. Allows different software and hardware
 * I2C libraries to be selected at compile time.
 */
template <typename T_WIRE>
class DS3231Module : public DS3231Interface {
  private:
    static const uint8_t kAddress = 0x68;

  public:
    /** Constructor. */
    explicit DS3231Module(T_WIRE& wireInterface)
        : mWireInterface(wireInterface) {}

    /** Read the time into the HardwareDateTime object. */
    void readDateTime(HardwareDateTime* dateTime) override {
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
      mWireInterface.endRequest();
    }

    /** Set the DS3231 with the HardwareDateTime values. */
    void setDateTime(const HardwareDateTime& dateTime) override {
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
    void readTemperature(HardwareTemperature* temperature) override {
      mWireInterface.beginTransmission(kAddress);
      mWireInterface.write(0x11); // set DS3231 register pointer to 11h
      mWireInterface.endTransmission();

      mWireInterface.requestFrom(kAddress, (uint8_t) 2);
      temperature->msb = mWireInterface.read();
      temperature->lsb = mWireInterface.read();
      mWireInterface.endRequest();
    }

  private:
    T_WIRE mWireInterface;
};

} // hw
} // ace_time

#endif
