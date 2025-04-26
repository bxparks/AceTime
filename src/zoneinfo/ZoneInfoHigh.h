/*
 * MIT License
 * Copyright (c) 2023 Brian T. Park
 */

#ifndef ACE_TIME_ZONE_INFO_HIGH_H
#define ACE_TIME_ZONE_INFO_HIGH_H

/**
 * @file ZoneInfoHigh.h
 *
 * Data structures that encodes the high resolution zoneinfo database
 * persistence format. It has a 1-second resolution for AT, UNTIL, STDOFF, and
 * DST offsets. The year fiels use 2-bytes which supporting years
 * `[-32767,32765]`.
 *
 * The BrokersHigh.h file provides an abtraction layer which converts these
 * low-level fields into a semantically consistent API which can be used by the
 * AceTime classes.
 *
 * The various zoneinfo database files (e.g. zonedb, zonedbx, zonedbc) will
 * use one of these persistence formats, as defined by infos.h.
 *
 * See also DEVELOPER.md for an overview of the ZoneInfoXXX layer.
 */

#include <stdint.h>
#include <Arduino.h> // pgm_read_xxx()
#include <AceCommon.h> // KString
#include "compat.h" // ACE_TIME_USE_PROGMEM
#include "BrokerCommon.h"

class __FlashStringHelper;
class Print;

namespace ace_time {

/**
 * Wrapper class so that the entire collection can be referenced as a singel
 * template parameter.
 */
class ZoneInfoHigh {
public:

/**
 * Metadata about the zone database. A ZoneInfo struct will contain a pointer
 * to this.
 */
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
struct ZonePolicy {
  const ZoneRule* const rules;
  uint8_t const numRules;
};

/**
 * An entry in ZoneInfo which describes which ZonePolicy was being followed
 * during a particular time period. Corresponds to one line of the ZONE record
 * in the TZ Database file ending with an UNTIL field. The ZonePolicy is
 * determined by the RULES column in the TZ Database file.
 *
 * There are 2 types of ZoneEra:
 *    1) zonePolicy == nullptr. Then ZoneEra.deltaMinutes determines the
 *    additional offset from offsetCode. A value of '-' in the TZ Database file
 *    is stored as 0.
 *    2) zonePolicy != nullptr. Then the deltaMinutes offset is given by the
 *    ZoneRule.deltaMinutes which matches the time instant of interest.
 */
struct ZoneEra {
  /**
   * Zone policy, determined by the RULES column. Set to nullptr if the RULES
   * column is '-' or an explicit DST shift in the form of 'hh:mm'.
   */
  const ZonePolicy* const zonePolicy;

  /**
   * Zone abbreviations (e.g. PST, EST) determined by the FORMAT column. It has
   * 4 encodings in the TZ DB files:
   *
   *  1) A fixed string, e.g. "GMT".
   *  2) Two strings separated by a '/', e.g. "-03/-02" indicating
   *     "{std}/{dst}" options.
   *  3) A single string with a substitution, e.g. "E%sT", where the "%s" is
   *  replaced by the LETTER value from the ZoneRule.
   *  4) An empty string representing the "%z" format.
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
   * This field will never be a 'nullptr' because the AceTime Compiler always
   * generates a ZoneEra entry with a non-null format.
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
  const ZoneEra* const eras;

  /** If Link, points to the target zone info. If Zone, nullptr. */
  const ZoneInfo* const targetInfo;
};

//-----------------------------------------------------------------------------
// Brokers are wrappers around the above data objects so that outside code
// can use the data objects with a consistent API.
//-----------------------------------------------------------------------------

/**
 * Convert the deltaMinutes holding the RULES/DSTOFF field in ZoneEra or the
 * SAVE field in ZoneRule to delta offset in seconds.
 */
static int32_t toDeltaSeconds(uint8_t deltaMinutes) {
  return int32_t(60) * (int8_t) deltaMinutes;
}

/**
 * Convert (code, remainder) holding the STDOFF field of ZoneEra into seconds.
 */
static int32_t toOffsetSeconds(uint16_t offsetCode, uint8_t offsetRemainder) {
  return int32_t(15) * (int16_t) offsetCode + (int32_t) offsetRemainder;
}

/**
 * Convert (code, modifier) holding the UNTIL time in ZoneInfo or AT time in
 * ZoneRule into seconds. The `code` parameter holds the AT or UNTIL time in
 * units of 15 seconds. The lower 4-bits of `modifier` holds the remainder
 * seconds.
 */
static uint32_t timeCodeToSeconds(uint16_t code, uint8_t modifier) {
  return code * (uint32_t) 15 + (modifier & 0x0f);
}

/**
 * Extract the 'w', 's' 'u' suffix from the 'modifier' field, so that they can
 * be compared against kSuffixW, kSuffixS and kSuffixU. Used for Zone.UNTIL and
 * Rule.AT  fields.
 */
static uint8_t toSuffix(uint8_t modifier) {
  return modifier & 0xf0;
}

//-----------------------------------------------------------------------------

/**
 * Data broker for accessing a ZoneContext.
 */
class ZoneContextBroker {
  public:
    explicit ZoneContextBroker(const ZoneContext* zoneContext = nullptr)
        : mZoneContext(zoneContext)
    {}

