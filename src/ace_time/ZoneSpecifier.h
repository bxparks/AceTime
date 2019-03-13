#ifndef ACE_TIME_ZONE_SPECIFIER_H
#define ACE_TIME_ZONE_SPECIFIER_H

#include "common/common.h"
#include "UtcOffset.h"

class Print;

namespace ace_time {

/**
 * Base interface for ZoneSpecifier classes. This contains just the getType()
 * discriminator distinguish between the 2 implementation classes
 * (ManualZoneSpecifier and AutoZoneSpecifier). The TimeZone class will use the
 * discriminator to determine which of the 2 subclasses to use at runtime.
 *
 * An alternative design was to lift various common methods (getUtcOffset(),
 * getDeltaOffset(), getAbbrev()) into this interface as virtual methods, then
 * add a virtual equals() method to implement the operator==(). I thought that
 * if the application used only AutoZoneSpecifier or ManualZoneSpecifier (but
 * not both), the compiler might be able to reduce the program code size by
 * removing the code for the unused class. However, in reality, this
 * alternative design caused the program size to *increase* by 200-300 bytes.
 * I'm not exactly sure why but that was the reality.
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

    /** Return the UTC offset at epochSeconds. */
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
