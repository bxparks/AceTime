/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#ifndef ACE_TIME_ZONE_MANAGER_H
#define ACE_TIME_ZONE_MANAGER_H

#include <stdint.h>
#include "common/common.h"

namespace ace_time {

/**
 * Look up the ZoneInfo (ZI) from its TZDB identifier (e.g.
 * "America/Los_Angeles")
 */
template<typename ZI>
class ZoneManager {
  public:
    /** Constructor. */
    ZoneManager(const ZI* const* zoneRegistry, uint16_t numZoneInfos):
        mZoneRegistry(zoneRegistry),
        mNumZoneInfos(numZoneInfos) {}

    /**
     * Return the ZoneSpecifier corresponding to the given zone name.
     * Return nullptr if not found, or if we have no more ZoneSpecifier
     * internal buffer.
     */
    const ZI* getZoneInfo(const char* name) const {
      return findZoneInfo(name);
    }

  private:
    // TODO: Implement a binary search if the zoneRegistry is sorted.
    const ZI* findZoneInfo(const char* name) const {
      for (uint16_t i = 0; i < mNumZoneInfos; ++i) {
        const ZI* zoneInfo = mZoneRegistry[i];
        if (strcmp(zoneInfo->name, name) == 0) {
          return zoneInfo;
        }
      }
      return nullptr;
    }

    const ZI* const* const mZoneRegistry;
    uint16_t const mNumZoneInfos;
};

}

#endif
