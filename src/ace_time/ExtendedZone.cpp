/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#include <Arduino.h>
#include <AceCommon.h> // KString
#include "ExtendedZone.h"

using ace_common::KString;

namespace ace_time {

#if ACE_TIME_USE_PROGMEM

void ExtendedZone::printNameTo(Print& printer) const {
  const auto* name = (const __FlashStringHelper*) mZoneInfoBroker.name();
  const extended::ZoneContext* zoneContext = mZoneInfoBroker.zoneContext();
  KString kname(name, zoneContext->fragments, zoneContext->numFragments);
  kname.printTo(printer);
}

void ExtendedZone::printShortNameTo(Print& printer) const {
  const char* name = mZoneInfoBroker.name();
  const auto* shortName = (const __FlashStringHelper*) findShortName(name);
  printer.print(shortName);
}

const char* ExtendedZone::findShortName(const char* name) {
  size_t len = strlen_P(name);
  const char* begin = name + len;
  bool separatorFound = false;
  while (len--) {
    begin--;
    char c = pgm_read_byte(begin);
    if (c == '/' || (0 < c && c < 32)) {
      separatorFound = true;
      break;
    }
  }
  if (separatorFound) begin++;
  return begin;
}

#else

void ExtendedZone::printNameTo(Print& printer) const {
  const char* name = mZoneInfoBroker.name();
  const extended::ZoneContext* zoneContext = mZoneInfoBroker.zoneContext();
  KString kname(name, zoneContext->fragments, zoneContext->numFragments);
  kname.printTo(printer);
}

void ExtendedZone::printShortNameTo(Print& printer) const {
  const char* name = mZoneInfoBroker.name();
  const char* shortName = findShortName(name);
  printer.print(shortName);
}

const char* ExtendedZone::findShortName(const char* name) {
  size_t len = strlen(name);
  const char* begin = name + len;
  bool separatorFound = false;
  while (len--) {
    begin--;
    char c = *begin;
    if (c == '/' || (0 < c && c < 32)) {
      separatorFound = true;
      break;
    }
  }
  if (separatorFound) begin++;
  return begin;
}

#endif // ACE_TIME_USE_PROGMEM

} // ace_time
