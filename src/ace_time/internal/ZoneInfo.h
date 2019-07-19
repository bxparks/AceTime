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

// The data structures in ZoneInfo.inc are #included into the basic and
// extended namespaces, instead of subclassing them into the namespaces,
// because C++11 does not allow subclassed structs to be initialized using the
// curly-brace initializers. I believe C++14 removes this restriction but
// Arduino is currently limited to C++11.

// Data structures for BasicZoneProcessor
namespace basic {
#include "ZoneInfo.inc"
}

// Data structures for ExtendedZoneProcessor
namespace extended {
#include "ZoneInfo.inc"
}

}

#endif
