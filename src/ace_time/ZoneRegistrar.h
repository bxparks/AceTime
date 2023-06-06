/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#ifndef ACE_TIME_ZONE_REGISTRAR_H
#define ACE_TIME_ZONE_REGISTRAR_H

#include <stdint.h>
#include <AceCommon.h> // KString, binarySearchByKey(), isSortedByKey()
#include "../zoneinfo/ZoneInfo.h"
#include "../zoneinfo/Brokers.h"
#include "../zoneinfo/ZoneInfoHires.h"
#include "../zoneinfo/BrokersHires.h"

// AutoBenchmark.ino
void runIndexForZoneIdBinary();
void runIndexForZoneIdLinear();

// Tests
class ZoneRegistrarTest_Sorted_isSorted;
class ZoneRegistrarTest_Unsorted_isSorted;
class ZoneRegistrarTest_Sorted_linearSearchById;
class ZoneRegistrarTest_Sorted_linearSearchById_not_found;
class ZoneRegistrarTest_Sorted_binarySearchById_zeroEntries;
class ZoneRegistrarTest_Sorted_binarySearchById;
class ZoneRegistrarTest_Sorted_binarySearchById_not_found;
class ZoneRegistrarTest_Unsorted_linearSearchById;
class ZoneRegistrarTest_Unsorted_linearSearchById_not_found;

namespace ace_time {
namespace internal {

/**
 * Class that allows looking up the ZoneInfo (ZI) from its TZDB identifier
 * (e.g. "America/Los_Angeles"), zoneId (hash from its name), or the index in
 * the zone registry.
 *
 * @tparam ZI ZoneInfo type (e.g. basic::ZoneInfo)
 * @tparam ZIB ZoneInfoBroker type (e.g. basic::ZoneInfoBroker)
 * @tparam ZRGB ZoneRegistryBroker type (e.g. basic::ZoneRegistryBroker)
 */
template<typename ZI, typename ZIB, typename ZRGB>
class ZoneRegistrarTemplate {
  public:
    /** Invalid index to indicate error or not found. */
    static const uint16_t kInvalidIndex = 0xffff;

    /** Constructor. */
    ZoneRegistrarTemplate(
        uint16_t zoneRegistrySize,
        const ZI* const* zoneRegistry
    ):
        mZoneRegistrySize(zoneRegistrySize),
        mIsSorted(isSorted(zoneRegistry, zoneRegistrySize)),
        mZoneRegistry(zoneRegistry)
    {}

    /** Return the number of zones and (fat) links. */
    uint16_t zoneRegistrySize() const { return mZoneRegistrySize; }

    /** Return the ZoneInfo at index i. Return nullptr if i is out of range. */
    const ZI* getZoneInfoForIndex(uint16_t i) const {
      return (i < mZoneRegistrySize)
          ? ZRGB(mZoneRegistry).zoneInfo(i)
          : nullptr;
    }

    /**
     * Return the ZoneInfo corresponding to the given zone name. Return nullptr
     * if not found.
     */
    const ZI* getZoneInfoForName(const char* name) const {
      uint16_t index = findIndexForName(name);
      if (index == kInvalidIndex) return nullptr;
      return ZRGB(mZoneRegistry).zoneInfo(index);
    }

    /** Return the ZoneInfo using the zoneId. Return nullptr if not found. */
    const ZI* getZoneInfoForId(uint32_t zoneId) const {
      uint16_t index = findIndexForId(zoneId);
      if (index == kInvalidIndex) return nullptr;
      return ZRGB(mZoneRegistry).zoneInfo(index);
    }

    /** Find the index for zone name. Return kInvalidIndex if not found. */
    uint16_t findIndexForName(const char* name) const {
      uint32_t zoneId = ace_common::hashDjb2(name);
      uint16_t index = findIndexForId(zoneId);
      if (index == kInvalidIndex) return kInvalidIndex;

      // Verify that the zoneName actually matches, in case of hash collision.
      ZIB zoneInfoBroker(ZRGB(mZoneRegistry).zoneInfo(index));
      ace_common::KString kname(
        zoneInfoBroker.name(),
        zoneInfoBroker.zoneContext().fragments(),
        zoneInfoBroker.zoneContext().numFragments()
      );
      return (kname.compareTo(name) == 0) ? index : kInvalidIndex;
    }

    /** Find the index for zone id. Return kInvalidIndex if not found. */
    uint16_t findIndexForId(uint32_t zoneId) const {
      if (mIsSorted && mZoneRegistrySize >= kBinarySearchThreshold) {
        return binarySearchById(mZoneRegistry, mZoneRegistrySize, zoneId);
      } else {
        return linearSearchById(mZoneRegistry, mZoneRegistrySize, zoneId);
      }
    }

