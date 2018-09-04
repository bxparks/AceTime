#include <Wire.h>
#include <Print.h> // Print
#include "../common/Util.h" // bcdToDec(), decToBcd()
#include "HardwareDateTime.h"
#include "HardwareTemperature.h"
#include "DS3231.h"

namespace ace_time {

using common::bcdToDec;
using common::decToBcd;

namespace hw {

void DS3231::readDateTime(HardwareDateTime* dateTime) const {
  Wire.beginTransmission(mAddress);
  Wire.write(0); // set DS3231 register pointer to 00h
  Wire.endTransmission();

  // request seven bytes of data from DS3231 starting from register 00h
  Wire.requestFrom(mAddress, (uint8_t) 7);
  dateTime->second = bcdToDec(Wire.read() & 0x7F);
  dateTime->minute = bcdToDec(Wire.read());
  dateTime->hour = bcdToDec(Wire.read() & 0x3F);
  Wire.read(); // read, then discard
  dateTime->day = bcdToDec(Wire.read());
  dateTime->month = bcdToDec(Wire.read());
  dateTime->year = bcdToDec(Wire.read());
}

void DS3231::readTemperature(HardwareTemperature* temperature) const {
  Wire.beginTransmission(mAddress);
  Wire.write(0x11); // set DS3231 register pointer to 11h
  Wire.endTransmission();

  Wire.requestFrom(mAddress, (uint8_t) 2);
  temperature->msb = Wire.read();
  temperature->lsb = Wire.read();
}

void DS3231::setDateTime(const HardwareDateTime& dateTime) const {
  Wire.beginTransmission(mAddress);
  Wire.write(0); // set next input to start at the seconds register
  Wire.write(decToBcd(dateTime.second));
  Wire.write(decToBcd(dateTime.minute));
  Wire.write(decToBcd(dateTime.hour));
  Wire.write(decToBcd(dateTime.dayOfWeek));
  Wire.write(decToBcd(dateTime.day));
  Wire.write(decToBcd(dateTime.month));
  Wire.write(decToBcd(dateTime.year));
  Wire.endTransmission();
}

}
}
