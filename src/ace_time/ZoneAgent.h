#ifndef ACE_TIME_ZONE_AGENT_H
#define ACE_TIME_ZONE_AGENT_H

#include "common/common.h"
#include "UtcOffset.h"

namespace ace_time {

class ZoneAgent {
  public:
    static const uint8_t kTypeDefault = 0;
    static const uint8_t kTypeManual = 1;
    static const uint8_t kTypeAuto = 2;

    static ZoneAgent sDefaultZoneAgent;

    /** Return the type of the zone agent. */
    virtual uint8_t getType() const { return kTypeDefault; }

    /** Return the UTC offset at epochSeconds. */
    virtual UtcOffset getUtcOffset(acetime_t /*epochSeconds*/) {
      return UtcOffset();
    }

    /** Return the DST delta offset at epochSeconds. */
    virtual UtcOffset getDeltaOffset(acetime_t /*epochSeconds*/) {
      return UtcOffset();
    }

    /** Return the time zone abbreviation. */
    virtual const char* getAbbrev(acetime_t /*epochSeconds*/) { return "UTC"; }

    /** Return the UTC offset at epochSeconds. */
    virtual UtcOffset getUtcOffset(bool /*isDst*/) { return UtcOffset(); }

    /** Return the DST delta offset at epochSeconds. */
    virtual UtcOffset getDeltaOffset(bool /*isDst*/) { return UtcOffset(); }

    /** Return the time zone abbreviation. */
    virtual const char* getAbbrev(bool /*isDst*/) { return "UTC"; }
};

}

#endif
