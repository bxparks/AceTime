/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#ifndef ACE_TIME_BASIC_ZONE_INFO_H
#define ACE_TIME_BASIC_ZONE_INFO_H

#include "common/ZoneInfo.h"
#include "common/Brokers.h"

namespace ace_time {

/**
 * A thin wrapper around a basic::ZoneInfo data structure to provide a stable
 * API access to some useful basic::ZoneInfo data. The basic::ZoneInfo data
 * struct is intended to be an opaque type to the users of this library, and
 * can be changed at any time without warning.
 */
class BasicZoneInfo {
  public:
    BasicZoneInfo(const basic::ZoneInfo* zoneInfo):
        mZoneInfo(zoneInfo) {}

#if ACE_TIME_USE_BASIC_PROGMEM
    const __FlashStringHelper* name() const {
      return (const __FlashStringHelper*) mZoneInfo.name();
    }
#else
    const char* name() const {
      return (const char*) mZoneInfo.name();
    }
#endif

  private:
    // diable copy constructor and assignment operator.
    BasicZoneInfo(const BasicZoneInfo&) = delete;
    BasicZoneInfo& operator=(const BasicZoneInfo&) = delete;

    const basic::ZoneInfoBroker mZoneInfo;
};

}

#endif