    // use the default copy constructor
    ZoneContextBroker(const ZoneContextBroker&) = default;

    // use the default assignment operator
    ZoneContextBroker& operator=(const ZoneContextBroker&) = default;

    bool isNull() const { return mZoneContext == nullptr; }

    const ZoneContext* raw() const { return mZoneContext; }

    int16_t startYear() const {
      return (int16_t) pgm_read_word(&mZoneContext->startYear);
    }

    int16_t untilYear() const {
      return (int16_t) pgm_read_word(&mZoneContext->untilYear);
    }

    int16_t startYearAccurate() const {
      return (int16_t) pgm_read_word(&mZoneContext->startYearAccurate);
    }

    int16_t untilYearAccurate() const {
      return (int16_t) pgm_read_word(&mZoneContext->untilYearAccurate);
    }

    int16_t baseYear() const {
      return (int16_t) pgm_read_word(&mZoneContext->baseYear);
    }

    int16_t maxTransitions() const {
      return (int16_t) pgm_read_word(&mZoneContext->maxTransitions);
    }

    const __FlashStringHelper* tzVersion() const {
      return (const __FlashStringHelper*)
          pgm_read_ptr(&mZoneContext->tzVersion);
    }

    uint8_t numFragments() const {
      return (uint8_t) pgm_read_byte(&mZoneContext->numFragments);
    }

    uint8_t numLetters() const {
      return (uint8_t) pgm_read_byte(&mZoneContext->numLetters);
    }

    const __FlashStringHelper* const* fragments() const {
      return (const __FlashStringHelper* const*)
          pgm_read_ptr(&mZoneContext->fragments);
    }

    const __FlashStringHelper* letter(uint8_t i) const {
      const char * const* letters = (const char* const*)
          pgm_read_ptr(&mZoneContext->letters);
      const char* letter = (const char*) pgm_read_ptr(letters + i);
      return (const __FlashStringHelper*) letter;
    }

  private:
    const ZoneContext* mZoneContext;
};

//-----------------------------------------------------------------------------

/**
 * Data broker for accessing ZoneRule.
 */
class ZoneRuleBroker {
  public:
    explicit ZoneRuleBroker(
        const ZoneContext* zoneContext = nullptr,
        const ZoneRule* zoneRule = nullptr)
        : mZoneContext(zoneContext)
        , mZoneRule(zoneRule)
    {}

    // use the default copy constructor
    ZoneRuleBroker(const ZoneRuleBroker&) = default;

    // use the default assignment operator
    ZoneRuleBroker& operator=(const ZoneRuleBroker&) = default;

    bool isNull() const { return mZoneRule == nullptr; }

    int16_t fromYear() const {
      return pgm_read_word(&mZoneRule->fromYear);
    }

    int16_t toYear() const {
      return pgm_read_word(&mZoneRule->toYear);
    }

    uint8_t inMonth() const {
      return pgm_read_byte(&mZoneRule->inMonth);
    }

    uint8_t onDayOfWeek() const {
      return pgm_read_byte(&mZoneRule->onDayOfWeek);
    }

    int8_t onDayOfMonth() const {
      return pgm_read_byte(&mZoneRule->onDayOfMonth);
    }

