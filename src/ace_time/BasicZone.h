/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#ifndef ACE_TIME_BASIC_ZONE_H
#define ACE_TIME_BASIC_ZONE_H

#include <AceCommon.h> // KString
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

    // Use default copy constructor and assignment operator
    BasicZone(const BasicZone&) = default;
    BasicZone& operator=(const BasicZone&) = default;

    /** Return true if zoneInfo is null. */
    bool isNull() const { return mZoneInfoBroker.isNull(); }

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

    /** Return the STDOFF of the last ZoneEra. */
    int16_t stdOffsetMinutes() const {
      uint8_t numEras = mZoneInfoBroker.numEras();
      basic::ZoneEraBroker zeb = mZoneInfoBroker.era(numEras - 1);
      return zeb.offsetMinutes();
    }

    /** Return the name as a KString. */
    ace_common::KString kname() const {
      const auto* name = isNull() ? nullptr : mZoneInfoBroker.name();
      const internal::ZoneContext* zoneContext = mZoneInfoBroker.zoneContext();
      return ace_common::KString(
          name, zoneContext->fragments, zoneContext->numFragments);
    }

  private:
    basic::ZoneInfoBroker mZoneInfoBroker;
};

}

#endif
