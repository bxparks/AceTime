#include <string.h> // strlen()
#include "common/Util.h"
#include "common/DateStrings.h"
#include "TimeZone.h"

namespace ace_time {

using common::printPad2;

void TimeZone::printTo(Print& printer) const {
  printer.print(F("UTC"));
  mZoneOffset.printTo(printer);
  printer.print(mIsDst ? F(" DST") : F(" STD"));
}

TimeZone& TimeZone::initFromOffsetString(const char* ts) {
  // verify exact ISO 8601 string length
  if (strlen(ts) != kTimeZoneLength) {
    return setError();
  }

  // '+' or '-'
  char utcSign = *ts++;
  int8_t sign;
  if (utcSign == '-') {
    sign = -1;
  } else if (utcSign == '+') {
    sign = 1;
  } else {
    return setError();
  }

  // hour
  uint8_t hour = (*ts++ - '0');
  hour = 10 * hour + (*ts++ - '0');

  // ':'
  ts++;

  // minute
  uint8_t minute = (*ts++ - '0');
  minute = 10 * minute + (*ts++ - '0');

  mZoneOffset = ZoneOffset::forHourMinute(sign, hour, minute);
  return *this;
}

}
