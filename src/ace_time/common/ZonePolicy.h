#ifndef ACE_TIME_ZONE_POLICY_H
#define ACE_TIME_ZONE_POLICY_H

#include <stdint.h>

namespace ace_time {
namespace common {

/**
 * A time zone transition rule. It is useful to think of this as a transition
 * rule that repeats on the given (month, day, hour) every year during the
 * interval [fromYear, toYear] inclusive.
 */
struct ZoneRule {
  static const uint16_t kMaxYear = 9999;

  /** Determined by the FROM column. Supports years before 2000. */
  uint16_t const fromYear;

  /**
   * Determined by the TO column. Supports years before 2000. "max" is
   * represented by 9999.
   */
  uint16_t const toYear;

  /** Determined by the IN column. 1=Jan, 12=Dec. */
  uint8_t const inMonth;

  /**
   * Determined by the ON column. Possible values are: 0, 1=Mon, 7=Sun.
   * There are 3 combinations:
   * @verbatim
   * onDayOfWeek=0, onDayOfMonth=(1-31): exact match
   * onDayOfWeek=1-7, onDayOfMonth=1-31: dayOfWeek>=dayOfMonth
   * onDayOfWeek=1-7, onDayOfMonth=0: last{dayOfWeek}
   * @endverbatim
   *
   * We support only the '>=' operator, not the '<=' operator which does not
   * seem to be used currently.
   */
  uint8_t const onDayOfWeek;

  /**
   * Determined by the ON column. Used with onDayOfWeek. Possible values are:
   * 0, 1-31.
   */
  uint8_t const onDayOfMonth;

  /** Determined by the AT column. 0-23. */
  uint8_t const atHour;

  /**
   * Determined by the suffix in the AT column:
   * 'w'=wall; 's'=standard; 'u'=meridian; ('g' and 'z' mean the same as 'u'
   * and are not supported because no current TZ file uses them)
   */
  uint8_t const atTimeModifier;

  /**
   * Determined by the SAVE column, containing the offset from UTC, in 15-min
   * increments.
   */
  int8_t const deltaCode;

  /**
   * Determined by the LETTER column. Determines the substitution into the '%s'
   * field (implemented here by just a '%') of the ZoneInfo::format field.
   * Possible values are 'S', 'D', '-'. There are only 2 Rule entries which
   * have LETTER fields longer than 1 characters as of TZ Database version
   * 2018g: Rule Namibia (used by Africa/Windhoek) and Rule Troll (used by
   * Antarctica/Troll).
   */
  uint8_t const letter;
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
}

#endif
