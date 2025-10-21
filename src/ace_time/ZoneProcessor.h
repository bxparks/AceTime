/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#ifndef ACE_TIME_ZONE_PROCESSOR_H
#define ACE_TIME_ZONE_PROCESSOR_H

#include "common/common.h" // kAbbrevSize
#include "OffsetDateTime.h"

class Print;

namespace ace_time {

class PlainDateTime;

/**
 * Result of a search for transition at a specific epochSeconds or a specific
 * PlainDateTime. More than one transition can match if the PlainDateTime occurs
 * during an overlap (e.g. during a "fall back" from DST to STD).
 */
class FindResult {
  public:
    /** The epochSeconds or PlainDateTime was not found. */
    static const uint8_t kTypeNotFound = 0;

    /** The epochSeconds or PlainDateTime matched a unique ZonedDateTime. */
    static const uint8_t kTypeExact = 1;

    /** The PlainDateTime matched a gap. */
    static const uint8_t kTypeGap = 2;

    /** The PlainDateTime matched an overlap. */
    static const uint8_t kTypeOverlap = 3;

    /**
     * Result of the findByEpochSeconds() or findByPlainDateTime() search
     * methods. There are 2 slightly different cases:
     *
     * Case findByPlainDateTime():
     *  * kTypeNotFound:
     *      * No matching Transition found.
     *  * kTypeExact:
     *      * A single Transition found.
     *  * kTypeGap:
     *      * PlainDateTime occurs in a gap.
     *      * ZonedDateTime::resolved is set to either returns the earlier
     *        transition in reqStdOffsetSeconds and reqDstOffsetSeconds, and the
     *        later transition in stdOffsetSeconds and dstOffsetSeconds.
     *      * ZonedDateTime::fold=1 returns the later transition in
     *        reqStdOffsetSeconds and reqDstOffsetSeconds, and the
     *        earlier transition in stdOffsetSeconds and dstOffsetSeconds.
     *  * kTypeOverlap:
     *      * PlainDateTime matches 2 Transitions due to an overlap.
     *      * ZonedDateTime::resolved is set to Resolved::kOverlapEarlier or
     *        Resolved::kOverlapLater depending on the
     *        'disambiguate' flag.
     *
     * Case findByEpochSeconds():
     *  * kTypeNotFound:
     *      * If no matching Transition found.
     *  * kTypeExact:
     *      * Only a single Transition found.
     *  * kTypeGap:
     *      * Cannot occur.
     *  * kTypeOverlap:
     *      * Cannot occur.
     */
    uint8_t type = kTypeNotFound;

    /**
     * Characterize the result in the gap or overlap further. The 'fold'
     * parameter is used only if type is 'kTypeGap' or 'kTypeOverlap'.
     *
     * For findByEpochSeconds(), the `fold` parameter is relevant only if
     * epochSeconds falls in an overlap (type==kTypeOverlap).
     *    * fold=0 means that the requested epochSeconds matched a backwards
     *    shadow of a later transition (e.g. the first time 1:30am was seen
     *    before a fallback from 2am to 1am).
     *    * fold=1 means that the requested epochSeconds matched the forward
     *    shadow of an earlier transition (e.g. the second time 1:30am was seen
     *    after a fallback from 2am to 1am).
     *
     * For findByPlainDateTime(), the `fold` parameter is relevant for both
     * kTypeGap and kTypeOverlap.
     *    * If the requested PlainDateTime is in an overlap:
     *        * `fold=0` means that the "select earlier"
     *        (Disambiguate::kCompatible or Disambiguate::kEarlier) was
     *        requested,
     *        * `fold=1` means that the "select later" (Disambiguate::kReversed
     *        or Disambiguate::kLater) option was requested.
     *    * If the requested PlainDateTime is in a gap:
     *        * `fold=0` means the earlier transition was requested by a "select
     *        later" was requested (specified by Disambiguate::kCompatible or
     *        Disambiguate::Earlier);
     *        * `fold=1` means that the later transition was selected using
     *        "select earlier" (specified by Disambiguate::kReversed or
     *        Disambiguate::Later).
     */
    uint8_t fold = 0;

