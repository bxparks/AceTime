/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#include "common/compat.h"
#include "LocalDate.h"
#include "ExtendedZoneProcessor.h"

namespace ace_time {

// This is the only instance of ZoneEra whose 'format' field is set to nullptr.
// Should it be set to something like "" instead?
template<>
const extended::ZoneEra ExtendedZoneProcessorTemplate<
    extended::ZoneInfoBroker,
    extended::ZoneEraBroker,
    extended::ZonePolicyBroker,
    extended::ZoneRuleBroker
>::kAnchorEra ACE_TIME_PROGMEM = {
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

}
