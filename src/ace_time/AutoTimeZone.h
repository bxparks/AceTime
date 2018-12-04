#ifndef ACE_TIME_AUTO_TIME_ZONE_H
#define ACE_TIME_AUTO_TIME_ZONE_H

#include <stdint.h>
#include "common/ZoneInfo.h"
#include "UtcOffset.h"
#include "ZoneAgent.h"
#include "TimeZone.h"

class Print;

namespace ace_time {

/**
 * Class that represents a time zone described by the TZ Database which
 * contains rules about when the transition occurs from standard to DST modes.
 * The calling code should create one more more instances of this during the
 * setup of the application and share it among as many DateTime objects as
 * needed.
 *
 * This class is not mutable except through the Copy Constructor and the
 * Assignmen Operator.
 */
class AutoTimeZone: public TimeZone {
  public:
    /** Factory method. Create from ZoneInfo. */
    static AutoTimeZone forZone(const common::ZoneInfo* zoneInfo) {
      return AutoTimeZone(zoneInfo);
    }

    /** Constructor. */
    explicit AutoTimeZone(const common::ZoneInfo* zoneInfo = nullptr):
        TimeZone(kTypeAuto),
        mZoneAgent(zoneInfo) {}

    /** Copy constructor. */
    AutoTimeZone(const AutoTimeZone& other):
      TimeZone(other.mType),
      mZoneAgent(other.mZoneAgent) {}

    /** Assignment operator. */
    AutoTimeZone& operator=(const AutoTimeZone& other) {
      mType = other.mType;
      mZoneAgent = other.mZoneAgent;
      return *this;
    }

    UtcOffset getUtcOffset(uint32_t epochSeconds) const override {
      return mZoneAgent.getUtcOffset(epochSeconds);
    }

    const char* getAbbrev(uint32_t epochSeconds) const override {
      return mZoneAgent.getAbbrev(epochSeconds);
    }

    bool getDst(uint32_t epochSeconds) const override {
      return mZoneAgent.isDst(epochSeconds);
    }

    void printTo(Print& printer) const override;

  private:
    bool equals(const TimeZone& that) const override {
      const AutoTimeZone& other = static_cast<const AutoTimeZone&>(that);
      return mZoneAgent.getZoneInfo() == other.mZoneAgent.getZoneInfo();
    }

    /** Manager of the time zone rules for the given ZoneInfo. */
    mutable ZoneAgent mZoneAgent;
};

}

#endif
