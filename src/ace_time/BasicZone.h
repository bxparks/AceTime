/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#ifndef ACE_TIME_BASIC_ZONE_H
#define ACE_TIME_BASIC_ZONE_H

#include "internal/ZoneInfo.h"
#include "internal/Brokers.h"
#include "common/flash.h"

class __FlashStringHelper;

namespace ace_time {

/**
 * A thin wrapper around a basic::ZoneInfo data structure to provide a stable
 * API access to some useful basic::ZoneInfo data. The basic::ZoneInfo data
 * struct is intended to be an opaque type to the users of this library.
 */
class BasicZone {
  public:
    BasicZone(const basic::ZoneInfo* zoneInfo):
        mZoneInfoBroker(zoneInfo) {}

    // use default copy constructor and assignment operator
    BasicZone(const BasicZone&) = default;
    BasicZone& operator=(const BasicZone&) = default;

// TODO: Merge this with ExtendedZone.h now that they both use the same
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
      return (const char*) mZoneInfoBroker.name();
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
    const basic::ZoneInfoBroker mZoneInfoBroker;
};

}

#endif
