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

class Basic {}; // dummy sentinel class
using ZoneContext = zoneinfolow::ZoneContext<Basic>;
using ZoneRule = zoneinfolow::ZoneRule<Basic>;
using ZonePolicy = zoneinfolow::ZonePolicy<Basic>;
using ZoneEra = zoneinfolow::ZoneEra<Basic>;
using ZoneInfo = zoneinfolow::ZoneInfo<Basic, ZoneContext>;

}

// Data structures for ExtendedZoneProcessor
namespace extended {

class Extended {}; // dummy sentinel class
using ZoneContext = zoneinfolow::ZoneContext<Extended>;
using ZoneRule = zoneinfolow::ZoneRule<Extended>;
using ZonePolicy = zoneinfolow::ZonePolicy<Extended>;
using ZoneEra = zoneinfolow::ZoneEra<Extended>;
using ZoneInfo = zoneinfolow::ZoneInfo<Extended, ZoneContext>;

}

// Data structures for CompleteZoneProcessor
namespace complete {

class Complete {}; // dummy sentinel class
using ZoneContext = zoneinfohigh::ZoneContext<Complete>;
using ZoneRule = zoneinfohigh::ZoneRule<Complete>;
using ZonePolicy = zoneinfohigh::ZonePolicy<Complete>;
using ZoneEra = zoneinfohigh::ZoneEra<Complete>;
using ZoneInfo = zoneinfohigh::ZoneInfo<Complete, ZoneContext>;

}

}

#endif
