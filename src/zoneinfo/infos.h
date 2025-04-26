/*
 * MIT License
 * Copyright (c) 2023 Brian T. Park
 */

#ifndef ACE_TIME_INFOS_H
#define ACE_TIME_INFOS_H

#include "ZoneInfoLow.h"
#include "ZoneInfoMid.h"
#include "ZoneInfoHigh.h"

namespace ace_time {

// Data structures for BasicZoneProcessor
namespace basic {
using Info = ZoneInfoLow;
}

// Data structures for ExtendedZoneProcessor. The ExtendedZoneProcessor
// uses the *same* storage format as BasicZoneProcessor (i.e. ZoneInfoLow)
// to save flash memory. It turns out that all timezones after the year 2000
// have parameters which can be accurately captured using the low-resolution
// ZoneInfoLow data types instead of the ZoneInfoMid data types.
namespace extended {
using Info = ZoneInfoLow;
}

// Data structures for CompleteZoneProcessor
namespace complete {
using Info = ZoneInfoHigh;
}

}

#endif
