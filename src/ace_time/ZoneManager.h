/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#ifndef ACE_TIME_ZONE_MANAGER_H
#define ACE_TIME_ZONE_MANAGER_H

#include <stdint.h>
#include "common/common.h"

class ZoneManagerTest_Basic_isSorted;
class ZoneManagerTest_Basic_linearSearch;
class ZoneManagerTest_Basic_binarySearch;

namespace ace_time {

/**
 * Look up the ZoneInfo (ZI) from its TZDB identifier (e.g.
 * "America/Los_Angeles")
 */
template<typename ZI>
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

  private:
    friend class ::ZoneManagerTest_Basic_isSorted;
    friend class ::ZoneManagerTest_Basic_linearSearch;
    friend class ::ZoneManagerTest_Basic_binarySearch;

    /** Use binarySearch() if registrySize > threshold. */
    static const uint8_t kBinarySearchThreshold = 5;

    static bool isSorted(const ZI* const* zoneRegistry, uint16_t registrySize) {
      if (registrySize == 0) {
        return false;
      }
      const char* prevName = zoneRegistry[0]->name;
      for (uint16_t i = 1; i < registrySize; ++i) {
        const char* currName = zoneRegistry[i]->name;
        if (strcmp(prevName, currName) > 0) {
          return false;
        }
        prevName = currName;
      }
      return true;
    }

    static const ZI* linearSearch(const ZI* const* zoneRegistry, uint16_t registrySize,
        const char* name) {
      for (uint16_t i = 0; i < registrySize; ++i) {
        const ZI* zoneInfo = zoneRegistry[i];
        if (strcmp(name, zoneInfo->name) == 0) {
          return zoneInfo;
        }
      }
      return nullptr;
    }

    static const ZI* binarySearch(const ZI* const* zoneRegistry, uint16_t registrySize,
        const char* name) {
      uint16_t a = 0;
      uint16_t b = registrySize - 1;
      while (b - a > 0) {
        uint16_t i = (a + b) / 2;
        const ZI* zi = zoneRegistry[i];
        int8_t compare = strcmp(name, zi->name);
        if (compare < 0) {
          b = i - 1;
        } else if (compare > 0) {
          a = i + 1;
        } else {
          return zi;
        }
      }

      const ZI* zi = zoneRegistry[a];
      if (strcmp(name, zi->name) == 0) {
        return zi;
      } else {
        return nullptr;
      }
    }

    const ZI* const* const mZoneRegistry;
    uint16_t const mRegistrySize;
    bool const mIsSorted;
};

}

#endif