    uint32_t atTimeSeconds() const {
      return timeCodeToSeconds(
          pgm_read_word(&mZoneRule->atTimeCode),
          pgm_read_byte(&mZoneRule->atTimeModifier));
    }

    uint8_t atTimeSuffix() const {
      return toSuffix(pgm_read_byte(&mZoneRule->atTimeModifier));
    }

    int32_t deltaSeconds() const {
      return toDeltaSeconds(pgm_read_byte(&mZoneRule->deltaMinutes));
    }

    const __FlashStringHelper* letter() const {
      uint8_t index = pgm_read_byte(&mZoneRule->letterIndex);
      return ZoneContextBroker(mZoneContext).letter(index);
    }

  private:
    const ZoneContext* mZoneContext;
    const ZoneRule* mZoneRule;
};

/**
 * Data broker for accessing ZonePolicy.
 */
class ZonePolicyBroker {
  public:
    explicit ZonePolicyBroker(
        const ZoneContext* zoneContext,
        const ZonePolicy* zonePolicy)
        : mZoneContext(zoneContext)
        , mZonePolicy(zonePolicy)
    {}

    // use default copy constructor
    ZonePolicyBroker(const ZonePolicyBroker&) = default;

    // use default assignment operator
    ZonePolicyBroker& operator=(const ZonePolicyBroker&) = default;

    bool isNull() const { return mZonePolicy == nullptr; }

    uint8_t numRules() const {
      return pgm_read_byte(&mZonePolicy->numRules);
    }

    const ZoneRuleBroker rule(uint8_t i) const {
      const ZoneRule* rules =
          (const ZoneRule*) pgm_read_ptr(&mZonePolicy->rules);
      return ZoneRuleBroker(mZoneContext, &rules[i]);
    }

  private:
    const ZoneContext* mZoneContext;
    const ZonePolicy* mZonePolicy;
};

//-----------------------------------------------------------------------------

/**
 * Data broker for accessing ZoneEra.
 */
class ZoneEraBroker {
  public:
    explicit ZoneEraBroker(
        const ZoneContext* zoneContext = nullptr,
        const ZoneEra* zoneEra = nullptr)
        : mZoneContext(zoneContext)
        , mZoneEra(zoneEra)
    {}

    // use default copy constructor
    ZoneEraBroker(const ZoneEraBroker&) = default;

    // use default assignment operator
    ZoneEraBroker& operator=(const ZoneEraBroker&) = default;

    bool isNull() const { return mZoneEra == nullptr; }

    bool equals(const ZoneEraBroker& other) const {
      return mZoneEra == other.mZoneEra;
    }

    const ZonePolicyBroker zonePolicy() const {
      return ZonePolicyBroker(
          mZoneContext,
          (const ZonePolicy*) pgm_read_ptr(&mZoneEra->zonePolicy));
    }

    int32_t offsetSeconds() const {
      return toOffsetSeconds(
          pgm_read_word(&mZoneEra->offsetCode),
          pgm_read_byte(&mZoneEra->offsetRemainder));
    }

    int32_t deltaSeconds() const {
      return toDeltaSeconds(pgm_read_byte(&mZoneEra->deltaMinutes));
    }

    const char* format() const {
      return (const char*) pgm_read_ptr(&mZoneEra->format);
    }

    int16_t untilYear() const {
      return pgm_read_word(&mZoneEra->untilYear);
    }

    uint8_t untilMonth() const {
      return pgm_read_byte(&mZoneEra->untilMonth);
    }

    uint8_t untilDay() const {
      return pgm_read_byte(&mZoneEra->untilDay);
    }

    uint32_t untilTimeSeconds() const {
      return timeCodeToSeconds(
        pgm_read_word(&mZoneEra->untilTimeCode),
        pgm_read_byte(&mZoneEra->untilTimeModifier));
    }

    uint8_t untilTimeSuffix() const {
      return toSuffix(pgm_read_byte(&mZoneEra->untilTimeModifier));
    }

  private:
    const ZoneContext* mZoneContext;
    const ZoneEra* mZoneEra;
};

/**
 * Data broker for accessing ZoneInfo.
 */
class ZoneInfoBroker {
  public:
    explicit ZoneInfoBroker(const ZoneInfo* zoneInfo = nullptr):
        mZoneInfo(zoneInfo) {}

