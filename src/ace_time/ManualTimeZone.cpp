#include <string.h> // strlen()
#include "common/Util.h"
#include "ManualTimeZone.h"

namespace ace_time {

using common::printPad2;

const ManualTimeZone ManualTimeZone::sUtc(UtcOffset(), false, "UTC", "UTC");

void ManualTimeZone::printTo(Print& printer) const {
  printer.print(F("UTC"));
  mUtcOffset.printTo(printer);
  printer.print(mIsDst ? F(" (DST)") : F(" (STD)"));
}

void ManualTimeZone::parseFromOffsetString(const char* ts,
    uint8_t* offsetCode) {

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

  *offsetCode = UtcOffset::forHourMinute(sign, hour, minute).toOffsetCode();
}

}
