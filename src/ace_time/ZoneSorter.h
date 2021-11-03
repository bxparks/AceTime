/*
 * MIT License
 * Copyright (c) 2021 Brian T. Park
 */

#ifndef ACE_TIME_ZONE_SORTER_H
#define ACE_TIME_ZONE_SORTER_H

#include <AceSorting.h>
#include "ZoneManager.h"

namespace ace_time {

/**
 * ZoneSorter, templatized on BasicZoneManager or ExtendedZoneManager.
 *
 * @tparam ZM ZoneManager
 */
template <typename ZM>
class ZoneSorter {
  public:
    ZoneSorter(const ZM& zoneManager) :
      mZoneManager(zoneManager)
    {}

    void sortIndexes(uint16_t indexes[], uint16_t size) const {
      ace_sorting::shellSortKnuth(indexes, size,
        [this](uint16_t indexA, uint16_t indexB) -> bool {
          auto za = this->mZoneManager.getZoneForIndex(indexA);
          auto zb = this->mZoneManager.getZoneForIndex(indexB);
          return this->compareZone(za, zb) < 0;
        }
      );
    }

    void sortIds(uint32_t ids[], uint16_t size) const {
      ace_sorting::shellSortKnuth(ids, size,
        [this](uint32_t a, uint32_t b) -> bool {
          uint16_t indexA = this->mZoneManager.indexForZoneId(a);
          uint16_t indexB = this->mZoneManager.indexForZoneId(b);
          auto za = this->mZoneManager.getZoneForIndex(indexA);
          auto zb = this->mZoneManager.getZoneForIndex(indexB);
          return this->compareZone(za, zb) < 0;
        }
      );
    }

    void sortNames(const char* names[], uint16_t size) const {
      ace_sorting::shellSortKnuth(names, size,
        [this](const char* a, const char* b) -> bool {
          uint16_t indexA = this->mZoneManager.indexForZoneName(a);
          uint16_t indexB = this->mZoneManager.indexForZoneName(b);
          auto za = this->mZoneManager.getZoneForIndex(indexA);
          auto zb = this->mZoneManager.getZoneForIndex(indexB);
          return this->compareZone(za, zb) < 0;
        }
      );
    }

  private:
    // disable copy constructor and assignment operator
    ZoneSorter(const ZoneSorter&) = delete;
    ZoneSorter& operator=(const ZoneSorter&) = delete;

    /**
     * Return <0, 0, or >0 depending on whether Zone a is <, ==, or > than Zone
     * b. We cannot use `auto` as the parameter type (apparently, that's a C++14
     * feature), so we are forced to use template function. The `Z` template
     * will be either the BasicZone or ExtendedZone class.
     */
    template <typename Z>
    static int compareZone(const Z& a, const Z& b) {
      if (a.isNull()) {
        if (b.isNull()) {
          return 0;
        } else {
          return -1;
        }
      }
      if (b.isNull()) return 1;

      int16_t offsetA = a.stdOffsetMinutes();
      int16_t offsetB = b.stdOffsetMinutes();
      if (offsetA < offsetB) return -1;
      if (offsetA > offsetB) return 1;
      return a.kname().compareTo(b.kname());
    }

  private:
    const ZM& mZoneManager;
};

}

#endif
