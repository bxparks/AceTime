/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#include <Print.h>
#include "TimePeriod.h"

namespace ace_time {

using common::printPad2;

void TimePeriod::printTo(Print& printer) const {
  if (mSign < 0) {
    printer.print('-');
  }
  printPad2(printer, mHour);
  printer.print(':');
  printPad2(printer, mMinute);
  printer.print(':');
  printPad2(printer, mSecond);
}

}
