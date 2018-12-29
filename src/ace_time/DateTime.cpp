#include "common/util.h"
#include "common/DateStrings.h"
#include "DateTime.h"

namespace ace_time {

using common::printPad2;
using common::DateStrings;

// Print DateTime in ISO 8601 format
void DateTime::printTo(Print& printer) const {
  if (isError()) {
    printer.print(F("<Invalid DateTime>"));
    return;
  }

  mOffsetDateTime.printTo(printer);
  if (mTimeZone.getType() == TimeZone::kTypeAuto) {
    printer.print(' ');
    mTimeZone.printTo(printer);
  }
}

}
