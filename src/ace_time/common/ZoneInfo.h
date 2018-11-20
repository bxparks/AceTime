#ifndef ACE_TIME_ZONE_INFO_H
#define ACE_TIME_ZONE_INFO_H

#include <stdint.h>
#include "ZonePolicy.h"

namespace ace_time {
namespace common {

/**
 * An entry in ZoneInfo which describes which ZonePolicy was being followed
 * during a particular time period. The ZonePolicy is determined by the RULES
 * column in the TZ Database file.
 */
struct ZoneEntry {

  /** UTC offset in 15 min increments. Determined by the GMTOFF column. */
  int8_t const offsetCode;

  /**
   * Zone policy, determined by the RULES column. Set to nullptr if the RULES
   * column is '-'
   */
  const ZonePolicy* const zonePolicy;

  /**
   * Zone abbreviations (e.g. PST, EST) determined by the FORMAT column. Only a
   * single letter subsitution is supported so that '%s' is changed to just
   * '%'. For example, 'E%ST' is stored as 'E%T', and the LETTER substitution
   * is performed on the '%' character.
   */
  const char* const format;

  /**
   * Entry is valid until currentTime < untilYear (Jan 1 00:00). 0 = 2000.
   * Comes from the UNTIL column.
   */
  uint8_t const untilYear;

  // TODO: add untilMonth, untilDayOfMonth, untilDayOfWeek, untilTime,
  // untilTimeModifier
};

/**
 * Representation of a given time zone, implemented as a collection of
 * ZoneEntry objects.
 */
struct ZoneInfo {
  /** Name of zone. */
  const char* const name; // name of zone

  /** ZoneEntry records in increasing order of untilYear. */
  const ZoneEntry* const entries;

  /** Number of ZoneEntry records. */
  uint8_t const numEntries;
};

}
}

#endif
