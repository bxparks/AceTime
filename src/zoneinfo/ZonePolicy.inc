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
   * The minimum value of fromYear and toYear. This value is used for ZoneRule
   * entries which are synthetically generated for certain time zones which do
   * not naturally generate a transition for the database year interval
   * specified by the ZoneContext. This value is guaranteed to be earlier than
   * any explicit year in the TZDB database, which guarantees that all time
   * zones have at least one transition.
   */
  static const int16_t kMinYear = -32767;

  /**
   * The maximum value fromYear and toYear. This value is used to represent the
   * sentinel value "max" in the TZDB database files. Must be less than
   * ZoneEra::kMaxUntilYear which is used to represent the entry "-" in the
   * UNTIL column of the TZDB files.
   */
  static const int16_t kMaxYear = 32766;

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
   * Determined by the AT column in units of 15-minutes from 00:00. The range
   * is (0 - 100) corresponding to 00:00 to 25:00.
   */
  uint8_t const atTimeCode;

  /**
   * The atTimeModifier is a packed field containing 2 pieces of info:
   *
   *    * The upper 4 bits represent the AT time suffix: 'w', 's' or 'u',
   *    represented by kSuffixW, kSuffixS and kSuffixU.
   *    * The lower 4 bits represent the remaining 0-14 minutes of the AT field
   *    after truncation into atTimeCode. In other words, the full AT field in
   *    one-minute resolution is (15 * atTimeCode + (atTimeModifier & 0x0f)).
   */
  uint8_t const atTimeModifier;

  /**
   * Determined by the SAVE column and contains the offset from UTC, in 15-min
   * increments. The deltaCode is equal to (originalDeltaCode + 4). Only the
   * lower 4-bits is used, for consistency with the ZoneEra::deltaCode field.
   * This allows the 4-bits to represent DST offsets from -1:00 to 2:45 in
   * 15-minute increments.
   *
   * The ZonePolicyBroker::deltaMinutes() method knows how to convert this
   * field into minutes.
   */
  uint8_t const deltaCode;

  /**
   * Determined by the LETTER column. Determines the substitution into the '%s'
   * field (implemented here by just a '%') of the ZoneInfo::format field. This
   * is an index offset into the global kLetters array. Most LETTER string is a
   * single character, e.g. "D", "S", or "". But a small number have LETTER
   * fields which are longer than one character. As of TZDB version 2018i,
   * these are:
   *
   *  - Belize ('CST'; used by America/Belize)
   *  - Namibia ('WAT', 'CAT'; used by Africa/Windhoek)
   *  - StJohns ('DD'; used by America/St_Johns and America/Goose_Bay)
   *  - Troll ('+00' '+02'; used by Antarctica/Troll)
   *
   * TODO: Current BasicZoneProcessor supports only single-character LETTER.
   * But I think it can be easily extended to support multi-character LETTER
   * fields.
   */
  uint8_t const letterIndex;
};

/**
 * A collection of transition rules which describe the DST rules of a given
 * administrative region. A given time zone (ZoneInfo) can follow a different
 * ZonePolicy at different times. Conversely, multiple time zones (ZoneInfo)
 * can choose to follow the same ZonePolicy at different times.
 */
struct ZonePolicy {
  const ZoneRule* const rules;
  uint8_t const numRules;
};
