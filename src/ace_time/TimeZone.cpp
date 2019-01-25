#include <Print.h>
#include "common/util.h"
#include "TimeZone.h"

namespace ace_time {

using common::printPad2;

void TimeZone::printTo(Print& printer) const {
  if (getType() == kTypeAuto) {
    AutoZoneSpec* spec = static_cast<AutoZoneSpec*>(mZoneSpec);
    printer.print('[');
    printer.print(spec->getZoneInfo()->name);
    printer.print(']');
  } else {
    ManualZoneSpec* spec = static_cast<ManualZoneSpec*>(mZoneSpec);
    printer.print(F("UTC"));
    spec->stdOffset().printTo(printer);
    printer.print(spec->isDst() ? F(" (DST)") : F(" (STD)"));
  }
}

void TimeZone::parseFromOffsetString(const char* ts, uint8_t* offsetCode) {
  // verify exact ISO 8601 string length
  if (strlen(ts) != kUtcOffsetStringLength) {
    *offsetCode = UtcOffset::kErrorCode;
    return;
  }

  // '+' or '-'
  char utcSign = *ts++;
  int8_t sign;
  if (utcSign == '-') {
    sign = -1;
  } else if (utcSign == '+') {
    sign = 1;
  } else {
    *offsetCode = UtcOffset::kErrorCode;
    return;
  }

  // hour
  uint8_t hour = (*ts++ - '0');
  hour = 10 * hour + (*ts++ - '0');

  // ':'
  ts++;

  // minute
  uint8_t minute = (*ts++ - '0');
  minute = 10 * minute + (*ts++ - '0');

  *offsetCode = UtcOffset::forHourMinute(sign, hour, minute).code();
}

}
