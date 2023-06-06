/*
 * MIT License
 * Copyright (c) 2023 Brian T. Park
 */

#include <Arduino.h>
#include <AceCommon.h> // KString
#include "../zoneinfo/BrokerCommon.h" // findShortName()
#include "CompleteZone.h"

using ace_common::KString;
using ace_common::printReplaceCharTo;
using ace_time::internal::findShortName;
using ace_time::complete::ZoneContextBroker;

namespace ace_time {

void CompleteZone::printNameTo(Print& printer) const {
  const __FlashStringHelper* name = mZoneInfoBroker.name();
  ZoneContextBroker zoneContext = mZoneInfoBroker.zoneContext();
  KString kname(name, zoneContext.fragments(), zoneContext.numFragments());
  kname.printTo(printer);
}

void CompleteZone::printShortNameTo(Print& printer) const {
  const __FlashStringHelper* name = mZoneInfoBroker.name();
  const __FlashStringHelper* shortName = findShortName(name);
  printReplaceCharTo(printer, shortName, '_', ' ');
}

} // ace_time
