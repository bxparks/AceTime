/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#include "LocalDate.h"
#include "ExtendedZone.h"
#include "ExtendedZoneSpecifier.h"

namespace ace_time {

const extended::ZoneEra ExtendedZoneSpecifier::kAnchorEra
    ACE_TIME_EXTENDED_PROGMEM = {
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

void ExtendedZoneSpecifier::printTo(Print& printer) const {
  printer.print(ExtendedZone(mZoneInfo.zoneInfo()).name());
}

void ExtendedZoneSpecifier::printShortTo(Print& printer) const {
  printer.print(ExtendedZone(mZoneInfo.zoneInfo()).shortName());
}

}
