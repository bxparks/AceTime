/*
 * MIT License
 * Copyright (c) 2023 Brian T. Park
 */

#ifndef ACE_TIME_INFOS_H
#define ACE_TIME_INFOS_H

/**
 * @file infos.h
 *
 * The low-level storage/broker formats are independent of the concrete
 * ZoneProcessor classes (implementing a specific algorithm for determining DST
 * transitions). There are 3 ZoneProcessors: BasicZoneProcessor,
 * ExtendedZoneProcessor, and CompleteZoneProcessor.
 *
 * This file provides a mapping between those two layers:
 *
 * - BasicZoneProcessor -> ZoneInfoLow
 * - ExtendedZoneProcessor -> ZoneInfoLow
 * - CompleteZoneProcessor -> ZoneInfoHigh
 */

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
