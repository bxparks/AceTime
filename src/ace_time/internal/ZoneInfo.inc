/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

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
 *    ZoneRule.deltaCode of the ZoneRule which matches the time instant of
 *    interest.
 */
struct ZoneEra {
  /** The maximum value of untilYearTiny. */
  static const int8_t kMaxUntilYearTiny = ZoneRule::kMaxYearTiny + 1;

  /**
   * Zone policy, determined by the RULES column. Set to nullptr if the RULES
   * column is '-' or an explicit DST shift in the form of 'hh:mm'.
   */
  const ZonePolicy* const zonePolicy;

  /**
   * Zone abbreviations (e.g. PST, EST) determined by the FORMAT column. Only a
   * single letter subsitution is supported so that '%s' is changed to just
   * '%'. For example, 'E%ST' is stored as 'E%T', and the LETTER substitution
   * is performed on the '%' character.
   */
  const char* const format;

  /** UTC offset in 15 min increments. Determined by the GMTOFF column. */
  int8_t const offsetCode;

  /**
   * If zonePolicy is nullptr, then this indicates the DST offset in 15 minute
   * increments. It could be 0, which means that the 'RULES' column was '-'.
   */
  int8_t const deltaCode;

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

  /**
   * The untilTimeModifier is a packed field containing 2 pieces of info:
   *
   *    * The upper 4 bits represent the UNTIL time suffix: 'w', 's' or 'u',
   *    represented by TIME_SUFFIX_W, TIME_SUFFIX_S and TIME_SUFFIX_U.
   *    * The lower 4 bits represent the remaining 0-14 minutes of the UNTIL
   *    field after truncation into untilTimeCode. In other words, the full
   *    UNTIL field in one-minute resolution is (15 * untilTimeCode +
   *    (untilTimeModifier & 0x0f)).
   */
  uint8_t const untilTimeModifier;
};

/**
 * Representation of a given time zone, implemented as an array of ZoneEra
 * records.
 */
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
  const ZoneContext* const zoneContext;

  /**
   * Estimated size of Transition buffer. Currently, this is used only in the
   * validation unit test for ExtendedZoneProcessor. It is slightly wasteful to
   * include it for the ZoneInfo and ZoneInfo classes used by
   * BasicZoneProcessor, but it's only one byte and it seemed worth having this
   * additional validation capability.
   */
  uint8_t const transitionBufSize;

  /** Number of ZoneEra entries. */
  uint8_t const numEras;

  /** ZoneEra entries in increasing order of UNTIL time. */
  const ZoneEra* const eras;
};
