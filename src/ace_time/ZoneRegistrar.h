/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#ifndef ACE_TIME_ZONE_REGISTRAR_H
#define ACE_TIME_ZONE_REGISTRAR_H

#include <stdint.h>
#include <string.h> // strcmp(), strcmp_P()
#include <AceCommon.h> // strcmp_PP()
#include "common/compat.h"
#include "internal/ZoneInfo.h"
#include "internal/Brokers.h"

class BasicZoneRegistrarTest_Sorted_isSorted;
class BasicZoneRegistrarTest_Sorted_linearSearchByName;
class BasicZoneRegistrarTest_Sorted_linearSearchByName_not_found;
class BasicZoneRegistrarTest_Sorted_binarySearchByName;
class BasicZoneRegistrarTest_Sorted_binarySearchByName_not_found;
class BasicZoneRegistrarTest_Sorted_linearSearchById;
class BasicZoneRegistrarTest_Sorted_linearSearchById_not_found;
class BasicZoneRegistrarTest_Unsorted_isSorted;
class BasicZoneRegistrarTest_Unsorted_linearSearchByName;
class BasicZoneRegistrarTest_Unsorted_linearSearchByName_not_found;
class BasicZoneRegistrarTest_Unsorted_binarySearchByName;
class BasicZoneRegistrarTest_Unsorted_binarySearchByName_not_found;
class BasicZoneRegistrarTest_Unsorted_linearSearchById;
class BasicZoneRegistrarTest_Unsorted_linearSearchById_not_found;

namespace ace_time {

/** Typedef for functions that work like a strcmp(). */
typedef int (*strcmp_t)(const char*, const char*);

/**
 * Class that allows looking up the ZoneInfo (ZI) from its TZDB identifier
 * (e.g. "America/Los_Angeles"), zoneId (hash from its name), or the index in
 * the zone registry.
 *
 * @tparam ZI ZoneInfo type (e.g. basic::ZoneInfo)
 * @tparam ZRB ZoneRegistryBroker type (e.g. basic::ZoneRegistryBroker)
 * @tparam ZIB ZoneInfoBroker type (e.g. basic::ZoneInfoBroker)
 * @tparam STRCMP_P a function that compares a normal string to flash string
 * (e.g strcmp_P())
 * @tparam STRCMP_PP a function that compares 2 flash strings (must be custom
 *    written)
 */
template<typename ZI, typename ZRB, typename ZIB, strcmp_t STRCMP_P,
    strcmp_t STRCMP_PP>
class ZoneRegistrar {
  public:
    /** Invalid index to indicate error or not found. */
    static const uint16_t kInvalidIndex = 0xffff;

    /** Constructor. */
    ZoneRegistrar(uint16_t registrySize, const ZI* const* zoneRegistry):
        mRegistrySize(registrySize),
        mZoneRegistry(zoneRegistry),
        mIsSorted(isSorted(zoneRegistry, registrySize)) {}

    /** Return the number of zones. */
    uint16_t registrySize() const { return mRegistrySize; }

    /**
     * Return true if zoneRegistry is sorted, and eligible to use a binary
     * search.
     */
    bool isSorted() const { return mIsSorted; }

    /** Return the ZoneInfo at index i. Return nullptr if i is out of range. */
    const ZI* getZoneInfoForIndex(uint16_t i) const {
      return (i < mRegistrySize) ? ZRB(mZoneRegistry).zoneInfo(i) : nullptr;
    }

    /**
     * Return the ZoneInfo corresponding to the given zone name. Return nullptr
     * if not found.
     */
    const ZI* getZoneInfoForName(const char* name) const {
      uint16_t index = findIndexForName(name);
      if (index == kInvalidIndex) return nullptr;
      return ZRB(mZoneRegistry).zoneInfo(index);
    }

    /** Return the ZoneInfo using the zoneId. Return nullptr if not found. */
    const ZI* getZoneInfoForId(uint32_t zoneId) const {
      uint16_t index = findIndexForId(zoneId);
      if (index == kInvalidIndex) return nullptr;
      return ZRB(mZoneRegistry).zoneInfo(index);
    }

    /** Find the index for zone name. Return kInvalidIndex if not found. */
    uint16_t findIndexForName(const char* name) const {
      if (mIsSorted && mRegistrySize >= kBinarySearchThreshold) {
        return binarySearchByName(mZoneRegistry, mRegistrySize, name);
      } else {
        return linearSearchByName(mZoneRegistry, mRegistrySize, name);
      }
    }

    /** Find the index for zone id. Return kInvalidIndex if not found. */
    uint16_t findIndexForId(uint32_t zoneId) const {
      return linearSearchById(mZoneRegistry, mRegistrySize, zoneId);
    }

