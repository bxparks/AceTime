#ifndef ACE_TIME_TIME_ZONE_H
#define ACE_TIME_TIME_ZONE_H

#include <stdint.h>
#include "UtcOffset.h"
#include "ZoneSpec.h"
#include "AutoZoneSpec.h"
#include "ManualZoneSpec.h"

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
 * ZoneSpec class. The ZoneSpec object should be created once at initialization
 * time of the application (either statically allocated or potentially on the
 * heap early in the application start up).
 *
 * The TimeZone class then becomes a thin wrapper around a ZoneSpec object
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
    static const uint8_t kTypeManual = ZoneSpec::kTypeManual;
    static const uint8_t kTypeAuto = ZoneSpec::kTypeAuto;

    /** Constructor. */
    explicit TimeZone(ZoneSpec* zoneSpec = &ManualZoneSpec::sUtcZoneSpec):
        mZoneSpec(zoneSpec) {}

    /** Return the type of TimeZone. */
    uint8_t getType() const {
      return mZoneSpec->getType();
    }

    /** Return the effective zone offset. */
    UtcOffset getUtcOffset(acetime_t epochSeconds) const {
      if (getType() == kTypeAuto) {
        return ((AutoZoneSpec*)mZoneSpec)->getUtcOffset(epochSeconds);
      } else {
        return ((ManualZoneSpec*)mZoneSpec)->getUtcOffset();
      }
    }

    /** Return true if the time zone observes DST at epochSeconds. */
    // TODO: Replace this with getDeltaOffset().
    bool getDst(acetime_t epochSeconds) const {
      UtcOffset offset = (getType() == kTypeAuto)
        ? ((AutoZoneSpec*)mZoneSpec)->getDeltaOffset(epochSeconds)
        : ((ManualZoneSpec*)mZoneSpec)->getDeltaOffset();
      return offset.isDst();
    }

    /** Return the abbreviation of the time zone. */
    const char* getAbbrev(acetime_t epochSeconds) const {
      if (getType() == kTypeAuto) {
        return ((AutoZoneSpec*)mZoneSpec)->getAbbrev(epochSeconds);
      } else {
        return ((ManualZoneSpec*)mZoneSpec)->getAbbrev();
      }
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

    /** Instance of ZoneSpec. */
    ZoneSpec* mZoneSpec;
};

inline bool operator==(const TimeZone& a, const TimeZone& b) {
  if (a.getType() != b.getType()) return false;
  if (a.mZoneSpec == b.mZoneSpec) return true;

  if (a.getType() == TimeZone::kTypeAuto) {
    AutoZoneSpec* aa = static_cast<AutoZoneSpec*>(a.mZoneSpec);
    AutoZoneSpec* bb = static_cast<AutoZoneSpec*>(b.mZoneSpec);
    return *aa == *bb;
  } else {
    ManualZoneSpec* aa = static_cast<ManualZoneSpec*>(a.mZoneSpec);
    ManualZoneSpec* bb = static_cast<ManualZoneSpec*>(b.mZoneSpec);
    return *aa == *bb;
  }
}

inline bool operator!=(const TimeZone& a, const TimeZone& b) {
  return ! (a == b);
}

}

#endif
