/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#ifndef ACE_TIME_ZONE_DATA_H
#define ACE_TIME_ZONE_DATA_H

#include <AceCommon.h> // KString
#include "../zoneinfo/infos.h"
#include "TimeOffset.h"

class Print;

namespace ace_time {

/**
 * A thin wrapper around a ZoneInfo data structure to provide a stable API
 * access to some useful ZoneInfo data. The ZoneInfo data struct is intended to
 * be an opaque type to the users of this library.
 *
 * @tparam D container type of ZoneInfo database
 */
template <typename D>
class ZoneTemplate {
  public:
    /**
     * Constructor from a raw Info::ZoneInfo* pointer, intended for
     * manual inspection of a ZoneInfo record.
     */
    ZoneTemplate(const typename D::ZoneInfo* zoneInfo):
        mZoneInfoBroker(zoneInfo) {}

    /**
     * Constructor from a Info:ZoneInfoBroker, used by various ZoneProcessor.
     * This allows the implementation details of ZoneInfoBroker to remain
     * hidden.
     */
    ZoneTemplate(const typename D::ZoneInfoBroker& zoneInfo):
        mZoneInfoBroker(zoneInfo) {}

    // Use default copy constructor and assignment operator
    ZoneTemplate(const ZoneTemplate&) = default;
    ZoneTemplate& operator=(const ZoneTemplate&) = default;

    /** Return true if zoneInfo is null. */
    bool isNull() const { return mZoneInfoBroker.isNull(); }

    /** Print the full zone name to printer. Example "America/Los_Angeles". */
    void printNameTo(Print& printer) const {
      const __FlashStringHelper* name = mZoneInfoBroker.name();
      typename D::ZoneContextBroker zoneContext =
          mZoneInfoBroker.zoneContext();
      ace_common::KString kname(
          name, zoneContext.fragments(), zoneContext.numFragments());
      kname.printTo(printer);
    }

    /**
     * Print the short pretty zone name to the printer.
     * Underscores are replaced with spaces.
     * Example "Los Angeles".
     */
    void printShortNameTo(Print& printer) const {
      const __FlashStringHelper* name = mZoneInfoBroker.name();
      const __FlashStringHelper* shortName = zoneinfo::findShortName(name);
      ace_common::printReplaceCharTo(printer, shortName, '_', ' ');
    }

    /** Return the zoneId of the current zoneInfo. */
    uint32_t zoneId() const {
      return mZoneInfoBroker.zoneId();
    }

    /** Return the STDOFF of the last ZoneEra. */
    TimeOffset stdOffset() const {
      uint8_t numEras = mZoneInfoBroker.numEras();
      typename D::ZoneEraBroker zeb = mZoneInfoBroker.era(numEras - 1);
      return TimeOffset::forSeconds(zeb.offsetSeconds());
    }

    /** Return the name as a KString. */
    ace_common::KString kname() const {
      const auto* name = isNull() ? nullptr : mZoneInfoBroker.name();
      typename D::ZoneContextBroker zoneContext =
          mZoneInfoBroker.zoneContext();
      return ace_common::KString(
          name, zoneContext.fragments(), zoneContext.numFragments());
    }

  private:
    typename D::ZoneInfoBroker mZoneInfoBroker;
};

using BasicZone = ZoneTemplate<basic::Info>;
using ExtendedZone = ZoneTemplate<extended::Info>;
using CompleteZone = ZoneTemplate<complete::Info>;

}

#endif
