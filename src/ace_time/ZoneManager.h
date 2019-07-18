/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#ifndef ACE_TIME_ZONE_MANAGER_H
#define ACE_TIME_ZONE_MANAGER_H

#include "ZoneSpecifierCache.h"
#include "ZoneRegistrar.h"

namespace ace_time {

class ZoneManager {
  public:
    virtual ZoneSpecifier* getZoneSpecifier(uint32_t zoneId) = 0;
    virtual ZoneSpecifier* getZoneSpecifier(const void* zoneInfo) = 0;
};

/**
 * @tparam SIZE number of ZoneSpecifiers to cache
 * @tparam ZI type of ZoneInfo (basic::ZoneInfo or extended::ZoneInfo)
 * @tparam ZR class of ZoneRegistrar
 * @tparam ZSC class of ZoneSpecifierCache
 */
template<typename ZI, typename ZR, typename ZSC>
class ZoneManagerImpl: public ZoneManager {
  public:
    ZoneManagerImpl(uint16_t registrySize, const ZI* const* zoneRegistry):
        mZoneRegistrar(registrySize, zoneRegistry),
        mZoneSpecifierCache() {}

    ZoneSpecifier* getZoneSpecifier(uint32_t zoneId) override {
      const ZI* zoneInfo = mZoneRegistrar.getZoneInfoFromId(zoneId);
      if (! zoneInfo) return nullptr;
      return mZoneSpecifierCache.getZoneSpecifier(zoneInfo);
    }

    ZoneSpecifier* getZoneSpecifier(const void* zoneInfo) override {
      return mZoneSpecifierCache.getZoneSpecifier(zoneInfo);
    }

  private:
    // disable copy constructor and assignment operator
    ZoneManagerImpl(const ZoneManagerImpl&) = delete;
    ZoneManagerImpl& operator=(const ZoneManagerImpl&) = delete;

    ZR mZoneRegistrar;
    ZSC mZoneSpecifierCache;
};

template<uint8_t SIZE>
using BasicZoneManager = ZoneManagerImpl<basic::ZoneInfo,
    BasicZoneRegistrar, BasicZoneSpecifierCache<SIZE>>;

template<uint8_t SIZE>
using ExtendedZoneManager = ZoneManagerImpl<extended::ZoneInfo,
    ExtendedZoneRegistrar, ExtendedZoneSpecifierCache<SIZE>>;

}

#endif
