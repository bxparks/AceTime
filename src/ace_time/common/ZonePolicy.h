#ifndef ACE_TIME_ZONE_POLICY_H
#define ACE_TIME_ZONE_POLICY_H

#include <stdint.h>

namespace ace_time {

// Data structures for BasicZoneSpecifier
namespace basic {
#include "ZonePolicy.inc"
}

// Data structures for ExtendedZoneSpecifier
namespace extended {
#include "ZonePolicy.inc"
}

}

#endif