    /** STD offset of the resulting OffsetDateTime. */
    int32_t stdOffsetSeconds = 0;

    /** DST offset of the resulting OffsetDateTime. */
    int32_t dstOffsetSeconds = 0;

    /**
     * STD offset of the Transition which matched the epochSeconds requested by
     * findByEpochSeconds(), or the PlainDateTime requested by
     * findByPlainDateTime().
     *
     * This may be different than the stdOffsetSeconds when
     * findByPlainDateTime() returns kTypeGap. For all other resulting types
     * from findByEpochSeconds(), and for all resulting types from
     * findByPlainDateTime(), the reqStdOffsetSeconds will be the same as
     * stdOffsetSeconds.
     */
    int32_t reqStdOffsetSeconds = 0;

    /**
     * DST offset of the Transition which matched the epochSeconds requested by
     * findByEpochSeconds(), or the PlainDateTime requested by
     * findByPlainDateTime().
     *
     * This may be different than the dstOffsetSeconds when
     * findByPlainDateTime() returns kTypeGap. For all other resulting types
     * from findByEpochSeconds(), and for all resulting types from
     * findByPlainDateTime(), the reqStdOffsetSeconds will be the same as
     * dstOffsetSeconds.
     */
    int32_t reqDstOffsetSeconds = 0;

    /**
     * Pointer to the abbreviation stored in the transient Transition::abbrev
     * variable. The calling code should copy the string into a local buffer
     * quickly, before any other timezone calculations are performed.
     */
    const char* abbrev = "";
};

/**
 * Base interface for ZoneProcessor classes. There were 2 options for
 * implmenting the various concrete implementations of ZoneProcessors:
 *
 * 1) Implement only a single getType() method to distinguish the different
 * runtime types of the object. Then use this type information in the TimeZone
 * class to downcast the ZoneProcessor pointer to the correct subclass, and
 * call the correct methods.
 *
 * 2) Fully implement a polymorphic class hierarchy, lifting various common
 * methods (e.g. findByPlainDateTime(), findByEpochSeconds()) into this
 * interface as virtual methods, then add a virtual equals() method to implement
 * the operator==().
 *
 * The problem with Option 1 is that the code for both subclasses would be
 * compiled into the program, even if the application used only one of the
 * subclasses. Instead I use Option 2, using a fully polymorphic class
 * hierarchy, adding 3-4 virtual methods. When a program uses only a single
 * subclass, only that particular subclass is included into the program.
 * Unfortunately, this comes at the cost of forcing programs to use the virtual
 * dispatch at runtime for some of the often-used methods.
 */
class ZoneProcessor {
  public:
    /** Return the kTypeXxx of the current instance. */
    uint8_t getType() const { return mType; }

    /** Return true if timezone is a Link entry pointing to a Zone entry. */
    virtual bool isLink() const = 0;

    /** Return the unique stable zoneId. */
    virtual uint32_t getZoneId() const = 0;

    /** Return the search results at given PlainDateTime. */
    virtual FindResult findByPlainDateTime(
        const PlainDateTime& pdt,
        Disambiguate disambiguate) const = 0;

    /** Return the search results at given epochSeconds. */
    virtual FindResult findByEpochSeconds(
        acetime_t epochSeconds) const = 0;

    /**
     * Print a human-readable identifier (e.g. "America/Los_Angeles").
     *
     * @param printer an instance of the Print class, usually Serial
     */
    virtual void printNameTo(Print& printer) const = 0;

    /**
     * Print a short human-readable identifier (e.g. "Los Angeles").
     * Any underscore in the short name is replaced with a space.
     *
     * @param printer an instance of the Print class, usually Serial
     */
    virtual void printShortNameTo(Print& printer) const = 0;

