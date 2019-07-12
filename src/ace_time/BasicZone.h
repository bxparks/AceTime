/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#ifndef ACE_TIME_BASIC_ZONE_H
#define ACE_TIME_BASIC_ZONE_H

#include "internal/ZoneInfo.h"
#include "internal/Brokers.h"

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

    // use default copy constructor and assignment operator.
    BasicZone(const BasicZone&) = delete;
    BasicZone& operator=(const BasicZone&) = delete;

#if ACE_TIME_USE_BASIC_PROGMEM
    const __FlashStringHelper* name() const {
      return (const __FlashStringHelper*) mZoneInfoBroker.name();
    }
#else
    const char* name() const {
      return (const char*) mZoneInfoBroker.name();
    }
#endif

  private:
    const basic::ZoneInfoBroker mZoneInfoBroker;
};

}

#endif
