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

UtcOffset UtcOffset::forOffsetString(const char* ts) {
  // verify exact ISO 8601 string length
  if (strlen(ts) != kUtcOffsetStringLength) {
    return forError();
  }

  // '+' or '-'
  char utcSign = *ts++;
  if (utcSign != '-' && utcSign != '+') {
    return forError();
  }

  // hour
  uint8_t hour = (*ts++ - '0');
  hour = 10 * hour + (*ts++ - '0');
  ts++;

  // minute
  uint8_t minute = (*ts++ - '0');
  minute = 10 * minute + (*ts++ - '0');
  ts++;

  uint8_t offsetCode = hour * 4 + (minute / 15);
  int8_t code = (utcSign == '-') ? -offsetCode : offsetCode;

  return UtcOffset(code);
}

}
