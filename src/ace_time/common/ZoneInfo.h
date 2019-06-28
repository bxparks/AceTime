/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#ifndef ACE_TIME_ZONE_INFO_H
#define ACE_TIME_ZONE_INFO_H

#include <stdint.h>
#include "ZoneContext.h"
#include "ZonePolicy.h"

namespace ace_time {

// Data structures for BasicZoneSpecifier
namespace basic {
#include "ZoneInfo.inc"
}

// Data structures for ExtendedZoneSpecifier
namespace extended {
#include "ZoneInfo.inc"
}

}

#endif
