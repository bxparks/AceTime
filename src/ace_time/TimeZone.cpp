#include <string.h> // strlen()
#include "common/Util.h"
#include "common/DateStrings.h"
#include "DateTime.h"

namespace ace_time {

using common::printPad2;

void TimeZone::printEffectiveOffsetTo(Print& printer) const {
  int8_t sign;
  uint8_t hour;
  uint8_t minute;
  extractEffectiveHourMinute(sign, hour, minute);

  printer.print((sign < 0) ? '-' : '+');
  common::printPad2(printer, hour);
  printer.print(':');
  common::printPad2(printer, minute);
}

void TimeZone::printTo(Print& printer) const {
  int8_t sign;
  uint8_t hour;
  uint8_t minute;
  extractStandardHourMinute(sign, hour, minute);

  printer.print(F("UTC"));
  printer.print((sign < 0) ? '-' : '+');
  common::printPad2(printer, hour);
  printer.print(':');
  common::printPad2(printer, minute);
  printer.print(mIsDst ? F(" DST") : F(" STD"));
}

void TimeZone::initFromOffsetString(const char* ts) {
  // verify exact ISO8601 string length
  if (strlen(ts) != kTimeZoneLength) {
    setError();
    return;
  }

  // '+' or '-'
  char utcSign = *ts++;
  if (utcSign != '-' && utcSign != '+') {
    setError();
    return;
  }

  // hour
  uint8_t hour = (*ts++ - '0');
  hour = 10 * hour + (*ts++ - '0');
  ts++;

  // minute
  uint8_t minute = (*ts++ - '0');
  minute = 10 * minute + (*ts++ - '0');
  ts++;

  uint8_t code = hour * 4 + (minute / 15);
  mTzCode = (utcSign == '-') ? -code : code;
}

}
