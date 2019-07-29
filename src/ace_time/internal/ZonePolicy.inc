/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

/**
 * A time zone transition rule. It is useful to think of this as a transition
 * rule that repeats on the given (month, day, hour) every year during the
 * interval [fromYear, toYear] inclusive.
 */
struct ZoneRule {
  /**
   * The maximum value of fromYearTiny and toYearTiny. Must be <
   * ZoneEra::kMaxUntilYear.
   */
  static const int8_t kMaxYearTiny = 126;

  /** FROM year as an offset from year 2000 stored as a single byte. */
  int8_t const fromYearTiny;

  /** TO year as an offset from year 2000 stored as a single byte. */
  int8_t const toYearTiny;

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
   * Determined by the AT column in units of 15-minutes from 00:00. The range
   * is (0 - 100) corresponding to 00:00 to 25:00.
   */
  uint8_t const atTimeCode;

  /**
   * Determined by the suffix in the AT column: 'w'=wall; 's'=standard;
   * 'u'=meridian ('g' and 'z' mean the same as 'u' and are not supported
   * because no current TZ file uses them).
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
   * Possible values are 'S', 'D', '-', or a number < 32 (i.e. a non-printable
   * character). If the value is < 32, then this number is an index offset into
   * the ZonePolicy.letters[] array which contains a (const char*) of the
   * actual multi-character letter.
   *
   * As of TZ DB version 2018i, there are 4 ZonePolicies which have ZoneRules
   * with a LETTER field longer than 1 character:
   *
   *  - Belize ('CST'; used by America/Belize)
   *  - Namibia ('WAT', 'CAT'; used by Africa/Windhoek)
   *  - StJohns ('DD'; used by America/St_Johns and America/Goose_Bay)
   *  - Troll ('+00' '+02'; used by Antarctica/Troll)
   */
  uint8_t const letter;
};

/**
 * A collection of transition rules which describe the DST rules of a given
 * administrative region. A given time zone (ZoneInfo) can follow a different
 * ZonePolicy at different times. Conversely, multiple time zones (ZoneInfo)
 * can choose to follow the same ZonePolicy at different times.
 *
 * If numLetters is non-zero, then 'letters' will be a pointer to an array of
 * (const char*) pointers. Any ZoneRule.letter < 32 (i.e. non-printable) will
 * be an offset into this array of pointers.
 */
struct ZonePolicy {
  const ZoneRule* const rules;
  const char* const* const letters;
  uint8_t const numRules;
  uint8_t const numLetters;
};
