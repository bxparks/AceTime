/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#ifndef ACE_TIME_ZONE_PROCESSOR_H
#define ACE_TIME_ZONE_PROCESSOR_H

#include "common/common.h"
#include "TimeOffset.h"
#include "OffsetDateTime.h"

class Print;

namespace ace_time {

class LocalDateTime;

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
 * methods (getUtcOffset(), getDeltaOffset(), getAbbrev()) into this interface
 * as virtual methods, then add a virtual equals() method to implement the
 * operator==().
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

    /**
     * Return the unique stable zoneId.
     *
     * @param followLink if true and the time zone is a Link, follow the link
     * and return the id of the target Zone instead. If the zone is not a link,
     * this flag is ignored.
     */
    virtual uint32_t getZoneId(bool followLink = false) const = 0;

    /**
     * Return the total UTC offset at epochSeconds, including DST offset.
     * Returns TimeOffset::forError() if an error occurs.
     */
    virtual TimeOffset getUtcOffset(acetime_t epochSeconds) const = 0;

    /**
     * Return the DST delta offset at epochSeconds. This is an experimental
     * method that has not been tested thoroughly. Use with caution.
     * Returns TimeOffset::forError() if an error occurs.
     */
    virtual TimeOffset getDeltaOffset(acetime_t epochSeconds) const = 0;

    /**
     * Return the time zone abbreviation at epochSeconds. Returns an empty
     * string ("") if an error occurs. The returned pointer points to a char
     * buffer that could get overriden by subsequent method calls to this
     * object. The pointer must not be stored permanently, it should be used as
     * soon as possible (e.g. printed out).
     *
     * This is an experimental method that has not been tested thoroughly. Use
     * with caution.
     */
    virtual const char* getAbbrev(acetime_t epochSeconds) const = 0;

    /**
     * Return the best estimate of the OffsetDateTime at the given
     * LocalDateTime for the timezone of the current ZoneProcessor.
     * Returns OffsetDateTime::forError() if an error occurs, for example, if
     * the LocalDateTime is outside of the support date range of the underlying
     * ZoneInfo files.
     */
    virtual OffsetDateTime getOffsetDateTime(const LocalDateTime& ldt)
        const = 0;

    /**
     * Return the best estimate of the OffsetDateTime at the given
     * epochSeconds for the timezone of the current ZoneProcessor, including
     * the fold parameter.
     *
     * Returns OffsetDateTime::forError() if an error occurs, for example, if
     * the epochSeconds is outside of the support date range of the underlying
     * ZoneInfo files.
     */
    virtual OffsetDateTime getOffsetDateTime(acetime_t epochSeconds) const = 0;

    /**
     * Print a human-readable identifier (e.g. "America/Los_Angeles").
     *
     * @param printer an instance of the Print class, usually Serial
     * @param followLink if true and the zone is actually a link, follow the
     * link and return the name of the target Zone instead. If the zone is not a
     * link, this flag is ignored.
     */
    virtual void printNameTo(Print& printer, bool followLink = false) const = 0;

    /**
     * Print a short human-readable identifier (e.g. "Los Angeles").
     * Any underscore in the short name is replaced with a space.
     *
     * @param printer an instance of the Print class, usually Serial
     * @param followLink if true and the zone is actually a link, follow the
     * link and return the short name of the target Zone instead. If the zone is
     * not a link, this flag is ignored.
     */
    virtual void printShortNameTo(Print& printer, bool followLink = false)
        const = 0;

    /**
     * Set the opaque zoneKey of this object to a new value, reseting any
     * internally cached information.
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
     */
    virtual void setZoneKey(uintptr_t zoneKey) = 0;

    /**
     * Return true if ZoneProcessor is associated with the given opaque
     * zoneKey. This method should be considered to be private.
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

    /** Return true if equal. */
    virtual bool equals(const ZoneProcessor& other) const = 0;

    uint8_t const mType;
};

inline bool operator==(const ZoneProcessor& a, const ZoneProcessor& b) {
  if (a.mType != b.mType) return false;
  return a.equals(b);
}

inline bool operator!=(const ZoneProcessor& a, const ZoneProcessor& b) {
  return ! (a == b);
}

namespace internal {

/**
  * Longest abbreviation currently seems to be 5 characters
  * (https://www.timeanddate.com/time/zones/) but the TZ database spec says
  * that abbreviations are 3 to 6 characters
  * (https://data.iana.org/time-zones/theory.html#abbreviations), so use 6 as
  * the maximum.
  */
static const uint8_t kAbbrevSize = 6 + 1;

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
