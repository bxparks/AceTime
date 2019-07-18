/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#ifndef ACE_TIME_ZONE_SPECIFIER_H
#define ACE_TIME_ZONE_SPECIFIER_H

#include "common/common.h"
#include "TimeOffset.h"
#include "OffsetDateTime.h"

class Print;

namespace ace_time {

class LocalDateTime;

/**
 * Base interface for ZoneSpecifier classes. There were 2 options for
 * implmenting the various concrete implementations of ZoneSpecifiers:
 *
 * 1) Implement only a single getType() method to distinguish the different
 * runtime types of the object. Then use this type information in the TimeZone
 * class to downcast the ZoneSpecifier pointer to the correct subclass, and
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
class ZoneSpecifier {
  public:
    /**
     * Indicate BasicZoneSpecifier. Must not be TimeZone::kTypeError (0) or
     * TimeZone::kTypeManual (1).
     */
    static const uint8_t kTypeBasic = 2;

    /**
     * Indicate ExtendedZoneSpecifier. Must not be TimeZone::kTypeError (0) or
     * TimeZone::kTypeManual (1).
     */
    static const uint8_t kTypeExtended = 3;

    /** Return the kTypeXxx of the current instance. */
    uint8_t getType() const { return mType; }

    /** Return the unique stable zoneId. */
    virtual uint32_t getZoneId() const = 0;

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
     * Return the time zone abbreviation at epochSeconds. This is an
     * experimental method that has not been tested thoroughly. Use with
     * caution. Returns an empty string ("") if an error occurs.
     */
    virtual const char* getAbbrev(acetime_t epochSeconds) const = 0;

    /**
     * Return the best estimate of the OffsetDateTime at the given
     * LocalDateTime for the timezone of the current ZoneSpecifier.
     * Returns OffsetDateTime::forError() if an error occurs, for example, if
     * the LocalDateTime is outside of the support date range of the underlying
     * ZoneInfo files.
     */
    virtual OffsetDateTime getOffsetDateTime(const LocalDateTime& ldt)
        const = 0;

    /** Print a human-readable identifier (e.g. "America/Los_Angeles"). */
    virtual void printTo(Print& printer) const = 0;

    /** Print a short human-readable identifier (e.g. "Los_Angeles") */
    virtual void printShortTo(Print& printer) const = 0;

  protected:
    friend bool operator==(const ZoneSpecifier& a, const ZoneSpecifier& b);

    // Use default copy constructor and assignment operator.
    ZoneSpecifier(const ZoneSpecifier&) = default;
    ZoneSpecifier& operator=(const ZoneSpecifier&) = default;

    /** Constructor. */
    ZoneSpecifier(uint8_t type):
      mType(type) {}

    /** Return true if equal. */
    virtual bool equals(const ZoneSpecifier& other) const = 0;

    uint8_t mType;
};

inline bool operator==(const ZoneSpecifier& a, const ZoneSpecifier& b) {
  if (a.getType() != b.getType()) return false;
  return a.equals(b);
}

inline bool operator!=(const ZoneSpecifier& a, const ZoneSpecifier& b) {
  return ! (a == b);
}

}

#endif
