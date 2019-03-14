#include "ExtendedZoneSpecifier.h"

namespace ace_time {

const common::ZoneEra ExtendedZoneSpecifier::kAnchorEra = {
  0 /*offsetCode*/,
  nullptr /*zonePolicy*/,
  0 /*deltaCode*/,
  nullptr /*format*/,
  -128 /*untilYearTiny*/, // TODO: change this to LocalDate::kMinYearTiny?
  1 /*untilMonth*/,
  1 /*untilDay*/,
  0 /*untilTimeCode*/,
  'w' /*untilTimeModifier*/
};

void ExtendedZoneSpecifier::printTo(Print& printer) const {
  printer.print('[');
  printer.print(mZoneInfo->name);
  printer.print(']');
}

}
