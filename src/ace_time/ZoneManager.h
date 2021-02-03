/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#ifndef ACE_TIME_ZONE_MANAGER_H
#define ACE_TIME_ZONE_MANAGER_H

#include "ZoneProcessorCache.h"
#include "ZoneRegistrar.h"
#include "TimeZoneData.h"
#include "TimeZone.h"

namespace ace_time {

/**
 * Common interface to the BasicZoneManager and ExtendedZoneManager so that a
 * single interface can be passed around to various helper objects. Various
 * methods return a TimeZone object from one of its identifiers. The identifer
 * can be a string (e.g. "America/Los_Angeles"), or a unique hashed zoneId, an
 * index into the ZoneRegistry, or the TimeZoneData object created by
 * TimeZone::toTimeZoneData().
 */
class ZoneManager {
  public:
    /** Registry index which is not valid. Indicates an error or not found. */
    static const uint16_t kInvalidIndex = 0xffff;

    /**
     * Create a TimeZone for the given zone name (e.g. "America/Los_Angeles").
     */
    virtual TimeZone createForZoneName(const char* name) = 0;

    /** Create a TimeZone for the given 32-bit zoneId. */
    virtual TimeZone createForZoneId(uint32_t id) = 0;

    /**
     * Create a TimeZone for the given index in the ZoneInfo registry that was
     * used to create this ZoneManager.
     */
    virtual TimeZone createForZoneIndex(uint16_t index) = 0;

    /**
     * Create a TimeZone from the TimeZoneData created by
     * TimeZone::toTimeZoneData().
     */
    virtual TimeZone createForTimeZoneData(const TimeZoneData& d) = 0;

    /**
     * Find the registry index for the given time zone name. Returns
     * kInvalidIndex if not found.
     */
    virtual uint16_t indexForZoneName(const char* name) const = 0;

    /**
     * Find the registry index for the given time zone id. Returns
     * kInvalidIndex if not found.
     */
    virtual uint16_t indexForZoneId(uint32_t id) const = 0;

    /** Return the number of elements in the registry. */
    virtual uint16_t registrySize() const = 0;
};

/**
 * A ZoneManager that implements only createForTimeZoneData() to create
 * TimeZones of type kTypeManual, in other words, time zones with fixed STD and
 * DST offsets. This is useful in applications designed to run on
 * microcontrollers with small memory where a full BasicZoneManager or
 * ExtendedZoneManager generate too much code. This object can be used anywhere
 * a ZoneManager is expected, which reduces the need for C-preprocessor
 * conditional code.
 */
class ManualZoneManager : public ZoneManager {
  public:

    TimeZone createForZoneName(const char* /*name*/) override {
      return TimeZone::forError();
    }

    TimeZone createForZoneId(uint32_t /*id*/) override {
      return TimeZone::forError();
    }

    TimeZone createForZoneIndex(uint16_t /*index*/) override {
      return TimeZone::forError();
    }

    TimeZone createForTimeZoneData(const TimeZoneData& d) override {
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

    uint16_t indexForZoneName(const char* /*name*/) const override {
      return kInvalidIndex;
    }

    uint16_t indexForZoneId(uint32_t /*id*/) const override {
      return kInvalidIndex;
    }

    uint16_t registrySize() const override { return 0; }
};

/**
 * A templatized implementation of ZoneManager that binds the
 * (Basic|Extended)ZoneRegistrar with the corresponding
 * (Basic|Extended)ZoneProcessorCache. Applications will normally use two
 * specific instantiation of this class: BasicZoneManager<SIZE> and
 * ExtendedZoneManager<SIZE>.
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
 *    (e.g. BasicZoneRegistrar, ExtendedZoneRegistrar)
 * @tparam ZP class of ZoneProcessor (e.g. BasicZoneProcessor,
 *    ExtendedZoneProcessor)
 * @tparam ZPC class of ZoneProcessorCache (e.g. BasicZoneProcessorCache,
 *    ExtendedZoneProcessorCache)
 */
template<typename ZI, typename ZRR, typename ZP, typename ZPC>
class ZoneManagerImpl : public ZoneManager {
  public:
    TimeZone createForZoneName(const char* name) override {
      const ZI* zoneInfo = mZoneRegistrar.getZoneInfoForName(name);
      return createForZoneInfo(zoneInfo);
    }

