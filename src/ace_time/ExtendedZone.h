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
    /**
     * Constructor from a raw extended::ZoneInfo* pointer, intended for manual
     * inspection of a ZoneInfo record.
     */
    ExtendedZone(const extended::ZoneInfo* zoneInfo):
        mZoneInfoBroker(zoneInfo) {}

    /**
     * Constructor from an extended::ZoneInfoBroker, used by
     * ExtendedZoneProcessor. This allows the implementation details of
     * ZoneInfoBroker to remain hidden.
     */
    ExtendedZone(const extended::ZoneInfoBroker& zoneInfo):
        mZoneInfoBroker(zoneInfo) {}

    /** Print the full zone name to printer. */
    void printNameTo(Print& printer) const;

    /** Print the short zone to the printer. */
    void printShortNameTo(Print& printer) const;

    /** Return the zoneId of the current zoneInfo. */
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
