#ifndef ACE_TIME_ZONE_RULE_H
#define ACE_TIME_ZONE_RULE_H

#include <stdint.h>

namespace ace_time {

/** A time zone transition rule. */
struct ZoneRule {
  uint8_t const fromYear; // 0=2000, 255=2255
  uint8_t const toYear; // 0=2000, 255=2255
  uint8_t const inMonth; // 1 - 12

  // onDayOfWeek=0, onDayOfMonth=(1-31): exact match
  // onDayOfWeek=1-7, onDayOfMonth=1-31: dayOfWeek>=dayOfMonth
  // onDayOfWeek=1-7, onDayOfMonth=0: last{dayOfWeek}
  uint8_t const onDayOfWeek; // 0, 1=Mon, 7=Sun
  uint8_t const onDayOfMonth; // 0, 1-31

  uint8_t const atHour; // 0-23
  uint8_t const atHourModifier; // 'w'=wall; 's'=standard; 'u'=g=z=meridian
  int8_t const deltaCode; // 0 - 12, DST shift in 15-min increments
  uint8_t const letter; // 'S', 'D', '-'
};

/**
 * A collection of transition rules which describe the DST rules of a given
 * administrative region. A given time zone (ZoneInfo) can follow a different
 * ZonePolicy at different times. Conversely, multiple time zones (ZoneInfo)
 * can choose to follow the same ZonePolicy at different times.
 */
struct ZonePolicy {
  uint8_t const numRules;
  const ZoneRule* const rules;
};

}

#endif
