/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#if ! defined(UNIX_HOST_DUINO)

#include "HardwareDateTime.h"
#include "../common/util.h"
#include "../common/DateStrings.h"

namespace ace_time {

using common::printPad2;

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
  printer.print(DateStrings().dayOfWeekLongString(dayOfWeek));
}


}
}

#endif
