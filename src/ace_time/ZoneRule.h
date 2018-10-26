#ifndef ACE_TIME_ZONE_RULE_H
#define ACE_TIME_ZONE_RULE_H

#include <stdint.h>

namespace ace_time {

/** A time zone transition rule. */
struct ZoneRule {
  uint8_t const fromYear; // 0 = 2000
  uint8_t const toYear; // 255 = 2255
  uint8_t const inMonth; // 1 - 12
  uint8_t const onDayOfWeek; // 1=Monday, 7=Sunday, per ISO 8601
  uint8_t const onDayOfMonth; // 1 - 31
  uint8_t const atHour; // 0-23
  uint8_t const atHourModifier; // 'w'=wall; 's'=standard; 'u'=g=z=meridian
  uint8_t const offsetHour; // 0 - 3
  uint8_t const letter; // 'S', 'D', '-'

  uint8_t compare(const ZoneRule& that) const {
    if (inMonth > that.inMonth) return 1;
    if (inMonth < that.inMonth) return -1;
    if (onDayOfMonth > that.onDayOfMonth) return 1;
    if (onDayOfMonth < that.onDayOfMonth) return -1;
    if (atHour > that.atHour) return 1;
    if (atHour < that.atHour) return -1;
    return 0;
  }
};

/**
 * A collection of transition rules for a given adminstrative region. A given
 * time zone (ZoneInfo) can follow different ZoneRules at different times.
 * Conversely, multiple time zones (ZoneInfo) can follow the same set of
 * ZoneRules at different times.
 */
struct ZoneRules {
  static const ZoneRules kUS;
  static const ZoneRules kEU;
  static const ZoneRules kAN;

  /**
   * Return the matching ZoneRules for the given year. The list is terminated
   * by a nullptr.
   */
  const ZoneRule* findRule(uint8_t year, uint8_t month, uint8_t day) const {
    const ZoneRule* closest = nullptr;
    for (uint8_t i = 0; i < numRules; i++) {
      const ZoneRule& candidate = rules[i];
      if (candidate.fromYear <= year && year <= candidate.toYear) {
        if (candidate.inMonth <= month && candidate.onDayOfMonth <= day) {
          if (closest == nullptr || (candidate.compare(*closest) > 1)) {
            closest = &candidate;
          }
        }
      }
    }
    return closest;
  }

  uint8_t const numRules;
  const ZoneRule* const rules;
};

}

#endif
