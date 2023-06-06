/*
 * MIT License
 * Copyright (c) 2023 Brian T. Park
 */

#ifndef ACE_TIME_INFOS_H
#define ACE_TIME_INFOS_H

#include "ZoneInfoMid.h"
#include "ZoneInfoHigh.h"

namespace ace_time {

// Data structures for BasicZoneProcessor
namespace basic {

class Basic {};
using ZoneContext = zoneinfomid::ZoneContext<Basic>;
using ZoneRule = zoneinfomid::ZoneRule<Basic>;
using ZonePolicy = zoneinfomid::ZonePolicy<Basic>;
using ZoneEra = zoneinfomid::ZoneEra<Basic>;
using ZoneInfo = zoneinfomid::ZoneInfo<Basic, ZoneContext>;

}

// Data structures for ExtendedZoneProcessor
namespace extended {

class Extended {};
using ZoneContext = zoneinfomid::ZoneContext<Extended>;
using ZoneRule = zoneinfomid::ZoneRule<Extended>;
using ZonePolicy = zoneinfomid::ZonePolicy<Extended>;
using ZoneEra = zoneinfomid::ZoneEra<Extended>;
using ZoneInfo = zoneinfomid::ZoneInfo<Extended, ZoneContext>;

}

// Data structures for CompleteZoneProcessor
namespace complete {

class Complete {};
using ZoneContext = zoneinfohigh::ZoneContext<Complete>;
using ZoneRule = zoneinfohigh::ZoneRule<Complete>;
using ZonePolicy = zoneinfohigh::ZonePolicy<Complete>;
using ZoneEra = zoneinfohigh::ZoneEra<Complete>;
using ZoneInfo = zoneinfohigh::ZoneInfo<Complete, ZoneContext>;

}

}

#endif
