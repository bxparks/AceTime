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

namespace ace_time {

/**
 * The template class of BasicZoneProcessorCacheBase or
 * ExtendedZoneProcessorCacheBase. The common implementation
 * BasicZoneProcessorCache<SIZE> and ExtendedZoneProcessorCacheBase<SIZE>
 * creates the caches inside the class itself, which will normally be created at
 * static initialization time. An alternative implementation would create the
 * cache on the heap.
 */
template <typename ZP>
class ZoneProcessorCacheBaseTemplate {
  public:
    ZoneProcessorCacheBaseTemplate(ZP* zoneProcessors, uint8_t size) :
      mSize(size),
      mZoneProcessors(zoneProcessors)
    {}

    /** Return the size of the cache. */
    uint8_t size() const { return mSize; }

    /** Get the ZoneProcessor at index i. */
    ZP* getZoneProcessorAtIndex(uint8_t i) { return &mZoneProcessors[i]; }

    /**
     * Get ZoneProcessor from either a ZoneKey, either a basic::ZoneInfo or an
     * extended::ZoneInfo. This will never return nullptr.
     */
    ZP* getZoneProcessor(uintptr_t zoneKey) {
      ZP* zoneProcessor = findUsingZoneKey(zoneKey);
      if (zoneProcessor) return zoneProcessor;

      // Allocate the next ZoneProcessor in the cache using round-robin.
      zoneProcessor = &mZoneProcessors[mCurrentIndex];
      mCurrentIndex++;
      if (mCurrentIndex >= mSize) mCurrentIndex = 0;
      zoneProcessor->setZoneKey(zoneKey);
      return zoneProcessor;
    }

    /**
     * Reset the transition cache of all zone processors in the cache.
     * Useful when LocalDate::localEpochYear() is changed at runtime.
     */
    void resetZoneProcessors() {
      for (uint8_t i = 0; i < mSize; i++) {
        ZP* zoneProcessor = &mZoneProcessors[i];
        zoneProcessor->resetTransitionCache();
      }
    }

  private:
    // disable copy constructor and assignment operator
    ZoneProcessorCacheBaseTemplate(const ZoneProcessorCacheBaseTemplate&)
        = delete;
    ZoneProcessorCacheBaseTemplate& operator=(
        const ZoneProcessorCacheBaseTemplate&) = delete;

    /**
     * Find an existing ZoneProcessor with the ZoneInfo given by zoneInfoKey.
     * Returns nullptr if not found. This is a linear search, which should
     * be perfectly ok if mSize is small, say <= 5.
     */
    ZP* findUsingZoneKey(uintptr_t zoneKey) {
      for (uint8_t i = 0; i < mSize; i++) {
        ZP* zoneProcessor = &mZoneProcessors[i];
        if (zoneProcessor->equalsZoneKey(zoneKey)) {
          return zoneProcessor;
        }
      }
      return nullptr;
    }

  private:
    uint8_t const mSize;
    uint8_t mCurrentIndex = 0;
    ZP* const mZoneProcessors;
};

/**
 * Base class for all ZoneProcessorCache implementations that use a
 * BasicZoneProcessor.
 */
using BasicZoneProcessorCacheBase =
    ZoneProcessorCacheBaseTemplate<BasicZoneProcessor>;

/**
 * Base class for all ZoneProcessorCache implementations that use an
 * ExtendedZoneProcessor.
 */
using ExtendedZoneProcessorCacheBase =
    ZoneProcessorCacheBaseTemplate<ExtendedZoneProcessor>;

#if 1
/**
 * An implementation of a BasicZoneProcessorCacheBase where the cache of size
 * SIZE is embedded into the class itself. This is expected to be created as a
 * global object and passed into the BasicZoneManager.
 *
 * @tparam SIZE number of zone processors, should be approximate the number
 *    zones *concurrently* used in the app. It is expected that this will be
 *    small. It can be 1 if the app never changes the TimeZone. It should be 2
 *    if the user is able to select different timezones from a menu.
 */
template <uint8_t SIZE>
class BasicZoneProcessorCache : public BasicZoneProcessorCacheBase {
  public:
    BasicZoneProcessorCache() :
      BasicZoneProcessorCacheBase(mZoneProcessors, SIZE)
    {}

  private:
    // disable copy constructor and assignment operator
    BasicZoneProcessorCache(const BasicZoneProcessorCache&) = delete;
    BasicZoneProcessorCache& operator=(const BasicZoneProcessorCache&) = delete;

  private:
    BasicZoneProcessor mZoneProcessors[SIZE];
};

/**
 * An implementation of an ExtendedZoneProcessorCacheBase where the cache of
 * size SIZE is embedded into the class itself. This is expected to be created
 * as a global object and passed into the ExtendedZoneManager.
 *
 * @tparam SIZE number of zone processors, should be approximate the number
 *    zones *concurrently* used in the app. It is expected that this will be
 *    small. It can be 1 if the app never changes the TimeZone. It should be 2
 *    if the user is able to select different timezones from a menu.
 */
template <uint8_t SIZE>
class ExtendedZoneProcessorCache: public ExtendedZoneProcessorCacheBase {
  public:
    ExtendedZoneProcessorCache() :
      ExtendedZoneProcessorCacheBase(mZoneProcessors, SIZE)
    {}

  private:
    // disable copy constructor and assignment operator
    ExtendedZoneProcessorCache(const ExtendedZoneProcessorCache&) = delete;
    ExtendedZoneProcessorCache& operator=(const ExtendedZoneProcessorCache&)
        = delete;

  private:
    ExtendedZoneProcessor mZoneProcessors[SIZE];
};
#else

// NOTE: The following typedef seems shorter and easier to maintain. The
// problem is that it makes error messages basically impossible to decipher
// because the immensely long full template class name is printed out. There
// seems to be no difference in code size between the two. The compiler seems
// to optimize away the vtables of the parent and child classes.

template <uint8_t SIZE>
using BasicZoneProcessorCache = ZoneProcessorCacheTemplate<
    SIZE, BasicZoneProcessor>;

template <uint8_t SIZE>
using ExtendedZoneProcessorCache  = ZoneProcessorCacheTemplate<
    SIZE, ExtendedZoneProcessor>;
#endif

}

#endif
