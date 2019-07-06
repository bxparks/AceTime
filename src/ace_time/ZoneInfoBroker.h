/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#ifndef ACE_TIME_ZONE_INFO_BROKER_H
#define ACE_TIME_ZONE_INFO_BROKER_H

#include "common/ZoneInfo.h"

namespace ace_time {

class BasicZoneInfoBroker {
  public:
    BasicZoneInfoBroker(const basic::ZoneInfo* zoneInfo):
        mZoneInfo(zoneInfo) {}

    const basic::ZoneInfo* zoneInfo() const { return mZoneInfo; }

    int16_t startYear() const {
      return mZoneInfo->zoneContext->startYear;
    }

    int16_t untilYear() const {
      return mZoneInfo->zoneContext->untilYear;
    }

    /**
     * Find the ZoneEra which applies to the given year. The era will
     * satisfy (year < ZoneEra.untilYearTiny + kEpochYear). Since the
     * largest untilYearTiny is 127, the largest supported 'year' is 2126.
     */
    const basic::ZoneEra* findZoneEra(int16_t year) const {
      for (uint8_t i = 0; i < numEras(); i++) {
        const basic::ZoneEra* ze = era(i);
        if (year < ze->untilYearTiny + LocalDate::kEpochYear) return ze;
      }
      // Return the last ZoneEra if we run off the end.
      return era(numEras() - 1);
    }

    /**
     * Find the most recent ZoneEra which was in effect just before the
     * beginning of the given year, in other words, just before {year}-01-01
     * 00:00:00. It will be first era just after the latest era whose untilYear
     * < year. Since the ZoneEras are in increasing order of untilYear, this is
     * the same as matching the first ZoneEra whose untilYear >= year.
     *
     * This should never return nullptr because the code generator for
     * zone_infos.cpp verified that the final ZoneEra contains an empty
     * untilYear, interpreted as 'max', and set to 127.
     */
    const basic::ZoneEra* findZoneEraPriorTo(int16_t year) const {
      for (uint8_t i = 0; i < numEras(); i++) {
        const basic::ZoneEra* ze = era(i);
        if (year <= ze->untilYearTiny + LocalDate::kEpochYear) return ze;
      }
      // Return the last ZoneEra if we run off the end.
      return era(numEras() - 1);
    }

  private:
    uint8_t numEras() const {
      return mZoneInfo->numEras;
    }

    const basic::ZoneEra* era(uint8_t i) const {
      return &mZoneInfo->eras[i];
    }

    const basic::ZoneInfo* const mZoneInfo;
};

}

#endif
