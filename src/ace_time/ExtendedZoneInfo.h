/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#ifndef ACE_TIME_EXTENDED_ZONE_INFO_H
#define ACE_TIME_EXTENDED_ZONE_INFO_H

#include "internal/ZoneInfo.h"
#include "internal/Brokers.h"

namespace ace_time {

/**
 * A thin wrapper around an extended::ZoneInfo data structure to provide a
 * stable API access to some useful extended::ZoneInfo data. The
 * extended::ZoneInfo data struct is intended to be an opaque type to the users
 * of this library, and can be changed at any time without warning.
 */
class ExtendedZoneInfo {
  public:
    ExtendedZoneInfo(const extended::ZoneInfo* zoneInfo):
        mZoneInfo(zoneInfo) {}

#if ACE_TIME_USE_EXTENDED_PROGMEM
    const __FlashStringHelper* name() const {
      return (const __FlashStringHelper*) mZoneInfo.name();
    }
#else
    const char* name() const {
      return mZoneInfo.name();
    }
#endif

  private:
    // diable copy constructor and assignment operator.
    ExtendedZoneInfo(const ExtendedZoneInfo&) = delete;
    ExtendedZoneInfo& operator=(const ExtendedZoneInfo&) = delete;

    const extended::ZoneInfoBroker mZoneInfo;
};

}

#endif