    TimeZone createForZoneId(uint32_t id) override {
      const ZI* zoneInfo = mZoneRegistrar.getZoneInfoForId(id);
      return createForZoneInfo(zoneInfo);
    }

    TimeZone createForZoneIndex(uint16_t index) override {
      const ZI* zoneInfo = mZoneRegistrar.getZoneInfoForIndex(index);
      return createForZoneInfo(zoneInfo);
    }

    TimeZone createForTimeZoneData(const TimeZoneData& d) override {
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

    uint16_t indexForZoneName(const char* name) const override {
      return mZoneRegistrar.findIndexForName(name);
    }

    uint16_t indexForZoneId(uint32_t id) const override {
      return mZoneRegistrar.findIndexForId(id);
    }

    uint16_t registrySize() const override {
      return mZoneRegistrar.registrySize();
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
      ZP* processor = (ZP*) mZoneProcessorCache.getZoneProcessor(zoneInfo);
      return TimeZone::forZoneInfo(zoneInfo, processor);
    }

  protected:
    /**
     * Constructor.
     *
     * @param registrySize number of ZoneInfo entries in the registry
     * @param zoneRegistry an array of ZoneInfo entries
     */
    ZoneManagerImpl(uint16_t registrySize, const ZI* const* zoneRegistry):
        mZoneRegistrar(registrySize, zoneRegistry),
        mZoneProcessorCache() {}

  private:
    // disable copy constructor and assignment operator
    ZoneManagerImpl(const ZoneManagerImpl&) = delete;
    ZoneManagerImpl& operator=(const ZoneManagerImpl&) = delete;

    const ZRR mZoneRegistrar;
    ZPC mZoneProcessorCache;
};

#if 1
/**
 * An implementation of the ZoneManager which uses a registry of basic::ZoneInfo
 * records.
 *
 * @tparam SIZE size of the BasicZoneProcessorCache
 */
template<uint16_t SIZE>
class BasicZoneManager: public ZoneManagerImpl<
      basic::ZoneInfo,
      BasicZoneRegistrar,
      BasicZoneProcessor,
      BasicZoneProcessorCache<SIZE>
    > {

  public:
    BasicZoneManager(
        uint16_t registrySize,
        const basic::ZoneInfo* const* zoneRegistry
    ):
        ZoneManagerImpl<
            basic::ZoneInfo,
            BasicZoneRegistrar,
            BasicZoneProcessor,
            BasicZoneProcessorCache<SIZE>
        >(
          registrySize,
          zoneRegistry
        )
    {}
};

/**
 * An implementation of the ZoneManager which uses a registry of
 * extended::ZoneInfo records.
 *
 * @tparam SIZE size of the ExtendedZoneProcessorCache
 */
template<uint16_t SIZE>
class ExtendedZoneManager: public ZoneManagerImpl<
      extended::ZoneInfo,
      ExtendedZoneRegistrar,
      ExtendedZoneProcessor,
      ExtendedZoneProcessorCache<SIZE>
    > {

  public:
    ExtendedZoneManager(
        uint16_t registrySize,
        const extended::ZoneInfo* const* zoneRegistry
    ):
        ZoneManagerImpl<
            extended::ZoneInfo,
            ExtendedZoneRegistrar,
            ExtendedZoneProcessor,
            ExtendedZoneProcessorCache<SIZE>
        >(
          registrySize,
          zoneRegistry
        )
    {}
};

#else

// NOTE: The following typedefs seem shorter and easier to maintain. The
// problem is that they make error messages basically impossible to decipher
// because the template class names are far too long for human comprehension.
// Fortunatley, there seems to be no difference in code size between the above
// solution using subclasses and this solution using typedefs. The compiler
// seems to optimize away the vtables of the parent and child classes. So we'll
// use the above subclassing solution to get better error messages.

template<uint8_t SIZE>
using BasicZoneManager = ZoneManagerImpl<basic::ZoneInfo,
    BasicZoneRegistrar, BasicZoneProcessorCache<SIZE>>;

template<uint8_t SIZE>
using ExtendedZoneManager = ZoneManagerImpl<extended::ZoneInfo,
    ExtendedZoneRegistrar, ExtendedZoneProcessorCache<SIZE>>;

#endif

}

#endif
