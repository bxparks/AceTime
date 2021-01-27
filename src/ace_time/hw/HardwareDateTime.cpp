/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#if ! defined(EPOXY_DUINO)

#include <AceCommon.h>
#include "HardwareDateTime.h"
#include "../common/DateStrings.h"

using ace_common::printPad2To;

namespace ace_time {

namespace hw {

// Print HardwareDateTime in ISO8601 format
void HardwareDateTime::printTo(Print& printer) const {
  // Date
  printer.print(F("20"));
  printPad2To(printer, year, '0');
  printer.print('-');
  printPad2To(printer, month, '0');
  printer.print('-');
  printPad2To(printer, day, '0');
  printer.print('T');

  // Time
  printPad2To(printer, hour, '0');
  printer.print(':');
  printPad2To(printer, minute, '0');
  printer.print(':');
  printPad2To(printer, second, '0');

  // Week day
  printer.print(DateStrings().dayOfWeekLongString(dayOfWeek));
}


}
}

#endif
