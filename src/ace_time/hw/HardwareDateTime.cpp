#include "HardwareDateTime.h"
#include "../common/Util.h"
#include "../common/DateStrings.h"

namespace ace_time {

using common::printPad2;
using common::DateStrings;

namespace hw {

// Print HardwareDateTime in ISO8601 format
void HardwareDateTime::printTo(Print& printer) const {
  // Date
  printer.print(F("20"));
  printPad2(printer, year);
  printer.print('-');
  printPad2(printer, month);
  printer.print('-');
  printPad2(printer, day);
  printer.print('T');

  // Time
  printPad2(printer, hour);
  printer.print(':');
  printPad2(printer, minute);
  printer.print(':');
  printPad2(printer, second);

  // Week day
  printer.print(DateStrings().weekDayLongString(dayOfWeek));
}


}
}
