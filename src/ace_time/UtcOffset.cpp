#include <string.h> // strlen()
#include "common/util.h"
#include "common/DateStrings.h"
#include "UtcOffset.h"

namespace ace_time {

using common::printPad2;

void UtcOffset::printTo(Print& printer) const {
  int8_t sign;
  uint8_t hour;
  uint8_t minute;
  toHourMinute(sign, hour, minute);

  printer.print((sign < 0) ? '-' : '+');
  common::printPad2(printer, hour);
  printer.print(':');
  common::printPad2(printer, minute);
}

UtcOffset UtcOffset::forOffsetString(const char* offsetString) {
  // verify exact ISO 8601 string length
  if (strlen(offsetString) != kUtcOffsetStringLength) {
    return forError();
  }

  return forOffsetStringChainable(offsetString);
}

UtcOffset UtcOffset::forOffsetStringChainable(const char*& offsetString) {
  const char* s = offsetString;

  // '+' or '-'
  char utcSign = *s++;
  if (utcSign != '-' && utcSign != '+') {
    return forError();
  }
  int8_t sign = (utcSign == '+') ? 1 : -1;

  // hour
  uint8_t hour = (*s++ - '0');
  hour = 10 * hour + (*s++ - '0');
  s++;

  // minute
  uint8_t minute = (*s++ - '0');
  minute = 10 * minute + (*s++ - '0');
  s++;

  offsetString = s;
  return UtcOffset::forHourMinute(sign, hour, minute);
}

}
