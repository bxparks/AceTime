/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#include <string.h> // strlen()
#include <Arduino.h> // strncpy_P()
#include <AceCommon.h>
#include "common/DateStrings.h"
#include "PlainDateTime.h"

using ace_common::printPad2To;

namespace ace_time {

void PlainDateTime::printTo(Print& printer) const {
  if (isError()) {
    printer.print(F("<Invalid PlainDateTime>"));
    return;
  }

  // Date
  printer.print(mPlainDate.year());
  printer.print('-');
  printPad2To(printer, mPlainDate.month(), '0');
  printer.print('-');
  printPad2To(printer, mPlainDate.day(), '0');

  // 'T' separator
  printer.print('T');

  // Time
  printPad2To(printer, mPlainTime.hour(), '0');
  printer.print(':');
  printPad2To(printer, mPlainTime.minute(), '0');
  printer.print(':');
  printPad2To(printer, mPlainTime.second(), '0');
}

PlainDateTime PlainDateTime::forDateString(const char* dateString) {
  if (strlen(dateString) < kDateTimeStringLength) {
    return PlainDateTime::forError();
  }
  return forDateStringChainable(dateString);
}

PlainDateTime PlainDateTime::forDateString(
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

PlainDateTime PlainDateTime::forDateStringChainable(const char*& dateString) {
  const char* s = dateString;

  // date
  PlainDate pd = PlainDate::forDateStringChainable(s);

  // 'T'
  s++;

  // time
  PlainTime pt = PlainTime::forTimeStringChainable(s);

  dateString = s;
  return PlainDateTime(pd, pt);
}

}
