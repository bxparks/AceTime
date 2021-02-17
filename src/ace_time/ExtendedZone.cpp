/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#include <Arduino.h>
#include <AceCommon.h> // KString
#include "internal/BrokerCommon.h" // findShortName()
#include "ExtendedZone.h"

using ace_common::KString;
using ace_time::internal::findShortName;
using ace_time::internal::ZoneContext;

namespace ace_time {

#if ACE_TIME_USE_PROGMEM

void ExtendedZone::printNameTo(Print& printer) const {
  const __FlashStringHelper* name = mZoneInfoBroker.name();
  const ZoneContext* zoneContext = mZoneInfoBroker.zoneContext();
  KString kname(name, zoneContext->fragments, zoneContext->numFragments);
  kname.printTo(printer);
}

void ExtendedZone::printShortNameTo(Print& printer) const {
  const __FlashStringHelper* name = mZoneInfoBroker.name();
  const __FlashStringHelper* shortName = findShortName(name);
  printer.print(shortName);
}

#else

void ExtendedZone::printNameTo(Print& printer) const {
  const char* name = mZoneInfoBroker.name();
  const ZoneContext* zoneContext = mZoneInfoBroker.zoneContext();
  KString kname(name, zoneContext->fragments, zoneContext->numFragments);
  kname.printTo(printer);
}

void ExtendedZone::printShortNameTo(Print& printer) const {
  const char* name = mZoneInfoBroker.name();
  const char* shortName = findShortName(name);
  printer.print(shortName);
}

#endif // ACE_TIME_USE_PROGMEM

} // ace_time
