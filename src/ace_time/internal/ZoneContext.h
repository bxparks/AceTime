/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#ifndef ACE_TIME_COMMON_ZONE_CONTEXT_H
#define ACE_TIME_COMMON_ZONE_CONTEXT_H

namespace ace_time {

// Data structure for BasicZoneProcessor
namespace basic {
#include "ZoneContext.inc"
}

// Data structure for ExtendedZoneProcessor
namespace extended {
#include "ZoneContext.inc"
}

}

#endif
