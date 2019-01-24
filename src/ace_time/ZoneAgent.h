#ifndef ACE_TIME_ZONE_AGENT_H
#define ACE_TIME_ZONE_AGENT_H

#include "common/common.h"
#include "UtcOffset.h"

namespace ace_time {

class ZoneAgent {
  public:
    static const uint8_t kTypeManual = 0;
    static const uint8_t kTypeAuto = 1;

    /** Return the type of the zone agent. */
    virtual uint8_t getType() const = 0;
};

}

#endif