    // use default copy constructor
    ZoneInfoBroker(const ZoneInfoBroker&) = default;

    // use default assignment operator
    ZoneInfoBroker& operator=(const ZoneInfoBroker&) = default;

    /**
     * @param zoneKey an opaque Zone primary key (e.g. const ZoneInfo*, or a
     *    uint16_t index into a database table of ZoneInfo records)
     */
    bool equals(uintptr_t zoneKey) const {
      return mZoneInfo == (const ZoneInfo*) zoneKey;
    }

    bool equals(const ZoneInfoBroker& zoneInfoBroker) const {
      return mZoneInfo == zoneInfoBroker.mZoneInfo;
    }

    bool isNull() const { return mZoneInfo == nullptr; }

    const ZoneContextBroker zoneContext() const {
      const ZoneContext* context =
          (const ZoneContext*) pgm_read_ptr(&mZoneInfo->zoneContext);
      return ZoneContextBroker(context);
    }

    const __FlashStringHelper* name() const {
      return FPSTR(pgm_read_ptr(&mZoneInfo->name));
    }

    uint32_t zoneId() const {
      return pgm_read_dword(&mZoneInfo->zoneId);
    }

    uint8_t numEras() const {
      return pgm_read_byte(&mZoneInfo->numEras);
    }

    const ZoneEraBroker era(uint8_t i) const {
      auto eras = (const ZoneEra*) pgm_read_ptr(&mZoneInfo->eras);
      return ZoneEraBroker(zoneContext().raw(), &eras[i]);
    }

    bool isLink() const {
      return mZoneInfo->targetInfo != nullptr;
    }

    ZoneInfoBroker targetInfo() const {
      return ZoneInfoBroker(
          (const ZoneInfo*) pgm_read_ptr(&mZoneInfo->targetInfo));
    }

    /** Print a human-readable identifier (e.g. "America/Los_Angeles"). */
    void printNameTo(Print& printer) const {
      ZoneContextBroker zc = zoneContext();
      ace_common::KString kname(name(), zc.fragments(), zc.numFragments());
      kname.printTo(printer);
    }

    /**
     * Print a short human-readable identifier (e.g. "Los Angeles").
     * Any underscore in the short name is replaced with a space.
     */
    void printShortNameTo(Print& printer) const {
      ace_common::printReplaceCharTo(
          printer, zoneinfo::findShortName(name()), '_', ' ');
    }

  private:
    const ZoneInfo* mZoneInfo;
};

//-----------------------------------------------------------------------------

/**
 * Data broker for accessing the ZoneRegistry. The ZoneRegistry is an
 * array of (const ZoneInfo*) in the zone_registry.cpp file.
 */
class ZoneRegistryBroker {
  public:
    ZoneRegistryBroker(const ZoneInfo* const* zoneRegistry):
        mZoneRegistry(zoneRegistry) {}

    // use default copy constructor
    ZoneRegistryBroker(const ZoneRegistryBroker&) = default;

    // use default assignment operator
    ZoneRegistryBroker& operator=(const ZoneRegistryBroker&) = default;

    const ZoneInfo* zoneInfo(uint16_t i) const {
      return (const ZoneInfo*) pgm_read_ptr(&mZoneRegistry[i]);
    }

  private:
    const ZoneInfo* const* mZoneRegistry;
};

//-----------------------------------------------------------------------------
// A factory class for a ZoneInfoBroker.
//-----------------------------------------------------------------------------

/**
 * A storage object that creates an ZoneInfoBroker from a key that identifies
 * the ZoneInfo. The key can be a pointer to flash memory, or an integer into
 * a list stored in a file.
 */
class ZoneInfoStore {
  public:
    /**
     * @param zoneKey an opaque Zone primary key (e.g. const ZoneInfo*, or a
     *    uint16_t index into a database table of ZoneInfo records)
     */
    ZoneInfoBroker createZoneInfoBroker(uintptr_t zoneKey) const {
      return ZoneInfoBroker((const ZoneInfo*) zoneKey);
    }
};

}; // ZoneInfoHigh

} // ace_time

#endif
