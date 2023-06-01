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

class LocalDateTime;

/**
 * Result of a search for transition at a specific epochSeconds or a specific
 * LocalDateTime. More than one transition can match if the LocalDateTime occurs
 * during an overlap (e.g. during a "fall back" from DST to STD).
 */
class FindResult {
  public:
    static const uint8_t kTypeNotFound = 0;
    static const uint8_t kTypeExact = 1;
    static const uint8_t kTypeGap = 2;
    static const uint8_t kTypeOverlap = 3;

    /**
     * Result of the findByEpochSeconds() or findByLocalDateTime() search
     * methods. There are 2 slightly different cases:
     *
     * Case 1: findByLocalDateTime()
     *  * kTypeNotFound:
     *      * No matching Transition found.
     *  * kTypeExact:
     *      * A single Transition found.
     *  * kTypeGap:
     *      * LocalDateTime occurs in a gap.
     *      * LocalDateTime::fold=0 returns the earlier transition in
     *        reqStdOffsetSeconds and reqDstOffsetSeconds, and the later
     *        transition in stdOffsetSeconds and dstOffsetSeconds.
     *      * LocalDateTime::fold=1 returns the later transition in
     *        reqStdOffsetSeconds and reqDstOffsetSeconds, and the
     *        earlier transition in stdOffsetSeconds and dstOffsetSeconds.
     *  * kTypeOverlap:
     *      * LocalDateTime matches 2 Transitions.
     *      * LocalDateTime::fold=0 selects the earlier transition.
     *      * LocalDateTime::fold=1 selects the later transition.
     *
     * Case 2: findByEpochSeconds()
     *  * kTypeNotFound:
     *      * If no matching Transition found.
     *  * kTypeExact:
     *      * Only a single Transition found.
     *  * kTypeGap:
     *      * Cannot occur.
     *  * kTypeOverlap:
     *      * A single Transition found, but the epochSeconds occurs during an
     *        overlap where two local times can occur.
     *      * The `fold` parameter contains 0 or 1 to indicate the earlier or
     *        later resulting OffsetDateTime.
     */
    uint8_t type = kTypeNotFound;

    /**
     * For findByLocalDateTime(), when type==kTypeOverlap, this is a copy of the
     * requested LocalDateTime::fold parameter. For all other resulting types,
     * including kTypeGap, this will be set to 0.
     *
     * For findByEpochSeconds(), when type==kTypeOverlap, this defines whether
     * the corresponding LocalDateTime occurs the first time (0) or the second
     * time (1). For all other resulting type, this will be set to 0.
     */
    uint8_t fold = 0;

    /** STD offset of the resulting OffsetDateTime. */
    int32_t stdOffsetSeconds = 0;

    /** DST offset of the resulting OffsetDateTime. */
    int32_t dstOffsetSeconds = 0;

    /**
     * STD offset of the Transition which matched the epochSeconds requested by
     * findByEpochSeconds(), or the LocalDateTime requested by
     * findByLocalDateTime().
     *
     * This may be different than the stdOffsetSeconds when
     * findByLocalDateTime() returns kTypeGap. For all other resulting types
     * from findByEpochSeconds(), and for all resulting types from
     * findByLocalDateTime(), the reqStdOffsetSeconds will be the same as
     * stdOffsetSeconds.
     */
    int32_t reqStdOffsetSeconds = 0;

    /**
     * DST offset of the Transition which matched the epochSeconds requested by
     * findByEpochSeconds(), or the LocalDateTime requested by
     * findByLocalDateTime().
     *
     * This may be different than the dstOffsetSeconds when
     * findByLocalDateTime() returns kTypeGap. For all other resulting types
     * from findByEpochSeconds(), and for all resulting types from
     * findByLocalDateTime(), the reqStdOffsetSeconds will be the same as
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
 * methods (e.g. findByLocalDateTime(), findByEpochSeconds()) into this
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

    /** Return the search results at given LocalDateTime. */
    virtual FindResult findByLocalDateTime(
        const LocalDateTime& ldt) const = 0;

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
     * to LocalDate::kInvalidYear to indicate invalid cache.
     */
    mutable int16_t mYear = LocalDate::kInvalidYear;

    /**
     * Epoch year that was used to calculate the transitions in the current
     * cache. Set to LocalDate::kInvalidYear to indicate invalid cache.
     */
    mutable int16_t mEpochYear = LocalDate::kInvalidYear;
};

inline bool operator==(const ZoneProcessor& a, const ZoneProcessor& b) {
  if (a.mType != b.mType) return false;
  return a.equals(b);
}

inline bool operator!=(const ZoneProcessor& a, const ZoneProcessor& b) {
  return ! (a == b);
}

namespace internal {

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

} // internal
} // ace_time

#endif
