/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#ifndef ACE_TIME_BASIC_ZONE_H
#define ACE_TIME_BASIC_ZONE_H

#include "internal/ZoneInfo.h"
#include "internal/BasicBrokers.h"

class Print;

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

    void printNameTo(Print& printer) const;
    void printShortNameTo(Print& printer) const;

    uint32_t zoneId() const {
      return mZoneInfoBroker.zoneId();
    }

  private:
    // disable default copy constructor and assignment operator
    BasicZone(const BasicZone&) = delete;
    BasicZone& operator=(const BasicZone&) = delete;

    const basic::ZoneInfoBroker mZoneInfoBroker;
};

}

#endif
