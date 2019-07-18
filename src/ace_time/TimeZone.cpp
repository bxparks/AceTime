/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#include <Print.h>
#include "common/util.h"
#include "TimeZone.h"

namespace ace_time {

ZoneManager* TimeZone::sZoneManager = nullptr;

void TimeZone::printTo(Print& printer) const {
  switch (mType) {
    case kTypeManual:
      if (isUtc()) {
        printer.print("UTC");
      } else {
        TimeOffset::forOffsetCode(mStdOffset).printTo(printer);
        TimeOffset::forOffsetCode(mDstOffset).printTo(printer);
      }
      break;
    case kTypeBasic:
    case kTypeExtended:
    {
      if (! sZoneManager) {
        printer.print("<Error>");
        return;
      }
      ZoneSpecifier* specifier =
          sZoneManager->getZoneSpecifier(mZoneInfo);
      if (! specifier) {
        printer.print("<Error>");
        return;
      }
      specifier->printTo(printer);
      break;
    }
    default:
      printer.print("<Error>");
      break;
  }
}

void TimeZone::printShortTo(Print& printer) const {
  switch (mType) {
    case kTypeManual:
      if (isUtc()) {
        printer.print("UTC");
      } else {
        auto utcOffset = TimeOffset::forOffsetCode(mStdOffset + mDstOffset);
        utcOffset.printTo(printer);
        printer.print('(');
        printer.print((mDstOffset != 0) ? "DST" : "STD");
        printer.print(')');
      }
      break;
    case kTypeBasic:
    case kTypeExtended:
    {
      if (! sZoneManager) {
        printer.print("<Error>");
        return;
      }
      ZoneSpecifier* specifier =
          sZoneManager->getZoneSpecifier(mZoneInfo);
      if (! specifier) {
        printer.print("<Error>");
        return;
      }
      specifier->printShortTo(printer);
      break;
    }
    default:
      printer.print("<Error>");
      break;
  }
}

void TimeZone::printAbbrevTo(Print& printer, acetime_t epochSeconds) const {
  switch (mType) {
    case kTypeManual:
      if (isUtc()) {
        printer.print("UTC");
      } else {
        printer.print((mDstOffset != 0) ? "DST" : "STD");
      }
      break;
    case kTypeBasic:
    case kTypeExtended:
    {
      if (! sZoneManager) {
        printer.print("<Error>");
        return;
      }
      ZoneSpecifier* specifier =
          sZoneManager->getZoneSpecifier(mZoneInfo);
      if (! specifier) {
        printer.print("<Error>");
        return;
      }
      printer.print(specifier->getAbbrev(epochSeconds));
      break;
    }
    default:
      printer.print("<Error>");
      break;
  }
}

}