  protected:
    friend class ::BasicZoneRegistrarTest_Sorted_isSorted;
    friend class ::BasicZoneRegistrarTest_Sorted_linearSearchByName;
    friend class ::BasicZoneRegistrarTest_Sorted_linearSearchByName_not_found;
    friend class ::BasicZoneRegistrarTest_Sorted_binarySearchByName;
    friend class ::BasicZoneRegistrarTest_Sorted_binarySearchByName_not_found;
    friend class ::BasicZoneRegistrarTest_Sorted_linearSearchById;
    friend class ::BasicZoneRegistrarTest_Sorted_linearSearchById_not_found;
    friend class ::BasicZoneRegistrarTest_Unsorted_isSorted;
    friend class ::BasicZoneRegistrarTest_Unsorted_linearSearchByName;
    friend class ::BasicZoneRegistrarTest_Unsorted_linearSearchByName_not_found;
    friend class ::BasicZoneRegistrarTest_Unsorted_binarySearchByName;
    friend class ::BasicZoneRegistrarTest_Unsorted_binarySearchByName_not_found;
    friend class ::BasicZoneRegistrarTest_Unsorted_linearSearchById;
    friend class ::BasicZoneRegistrarTest_Unsorted_linearSearchById_not_found;

    /** Use binarySearch() if registrySize >= threshold. */
    static const uint8_t kBinarySearchThreshold = 6;

    /** Determine if the given zone registry is sorted by name. */
    static bool isSorted(const ZI* const* registry, uint16_t registrySize) {
      if (registrySize == 0) {
        return false;
      }

      const ZRB zoneRegistry(registry);
      const char* prevName = ZIB(zoneRegistry.zoneInfo(0)).name();
      for (uint16_t i = 1; i < registrySize; ++i) {
        const char* currName = ZIB(zoneRegistry.zoneInfo(i)).name();
        if (STRCMP_PP(prevName, currName) > 0) {
          return false;
        }
        prevName = currName;
      }
      return true;
    }

    /**
     * Find the registry index corresponding to name using a linear search.
     * Returns kInvalidIndex if not found.
     */
    static uint16_t linearSearchByName(const ZI* const* registry,
        uint16_t registrySize, const char* name) {
      const ZRB zoneRegistry(registry);
      for (uint16_t i = 0; i < registrySize; ++i) {
        const ZI* zoneInfo = zoneRegistry.zoneInfo(i);
        if (STRCMP_P(name, ZIB(zoneInfo).name()) == 0) {
          return i;
        }
      }
      return kInvalidIndex;
    }

    /**
     * Find the registry index corresponding to name using a binary search.
     * Returns kInvalidIndex if not found. The largest registrySize is
     * UINT16_MAX (so that the highest index is UINT16_MAX - 1), since
     * UINT16_MAX is equal to kInvalidIndex which is used to indicate "Not
     * Found".
     *
     * See also https://www.solipsys.co.uk/new/BinarySearchReconsidered.html
     * which shows how hard it is to write a binary search correctly. I got it
     * wrong in my first iteration.
     */
    static uint16_t binarySearchByName(const ZI* const* registry,
        uint16_t registrySize, const char* name) {
      uint16_t a = 0;
      uint16_t b = registrySize - 1;
      const ZRB zoneRegistry(registry);
      while (a <= b) {
        uint16_t c = a + (b - a) / 2;
        const ZI* zoneInfo = zoneRegistry.zoneInfo(c);
        int8_t compare = STRCMP_P(name, ZIB(zoneInfo).name());
        if (compare == 0) return c;
        if (compare < 0) {
          b = c - 1;
        } else {
          a = c + 1;
        }
      }
      return kInvalidIndex;
    }

    /** Find the registry index corresponding to id using linear search. */
    static uint16_t linearSearchById(const ZI* const* registry,
        uint16_t registrySize, uint32_t zoneId) {
      const ZRB zoneRegistry(registry);
      for (uint16_t i = 0; i < registrySize; ++i) {
        const ZI* zoneInfo = zoneRegistry.zoneInfo(i);
        if (zoneId == ZIB(zoneInfo).zoneId()) {
          return i;
        }
      }
      return kInvalidIndex;
    }

    uint16_t const mRegistrySize;
    const ZI* const* const mZoneRegistry;
    bool const mIsSorted;
};

/**
 * Concrete template instantiation of ZoneRegistrar for basic::ZoneInfo, which
 * can be used with BasicZoneProcessor.
 */
#if ACE_TIME_USE_PROGMEM
typedef ZoneRegistrar<basic::ZoneInfo, basic::ZoneRegistryBroker,
    basic::ZoneInfoBroker, acetime_strcmp_P, ace_common::strcmp_PP>
    BasicZoneRegistrar;
#else
typedef ZoneRegistrar<basic::ZoneInfo, basic::ZoneRegistryBroker,
    basic::ZoneInfoBroker, strcmp, strcmp> BasicZoneRegistrar;
#endif

/**
 * Concrete template instantiation of ZoneRegistrar for extended::ZoneInfo,
 * which can be used with ExtendedZoneProcessor.
 */
#if ACE_TIME_USE_PROGMEM
typedef ZoneRegistrar<extended::ZoneInfo, extended::ZoneRegistryBroker,
    extended::ZoneInfoBroker, acetime_strcmp_P, ace_common::strcmp_PP>
    ExtendedZoneRegistrar;
#else
typedef ZoneRegistrar<extended::ZoneInfo, extended::ZoneRegistryBroker,
    extended::ZoneInfoBroker, strcmp, strcmp> ExtendedZoneRegistrar;
#endif

}

#endif
