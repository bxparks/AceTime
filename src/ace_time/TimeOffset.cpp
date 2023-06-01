/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#include <string.h> // strlen()
#include <AceCommon.h>
#include "common/DateStrings.h"
#include "TimeOffset.h"

using ace_common::printPad2To;

namespace ace_time {

void TimeOffset::printTo(Print& printer) const {
  int8_t hour;
  int8_t minute;
  int8_t second;
  toHourMinuteSecond(hour, minute, second);

  if (mSeconds < 0) {
    printer.print('-');
    hour = -hour;
    minute = -minute;
    second = -second;
  } else {
    printer.print('+');
  }
  printPad2To(printer, hour, '0');
  printer.print(':');
  printPad2To(printer, minute, '0');
  if (second != 0) {
    printer.print(':');
    printPad2To(printer, second, '0');
  }
}

TimeOffset TimeOffset::forOffsetString(const char* offsetString) {
  // Verify length of ISO 8601 string, either 6 ("-hh:mm") or 9 ("-hh:mm:ss").
  uint8_t len = strlen(offsetString);
  if (len != 6 && len != 9) {
    return forError();
  }

  return forOffsetStringChainable(offsetString);
}

TimeOffset TimeOffset::forOffsetStringChainable(const char*& offsetString) {
  const char* s = offsetString;

  // '+' or '-'
  char sign = *s++;
  if (sign != '-' && sign != '+') {
    return forError();
  }

  // hour
  int8_t hour = (*s++ - '0');
  hour = 10 * hour + (*s++ - '0');
  s++; // skip ':'

  // minute
  int8_t minute = (*s++ - '0');
  minute = 10 * minute + (*s++ - '0');

  // second if necessary
  int8_t second = 0;
  if (*s) {
    s++; // skip ':'
    second = (*s++ - '0');
    second = 10 * second + (*s++ - '0');
    s++;
  }

  offsetString = s;
  if (sign == '+') {
    return forHourMinuteSecond(hour, minute, second);
  } else {
    return forHourMinuteSecond(-hour, -minute, -second);
  }
}

}
