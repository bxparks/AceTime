/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#ifndef ACE_TIME_EXTENDED_ZONE_H
#define ACE_TIME_EXTENDED_ZONE_H

#include "internal/ZoneInfo.h"
#include "internal/Brokers.h"
#include "common/compat.h"

class __FlashStringHelper;

namespace ace_time {

/**
 * A thin wrapper around an extended::ZoneInfo data structure to provide a
 * stable API access to some useful extended::ZoneInfo data. The
 * extended::ZoneInfo data struct is intended to be an opaque type to the users
 * of this library.
 */
class ExtendedZone {
  public:
    ExtendedZone(const extended::ZoneInfo* zoneInfo):
        mZoneInfoBroker(zoneInfo) {}

// TODO: Merge this with BasicZone.h now that they both use the same
// ACE_TIME_USE_PROGMEM macro.
#if ACE_TIME_USE_PROGMEM
    const __FlashStringHelper* name() const {
      return (const __FlashStringHelper*) mZoneInfoBroker.name();
    }

    const __FlashStringHelper* shortName() const {
      const char* name = mZoneInfoBroker.name();
      const char* slash = strrchr_P(name, '/');
      return (slash) ? (const __FlashStringHelper*) (slash + 1)
          : (const __FlashStringHelper*) name;
    }
#else
    const char* name() const {
      return mZoneInfoBroker.name();
    }

    const char* shortName() const {
      const char* name = mZoneInfoBroker.name();
      const char* slash = strrchr(name, '/');
      return (slash) ? (slash + 1) : name;
    }
#endif

    uint32_t zoneId() const {
      return mZoneInfoBroker.zoneId();
    }

  private:
    // disable copy constructor and assignment operator
    ExtendedZone(const ExtendedZone&) = delete;
    ExtendedZone& operator=(const ExtendedZone&) = delete;

    const extended::ZoneInfoBroker mZoneInfoBroker;
};

}

#endif
