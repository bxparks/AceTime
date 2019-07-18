/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#ifndef ACE_TIME_ZONE_MANAGER_H
#define ACE_TIME_ZONE_MANAGER_H

#include "ZoneSpecifierCache.h"
#include "ZoneRegistrar.h"

namespace ace_time {

/**
 * Returns the ZoneSpecifier given an identifier for the ZoneInfo. Looks up the
 * ZoneInfo in the ZoneRegistrar, then find the ZoneSpecifier that matches the
 * best. If an existing ZoneSpecifier exists in the ZoneSpecifierCache, then it
 * is returned. If not, another ZoneSpecifier is picked from the cache in a
 * round-robin fashion.
 */
class ZoneManager {
  public:
    static const uint8_t kTypeBasic = ZoneSpecifier::kTypeBasic;
    static const uint8_t kTypeExtended = ZoneSpecifier::kTypeExtended;

    virtual uint8_t getType() const = 0;
    virtual ZoneSpecifier* getZoneSpecifier(const void* zoneInfo) = 0;
    virtual ZoneSpecifier* getZoneSpecifier(uint32_t zoneId) = 0;
    virtual ZoneSpecifier* getZoneSpecifier(const char* name) = 0;
    virtual const void* getZoneInfo(const char* name) const = 0;
};

/**
 * @tparam TYPE the type of ZoneManager (either kTypeBasic or kTypeExtended)
 * @tparam ZI type of ZoneInfo (basic::ZoneInfo or extended::ZoneInfo) which
 *    make up the zone registry
 * @tparam ZR class of ZoneRegistrar
 * @tparam ZSC class of ZoneSpecifierCache
 */
template<uint8_t TYPE, typename ZI, typename ZR, typename ZSC>
class ZoneManagerImpl: public ZoneManager {
  public:
    ZoneManagerImpl(uint16_t registrySize, const ZI* const* zoneRegistry):
        mZoneRegistrar(registrySize, zoneRegistry),
        mZoneSpecifierCache() {}

    uint8_t getType() const override { return TYPE;}

    ZoneSpecifier* getZoneSpecifier(const void* zoneInfo) override {
      return mZoneSpecifierCache.getZoneSpecifier(zoneInfo);
    }

    ZoneSpecifier* getZoneSpecifier(uint32_t zoneId) override {
      const ZI* zoneInfo = mZoneRegistrar.getZoneInfoFromId(zoneId);
      if (! zoneInfo) return nullptr;
      return mZoneSpecifierCache.getZoneSpecifier(zoneInfo);
    }

    ZoneSpecifier* getZoneSpecifier(const char* name) override {
      const ZI* zoneInfo = mZoneRegistrar.getZoneInfo(name);
      if (! zoneInfo) return nullptr;
      return mZoneSpecifierCache.getZoneSpecifier(zoneInfo);
    }

    const void* getZoneInfo(const char* name) const override {
      return mZoneRegistrar.getZoneInfo(name);
    }

  private:
    // disable copy constructor and assignment operator
    ZoneManagerImpl(const ZoneManagerImpl&) = delete;
    ZoneManagerImpl& operator=(const ZoneManagerImpl&) = delete;

    const ZR mZoneRegistrar;
    ZSC mZoneSpecifierCache;
};

/*
 * @tparam SIZE size of the ZoneSpecifierCache
 */
template<uint8_t SIZE>
using BasicZoneManager = ZoneManagerImpl<ZoneManager::kTypeBasic,
    basic::ZoneInfo, BasicZoneRegistrar, BasicZoneSpecifierCache<SIZE>>;

/*
 * @tparam SIZE size of the ZoneSpecifierCache
 */
template<uint8_t SIZE>
using ExtendedZoneManager = ZoneManagerImpl<ZoneManager::kTypeExtended,
    extended::ZoneInfo, ExtendedZoneRegistrar,
    ExtendedZoneSpecifierCache<SIZE>>;

}

#endif
