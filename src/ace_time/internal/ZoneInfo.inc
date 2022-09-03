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
  /** The maximum value of untilYear. */
  static const int16_t kMaxUntilYear = ZoneRule::kMaxYear + 1;

  /**
   * Zone policy, determined by the RULES column. Set to nullptr if the RULES
   * column is '-' or an explicit DST shift in the form of 'hh:mm'.
   */
  const ZonePolicy* const zonePolicy;

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

  /** UTC offset in 15 min increments. Determined by the STDOFF column. */
  int8_t const offsetCode;

  /**
   * If zonePolicy is nullptr, then this indicates the DST offset in 15 minute
   * increments as defined by the RULES column in 'hh:mm' format. If the
   * 'RULES' column is '-', then the deltaCode is 0.
   *
   * If the '--scope extended' flag is given to tzcompiler.py, the 'deltaCode`
   * should be interpreted as a uint8_t field, composed of two 4-bit fields:
   *
   *    * The upper 4-bits is an unsigned integer from 0 to 14 that represents
   *    the one-minute remainder from the offsetCode. This allows us to capture
   *    STDOFF offsets in 1-minute resolution.
   *    * The lower 4-bits is an unsigned integer that holds (originalDeltaCode
   *    + 4). This allows us to represent DST offsets from -1:00 to +2:45, in
   *    15-minute increments.
   *
   * The extended::ZoneEraBroker::deltaMinutes() and offsetMinutes() know how
   * to convert offsetCode and deltaCode into the appropriate minutes.
   */
  int8_t const deltaCode;

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
   * The time field of UNTIL field in 15-minute increments. A range of 00:00 to
   * 25:00 corresponds to 0-100.
   */
  uint8_t const untilTimeCode;

  /**
   * The untilTimeModifier is a packed field containing 2 pieces of info:
   *
   *    * The upper 4 bits represent the UNTIL time suffix: 'w', 's' or 'u',
   *    represented by kSuffixW, kSuffixS and kSuffixU.
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
  const internal::ZoneContext* const zoneContext;

  /** Number of ZoneEra entries. Set to 0 if this Zone is a actually a Link. */
  uint8_t const numEras;

  /**
   * A `const ZoneEras*` pointer or a `const ZoneInfo*` pointer. For a normal
   * Zone, numEras is greater than 0, and this field is a pointer to the
   * ZoneEra entries in increasing order of UNTIL time. For a Link entry,
   * numEras == 0, and this field provides a level of indirection to a (const
   * ZoneInfo*) pointer to the target Zone. We have to follow the indirect
   * pointer, and resolve the target numEras and eras to obtain the actual
   * ZoneEra entries.
   */
  const void* const eras;
};
