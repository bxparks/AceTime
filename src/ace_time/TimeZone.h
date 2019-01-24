#ifndef ACE_TIME_TIME_ZONE_H
#define ACE_TIME_TIME_ZONE_H

#include <stdint.h>
#include "UtcOffset.h"
#include "ZoneAgent.h"
#include "AutoZoneAgent.h"
#include "ManualZoneAgent.h"

class Print;

namespace ace_time {

/**
 * Class that describes a time zone. There are 2 types:
 *
 *    - kTypeManual represents an offset from UTC with a
 *    DST flag, both of which can be adjusted by the user. This type is
 *    mutable.
 *
 *    - kTypeAuto represents a time zone described by the
 *    TZ Database which contains rules about when the transition occurs from
 *    standard to DST modes. This type is immutable.
 *
 * This class should be treated as a value type and passed around by value or
 * by const reference. If the user wants to change the offset and DST of a
 * Manual TimeZone, copy the TimeZone by value and call one of the mutators
 * which are valid for kTypeManual.
 *
 * An alternative implementation would make TimeZone a base class of an
 * inheritance hierarchy with 2 subclasses (ManualTimeZone and AutoTimeZone).
 * However this means that the TimeZone object can no longer be passed around
 * by value, and the ZonedDateTime is forced to hold on to the TimeZone object
 * using a pointer. It then becomes very difficult to change the offset and DST
 * fields of the ManualTimeZone. Using a single TimeZone class and implementing
 * it as a value type simplifies a lot of code.
 *
 * The disadvantage of merging the Manual and Auto types is that for the Auto
 * type, the ZoneAgent needed to be split off as an external dependency to the
 * TimeZone.
 */
class TimeZone {
  public:
    static const uint8_t kTypeDefault = ZoneAgent::kTypeDefault;
    static const uint8_t kTypeManual = ZoneAgent::kTypeManual;
    static const uint8_t kTypeAuto = ZoneAgent::kTypeAuto;

    /** Constructor. */
    explicit TimeZone(ZoneAgent* zoneAgent = &ZoneAgent::sDefaultZoneAgent):
        mZoneAgent(zoneAgent) {}

    /** Return the type of TimeZone. */
    uint8_t getType() const {
      return mZoneAgent->getType();
    }

    /** Return the effective zone offset. */
    UtcOffset getUtcOffset(acetime_t epochSeconds) const {
      if (getType() == kTypeAuto) {
        return mZoneAgent->getUtcOffset(epochSeconds);
      } else {
        return mZoneAgent->getUtcOffset(mIsDst);
      }
    }

    /** Return true if the time zone observes DST at epochSeconds. */
    bool getDst(acetime_t epochSeconds) const {
      UtcOffset offset = (getType() == kTypeAuto)
        ? mZoneAgent->getDeltaOffset(epochSeconds)
        : mZoneAgent->getDeltaOffset(mIsDst);
      return offset.isDst();
    }

    /** Return the abbreviation of the time zone. */
    const char* getAbbrev(acetime_t epochSeconds) const {
      if (getType() == kTypeAuto) {
        return mZoneAgent->getAbbrev(epochSeconds);
      } else {
        return mZoneAgent->getAbbrev(mIsDst);
      }
    }

    /** Return the base isDst flag. Valid only for AutoZoneAgent. */
    bool isDst() const { return mIsDst; }

    /** Set the base isDst flag. Valid only for ManualZoneAgent. */
    void isDst(bool isDst) { mIsDst = isDst; }

    /** Print the human readable representation of the time zone. */
    void printTo(Print& printer) const;

  private:
    friend bool operator==(const TimeZone& a, const TimeZone& b);

    /** Length of UTC offset string (e.g. "-07:00", "+01:30"). */
    static const uint8_t kUtcOffsetStringLength = 6;

    /** Convert offsetString to offsetCode. */
    static void parseFromOffsetString(const char* offsetString,
        uint8_t* offsetCode);

    /** Instance of ZoneAgent. */
    ZoneAgent* mZoneAgent;

    /** Set to true if DST is enabled, when using ManualZoneAgent. */
    bool mIsDst = false;
};

inline bool operator==(const TimeZone& a, const TimeZone& b) {
  if (a.getType() != b.getType()) return false;

  if (a.getType() == TimeZone::kTypeDefault) {
    return true;
  } else if (a.getType() == TimeZone::kTypeAuto) {
    AutoZoneAgent* aAgent = static_cast<AutoZoneAgent*>(a.mZoneAgent);
    AutoZoneAgent* bAgent = static_cast<AutoZoneAgent*>(b.mZoneAgent);
    return aAgent->getZoneInfo() == bAgent->getZoneInfo();
  } else {
    ManualZoneAgent* aAgent = static_cast<ManualZoneAgent*>(a.mZoneAgent);
    ManualZoneAgent* bAgent = static_cast<ManualZoneAgent*>(b.mZoneAgent);
    return a.mIsDst == b.mIsDst
        && aAgent->stdOffset() == bAgent->stdOffset()
        && aAgent->deltaOffset() == bAgent->deltaOffset()
        && aAgent->stdAbbrev() == bAgent->stdAbbrev()
        && aAgent->dstAbbrev() == bAgent->dstAbbrev();
  }
}

inline bool operator!=(const TimeZone& a, const TimeZone& b) {
  return ! (a == b);
}

}

#endif
