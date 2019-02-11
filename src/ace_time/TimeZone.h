#ifndef ACE_TIME_TIME_ZONE_H
#define ACE_TIME_TIME_ZONE_H

#include <stdint.h>
#include "UtcOffset.h"
#include "ZoneSpecifier.h"
#include "ManualZoneSpecifier.h"

class Print;

namespace ace_time {

/**
 * Class that describes a time zone. There are 2 types:
 *
 *    - kTypeManual represents an offset from UTC with a DST flag, both of
 *    which can be adjusted by the user. This type is mutable.
 *
 *    - kTypeAuto represents a time zone described by the TZ Database which
 *    contains rules about when the transition occurs from standard to DST
 *    modes. This type is immutable.
 *
 * The TimeZone class really really wants to be a reference type. In other
 * words, it would be far more convenient for the client code to create this on
 * the heap, and passed around using a pointer (or smart pointer) to the
 * ZonedDateTime class. This would allow new TimeZones to be created, while
 * older instances of ZonedDateTime would continued to hold on to the previous
 * versions of TimeZone.
 *
 * However, in a small memory embedded environment (like Arduino Nano or Micro
 * with only 2kB of RAM), I want to avoid any use of the heap (new operator or
 * malloc()) inside the AceTime library. So, I separated out the memory
 * intensive or mutable features of the TimeZone class into the separate
 * ZoneSpecifier class. The ZoneSpecifier object should be created once at
 * initialization time of the application (either statically allocated or
 * potentially on the heap early in the application start up).
 *
 * The TimeZone class then becomes a thin wrapper around a ZoneSpecifier object
 * (essentially acting like a smart pointer in some sense). It should be
 * treated as a value type and passed around by value or by const reference.
 *
 * An alternative implementation would make TimeZone a base class of an
 * inheritance hierarchy with 2 subclasses (ManualTimeZone and AutoTimeZone).
 * However this means that the TimeZone object can no longer be passed around
 * by value, and the ZonedDateTime is forced to hold on to the TimeZone object
 * using a pointer. It then becomes very difficult to change the offset and DST
 * fields of the ManualTimeZone. Using a single TimeZone class and implementing
 * it as a value type simplifies a lot of code.
 */
class TimeZone {
  public:
    static const uint8_t kTypeManual = ZoneSpecifier::kTypeManual;
    static const uint8_t kTypeAuto = ZoneSpecifier::kTypeAuto;

    /** Constructor. */
    explicit TimeZone(
        ZoneSpecifier* zoneSpecifier = &ManualZoneSpecifier::sUtcZoneSpecifier):
        mZoneSpecifier(zoneSpecifier) {}

    /** Return the ZoneSpecifier. */
    ZoneSpecifier* getZoneSpecifier() const { return mZoneSpecifier; }

    /** Return the type of TimeZone. */
    uint8_t getType() const {
      return mZoneSpecifier->getType();
    }

    /** Return the UTC offset at epochSeconds. */
    UtcOffset getUtcOffset(acetime_t epochSeconds) const {
      return mZoneSpecifier->getUtcOffset(epochSeconds);
    }

    /** Return the abbreviation at epochSeconds. */
    const char* getAbbrev(acetime_t epochSeconds) const {
      return mZoneSpecifier->getAbbrev(epochSeconds);
    }

    /** Print the human readable representation of the time zone. */
    void printTo(Print& printer) const;

  private:
    friend bool operator==(const TimeZone& a, const TimeZone& b);

    /** Length of UTC offset string (e.g. "-07:00", "+01:30"). */
    static const uint8_t kUtcOffsetStringLength = 6;

    /** Convert offsetString to offsetCode. */
    static void parseFromOffsetString(const char* offsetString,
        uint8_t* offsetCode);

    /** Instance of ZoneSpecifier. */
    ZoneSpecifier* mZoneSpecifier;
};

inline bool operator==(const TimeZone& a, const TimeZone& b) {
  if (a.mZoneSpecifier == b.mZoneSpecifier) return true;
  if (a.getType() != b.getType()) return false;
  return a.mZoneSpecifier->equals(*b.mZoneSpecifier);
}

inline bool operator!=(const TimeZone& a, const TimeZone& b) {
  return ! (a == b);
}

}

#endif
