#include "common/util.h"
#include "common/DateStrings.h"
#include "ZonedDateTime.h"

namespace ace_time {

using common::printPad2;
using common::DateStrings;

// Print ZonedDateTime in ISO 8601 format
void ZonedDateTime::printTo(Print& printer) const {
  if (isError()) {
    printer.print(F("<Invalid ZonedDateTime>"));
    return;
  }

  mOffsetDateTime.printTo(printer);
  mTimeZone.printTo(printer);
}

}
