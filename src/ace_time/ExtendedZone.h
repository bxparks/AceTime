/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#ifndef ACE_TIME_EXTENDED_ZONE_H
#define ACE_TIME_EXTENDED_ZONE_H

#include <AceCommon.h> // KString
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

    // Use default copy constructor and assignment operator
    ExtendedZone(const ExtendedZone&) = default;
    ExtendedZone& operator=(const ExtendedZone&) = default;

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
      extended::ZoneEraBroker zeb = mZoneInfoBroker.era(numEras - 1);
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
    const extended::ZoneInfoBroker mZoneInfoBroker;
};

}

#endif
