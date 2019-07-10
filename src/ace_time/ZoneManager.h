/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#ifndef ACE_TIME_ZONE_MANAGER_H
#define ACE_TIME_ZONE_MANAGER_H

#include <stdint.h>
#include <string.h> // strcmp(), strcmp_P()
#include "common/flash.h"

class BasicZoneManagerTest_Sorted_isSorted;
class BasicZoneManagerTest_Sorted_linearSearch;
class BasicZoneManagerTest_Sorted_linearSearch_not_found;
class BasicZoneManagerTest_Sorted_binarySearch;
class BasicZoneManagerTest_Sorted_binarySearch_not_found;
class BasicZoneManagerTest_Unsorted_isSorted;
class BasicZoneManagerTest_Unsorted_linearSearch;

namespace ace_time {

/** Typedef for functions that work like a strcmp(). */
typedef int (*strcmp_t)(const char*, const char*);

/**
 * Look up the ZoneInfo (ZI) from its TZDB identifier (e.g.
 * "America/Los_Angeles").
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
class ZoneManager {
  public:
    /** Constructor. */
    ZoneManager(const ZI* const* zoneRegistry, uint16_t registrySize):
        mZoneRegistry(zoneRegistry),
        mRegistrySize(registrySize),
        mIsSorted(isSorted(zoneRegistry, registrySize)) {}

    /**
     * Return the ZoneSpecifier corresponding to the given zone name. Return
     * nullptr if not found, or if we have no more ZoneSpecifier internal
     * buffer.
     */
    const ZI* getZoneInfo(const char* name) const {
      if (mIsSorted && mRegistrySize > kBinarySearchThreshold) {
        return binarySearch(mZoneRegistry, mRegistrySize, name);
      } else {
        return linearSearch(mZoneRegistry, mRegistrySize, name);
      }
    }

    /**
     * Return true if zoneRegistry is sorted, and eligible to use a binary
     * search.
     */
    bool isSorted() const { return mIsSorted; }

  protected:
    friend class ::BasicZoneManagerTest_Sorted_isSorted;
    friend class ::BasicZoneManagerTest_Sorted_linearSearch;
    friend class ::BasicZoneManagerTest_Sorted_linearSearch_not_found;
    friend class ::BasicZoneManagerTest_Sorted_binarySearch;
    friend class ::BasicZoneManagerTest_Sorted_binarySearch_not_found;
    friend class ::BasicZoneManagerTest_Unsorted_isSorted;
    friend class ::BasicZoneManagerTest_Unsorted_linearSearch;

    /** Use binarySearch() if registrySize > threshold. */
    static const uint8_t kBinarySearchThreshold = 5;

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

    const ZI* const* mZoneRegistry;
    uint16_t const mRegistrySize;
    bool const mIsSorted;
};

/**
 * Concrete template instantiation of ZoneManager for basic::ZoneInfo, which
 * can be used with BasicZoneSpecifier.
 */
#if ACE_TIME_USE_BASIC_PROGMEM
typedef ZoneManager<basic::ZoneInfo, basic::ZoneRegistryBroker,
    basic::ZoneInfoBroker, acetime_strcmp_P, acetime_strcmp_PP>
    BasicZoneManager;
#else
typedef ZoneManager<basic::ZoneInfo, basic::ZoneRegistryBroker,
    basic::ZoneInfoBroker, strcmp, strcmp> BasicZoneManager;
#endif

/**
 * Concrete template instantiation of ZoneManager for extended::ZoneInfo, which
 * can be used with ExtendedZoneSpecifier.
 */
#if ACE_TIME_USE_EXTENDED_PROGMEM
typedef ZoneManager<extended::ZoneInfo, extended::ZoneRegistryBroker,
    extended::ZoneInfoBroker, acetime_strcmp_P, acetime_strcmp_PP>
    ExtendedZoneManager;
#else
typedef ZoneManager<extended::ZoneInfo, extended::ZoneRegistryBroker,
    extended::ZoneInfoBroker, strcmp, strcmp> ExtendedZoneManager;
#endif

}

#endif
