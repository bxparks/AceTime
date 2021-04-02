/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#include <Arduino.h> // Print, F()
#include "ZonedDateTime.h"

namespace ace_time {

// Print ZonedDateTime in ISO 8601 format
void ZonedDateTime::printTo(Print& printer) const {
  if (isError()) {
    printer.print(F("<Invalid ZonedDateTime>"));
    return;
  }

  mOffsetDateTime.printTo(printer);
  printer.print('[');
  mTimeZone.printTo(printer);
  printer.print(']');
}

}