    /**
     * Print the full identifier (e.g. "America/Los_Angeles") of the target zone
     * if the current zone is a Link entry. Otherwise, print nothing.
     *
     * @param printer an instance of the Print class, usually Serial
     */
    virtual void printTargetNameTo(Print& printer) const = 0;

    /**
     * Set the opaque zoneKey of this object to a new value, reseting any
     * internally cached information. If the new zoneKey is the same as the old
     * zoneKey, the cache remains valid.
     *
     * Normally a ZoneProcessor object is associated with a single TimeZone.
     * However, the ZoneProcessorCache will sometimes "take over" a
     * ZoneProcessor from another TimeZone using this method. The other TimeZone
     * will take back control of the ZoneProcessor if needed. To avoid bouncing
     * the ownership of this object repeatedly, the application should configure
     * the ZoneProcessorCache with enough ZoneProcessors to handle the usage
     * pattern of the given application.
     *
     * This method should be considered to be private, to be used only by the
     * TimeZone and ZoneProcessorCache classes. I had to make it public because
     * it got too ugly to maintain the `friend` list in C++.
     *
     * @param zoneKey an opaque Zone primary key (e.g. const ZoneInfo*, or a
     *    uint16_t index into a database table of ZoneInfo records)
     */
    virtual void setZoneKey(uintptr_t zoneKey) = 0;

    /**
     * Return true if ZoneProcessor is associated with the given opaque
     * zoneKey. This method should be considered to be private.
     *
     * @param zoneKey an opaque Zone primary key (e.g. const ZoneInfo*, or a
     *    uint16_t index into a database table of ZoneInfo records)
     */
    virtual bool equalsZoneKey(uintptr_t zoneKey) const = 0;

  protected:
    friend bool operator==(const ZoneProcessor& a, const ZoneProcessor& b);

    // Disable copy constructor and assignment operator.
    ZoneProcessor(const ZoneProcessor&) = delete;
    ZoneProcessor& operator=(const ZoneProcessor&) = delete;

    /** Constructor. */
    ZoneProcessor(uint8_t type):
      mType(type) {}

    /**
     * Check if the Transition cache is filled for the given year and current
     * epochYear. Checking the epoch year allows the cache to be automatically
     * invalidated and regenerated if the epoch year is changed.
     */
    bool isFilled(int16_t year) const {
      return year == mYear && mEpochYear == Epoch::currentEpochYear();
    }

    /** Return true if equal. */
    virtual bool equals(const ZoneProcessor& other) const = 0;

  protected:
    // The order of the fields is optimized to save space on 32-bit processors.
    /**
     * User-visible indicator of the subclass of ZoneProcessor, which implments
     * a specific time-zone algorithm. Three common ones are
     * BasicZoneProcessor::kTypeBasic and ExtendedZoneProcessor::kTypeExtended.
     */
    uint8_t const mType;

    /**
     * Year that was used to calculate the transitions in the current cache. Set
     * to PlainDate::kInvalidYear to indicate invalid cache.
     */
    mutable int16_t mYear = PlainDate::kInvalidYear;

