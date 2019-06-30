/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#include "common/util.h"
#include "LocalTime.h"

namespace ace_time {

using common::printPad2;

void LocalTime::printTo(Print& printer) const {
  if (isError()) {
    printer.print(F("<Invalid LocalTime>"));
    return;
  }

  // Time
  printPad2(printer, mHour);
  printer.print(':');
  printPad2(printer, mMinute);
  printer.print(':');
  printPad2(printer, mSecond);
}

LocalTime LocalTime::forTimeString(const char* timeString) {
  if (strlen(timeString) < kTimeStringLength) {
    return forError();
  }
  return forTimeStringChainable(timeString);
}

// This assumes that the dateString is always long enough.
LocalTime LocalTime::forTimeStringChainable(const char*& timeString) {
  const char* s = timeString;

  // hour
  uint8_t hour = (*s++ - '0');
  hour = 10 * hour + (*s++ - '0');

  // ':'
  s++;

  // minute
  uint8_t minute = (*s++ - '0');
  minute = 10 * minute + (*s++ - '0');

  // ':'
  s++;

  // second
  uint8_t second = (*s++ - '0');
  second = 10 * second + (*s++ - '0');

  timeString = s;
  return LocalTime(hour, minute, second);
}

}
