/*
 * MIT License
 * Copyright (c) 2023 Brian T. Park
 */

#ifndef ACE_TIME_COMPLETE_ZONE_H
#define ACE_TIME_COMPLETE_ZONE_H

#include <AceCommon.h> // KString
#include "../zoneinfo/infos.h"
#include "../zoneinfo/brokers.h"
#include "TimeOffset.h"

class Print;

namespace ace_time {

/**
 * A thin wrapper around an complete::ZoneInfo data structure to provide a
 * stable API access to some useful complete::ZoneInfo data. The
 * complete::ZoneInfo data struct is intended to be an opaque type to the users
 * of this library.
 */
class CompleteZone {
  public:
    /**
     * Constructor from a raw complete::ZoneInfo* pointer, intended for manual
     * inspection of a ZoneInfo record.
     */
    CompleteZone(const complete::ZoneInfo* zoneInfo):
        mZoneInfoBroker(zoneInfo) {}

    /**
     * Constructor from an complete::ZoneInfoBroker, used by
     * CompleteZoneProcessor. This allows the implementation details of
     * ZoneInfoBroker to remain hidden.
     */
    CompleteZone(const complete::ZoneInfoBroker& zoneInfo):
        mZoneInfoBroker(zoneInfo) {}

    // Use default copy constructor and assignment operator
    CompleteZone(const CompleteZone&) = default;
    CompleteZone& operator=(const CompleteZone&) = default;

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
    TimeOffset stdOffset() const {
      uint8_t numEras = mZoneInfoBroker.numEras();
      complete::ZoneEraBroker zeb = mZoneInfoBroker.era(numEras - 1);
      return TimeOffset::forSeconds(zeb.offsetSeconds());
    }

    /** Return the name as a KString. */
    ace_common::KString kname() const {
      const auto* name = isNull() ? nullptr : mZoneInfoBroker.name();
      complete::ZoneContextBroker zoneContext = mZoneInfoBroker.zoneContext();
      return ace_common::KString(
          name, zoneContext.fragments(), zoneContext.numFragments());
    }

  private:
    complete::ZoneInfoBroker mZoneInfoBroker;
};

}

#endif
