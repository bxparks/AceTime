#ifndef ACE_TIME_ZONEDBX_ZONE_INFO_H
#define ACE_TIME_ZONEDBX_ZONE_INFO_H

#include <stdint.h>
#include "ZonePolicy.h"

namespace ace_time {
namespace zonedbx {

/**
 * An entry in ZoneInfo which describes which ZonePolicy was being followed
 * during a particular time period. Corresponds to one line of the ZONE record
 * in the TZ Database file ending with an UNTIL field. The ZonePolicy is
 * determined by the RULES column in the TZ Database file.
 */
struct ZoneEra {
  /** The maximum value of untilYearTiny. */
  static const int8_t kMaxUntilYearTiny = ZoneRule::kMaxYearTiny + 1;

  /** UTC offset in 15 min increments. Determined by the GMTOFF column. */
  int8_t const offsetCode;

  /**
   * Zone policy, determined by the RULES column. Set to nullptr if the RULES
   * column is '-' or an explicit DST shift in the form of 'hh:mm'.
   */
  const ZonePolicy* const zonePolicy;

  /**
   * If zonePolicy is nullptr, then this indicates the DST offset in 15 minute
   * increments. It could be 0, which means that the 'RULES' column was '-'.
   */
  int8_t const deltaCode;

  /**
   * Zone abbreviations (e.g. PST, EST) determined by the FORMAT column. Only a
   * single letter subsitution is supported so that '%s' is changed to just
   * '%'. For example, 'E%ST' is stored as 'E%T', and the LETTER substitution
   * is performed on the '%' character.
   */
  const char* const format;

  /**
   * Era is valid until currentTime < untilYear. Stored as (year - 2000) in a
   * single byte to save space. Comes from the UNTIL column.
   */
  int8_t const untilYearTiny;

  /** The month field in UNTIL (1-12). Will never be 0. */
  uint8_t const untilMonth;

  /**
   * The day field in UNTIL (1-31). Will never be 0. Also, there's no need for
   * untilDayOfWeek, because the database generator will resolve the exact day
   * of month based on the known year and month.
   */
  uint8_t const untilDay;

  /**
   * The time field of UNTIL field in 15-minute increments. A range of 00:00 to
   * 25:00 corresponds to 0-100.
   */
  uint8_t const untilTimeCode;

  /** UNTIL time modifier suffix: 'w', 's' or 'u'. */
  uint8_t const untilTimeModifier;
};

/**
 * Representation of a given time zone, implemented as an array of ZoneEra
 * records.
 */
struct ZoneInfo {
  /** Name of zone. */
  const char* const name; // name of zone

  /** ZoneEra entries in increasing order of UNTIL time. */
  const ZoneEra* const eras;

  /** Number of ZoneEra entries. */
  uint8_t const numEras;
};

}
}

#endif
