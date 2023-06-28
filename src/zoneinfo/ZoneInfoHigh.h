/*
 * MIT License
 * Copyright (c) 2023 Brian T. Park
 */

#ifndef ACE_TIME_ZONE_INFO_HIGH_H
#define ACE_TIME_ZONE_INFO_HIGH_H

#include <stdint.h>

namespace ace_time {
namespace zoneinfohigh {

/**
 * Metadata about the zone database. A ZoneInfo struct will contain a pointer
 * to this.
 */
template<typename S>
struct ZoneContext {
  /**
   * The maximum value of untilYear. This value is used to represent the
   * sentinel value "-" in the UNTIL column of the TZDB files which means
   * "infinity". Must be greater than ZoneRule::kMaxYear which represents the
   * value "max" in the TO and FROM columns of the TZDB files.
   */
  static const int16_t kMaxUntilYear = 32767;

  /**
   * The maximum value fromYear and toYear. This value is used to represent the
   * sentinel value "max" in the TZDB database files. Must be less than
   * ZoneEra::kMaxUntilYear which is used to represent the entry "-" in the
   * UNTIL column of the TZDB files.
   */
  static const int16_t kMaxYear = kMaxUntilYear - 1;

  /**
   * The minimum value of fromYear and toYear. This value is used for ZoneRule
   * entries which are synthetically generated for certain time zones which do
   * not naturally generate a transition for the database year interval
   * specified by the ZoneContext. This value is guaranteed to be earlier than
   * any explicit year in the TZDB database, which guarantees that all time
   * zones have at least one transition.
   */
  static const int16_t kMinYear = -32767;

  /** Represents 'w' or wall time. */
  static const uint8_t kSuffixW = 0x00;

  /** Represents 's' or standard time. */
  static const uint8_t kSuffixS = 0x10;

  /** Represents 'u' or UTC time. */
  static const uint8_t kSuffixU = 0x20;

  /** Start year of the zone files as requested. */
  int16_t const startYear;

  /** Until year of the zone files as requested. */
  int16_t const untilYear;

  /** Start year of accurate transitions. kMinYear indicates -Infinity. */
  int16_t const startYearAccurate;

  /** Until year of accurate transitions. kMaxUntilYear indicates +Infinity. */
  int16_t const untilYearAccurate;

  /** Base year for tiny years. Unused. */
  int16_t const baseYear;

  /** Max number of transitions required in TransitionStorage. */
  int16_t const maxTransitions;

  /** TZ Database version which generated the zone info. */
  const char* const tzVersion;

  /** Number of fragments. */
  uint8_t const numFragments;

  /** Number of fragments. */
  uint8_t const numLetters;;

  /** Zone Name fragment list. */
  const char* const* const fragments;

  /** Zone Rule letters list. */
  const char* const* const letters;
};

/**
 * A time zone transition rule. It is useful to think of this as a transition
 * rule that repeats on the given (month, day, hour) every year during the
 * interval [fromYear, toYear] inclusive.
 */
template<typename S>
struct ZoneRule {
  /** FROM year */
  int16_t const fromYear;

  /** TO year */
  int16_t const toYear;

  /** Determined by the IN column. 1=Jan, 12=Dec. */
  uint8_t const inMonth;

  /**
   * Determined by the ON column. Possible values are: 0, 1=Mon, 7=Sun.
   * There are 4 combinations:
   * @verbatim
   * onDayOfWeek=0, onDayOfMonth=(1-31): exact match
   * onDayOfWeek=1-7, onDayOfMonth=1-31: dayOfWeek>=dayOfMonth
   * onDayOfWeek=1-7, onDayOfMonth=-(1-31): dayOfWeek<=dayOfMonth
   * onDayOfWeek=1-7, onDayOfMonth=0: last{dayOfWeek}
   * @endverbatim
   */
  uint8_t const onDayOfWeek;

  /**
   * Determined by the ON column. Used with onDayOfWeek. Possible values are:
   * 0, 1-31, or its corresponding negative values.
   */
  int8_t const onDayOfMonth;

  /**
   * The atTimeModifier is a packed field containing 2 pieces of info:
   *
   *    * The upper 4 bits represent the AT time suffix: 'w', 's' or 'u',
   *    represented by kSuffixW, kSuffixS and kSuffixU.
   *    * The lower 4 bits represent the remaining 0-14 seconds of the AT field
   *    after truncation into atTimeCode. In other words, the full AT field in
   *    one-second resolution is (15 * atTimeCode + (atTimeModifier & 0x0f)).
   */
  uint8_t const atTimeModifier;

  /**
   * Determined by the AT column in units of 15-seconds from 00:00. The range
   * is [0,6000] or [0h,25h] in 15-second increments.
   */
  uint16_t const atTimeCode;

  /**
   * Determined by the SAVE column and contains the offset from UTC in minutes.
   * The range is [-128,+127] which allows it to represent DST offset in the
   * range of [-02:00,02:00].
   */
  int8_t const deltaMinutes;

