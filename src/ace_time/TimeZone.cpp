#include <Print.h>
#include "common/util.h"
#include "TimeZone.h"

namespace ace_time {

void TimeZone::printTo(Print& printer) const {
    printer.print('[');
  if (mType == kTypeFixed) {
    if (mOffset.isNonZero()) {
      mOffset.printTo(printer);
    } else{
      printer.print("UTC");
    }
  } else {
    mZoneSpecifier->printTo(printer);
  }
  printer.print(']');
}

void TimeZone::printAbbrevTo(Print& printer, acetime_t epochSeconds) const {
  if (mType == kTypeFixed) {
    if (mOffset.isNonZero()) {
      mOffset.printTo(printer);
    } else{
      printer.print("UTC");
    }
  } else {
    printer.print(mZoneSpecifier->getAbbrev(epochSeconds));
  }
}

}
