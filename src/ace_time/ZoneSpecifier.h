#ifndef ACE_TIME_ZONE_SPECIFIER_H
#define ACE_TIME_ZONE_SPECIFIER_H

#include "common/common.h"
#include "UtcOffset.h"

class Print;

namespace ace_time {

/**
 * Base interface for ZoneSpecifier classes. There were 2 options for
 * implmenting the differing ZoneSpecifiers:
 *
 * 1) Implement only a single getType() virtual method. Then use this type
 * information in the TimeZone class to downcast the ZoneSpecifier pointer to
 * the correct subclass, and call the correct methods.
 * 2) Fully implement a polymorphic class hierarchy, lifting various common
 * methods (getUtcOffset(), getDeltaOffset(), getAbbrev()) into this interface
 * as virtual methods, then add a virtual equals() method to implement the
 * operator==().
 *
 * When I had only 2 ZoneSpecifier implementations (BasicZoneSpecifier and
 * ManualZoneSpecifier), Option 1 (using a single virtual getType() and
 * downcasting) seemed to smaller program sizes, by 200-300 bytes. The problem
 * with this design is that the code for both subclasses would be compiled into
 * the program, even if the runtime used only one of the subclasses. When a 3rd
 * ZoneSpecifier subclass was added (ExtendedZoneSpecifier), the overhead
 * became untenable. So I switched to Option 2, using a fully polymorphic class
 * hierarchy, adding 3-4 virtual methods. When a program uses only a single
 * subclass, we pay for the code of only that subclass, at the cost of a
 * virtual dispatch for some of the often-used methods.
 */
class ZoneSpecifier {
  public:
    static const uint8_t kTypeManual = 0;
    static const uint8_t kTypeAuto = 1;
    static const uint8_t kTypeExtended = 2;

    // TODO: Consider moving this into ZoneSpecifier::mType member variable
    // to eliminate this virtual method.
    /** Return the type of the zone spec. */
    virtual uint8_t getType() const = 0;

    /** Return the total effective UTC offset at epochSeconds, including DST. */
    virtual UtcOffset getUtcOffset(acetime_t epochSeconds) = 0;

    /** Return the time zone abbreviation at epochSeconds. */
    virtual const char* getAbbrev(acetime_t epochSeconds) = 0;

    /** Print a human-readable identifier. */
    virtual void printTo(Print& printer) const = 0;

  protected:
    friend bool operator==(const ZoneSpecifier& a, const ZoneSpecifier& b);

    /** Return true if equal. */
    virtual bool equals(const ZoneSpecifier& other) const = 0;
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
