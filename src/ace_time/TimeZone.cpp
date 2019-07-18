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
      return;
    case kTypeBasic:
    case kTypeExtended:
    {
      if (! sZoneManager) break;
      ZoneSpecifier* specifier =
          sZoneManager->getZoneSpecifier(mZoneInfo);
      if (! specifier) break;
      specifier->printTo(printer);
      return;
    }
    case kTypeBasicSpecifier:
    case kTypeExtendedSpecifier:
    {
      ZoneSpecifier* specifier = (ZoneSpecifier*) mZoneInfo;
      if (! specifier) break;
      specifier->printTo(printer);
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
        auto utcOffset = TimeOffset::forOffsetCode(mStdOffset + mDstOffset);
        utcOffset.printTo(printer);
        printer.print('(');
        printer.print((mDstOffset != 0) ? "DST" : "STD");
        printer.print(')');
      }
      return;
    case kTypeBasic:
    case kTypeExtended:
    {
      if (! sZoneManager) break;
      ZoneSpecifier* specifier =
          sZoneManager->getZoneSpecifier(mZoneInfo);
      if (! specifier) break;
      specifier->printShortTo(printer);
      return;
    }
    case kTypeBasicSpecifier:
    case kTypeExtendedSpecifier:
    {
      ZoneSpecifier* specifier = (ZoneSpecifier*) mZoneInfo;
      if (! specifier) break;
      specifier->printShortTo(printer);
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
        printer.print((mDstOffset != 0) ? "DST" : "STD");
      }
      return;
    case kTypeBasic:
    case kTypeExtended:
    {
      if (! sZoneManager) break;
      ZoneSpecifier* specifier =
          sZoneManager->getZoneSpecifier(mZoneInfo);
      if (! specifier) break;
      printer.print(specifier->getAbbrev(epochSeconds));
      return;
    }
    case kTypeBasicSpecifier:
    case kTypeExtendedSpecifier:
    {
      ZoneSpecifier* specifier = (ZoneSpecifier*) mZoneInfo;
      if (! specifier) break;
      printer.print(specifier->getAbbrev(epochSeconds));
      return;
    }
  }
  printer.print("<Error>");
}

}
