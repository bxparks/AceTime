/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#ifndef ACE_TIME_ZONE_MANAGER_H
#define ACE_TIME_ZONE_MANAGER_H

#include "ZoneSpecifierCache.h"
#include "ZoneRegistrar.h"
#include "TimeZone.h"

namespace ace_time {

/**
 * Returns the TimeZone given the zoneInfo, zoneName, or zoneId. Looks up the
 * ZoneInfo in the ZoneRegistrar. If an existing ZoneSpecifier exists in the
 * ZoneSpecifierCache, then it is used. If not, another ZoneSpecifier is picked
 * from the cache in a round-robin fashion.
 *
 * @tparam ZI type of ZoneInfo (basic::ZoneInfo or extended::ZoneInfo) which
 *    make up the zone registry
 * @tparam ZR class of ZoneRegistrar
 * @tparam ZSC class of ZoneSpecifierCache
 */
template<typename ZI, typename ZR, typename ZSC>
class ZoneManager {
  public:
    ZoneManager(uint16_t registrySize, const ZI* const* zoneRegistry):
        mZoneRegistrar(registrySize, zoneRegistry),
        mZoneSpecifierCache() {}

    TimeZone createForZoneName(const char* name) {
      const void* zoneInfo = mZoneRegistrar.getZoneInfoForName(name);
      if (! zoneInfo) return TimeZone::forError();
      return TimeZone(&mZoneSpecifierCache, zoneInfo);
    }

    TimeZone createForZoneId(uint32_t id) {
      const void* zoneInfo = mZoneRegistrar.getZoneInfoForId(id);
      if (! zoneInfo) return TimeZone::forError();
      return TimeZone(&mZoneSpecifierCache, zoneInfo);
    }

    TimeZone createForZoneInfo(const ZI* zoneInfo) {
      if (! zoneInfo) return TimeZone::forError();
      return TimeZone(&mZoneSpecifierCache, zoneInfo);
    }

  private:
    // disable copy constructor and assignment operator
    ZoneManager(const ZoneManager&) = delete;
    ZoneManager& operator=(const ZoneManager&) = delete;

    const ZR mZoneRegistrar;
    ZSC mZoneSpecifierCache;
};

/**
 * @tparam SIZE size of the ZoneSpecifierCache
 */
template<uint8_t SIZE>
using BasicZoneManager = ZoneManager<basic::ZoneInfo,
    BasicZoneRegistrar, BasicZoneSpecifierCache<SIZE>>;

/**
 * @tparam SIZE size of the ZoneSpecifierCache
 */
template<uint8_t SIZE>
using ExtendedZoneManager = ZoneManager<extended::ZoneInfo,
    ExtendedZoneRegistrar, ExtendedZoneSpecifierCache<SIZE>>;

}

#endif
