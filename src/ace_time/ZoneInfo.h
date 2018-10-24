#ifndef ACE_TIME_ZONE_RULE_H
#define ACE_TIME_ZONE_RULE_H

#include <stdint.h>
#include "ZoneRule.h"

namespace ace_time {

struct ZoneInfo {
  static const ZoneInfo kZoneInfoLosAngeles;
  static const ZoneInfo kZoneInfoToronto;

  const char* const name; // name of zone
  int8_t const offsetCode; // UTC offset in 15 min increments
  const ZoneRule* const* rules;
  uint8_t const numRules;
  const char* const zoneFormat; // PST, EST, etc
};

}

#endif
