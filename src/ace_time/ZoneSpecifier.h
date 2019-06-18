#ifndef ACE_TIME_ZONE_SPECIFIER_H
#define ACE_TIME_ZONE_SPECIFIER_H

#include "common/common.h"
#include "TimeOffset.h"

class Print;

namespace ace_time {

class LocalDateTime;

/**
 * Base interface for ZoneSpecifier classes. There were 2 options for
 * implmenting the differing ZoneSpecifiers:
 *
 * 1) Implement only a single getType() method to distinguish the different
 * runtime types of the object. Then use this type
 * information in the TimeZone class to downcast the ZoneSpecifier pointer to
 * the correct subclass, and call the correct methods.
 *
 * 2) Fully implement a polymorphic class hierarchy, lifting various common
 * methods (getUtcOffset(), getDeltaOffset(), getAbbrev()) into this interface
 * as virtual methods, then add a virtual equals() method to implement the
 * operator==().
 *
 * When I had only 2 ZoneSpecifier implementations (BasicZoneSpecifier and
 * ManualZoneSpecifier), Option 1 (using a single getType() and downcasting)
 * seemed to smaller program sizes, by 200-300 bytes. The problem with this
 * design is that the code for both subclasses would be compiled into the
 * program, even if the application used only one of the subclasses. When a 3rd
 * ZoneSpecifier subclass was added (ExtendedZoneSpecifier), the overhead
 * became untenable. So I switched to Option 2, using a fully polymorphic class
 * hierarchy, adding 3-4 virtual methods. When a program uses only a single
 * subclass, only that particular subclass is included into the program.
 * Unfortunately, this comes at the cost of forcing the program to use a
 * virtual dispatch for some of the often-used methods.
 */
class ZoneSpecifier {
  public:
    /** Indicate ManualZoneSpecifier. Must not be TimeZone::kTypeFixed. */
    static const uint8_t kTypeManual = 1;

    /** Indicate BasicZoneSpecifier. Must not be TimeZone::kTypeFixed. */
    static const uint8_t kTypeBasic = 2;

    /** Indicate ExtendedZoneSpecifier. Must not be TimeZone::kTypeFixed. */
    static const uint8_t kTypeExtended = 3;

    /** Return the kTypeXxx of the current instance. */
    uint8_t getType() const { return mType; }

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
     * Return the UTC offset matching the given the date/time components.
     * Returns TimeOffset::forError() if an error occurs, for example, if the
     * localDateTime is outside of the support date range of the underlying
     * ZoneInfo files.
     */
    virtual TimeOffset getUtcOffsetForDateTime(const LocalDateTime& ldt)
        const = 0;

    /** Print a human-readable identifier. */
    virtual void printTo(Print& printer) const = 0;

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
