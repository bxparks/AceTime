/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#ifndef ACE_TIME_ZONE_PROCESSOR_CACHE_H
#define ACE_TIME_ZONE_PROCESSOR_CACHE_H

#include "common/common.h"
#include "TimeOffset.h"
#include "OffsetDateTime.h"
#include "BasicZoneProcessor.h"
#include "ExtendedZoneProcessor.h"
#include "ZoneRegistrar.h"

namespace ace_time {

/**
 * Common interface to BasicZoneProcessorCache and ExtendedZoneProcessorCache.
 * This allows TimeZone to hold only a single implementation of
 * ZoneProcessorCache, without having to load the code for both
 * implementations.
 */
class ZoneProcessorCache {
  public:
    static const uint8_t kTypeBasicManaged = ZoneProcessor::kTypeBasic + 2;
    static const uint8_t kTypeExtendedManaged =
        ZoneProcessor::kTypeExtended + 2;

    /** Return the type of this cache. */
    virtual uint8_t getType() = 0;

    /**
     * Get ZoneProcessor from either a basic::ZoneInfo or an
     * extended::ZoneInfo. Unfortunately, this is not type-safe, but that's the
     * only way we can avoid compile-time dependencies to both implementation
     * classes.
     */
    virtual ZoneProcessor* getZoneProcessor(const void* zoneInfo) = 0;
};

/**
 * A cache of ZoneProcessors that provides a ZoneProcessor to the TimeZone
 * upon request.
 *
 * @tparam SIZE number of zone processors, should be approximate the number
 *    zones *concurrently* used in the app. It is expected that this will be
 *    small. It can be 1 if the app never changes the TimeZone. It should be 2
 *    if the user is able to select different timezones from a menu.
 * @tparam ZP type of ZoneProcessor (BasicZoneProcessor or
 *    ExtendedZoneProcessor)
 * @tparam ZI type of ZoneInfo (basic::ZoneInfo or extended::ZoneInfo)
 * @tparam ZIB type of ZoneInfoBroker (basic::ZoneInfoBroker or 
 *    extended::ZoneInfoBroker)
 */
template<uint8_t SIZE, uint8_t TYPE, typename ZP, typename ZI, typename ZIB>
class ZoneProcessorCacheImpl: public ZoneProcessorCache {
  public:
    ZoneProcessorCacheImpl() {}

    uint8_t getType() override { return TYPE; }

    /** Get the ZoneProcessor from the zoneInfo. Will never return nullptr. */
    ZoneProcessor* getZoneProcessor(const void* zoneInfo) override {
      ZP* zoneProcessor = findUsingZoneInfo((const ZI*) zoneInfo);
      if (zoneProcessor) return zoneProcessor;

      // Allocate the next ZoneProcessor in the cache using round-robin.
      zoneProcessor = &mZoneProcessors[mCurrentIndex];
      mCurrentIndex++;
      if (mCurrentIndex >= SIZE) mCurrentIndex = 0;
      zoneProcessor->setZoneInfo((const ZI*) zoneInfo);
      return zoneProcessor;
    }

  private:
    // disable copy constructor and assignment operator
    ZoneProcessorCacheImpl(const ZoneProcessorCacheImpl&) = delete;
    ZoneProcessorCacheImpl& operator=(const ZoneProcessorCacheImpl&) = delete;

    /**
     * Find an existing ZoneProcessor with the same zoneInfo.
     * Returns nullptr if not found. This is a linear search, which should
     * be perfectly ok if SIZE is small, say <= 5.
     */
    ZP* findUsingZoneInfo(const ZI* zoneInfoKey) {
      for (uint8_t i = 0; i < SIZE; i++) {
        const ZI* zoneInfo = (const ZI*) mZoneProcessors[i].getZoneInfo();
        if (zoneInfo == zoneInfoKey) {
          return &mZoneProcessors[i];
        }
      }
      return nullptr;
    }

    ZP mZoneProcessors[SIZE];
    uint8_t mCurrentIndex = 0;
};

#if 1
template<uint8_t SIZE>
class BasicZoneProcessorCache: public ZoneProcessorCacheImpl<
    SIZE, ZoneProcessorCache::kTypeBasicManaged,
    BasicZoneProcessor, basic::ZoneInfo, basic::ZoneInfoBroker> {
};

template<uint8_t SIZE>
class ExtendedZoneProcessorCache: public ZoneProcessorCacheImpl<
    SIZE, ZoneProcessorCache::kTypeExtendedManaged,
    ExtendedZoneProcessor, extended::ZoneInfo, extended::ZoneInfoBroker> {
};
#else

// NOTE: The following typedef seems shorter and easier to maintain. The
// problem is that it makes error messages basically impossible to decipher
// because the immensely long full template class name is printed out. There
// seems to be no difference in code size between the two. The compiler seems
// to optimize away the vtables of the parent and child classes.

template<uint8_t SIZE>
using BasicZoneProcessorCache = ZoneProcessorCacheImpl<
    SIZE, ZoneProcessorCache::kTypeBasicManaged,
    BasicZoneProcessor, basic::ZoneInfo, basic::ZoneInfoBroker>;

template<uint8_t SIZE>
using ExtendedZoneProcessorCache  = ZoneProcessorCacheImpl<
    SIZE, ZoneProcessorCache::kTypeExtendedManaged,
    ExtendedZoneProcessor, extended::ZoneInfo, extended::ZoneInfoBroker>;
#endif

}

#endif