  /**
   * Determined by the LETTER column. Determines the substitution into the '%s'
   * field (implemented here by just a '%') of the ZoneInfo::format field. This
   * is an index offset into the global kLetters array. Most LETTER string is a
   * single character, e.g. "D", "S", or "". But a small number have LETTER
   * fields which are longer than one character. For example:
   *
   *  - Belize ('CST'; used by America/Belize)
   *  - Namibia ('WAT', 'CAT'; used by Africa/Windhoek)
   *  - StJohns ('DD'; used by America/St_Johns and America/Goose_Bay)
   *  - Troll ('+00' '+02'; used by Antarctica/Troll)
   */
  uint8_t const letterIndex;
};

/**
 * A collection of transition rules which describe the DST rules of a given
 * administrative region. A given time zone (ZoneInfo) can follow a different
 * ZonePolicy at different times. Conversely, multiple time zones (ZoneInfo)
 * can choose to follow the same ZonePolicy at different times.
 */
template<typename S>
struct ZonePolicy {
  const ZoneRule<S>* const rules;
  uint8_t const numRules;
};

/**
 * An entry in ZoneInfo which describes which ZonePolicy was being followed
 * during a particular time period. Corresponds to one line of the ZONE record
 * in the TZ Database file ending with an UNTIL field. The ZonePolicy is
 * determined by the RULES column in the TZ Database file.
 *
 * There are 2 types of ZoneEra:
 *    1) zonePolicy == nullptr. Then deltaCode determines the additional offset
 *    from offsetCode. A value of '-' in the TZ Database file is stored as 0.
 *    2) zonePolicy != nullptr. Then the deltaCode offset is given by the
 *    ZoneRule.deltaCode which matches the time instant of interest.
 */
template<typename S>
struct ZoneEra {
  /**
   * Zone policy, determined by the RULES column. Set to nullptr if the RULES
   * column is '-' or an explicit DST shift in the form of 'hh:mm'.
   */
  const ZonePolicy<S>* const zonePolicy;

  /**
   * Zone abbreviations (e.g. PST, EST) determined by the FORMAT column. It has
   * 3 encodings in the TZ DB files:
   *
   *  1) A fixed string, e.g. "GMT".
   *  2) Two strings separated by a '/', e.g. "-03/-02" indicating
   *     "{std}/{dst}" options.
   *  3) A single string with a substitution, e.g. "E%sT", where the "%s" is
   *  replaced by the LETTER value from the ZoneRule.
   *
   * BasicZoneProcessor supports only a single letter subsitution from LETTER,
   * but ExtendedZoneProcessor supports substituting multi-character strings
   * (e.g. "CAT", "DD", "+00").
   *
   * The TZ DB files use '%s' to indicate the substitution, but for simplicity,
   * AceTime replaces the "%s" with just a '%' character with no loss of
   * functionality. This also makes the string-replacement code a little
   * simpler. For example, 'E%sT' is stored as 'E%T', and the LETTER
   * substitution is performed on the '%' character.
   *
   * This field will never be a 'nullptr' if it was derived from an actual
   * entry from the TZ dtabase. There is an internal object named
   * `ExtendedZoneProcessor::kAnchorEra` which does set this field to nullptr.
   * Maybe it should be set to ""?
   */
  const char* const format;

  /**
   * UTC offset in 15-second increments. Determined by the STDOFF column.
   * The remainder goes into the offsetsRemainder field.
   */
  int16_t const offsetCode;

  /** The remainder seconds from offsetCode. */
  uint8_t const offsetRemainder;

  /**
   * If zonePolicy is nullptr, this is the DST offset in minutes as defined by
   * the RULES column in 'hh:mm' format. An 8-bit integer can handle DST
   * offsets of [-128,127] minutes which allows it to handle DST offsets of
   * [-02:00,02:00].
   */
  int8_t const deltaMinutes;

  /**
   * Era is valid until currentTime < untilYear. Comes from the UNTIL column.
   */
  int16_t const untilYear;

  /** The month field in UNTIL (1-12). Will never be 0. */
  uint8_t const untilMonth;

  /**
   * The day field in UNTIL (1-31). Will never be 0. Also, there's no need for
   * untilDayOfWeek, because the database generator will resolve the exact day
   * of month based on the known year and month.
   */
  uint8_t const untilDay;

  /**
   * The time field of UNTIL field in 15-second increments. A range is [0,6000]
   * corresponds to [0h,25h].
   */
  uint16_t const untilTimeCode;

  /**
   * The untilTimeModifier is a packed field containing 2 pieces of info:
   *
   *    * The upper 4 bits represent the UNTIL time suffix: 'w', 's' or 'u',
   *    represented by kSuffixW, kSuffixS and kSuffixU.
   *    * The lower 4 bits represent the remaining 0-14 seconds of the UNTIL
   *    field after truncation into untilTimeCode. In other words, the full
   *    UNTIL field in one-second resolution is (15 * untilTimeCode +
   *    (untilTimeModifier & 0x0f)).
   */
  uint8_t const untilTimeModifier;
};

/**
 * Representation of a given time zone, implemented as an array of ZoneEra
 * records.
 */
template<typename S, typename ZC>
struct ZoneInfo {
  /** Full name of zone (e.g. "America/Los_Angeles"). */
  const char* const name;

  /**
   * Unique, stable ID of the zone name, created from a hash of the name.
   * This ID will never change once assigned. This can be used for presistence
   * and serialization.
   */
  uint32_t const zoneId;

  /** ZoneContext metadata. */
  const ZC* const zoneContext;

  /**
   * Number of ZoneEra entries.
   *
   * If this Zone is a actually a Link to a target Zone, the ZoneEra and
   * numEras of the target Zone is placed here, and the targetInfo pointer
   * below is set to the target ZoneInfo. This allows a Link entry to be
   * self-contained, acting like any other Zone entry, which simplifies some of
   * the code the ZoneProcessor because it does not need to traverse the link
   * to find the required information. In essence, a Link is a "hard link" to a
   * Zone.
   *
   * An alternative implementation is to set numEras to 0 for a Link, and
   * traverse the targetInfo to find the required numEras and eras. This led to
   * more complicated code.
   */
  uint8_t const numEras;

  /**
   * A `const ZoneEras*` pointer to `numEras` ZoneEra entries in increasing
   * order of UNTIL time.
   */
  const ZoneEra<S>* const eras;

  /** If Link, points to the target zone info. If Zone, nullptr. */
  const ZoneInfo* const targetInfo;
};

} // zoneinfohigh
} // ace_time

#endif
