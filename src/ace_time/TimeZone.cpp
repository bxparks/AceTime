/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#include <Print.h>
#include "common/util.h"
#include "TimeZone.h"

namespace ace_time {

void TimeZone::printTo(Print& printer) const {
  if (mType == kTypeManual) {
    if (isUtc()) {
      printer.print("UTC");
    } else {
      TimeOffset::forOffsetCode(mStdOffset).printTo(printer);
      TimeOffset::forOffsetCode(mDstOffset).printTo(printer);
    }
  } else {
    mZoneSpecifier->printTo(printer);
  }
}

void TimeZone::printShortTo(Print& printer) const {
  if (mType == kTypeManual) {
    if (isUtc()) {
      printer.print("UTC");
    } else {
      auto utcOffset = TimeOffset::forOffsetCode(mStdOffset + mDstOffset);
      utcOffset.printTo(printer);
      printer.print('(');
      printer.print((mDstOffset != 0) ? "DST" : "STD");
      printer.print(')');
    }
  } else {
    mZoneSpecifier->printShortTo(printer);
  }
}

void TimeZone::printAbbrevTo(Print& printer, acetime_t epochSeconds) const {
  if (mType == kTypeManual) {
    if (isUtc()) {
      printer.print("UTC");
    } else {
      printer.print((mDstOffset != 0) ? "DST" : "STD");
    }
  } else {
    printer.print(mZoneSpecifier->getAbbrev(epochSeconds));
  }
}

}
