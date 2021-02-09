/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#include <Print.h>
#include "TimeZone.h"

namespace ace_time {

void TimeZone::printTo(Print& printer) const {
  switch (mType) {
    case kTypeManual:
      if (isUtc()) {
        printer.print("UTC");
      } else {
        TimeOffset::forMinutes(mStdOffsetMinutes).printTo(printer);
        TimeOffset::forMinutes(mDstOffsetMinutes).printTo(printer);
      }
      return;
    case kTypeBasic:
    case kTypeExtended:
      mZoneProcessor->printNameTo(printer);
      return;
  }
  printer.print("<Error>");
}

void TimeZone::printShortTo(Print& printer) const {
  switch (mType) {
    case kTypeManual:
      if (isUtc()) {
        printer.print("UTC");
      } else {
        auto utcOffset = TimeOffset::forMinutes(
            mStdOffsetMinutes + mDstOffsetMinutes);
        utcOffset.printTo(printer);
        printer.print('(');
        printer.print((mDstOffsetMinutes != 0) ? "DST" : "STD");
        printer.print(')');
      }
      return;
    case kTypeBasic:
    case kTypeExtended:
      mZoneProcessor->printShortNameTo(printer);
      return;
  }
  printer.print("<Error>");
}

}
