#ifndef ACE_TIME_ZONE_RULE_H
#define ACE_TIME_ZONE_RULE_H

#include <stdint.h>
#include "ZoneRule.h"

namespace ace_time {

/**
 * An entry in ZoneInfo which describe which rule the zone followed during
 * a particular time period, starting with fromYear.
 */
struct ZoneInfoEntry {
  int8_t const offsetCode; // UTC offset in 15 min increments
  const ZoneRules* const zoneRules; // ZoneRules
  const char* const format; // PST, EST, etc
  uint8_t const untilYear; // 0 = 2000
};

/** Data structure that represents a given time zone. */
struct ZoneInfo {
  static const ZoneInfo kLosAngeles;
  static const ZoneInfo kToronto;
  static const ZoneInfo kPetersburg;
  static const ZoneInfo kIndianapolis;
  static const ZoneInfo kLondon;

  const char* const name; // name of zone
  ZoneInfoEntry const* entries;
  uint8_t const numEntries;
};

}

#endif
