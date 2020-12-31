/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#include <Print.h>
#include "LocalDate.h"
#include "ExtendedZone.h"
#include "ExtendedZoneProcessor.h"

namespace ace_time {

// This is the only instance of ZoneEra whose 'format' field is set to nullptr.
// Should it be set to something like "" instead?
const extended::ZoneEra ExtendedZoneProcessor::kAnchorEra ACE_TIME_PROGMEM = {
  nullptr /*zonePolicy*/,
  nullptr /*format*/,
  0 /*offsetCode*/,
  0 /*deltaCode*/,
  LocalDate::kInvalidYearTiny /*untilYearTiny*/,
  1 /*untilMonth*/,
  1 /*untilDay*/,
  0 /*untilTimeCode*/,
  extended::ZoneContext::kSuffixW /*untilTimeModifier*/
};

void ExtendedZoneProcessor::printTo(Print& printer) const {
  printer.print(ExtendedZone(mZoneInfo.zoneInfo()).name());
}

void ExtendedZoneProcessor::printShortTo(Print& printer) const {
  printer.print(ExtendedZone(mZoneInfo.zoneInfo()).shortName());
}

}