    /**
     * Epoch year that was used to calculate the transitions in the current
     * cache. Set to PlainDate::kInvalidYear to indicate invalid cache.
     */
    mutable int16_t mEpochYear = PlainDate::kInvalidYear;
};

inline bool operator==(const ZoneProcessor& a, const ZoneProcessor& b) {
  if (a.mType != b.mType) return false;
  return a.equals(b);
}

inline bool operator!=(const ZoneProcessor& a, const ZoneProcessor& b) {
  return ! (a == b);
}

/** The result of calcStartDayOfMonth(). */
struct MonthDay {
  uint8_t month;
  uint8_t day;
};

/**
  * Calculate the actual (month, day) of the expresssion (onDayOfWeek >=
  * onDayOfMonth) or (onDayOfWeek <= onDayOfMonth).
  *
  * There are 4 combinations:
  *
  * @verbatim
  * onDayOfWeek=0, onDayOfMonth=(1-31): exact match
  * onDayOfWeek=1-7, onDayOfMonth=1-31: dayOfWeek>=dayOfMonth
  * onDayOfWeek=1-7, onDayOfMonth=0: last{dayOfWeek}
  * onDayOfWeek=1-7, onDayOfMonth=-(1-31): dayOfWeek<=dayOfMonth
  * @endverbatim
  *
  * Caveats: This method handles expressions which crosses month boundaries,
  * but not year boundaries (e.g. Jan to Dec of the previous year, or Dec to
  * Jan of the following year.)
  */
MonthDay calcStartDayOfMonth(int16_t year, uint8_t month,
    uint8_t onDayOfWeek, int8_t onDayOfMonth);

/**
  * Create the time zone abbreviation in dest from the format string
  * (e.g. "P%T", "E%T"), the time zone deltaMinutes (!= 0 means DST), and the
  * replacement letter (e.g. 'S', 'D', '\0' (represented as '-' in the
  * Rule.LETTER entry). If the Zone.RULES column is '-' or 'hh:mm', then
  * 'letter' will be set to '\0' also, although
  * AceTimeSuite/compiler/src/acetimecompiler/transformer/transformer.py
  * should have detected this condition and filtered that zone out.
  *
  * Starting from v2.3, the same algorithm is used by both BasicZoneProcessor
  * and ExtendedZoneProcessor which simplifies the maintenance of the code,
  * and allows the BasicZoneProcessor to support a few more timezones which
  * have `letter` fields that are longer than a single-character.
  *
  * The abbreviation algorithm is roughly the following:
  *
  * 1) If the FORMAT contains a '%', then:
  *
  *    1a) If the letter is '\0', then the '%' is removed. This indicates the
  *    Zone.Rule was ('-', 'hh:mm'), or Rule.LETTER was a '-'.
  *
  *    1b) Else the 'letter' is a string (e.g. 'S', 'D', "DD" etc) from the
  *    Rule.LETTER column, so replace '%' with with the given 'letter' string.
  *
  * 2) If the FORMAT contains a '/', then, ignore the 'letter' string and just
  * use deltaMinutes in the following way:
  *
  *    2a) If deltaMinutes is 0, pick the first component, i.e. before the '/'.
  *
  *    2b) Else deltaMinutes != 0, pick the second component, i.e. after the
  *    '/'.
  *
  * The above algorithm supports the following edge cases from the TZ
  * Database:
  *
  * A) Asia/Dushanbe in 1991 has a ZoneEra with a fixed hh:mm in the RULES
  * and a '/' in the FORMAT, the fixed hh:mm selects the DST abbreviation
  * in FORMAT. (This seems have been fixed in TZDB sometime before 2022g).
  *
  * B) Africa/Johannesburg 1942-1944 where the RULES which contains a
  * reference to named RULEs with DST transitions but there is no '/' or '%'
  * to distinguish between the 2.
  *
  * @param dest destination string buffer
  * @param destSize size of buffer
  * @param format encoded abbreviation, '%' is a character substitution
  * @param stdSeconds the offset seconds during standard time
  * @param dstSeconds the additional offset seconds for daylight saving time
  *    (0 for standard, != 0 for DST)
  * @param letterString the string corrresonding to the LETTER field in the
  * ZoneRule record. It is `nullptr` if ZoneEra.RULES is a '- or an 'hh:mm';
  * an empty string if the ZoneRule.LETTER was a '-'; or a pointer to a
  * non-empty string if ZoneRule.LETTER was a 'S', 'D', 'WAT' and so on. It
  * is possible for `letterString` to be the same buffer as the `dest`
  * string. Therefore we must copy the `letterString` before overwriting
  * `dest`.
  */
void createAbbreviation(
    char* dest,
    uint8_t destSize,
    const char* format,
    int32_t stdSeconds,
    int32_t dstSeconds,
    const char* letterString);

} // ace_time

#endif
