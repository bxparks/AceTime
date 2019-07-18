/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#ifndef ACE_TIME_ZONE_REGISTRAR_H
#define ACE_TIME_ZONE_REGISTRAR_H

#include <stdint.h>
#include <string.h> // strcmp(), strcmp_P()
#include "common/flash.h"
#include "internal/ZoneInfo.h"
#include "internal/Brokers.h"

class BasicZoneRegistrarTest_Sorted_isSorted;
class BasicZoneRegistrarTest_Sorted_linearSearch;
class BasicZoneRegistrarTest_Sorted_linearSearch_not_found;
class BasicZoneRegistrarTest_Sorted_binarySearch;
class BasicZoneRegistrarTest_Sorted_binarySearch_not_found;
class BasicZoneRegistrarTest_Unsorted_isSorted;
class BasicZoneRegistrarTest_Unsorted_linearSearch;

namespace ace_time {

/** Typedef for functions that work like a strcmp(). */
typedef int (*strcmp_t)(const char*, const char*);

/**
 * Class that allows looking up the ZoneInfo (ZI) from its TZDB identifier
 * (e.g. "America/Los_Angeles"), or index, or zoneId (hash from its name).
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

    /* Return the ZoneInfo at index i. Return nullptr if i is out of range. */
    const ZI* getZoneInfo(uint16_t i) const {
      return (i < mRegistrySize) ? ZRB(mZoneRegistry).zoneInfo(i) : nullptr;
    }

    /**
     * Return the ZoneInfo corresponding to the given zone name. Return nullptr
     * if not found.
     */
    const ZI* getZoneInfoForName(const char* name) const {
      if (mIsSorted && mRegistrySize >= kBinarySearchThreshold) {
        return binarySearch(mZoneRegistry, mRegistrySize, name);
      } else {
        return linearSearch(mZoneRegistry, mRegistrySize, name);
      }
    }

    /* Return the ZoneInfo using the zoneId. Return nullptr if not found. */
    const ZI* getZoneInfoForId(uint32_t zoneId) const {
        return linearSearchUsingId(mZoneRegistry, mRegistrySize, zoneId);
    }

  protected:
    friend class ::BasicZoneRegistrarTest_Sorted_isSorted;
    friend class ::BasicZoneRegistrarTest_Sorted_linearSearch;
    friend class ::BasicZoneRegistrarTest_Sorted_linearSearch_not_found;
    friend class ::BasicZoneRegistrarTest_Sorted_binarySearch;
    friend class ::BasicZoneRegistrarTest_Sorted_binarySearch_not_found;
    friend class ::BasicZoneRegistrarTest_Unsorted_isSorted;
    friend class ::BasicZoneRegistrarTest_Unsorted_linearSearch;

    /** Use binarySearch() if registrySize >= threshold. */
    static const uint8_t kBinarySearchThreshold = 6;

    static bool isSorted(const ZI* const* zr, uint16_t registrySize) {
      if (registrySize == 0) {
        return false;
      }

      const ZRB zoneRegistry(zr);
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

    static const ZI* linearSearch(const ZI* const* zr,
        uint16_t registrySize, const char* name) {
      const ZRB zoneRegistry(zr);
      for (uint16_t i = 0; i < registrySize; ++i) {
        const ZI* zoneInfo = zoneRegistry.zoneInfo(i);
        if (STRCMP_P(name, ZIB(zoneInfo).name()) == 0) {
          return zoneInfo;
        }
      }
      return nullptr;
    }

    static const ZI* binarySearch(const ZI* const* zr,
        uint16_t registrySize, const char* name) {
      uint16_t a = 0;
      uint16_t b = registrySize - 1;
      const ZRB zoneRegistry(zr);
      while (true) {
        uint16_t c = (a + b) / 2;
        const ZI* zoneInfo = zoneRegistry.zoneInfo(c);
        int8_t compare = STRCMP_P(name, ZIB(zoneInfo).name());
        if (compare == 0) return zoneInfo;
        if (a == b) return nullptr;
        if (compare < 0) {
          b = c - 1;
        } else {
          a = c + 1;
        }
      }
    }

    static const ZI* linearSearchUsingId(const ZI* const* zr,
        uint16_t registrySize, uint32_t zoneId) {
      const ZRB zoneRegistry(zr);
      for (uint16_t i = 0; i < registrySize; ++i) {
        const ZI* zoneInfo = zoneRegistry.zoneInfo(i);
        if (zoneId == ZIB(zoneInfo).zoneId()) {
          return zoneInfo;
        }
      }
      return nullptr;
    }

    uint16_t const mRegistrySize;
    const ZI* const* const mZoneRegistry;
    bool const mIsSorted;
};

/**
 * Concrete template instantiation of ZoneRegistrar for basic::ZoneInfo, which
 * can be used with BasicZoneSpecifier.
 */
#if ACE_TIME_USE_BASIC_PROGMEM
typedef ZoneRegistrar<basic::ZoneInfo, basic::ZoneRegistryBroker,
    basic::ZoneInfoBroker, acetime_strcmp_P, acetime_strcmp_PP>
    BasicZoneRegistrar;
#else
typedef ZoneRegistrar<basic::ZoneInfo, basic::ZoneRegistryBroker,
    basic::ZoneInfoBroker, strcmp, strcmp> BasicZoneRegistrar;
#endif

/**
 * Concrete template instantiation of ZoneRegistrar for extended::ZoneInfo, which
 * can be used with ExtendedZoneSpecifier.
 */
#if ACE_TIME_USE_EXTENDED_PROGMEM
typedef ZoneRegistrar<extended::ZoneInfo, extended::ZoneRegistryBroker,
    extended::ZoneInfoBroker, acetime_strcmp_P, acetime_strcmp_PP>
    ExtendedZoneRegistrar;
#else
typedef ZoneRegistrar<extended::ZoneInfo, extended::ZoneRegistryBroker,
    extended::ZoneInfoBroker, strcmp, strcmp> ExtendedZoneRegistrar;
#endif

}

#endif
