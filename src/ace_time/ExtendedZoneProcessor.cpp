/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#include <Print.h>
#include "LocalDate.h"
#include "ExtendedZone.h"
#include "ExtendedZoneProcessor.h"

namespace ace_time {

const extended::ZoneEra ExtendedZoneProcessor::kAnchorEra ACE_TIME_PROGMEM = {
  0 /*offsetCode*/,
  nullptr /*zonePolicy*/,
  0 /*deltaCode*/,
  nullptr /*format*/,
  LocalDate::kInvalidYearTiny /*untilYearTiny*/,
  1 /*untilMonth*/,
  1 /*untilDay*/,
  0 /*untilTimeCode*/,
  'w' /*untilTimeModifier*/
};

void ExtendedZoneProcessor::printTo(Print& printer) const {
  printer.print(ExtendedZone(mZoneInfo.zoneInfo()).name());
}

void ExtendedZoneProcessor::printShortTo(Print& printer) const {
  printer.print(ExtendedZone(mZoneInfo.zoneInfo()).shortName());
}

}
