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
    /**
     * Constructor from a raw basic::ZoneInfo* pointer, intended for manual
     * inspection of a ZoneInfo record.
     */
    BasicZone(const basic::ZoneInfo* zoneInfo):
        mZoneInfoBroker(zoneInfo) {}

    /**
     * Constructor from a basic::ZoneInfoBroker, used by BasicZoneProcessor.
     * This allows the implementation details of ZoneInfoBroker to remain
     * hidden.
     */
    BasicZone(const basic::ZoneInfoBroker& zoneInfo):
        mZoneInfoBroker(zoneInfo) {}

    /** Print the full zone name to printer. Example "America/Los_Angeles". */
    void printNameTo(Print& printer) const;

    /**
     * Print the short pretty zone name to the printer.
     * Underscores are replaced with spaces.
     * Example "Los Angeles".
     */
    void printShortNameTo(Print& printer) const;

    /** Return the zoneId of the current zoneInfo. */
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
