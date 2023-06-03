/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#include <Arduino.h>
#include <AceCommon.h> // KString
#include "../zoneinfo/BrokerCommon.h" // findShortName()
#include "BasicZone.h"

using ace_common::KString;
using ace_common::printReplaceCharTo;
using ace_time::internal::findShortName;
using ace_time::basic::ZoneContextBroker;

namespace ace_time {

#if ACE_TIME_USE_PROGMEM

void BasicZone::printNameTo(Print& printer) const {
  const __FlashStringHelper* name = mZoneInfoBroker.name();
  ZoneContextBroker zoneContext = mZoneInfoBroker.zoneContext();
  KString kname(name, zoneContext.fragments(), zoneContext.numFragments());
  kname.printTo(printer);
}

void BasicZone::printShortNameTo(Print& printer) const {
  const __FlashStringHelper* name = mZoneInfoBroker.name();
  const __FlashStringHelper* shortName = findShortName(name);
  printReplaceCharTo(printer, shortName, '_', ' ');
}

#else

void BasicZone::printNameTo(Print& printer) const {
  const char* name = mZoneInfoBroker.name();
  ZoneContextBroker zoneContext = mZoneInfoBroker.zoneContext();
  KString kname(name, zoneContext.fragments(), zoneContext.numFragments());
  kname.printTo(printer);
}

void BasicZone::printShortNameTo(Print& printer) const {
  const char* name = mZoneInfoBroker.name();
  const char* shortName = findShortName(name);
  printReplaceCharTo(printer, shortName, '_', ' ');
}

#endif // ACE_TIME_USE_PROGMEM

} // ace_time
