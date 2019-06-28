/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#include <Print.h>
#include "common/util.h"
#include "TimeZone.h"

namespace ace_time {

void TimeZone::printTo(Print& printer) const {
  if (mType == kTypeFixed) {
    if (mOffset.isZero()) {
      printer.print("UTC");
    } else{
      mOffset.printTo(printer);
    }
  } else {
    mZoneSpecifier->printTo(printer);
  }
}

void TimeZone::printAbbrevTo(Print& printer, acetime_t epochSeconds) const {
  if (mType == kTypeFixed) {
    if (mOffset.isZero()) {
      printer.print("UTC");
    } else{
      mOffset.printTo(printer);
    }
  } else {
    printer.print(mZoneSpecifier->getAbbrev(epochSeconds));
  }
}

}
