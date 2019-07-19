/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#ifndef ACE_TIME_ZONE_SPECIFIER_CACHE_H
#define ACE_TIME_ZONE_SPECIFIER_CACHE_H

#include "common/common.h"
#include "TimeOffset.h"
#include "OffsetDateTime.h"
#include "BasicZoneSpecifier.h"
#include "ExtendedZoneSpecifier.h"
#include "ZoneRegistrar.h"

namespace ace_time {

/**
 * Common interface to BasicZoneSpecifierCache and ExtendedZoneSpecifierCache.
 * This allows TimeZone to hold only a single implementation of
 * ZoneSpecifierCache, without having to load the code for both
 * implementations.
 */
class ZoneSpecifierCache {
  public:
    /**
     * Get ZoneSpecifier from either a basic::ZoneInfo or an
     * extended::ZoneInfo. Unfortunately, this is not type-safe, but that's the
     * only way we can avoid compile-time dependencies to both implementation
     * classes.
     */
    virtual ZoneSpecifier* getZoneSpecifier(const void* zoneInfo) = 0;
};

/**
 * A cache of ZoneSpecifiers that provides a ZoneSpecifier to the TimeZone
 * upon request.
 *
 * @tparam SIZE number of zone specifiers, should be approximate the number
 *    zones *concurrently* used in the app. It is expected that this will be
 *    small. It can be 1 if the app never changes the TimeZone. It should be 2
 *    if the user is able to select different timezones from a menu.
 * @tparam ZS type of ZoneSpecifier (BasicZoneSpecifier or
 * ExtendedZoneSpecifier)
 * @tparam ZI type of ZoneInfo (basic::ZoneInfo or extended::ZoneInfo)
 * @tparam ZIB type of ZoneInfoBroker (basic::ZoneInfoBroker or 
 *    extended::ZoneInfoBroker)
 */
template<uint8_t SIZE, typename ZS, typename ZI, typename ZIB>
class ZoneSpecifierCacheImpl: public ZoneSpecifierCache {
  public:
    ZoneSpecifierCacheImpl() {}

    /** Get the ZoneSpecifier from the zoneInfo. Will never return nullptr. */
    ZoneSpecifier* getZoneSpecifier(const void* zoneInfo) override {
      ZS* zoneSpecifier = findUsingZoneInfo((const ZI*) zoneInfo);
      if (zoneSpecifier) return zoneSpecifier;

      // Allocate the next ZoneSpecifier in the cache using round-robin.
      zoneSpecifier = &mZoneSpecifiers[mCurrentIndex];
      mCurrentIndex++;
      if (mCurrentIndex >= SIZE) mCurrentIndex = 0;
      zoneSpecifier->setZoneInfo((const ZI*) zoneInfo);
      return zoneSpecifier;
    }

  private:
    // disable copy constructor and assignment operator
    ZoneSpecifierCacheImpl(const ZoneSpecifierCacheImpl&) = delete;
    ZoneSpecifierCacheImpl& operator=(const ZoneSpecifierCacheImpl&) = delete;

    /**
     * Find an existing ZoneSpecifier with the same zoneInfo.
     * Returns nullptr if not found. This is a linear search, which should
     * be perfectly ok if SIZE is small, say <= 5.
     */
    ZS* findUsingZoneInfo(const ZI* zoneInfoKey) {
      for (uint8_t i = 0; i < SIZE; i++) {
        const ZI* zoneInfo = (const ZI*) mZoneSpecifiers[i].getZoneInfo();
        if (zoneInfo == zoneInfoKey) {
          return &mZoneSpecifiers[i];
        }
      }
      return nullptr;
    }

    ZS mZoneSpecifiers[SIZE];
    uint8_t mCurrentIndex = 0;
};

template<uint8_t SIZE>
using BasicZoneSpecifierCache = ZoneSpecifierCacheImpl<SIZE,
    BasicZoneSpecifier, basic::ZoneInfo, basic::ZoneInfoBroker>;

template<uint8_t SIZE>
using ExtendedZoneSpecifierCache  = ZoneSpecifierCacheImpl<SIZE,
    ExtendedZoneSpecifier, extended::ZoneInfo, extended::ZoneInfoBroker>;

}

#endif
