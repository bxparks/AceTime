/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#ifndef ACE_TIME_EXTENDED_ZONE_H
#define ACE_TIME_EXTENDED_ZONE_H

#include "internal/ZoneInfo.h"
#include "internal/ExtendedBrokers.h"

class Print;

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

    void printNameTo(Print& printer) const;
    void printShortNameTo(Print& printer) const;

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
