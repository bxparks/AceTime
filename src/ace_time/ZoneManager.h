/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#ifndef ACE_TIME_ZONE_MANAGER_H
#define ACE_TIME_ZONE_MANAGER_H

#include <AceCommon.h>
#include <AceSorting.h>
#include "internal/ZoneRegistrar.h"
#include "ZoneProcessorCache.h"
#include "TimeZoneData.h"
#include "TimeZone.h"
#include "BasicZone.h"
#include "ExtendedZone.h"

namespace ace_time {

/**
 * Base class for ManualZoneManager, BasicZoneManager, and ExtendedZoneManager
 * to keep ZoneManager::kInvalidIndex for backwards compatibility. Subclasses
 * are not meant to be used polymorphically because none of the methods are
 * virtual.
 */
class ZoneManager {
  public:

    /** Registry index which is not valid. Indicates an error or not found. */
    static const uint16_t kInvalidIndex = 0xffff;
};

/**
 * A simple version of ZoneManager that converts a manual TimeZoneData
 * with fixed STD and DST offsets into a TimeZone.
 */
class ManualZoneManager {
  public:
    /**
     * Create a TimeZone with fixed STD and DST offsets stored in the
     * TimeZoneData which was created by TimeZone::toTimeZoneData().
     * IANA timezones are not supported.
     */
    TimeZone createForTimeZoneData(const TimeZoneData& d) {
      switch (d.type) {
        case TimeZoneData::kTypeError:
          return TimeZone::forError();
        case TimeZoneData::kTypeManual:
          return TimeZone::forTimeOffset(
              TimeOffset::forMinutes(d.stdOffsetMinutes),
              TimeOffset::forMinutes(d.dstOffsetMinutes));
        default:
          return TimeZone::forError();
      }
    }

    uint16_t zoneRegistrySize() const { return 0; }
};

/**
 * A templatized implementation of ZoneManager that binds the ZoneRegistrar
 * with the corresponding (Basic|Extended)ZoneProcessorCache. Applications will
 * normally use two specific instantiation of this class:
 * BasicZoneManager and ExtendedZoneManager.
 *
 * If an entry in the ZoneRegistrar is not found, then TimeZone::forError() will
 * be returned.
 *
 * If a ZoneProcessor exists in the ZoneProcessorCache that is already bound to
 * the given ZoneInfo, then the ZoneProcessor is reused. If not, another
 * ZoneProcessor is picked from the cache in a round-robin fashion (kicking off
 * the previously bound TimeZone). The type of the TimeZone will be assigned
 * based on the ZoneProcessor, which will be either kTypeBasic or kTypeExtended.
 *
 * @tparam ZI type of ZoneInfo (basic::ZoneInfo or extended::ZoneInfo) which
 *    make up the zone registry
 * @tparam ZRR class of ZoneRegistrar which holds the registry of ZoneInfo
 *    (e.g. basic::ZoneRegistrar, extended::ZoneRegistrar)
 * @tparam ZP class of ZoneProcessor (e.g. BasicZoneProcessor,
 *    ExtendedZoneProcessor)
 * @tparam Z zone wrapper class, either BasicZone or ExtendedZone
 */
template <
    typename ZI, typename ZRR,
    typename ZP, typename Z
>
class ZoneManagerTemplate : public ZoneManager {
  public:
    /**
     * Create a TimeZone for the given zone name (e.g. "America/Los_Angeles").
     */
    TimeZone createForZoneName(const char* name) {
      const ZI* zoneInfo = mZoneRegistrar.getZoneInfoForName(name);
      return createForZoneInfo(zoneInfo);
    }

    /** Create a TimeZone for the given 32-bit zoneId. */
    TimeZone createForZoneId(uint32_t id) {
      const ZI* zoneInfo = mZoneRegistrar.getZoneInfoForId(id);
      return createForZoneInfo(zoneInfo);
    }

    /**
     * Create a TimeZone for the given index in the ZoneInfo registry that was
     * used to create this ZoneManager.
     */
    TimeZone createForZoneIndex(uint16_t index) {
      const ZI* zoneInfo = mZoneRegistrar.getZoneInfoForIndex(index);
      return createForZoneInfo(zoneInfo);
    }

    /**
     * Create a TimeZone from the TimeZoneData created by
     * TimeZone::toTimeZoneData().
     */
    TimeZone createForTimeZoneData(const TimeZoneData& d) {
      switch (d.type) {
        case TimeZoneData::kTypeError:
          return TimeZone::forError();
        case TimeZoneData::kTypeManual:
          return TimeZone::forTimeOffset(
              TimeOffset::forMinutes(d.stdOffsetMinutes),
              TimeOffset::forMinutes(d.dstOffsetMinutes));
        case TimeZoneData::kTypeZoneId:
          return createForZoneId(d.zoneId);
        default:
          // Maybe this should return TimeZone::forError()?
          return TimeZone();
      }
    }

    /**
     * Find the registry index for the given time zone name. Returns
     * kInvalidIndex if not found.
     */
    uint16_t indexForZoneName(const char* name) const {
      return mZoneRegistrar.findIndexForName(name);
    }

