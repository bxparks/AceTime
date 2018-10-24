#include <string.h> // strlen()
#include "common/Util.h"
#include "common/DateStrings.h"
#include "ZoneOffset.h"

namespace ace_time {

using common::printPad2;

void ZoneOffset::printTo(Print& printer) const {
  int8_t sign;
  uint8_t hour;
  uint8_t minute;
  asHourMinute(sign, hour, minute);

  printer.print((sign < 0) ? '-' : '+');
  common::printPad2(printer, hour);
  printer.print(':');
  common::printPad2(printer, minute);
}

ZoneOffset& ZoneOffset::initFromOffsetString(const char* ts) {
  // verify exact ISO8601 string length
  if (strlen(ts) != kTimeZoneLength) {
    return setError();
  }

  // '+' or '-'
  char utcSign = *ts++;
  if (utcSign != '-' && utcSign != '+') {
    return setError();
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
  mOffsetCode = (utcSign == '-') ? -offsetCode : offsetCode;

  return *this;
}

}
