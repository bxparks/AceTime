#ifndef ACE_TIME_ZONE_SPEC_H
#define ACE_TIME_ZONE_SPEC_H

#include "common/common.h"
#include "UtcOffset.h"

namespace ace_time {

class ZoneSpec {
  public:
    static const uint8_t kTypeManual = 0;
    static const uint8_t kTypeAuto = 1;

    /** Return the type of the zone spec. */
    virtual uint8_t getType() const = 0;
};

}

#endif
