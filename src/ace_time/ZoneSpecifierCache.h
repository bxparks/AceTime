/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#ifndef ACE_TIME_ZONE_SPECIFIER_CACHE_H
#define ACE_TIME_ZONE_SPECIFIER_CACHE_H

#include "common/common.h"
#include "TimeOffset.h"
#include "OffsetDateTime.h"
#include "ZoneManager.h"

namespace ace_time {

namespace common {

/**
 * A cache of ZoneSpecifiers that provides a ZoneSpecifier to the TimeZone
 * upon request.
 *
 * @tparam ZM type of ZoneManager (BasicZoneManager or ExtendedZoneManager)
 * @tparam ZS type of ZoneSpecifier (BasicZoneSpecifier or
 * ExtendedZoneSpecifier)
 * @tparam ZI type of ZoneInfo (basic::ZoneInfo or extended::ZoneInfo)
 * @tparam ZIB type of ZoneInfoBroker (basic::ZoneInfoBroker or 
 *    extended::ZoneInfoBroker)
 * @tparam SIZE number of zone specifiers, should be approximate the number
 *    zones *concurrently* used in the app. It is expected that this will be
 *    small. It can be 1 if the app never changes the TimeZone. It should be 2
 *    if the user is able to select different timezones from a menu.
 */
template<typename ZM, typename ZS, typename ZI, typename ZIB, uint8_t SIZE>
class ZoneSpecifierCache {
  public:
    ZoneSpecifierCache(const ZM& zoneManager):
        mZoneManager(zoneManager) {}

    /**
     * Get the ZoneSpecifier for the given zoneId. Returns nullptr if the
     * zoneId is not recognized by the given ZoneManager. Return a previously
     * allocated ZoneSpecifier or a new ZoneSpecifier.
     */
    ZS* getZoneSpecifier(uint32_t zoneId) {
      const ZI* zoneInfo = mZoneManager.getZoneInfo(zoneId);
      if (! zoneInfo) return nullptr;

      return getZoneSpecifier(zoneInfo);
    }

    /** Get the ZoneSpecifier from the zoneInfo. Will never return nullptr. */
    ZS* getZoneSpecifier(const ZI* zoneInfo) {
      ZS* zoneSpecifier = findUsingZoneInfo(zoneInfo);
      if (zoneSpecifier) return zoneSpecifier;

      // Allocate the next ZoneSpecifier in the cache using round-robin.
      zoneSpecifier = &mZoneSpecifiers[mCurrentIndex];
      mCurrentIndex++;
      if (mCurrentIndex >= SIZE) mCurrentIndex = 0;
      zoneSpecifier->setZoneInfo(zoneInfo);
      return zoneSpecifier;
    }

  private:
    // disable copy constructor and assignment operator
    ZoneSpecifierCache(const ZoneSpecifierCache&) = delete;
    ZoneSpecifierCache& operator=(const ZoneSpecifierCache&) = delete;

    /**
     * Find an existing ZoneSpecifier with the same zoneInfo.
     * Returns nullptr if not found. This is a linear search, which should
     * be perfectly ok if SIZE is small, say <= 5.
     */
    ZS* findUsingZoneInfo(const ZI* zoneInfoKey) {
      for (uint8_t i = 0; i < SIZE; i++) {
        const ZI* zoneInfo = mZoneSpecifiers[i].getZoneInfo();
        if (zoneInfo == zoneInfoKey) {
          return &mZoneSpecifiers[i];
        }
      }
      return nullptr;
    }

    const ZM& mZoneManager;

    ZS mZoneSpecifiers[SIZE];
    uint8_t mCurrentIndex = 0;
};

}

template<uint8_t SIZE>
using BasicZoneSpecifierCache = common::ZoneSpecifierCache<
    BasicZoneManager, BasicZoneSpecifier, basic::ZoneInfo,
    basic::ZoneInfoBroker, SIZE>;

template<uint8_t SIZE>
using ExtendedZoneSpecifierCache  = common::ZoneSpecifierCache<
    ExtendedZoneManager, ExtendedZoneSpecifier,
    extended::ZoneInfo, extended::ZoneInfoBroker, SIZE>;

}

#endif
