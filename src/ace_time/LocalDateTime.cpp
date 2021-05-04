/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#include <string.h> // strlen()
#include <Arduino.h> // strncpy_P(), Print
#include <AceCommon.h>
#include "common/DateStrings.h"
#include "LocalDateTime.h"

using ace_common::printPad2To;

namespace ace_time {

void LocalDateTime::printTo(Print& printer) const {
  if (isError()) {
    printer.print(F("<Invalid LocalDateTime>"));
    return;
  }

  // Date
  printer.print(mLocalDate.year());
  printer.print('-');
  printPad2To(printer, mLocalDate.month(), '0');
  printer.print('-');
  printPad2To(printer, mLocalDate.day(), '0');

  // 'T' separator
  printer.print('T');

  // Time
  printPad2To(printer, mLocalTime.hour(), '0');
  printer.print(':');
  printPad2To(printer, mLocalTime.minute(), '0');
  printer.print(':');
  printPad2To(printer, mLocalTime.second(), '0');
}

LocalDateTime LocalDateTime::forDateString(const char* dateString) {
  if (strlen(dateString) < kDateTimeStringLength) {
    return LocalDateTime::forError();
  }
  return forDateStringChainable(dateString);
}


LocalDateTime LocalDateTime::forDateStringChainable(const char*& dateString) {
  const char* s = dateString;

  // date
  LocalDate ld = LocalDate::forDateStringChainable(s);

  // 'T'
  s++;

  // time
  LocalTime lt = LocalTime::forTimeStringChainable(s);

  dateString = s;
  return LocalDateTime(ld, lt);
}

LocalDateTime LocalDateTime::forDateString(
    const __FlashStringHelper* dateString) {
  // Copy the F() string into a buffer. Use strncpy_P() because ESP32 and
  // ESP8266 do not have strlcpy_P(). We need +1 for the '\0' character and
  // another +1 to determine if the dateString is too long to fit.
  char buffer[kDateTimeStringLength + 2];
  strncpy_P(buffer, (const char*) dateString, sizeof(buffer));
  buffer[kDateTimeStringLength + 1] = 0;

  // check if the original F() was too long
  size_t len = strlen(buffer);
  if (len > kDateTimeStringLength) {
    return forError();
  }

  return forDateString(buffer);
}

} // ace_time