  protected:
    friend void ::runIndexForZoneIdBinary();
    friend void ::runIndexForZoneIdLinear();
    friend class ::ZoneRegistrarTest_Sorted_isSorted;
    friend class ::ZoneRegistrarTest_Unsorted_isSorted;
    friend class ::ZoneRegistrarTest_Sorted_linearSearchById;
    friend class ::ZoneRegistrarTest_Sorted_linearSearchById_not_found;
    friend class ::ZoneRegistrarTest_Sorted_binarySearchById_zeroEntries;
    friend class ::ZoneRegistrarTest_Sorted_binarySearchById;
    friend class ::ZoneRegistrarTest_Sorted_binarySearchById_not_found;
    friend class ::ZoneRegistrarTest_Unsorted_linearSearchById;
    friend class ::ZoneRegistrarTest_Unsorted_linearSearchById_not_found;

    /** Use binarySearchById() if zoneRegistrySize >= threshold. */
    static const uint8_t kBinarySearchThreshold = 8;

    /** Determine if the given zone registry is sorted by id. */
    static bool isSorted(const ZI* const* registry, uint16_t registrySize) {
      const ZRGB zoneRegistry(registry);
      return ace_common::isSortedByKey(
          (size_t) registrySize,
          [&zoneRegistry](size_t i) {
            const ZI* zoneInfo = zoneRegistry.zoneInfo(i);
            return ZIB(zoneInfo).zoneId();
          } // lambda expression returns zoneId at index i
      );
    }

    /**
     * Find the registry index corresponding to zoneId using linear search.
     * Returns kInvalidIndex if not found.
     */
    static uint16_t linearSearchById(const ZI* const* registry,
        uint16_t registrySize, uint32_t zoneId) {
      const ZRGB zoneRegistry(registry);
      for (uint16_t i = 0; i < registrySize; ++i) {
        const ZI* zoneInfo = zoneRegistry.zoneInfo(i);
        if (zoneId == ZIB(zoneInfo).zoneId()) {
          return i;
        }
      }
      return kInvalidIndex;

      // The templatized version is 20-40% slower on some compilers (but not
      // all), so let's use the hand-rolled version above.
      /*
      return (uint16_t) ace_common::linearSearchByKey(
          (size_t) registrySize,
          zoneId,
          [&zoneRegistry](size_t i) {
            const ZI* zoneInfo = zoneRegistry.zoneInfo(i);
            return ZIB(zoneInfo).zoneId();
          } // lambda expression returns zoneId at index i
      );
      */
    }

    /**
     * Find the registry index corresponding to zoneId using a binary search.
     * Returns kInvalidIndex if not found.
     *
     * The largest registrySize is UINT16_MAX so the largest valid index is
     * UINT16_MAX - 1. This allows us to set kInvalidIndex to UINT16_MAX to
     * indicate "Not Found".
     */
    static uint16_t binarySearchById(const ZI* const* registry,
        uint16_t registrySize, uint32_t zoneId) {
      const ZRGB zoneRegistry(registry);
      return (uint16_t) ace_common::binarySearchByKey(
          (size_t) registrySize,
          zoneId,
          [&zoneRegistry](size_t i) -> uint32_t {
            const ZI* zoneInfo = zoneRegistry.zoneInfo(i);
            return ZIB(zoneInfo).zoneId();
          } // lambda expression returns zoneId at index i
      );
    }

    /** Exposed only for benchmarking purposes. */
    uint16_t findIndexForIdLinear(uint32_t zoneId) const {
      return linearSearchById(mZoneRegistry, mZoneRegistrySize, zoneId);
    }

    /** Exposed only for benchmarking purposes. */
    uint16_t findIndexForIdBinary(uint32_t zoneId) const {
      return binarySearchById(mZoneRegistry, mZoneRegistrySize, zoneId);
    }

  private:
    // Ordering of fields optimized for 32-bit alignment.
    uint16_t const mZoneRegistrySize;
    bool const mIsSorted;
    const ZI* const* const mZoneRegistry; // not nullable
};

} // internal

namespace basic {

/**
 * Concrete template instantiation of ZoneRegistrarTemplate for
 * basic::ZoneInfo, which can be used with BasicZoneProcessor.
 */
using ZoneRegistrar = internal::ZoneRegistrarTemplate<
    basic::ZoneInfo,
    basic::ZoneInfoBroker,
    basic::ZoneRegistryBroker
>;

} // basic

namespace extended {

/**
 * Concrete template instantiation of ZoneRegistrarTemplate for
 * extended::ZoneInfo, which can be used with ExtendedZoneProcessor.
 */
using ZoneRegistrar = internal::ZoneRegistrarTemplate<
    extended::ZoneInfo,
    extended::ZoneInfoBroker,
    extended::ZoneRegistryBroker
>;

} // extended

namespace complete {

/**
 * Concrete template instantiation of ZoneRegistrarTemplate for
 * extended::ZoneInfo, which can be used with ExtendedZoneProcessor.
 */
using ZoneRegistrar = internal::ZoneRegistrarTemplate<
    complete::ZoneInfo,
    complete::ZoneInfoBroker,
    complete::ZoneRegistryBroker
>;

} // complete

} // ace_time

#endif // ACE_TIME_ZONE_REGISTRAR_H
