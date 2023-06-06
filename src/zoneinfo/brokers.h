/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#ifndef ACE_TIME_BROKERS_H
#define ACE_TIME_BROKERS_H

#include "infos.h"
#include "BrokersMid.h"
#include "BrokersHigh.h"

/**
 * @file brokers.h
 *
 * The brokers in the basic:: and extended:: namespaces are identical in code.
 * The purpose for having separate class hierarchies is to provide compile-time
 * assurance that the BasicZoneProcessor and ExtendedZoneProcessor are given the
 * correct zoneinfo files from the appropriate zonedb database.
 */

namespace ace_time {

namespace basic {

/** Data broker for accessing ZoneContext. */
using ZoneContextBroker = zoneinfomid::ZoneContextBroker<ZoneContext>;

/** Data broker for accessing ZoneRule. */
using ZoneRuleBroker = zoneinfomid::ZoneRuleBroker<ZoneContext, ZoneRule>;

/** Data broker for accessing ZonePolicy. */
using ZonePolicyBroker = zoneinfomid::ZonePolicyBroker<
    ZoneContext, ZonePolicy, ZoneRule>;

/** Data broker for accessing ZoneEra. */
using ZoneEraBroker = zoneinfomid::ZoneEraBroker<
    ZoneContext, ZoneEra, ZonePolicy, ZoneRule>;

/** Data broker for accessing ZoneInfo. */
using ZoneInfoBroker = zoneinfomid::ZoneInfoBroker<
    ZoneContext, ZoneInfo, ZoneEra, ZonePolicy, ZoneRule>;

/**
 * Data broker for accessing the ZoneRegistry. The ZoneRegistry is an
 * array of (const ZoneInfo*) in the zone_registry.cpp file.
 */
using ZoneRegistryBroker = zoneinfomid::ZoneRegistryBroker<ZoneInfo>;

/** Storage object that returns a ZoneInfoBroker from a ZoneInfo pointer. */
using ZoneInfoStore = zoneinfomid::ZoneInfoStore<
    ZoneContext, ZoneInfo, ZoneEra, ZonePolicy, ZoneRule>;

} // basic

namespace extended {

/** Data broker for accessing ZoneContext. */
using ZoneContextBroker = zoneinfomid::ZoneContextBroker<ZoneContext>;

/** Data broker for accessing ZoneRule. */
using ZoneRuleBroker = zoneinfomid::ZoneRuleBroker<ZoneContext, ZoneRule>;

/** Data broker for accessing ZonePolicy. */
using ZonePolicyBroker = zoneinfomid::ZonePolicyBroker<
    ZoneContext, ZonePolicy, ZoneRule>;

/** Data broker for accessing ZoneEra. */
using ZoneEraBroker = zoneinfomid::ZoneEraBroker<
    ZoneContext, ZoneEra, ZonePolicy, ZoneRule>;

/** Data broker for accessing ZoneInfo. */
using ZoneInfoBroker = zoneinfomid::ZoneInfoBroker<
    ZoneContext, ZoneInfo, ZoneEra, ZonePolicy, ZoneRule>;

/**
 * Data broker for accessing the ZoneRegistry. The ZoneRegistry is an
 * array of (const ZoneInfo*) in the zone_registry.cpp file.
 */
using ZoneRegistryBroker = zoneinfomid::ZoneRegistryBroker<ZoneInfo>;

/** Storage object that returns a ZoneInfoBroker from a ZoneInfo pointer. */
using ZoneInfoStore = zoneinfomid::ZoneInfoStore<
    ZoneContext, ZoneInfo, ZoneEra, ZonePolicy, ZoneRule>;

} // extended

namespace complete {

/** Data broker for accessing ZoneContext. */
using ZoneContextBroker = zoneinfohigh::ZoneContextBroker<ZoneContext>;

/** Data broker for accessing ZoneRule. */
using ZoneRuleBroker = zoneinfohigh::ZoneRuleBroker<ZoneContext, ZoneRule>;

/** Data broker for accessing ZonePolicy. */
using ZonePolicyBroker = zoneinfohigh::ZonePolicyBroker<
    ZoneContext, ZonePolicy, ZoneRule>;

/** Data broker for accessing ZoneEra. */
using ZoneEraBroker = zoneinfohigh::ZoneEraBroker<
    ZoneContext, ZoneEra, ZonePolicy, ZoneRule>;

/** Data broker for accessing ZoneInfo. */
using ZoneInfoBroker = zoneinfohigh::ZoneInfoBroker<
    ZoneContext, ZoneInfo, ZoneEra, ZonePolicy, ZoneRule>;

/**
 * Data broker for accessing the ZoneRegistry. The ZoneRegistry is an
 * array of (const ZoneInfo*) in the zone_registry.cpp file.
 */
using ZoneRegistryBroker = zoneinfohigh::ZoneRegistryBroker<ZoneInfo>;

using ZoneInfoStore = zoneinfohigh::ZoneInfoStore<
    ZoneContext, ZoneInfo, ZoneEra, ZonePolicy, ZoneRule>;

} // complete

} // ace_time

#endif
