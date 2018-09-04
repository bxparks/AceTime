#include "common/Util.h"
#include "common/DateStrings.h"
#include "DateTime.h"

namespace ace_time {

using common::printPad2;

void TimeZone::printTo(Print& printer) const {
  uint8_t hour;
  uint8_t minute;
  extractHourMinute(hour, minute);

  printer.print((mTzCode < 0) ? '-' : '+');
  common::printPad2(printer, hour);
  printer.print(':');
  common::printPad2(printer, minute);
}

}
