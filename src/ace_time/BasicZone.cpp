/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#include <Arduino.h>
#include <AceCommon.h> // KString
#include "internal/BrokerCommon.h" // findShortName()
#include "BasicZone.h"

using ace_common::KString;
using ace_time::internal::findShortName;

namespace ace_time {

#if ACE_TIME_USE_PROGMEM

void BasicZone::printNameTo(Print& printer) const {
  const __FlashStringHelper* name = mZoneInfoBroker.name();
  const basic::ZoneContext* zoneContext = mZoneInfoBroker.zoneContext();
  KString kname(name, zoneContext->fragments, zoneContext->numFragments);
  kname.printTo(printer);
}

void BasicZone::printShortNameTo(Print& printer) const {
  const __FlashStringHelper* name = mZoneInfoBroker.name();
  const __FlashStringHelper* shortName = findShortName(name);
  printer.print(shortName);
}

#else

void BasicZone::printNameTo(Print& printer) const {
  const char* name = mZoneInfoBroker.name();
  const basic::ZoneContext* zoneContext = mZoneInfoBroker.zoneContext();
  KString kname(name, zoneContext->fragments, zoneContext->numFragments);
  kname.printTo(printer);
}

void BasicZone::printShortNameTo(Print& printer) const {
  const char* name = mZoneInfoBroker.name();
  const char* shortName = findShortName(name);
  printer.print(shortName);
}

#endif // ACE_TIME_USE_PROGMEM

} // ace_time