    /**
     * Find the registry index for the given time zone id. Returns
     * kInvalidIndex if not found.
     */
    uint16_t indexForZoneId(uint32_t id) const {
      return mZoneRegistrar.findIndexForId(id);
    }

    /**
     * Return the number of elements in the Zone and Fat Link registry.
     * Previously named registrySize().
     */
    uint16_t zoneRegistrySize() const {
      return mZoneRegistrar.zoneRegistrySize();
    }

    /**
     * Create a TimeZone from an explicit ZoneInfo reference. The ZoneRegistrar
     * will be bypassed because the ZoneInfo is already available, but the
     * TimeZone will use a ZoneProcessor from its ZoneProcessorCache. This is
     * expected to be used mostly in tests, but it could be useful for
     * applications.
     */
    TimeZone createForZoneInfo(const ZI* zoneInfo) {
      if (! zoneInfo) return TimeZone::forError();
      ZP* processor = mZoneProcessorCache.getZoneProcessor(
          (uintptr_t) zoneInfo);
      return TimeZone::forZoneInfo(zoneInfo, processor);
    }

    /**
     * Return the ZoneProcessor for given zone name. Mostly for debugging
     * purposes.
     */
    ZP* getZoneProcessor(const char* name) {
      const ZI* zoneInfo = this->mZoneRegistrar.getZoneInfoForName(name);
      if (! zoneInfo) return nullptr;
      return this->mZoneProcessorCache.getZoneProcessor((uintptr_t) zoneInfo);
    }

    /** Return the Zone wrapper object for the given index. */
    Z getZoneForIndex(uint16_t index) const {
      const ZI* zoneInfo = this->mZoneRegistrar.getZoneInfoForIndex(index);
      return Z(zoneInfo);
    }

  protected:
    /**
     * Constructor.
     *
     * @param zoneRegistrySize number of ZoneInfo entries in zoneRegistry
     * @param zoneRegistry an array of ZoneInfo entries
     */
    ZoneManagerTemplate(
        uint16_t zoneRegistrySize,
        const ZI* const* zoneRegistry,
        ZoneProcessorCacheBaseTemplate<ZP>& zoneProcessorCache
    ):
        mZoneRegistrar(zoneRegistrySize, zoneRegistry),
        mZoneProcessorCache(zoneProcessorCache)
    {}

    // disable copy constructor and assignment operator
    ZoneManagerTemplate(const ZoneManagerTemplate&) = delete;
    ZoneManagerTemplate& operator=(const ZoneManagerTemplate&) = delete;

  protected:
    const ZRR mZoneRegistrar;
    ZoneProcessorCacheBaseTemplate<ZP>& mZoneProcessorCache;
};

#if 1
/**
 * An implementation of the ZoneManager which uses a registry of basic::ZoneInfo
 * records.
 */
class BasicZoneManager: public ZoneManagerTemplate<
    basic::ZoneInfo,
    basic::ZoneRegistrar,
    BasicZoneProcessor,
    BasicZone
> {

  public:
    BasicZoneManager(
        uint16_t zoneRegistrySize,
        const basic::ZoneInfo* const* zoneRegistry,
        BasicZoneProcessorCacheBase& zoneProcessorCache
    ):
        ZoneManagerTemplate<
            basic::ZoneInfo,
            basic::ZoneRegistrar,
            BasicZoneProcessor,
            BasicZone
        >(
            zoneRegistrySize,
            zoneRegistry,
            zoneProcessorCache
        )
    {}

};

/**
 * An implementation of the ZoneManager which uses a registry of
 * extended::ZoneInfo records.
 */
class ExtendedZoneManager: public ZoneManagerTemplate<
    extended::ZoneInfo,
    extended::ZoneRegistrar,
    ExtendedZoneProcessor,
    ExtendedZone
> {

  public:
    ExtendedZoneManager(
        uint16_t zoneRegistrySize,
        const extended::ZoneInfo* const* zoneRegistry,
        ExtendedZoneProcessorCacheBase& zoneProcessorCache
    ):
        ZoneManagerTemplate<
            extended::ZoneInfo,
            extended::ZoneRegistrar,
            ExtendedZoneProcessor,
            ExtendedZone
        >(
            zoneRegistrySize,
            zoneRegistry,
            zoneProcessorCache
        )
    {}
};

#else

// NOTE: The following typedefs seem shorter and easier to maintain. The
// problem is that they make error messages basically impossible to decipher
// because the template class names are far too long for human comprehension.
// Fortunately, there seems to be no difference in code size between the above
// solution using subclasses and this solution using typedefs. The compiler
// seems to optimize away the vtables of the parent and child classes. So we'll
// use the above subclassing solution to get better error messages.

using BasicZoneManager = ZoneManagerTemplate<
    basic::ZoneInfo,
    basic::ZoneRegistrar,
    BasicZoneProcessor,
    BasicZone
>;

template <uint8_t SIZE>
using ExtendedZoneManager = ZoneManagerTemplate<
    extended::ZoneInfo,
    extended::ZoneRegistrar,
    ExtendedZoneProcessor,
    ExtendedZone
>;

#endif

}

#endif
