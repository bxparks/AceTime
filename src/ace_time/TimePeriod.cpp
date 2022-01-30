/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#include <Print.h>
#include <AceCommon.h>
#include "TimePeriod.h"

using ace_common::printPad2To;

namespace ace_time {

void TimePeriod::printTo(Print& printer) const {
  if (isError()) {
    const __FlashStringHelper* message;
    if (mSign == 1) {
      message = F("<+Inf>");
    } else if (mSign == -1) {
      message = F("<-Inf>");
    } else {
      message = F("<Error>");
    }
    printer.print(message);
    return;
  }

  if (mSign < 0) {
    printer.print('-');
  }
  printPad2To(printer, mHour, '0');
  printer.print(':');
  printPad2To(printer, mMinute, '0');
  printer.print(':');
  printPad2To(printer, mSecond, '0');
}

}
