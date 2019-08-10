/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#include <Print.h>
#include "common/util.h"
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
      mZoneProcessor->printTo(printer);
      return;
    case kTypeBasicManaged:
    case kTypeExtendedManaged:
    {
      ZoneProcessor* processor =
          mZoneProcessorCache->getZoneProcessor(mZoneInfo);
      if (! processor) break;
      processor->printTo(printer);
      return;
    }
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
      mZoneProcessor->printShortTo(printer);
      return;
    case kTypeBasicManaged:
    case kTypeExtendedManaged:
    {
      ZoneProcessor* processor =
          mZoneProcessorCache->getZoneProcessor(mZoneInfo);
      if (! processor) break;
      processor->printShortTo(printer);
      return;
    }
  }
  printer.print("<Error>");
}

void TimeZone::printAbbrevTo(Print& printer, acetime_t epochSeconds) const {
  switch (mType) {
    case kTypeManual:
      if (isUtc()) {
        printer.print("UTC");
      } else {
        printer.print((mDstOffsetMinutes != 0) ? "DST" : "STD");
      }
      return;
    case kTypeBasic:
    case kTypeExtended:
      printer.print(mZoneProcessor->getAbbrev(epochSeconds));
      return;
    case kTypeBasicManaged:
    case kTypeExtendedManaged:
    {
      ZoneProcessor* processor =
          mZoneProcessorCache->getZoneProcessor(mZoneInfo);
      if (! processor) break;
      printer.print(processor->getAbbrev(epochSeconds));
      return;
    }
  }
  printer.print("<Error>");
}

}
