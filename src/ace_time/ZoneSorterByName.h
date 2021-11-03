/*
 * MIT License
 * Copyright (c) 2021 Brian T. Park
 */

#ifndef ACE_TIME_ZONE_SORTER_BY_NAME_H
#define ACE_TIME_ZONE_SORTER_BY_NAME_H

#include <AceSorting.h>
#include "ZoneManager.h"

namespace ace_time {

/**
 * ZoneSorterByName, templatized on BasicZoneManager or ExtendedZoneManager.
 * Sorts the array of zones by name.
 *
 * @tparam ZM ZoneManager
 */
template <typename ZM>
class ZoneSorterByName {
  public:
    /**
     * Constructor.
     * @param zoneManager instance of the ZoneManager
     */
    ZoneSorterByName(const ZM& zoneManager) :
      mZoneManager(zoneManager)
    {}

    /**
     * Fill the given array of indexes with index from [0, size). The
     * resulting indexes array can be sorted using sortIndexes().
     */
    void fillIndexes(uint16_t indexes[], uint16_t size) const {
      for (uint16_t i = 0; i < size; i++) {
        indexes[i] = i;
      }
    }

    /** Sort the given array of indexes by UTC offset, then by name. */
    void sortIndexes(uint16_t indexes[], uint16_t size) const {
      ace_sorting::shellSortKnuth(indexes, size,
        [this](uint16_t indexA, uint16_t indexB) -> bool {
          auto za = this->mZoneManager.getZoneForIndex(indexA);
          auto zb = this->mZoneManager.getZoneForIndex(indexB);
          return za.kname().compareTo(zb.kname()) < 0;
        }
      );
    }

    /** Sort the given array of zone ids by UTC offset, then by name. */
    void sortIds(uint32_t ids[], uint16_t size) const {
      ace_sorting::shellSortKnuth(ids, size,
        [this](uint32_t a, uint32_t b) -> bool {
          uint16_t indexA = this->mZoneManager.indexForZoneId(a);
          uint16_t indexB = this->mZoneManager.indexForZoneId(b);
          auto za = this->mZoneManager.getZoneForIndex(indexA);
          auto zb = this->mZoneManager.getZoneForIndex(indexB);
          return za.kname().compareTo(zb.kname()) < 0;
        }
      );
    }

    /** Sort the given array of zone names by UTC offset, then by name. */
    void sortNames(const char* names[], uint16_t size) const {
      ace_sorting::shellSortKnuth(names, size,
        [this](const char* a, const char* b) -> bool {
          uint16_t indexA = this->mZoneManager.indexForZoneName(a);
          uint16_t indexB = this->mZoneManager.indexForZoneName(b);
          auto za = this->mZoneManager.getZoneForIndex(indexA);
          auto zb = this->mZoneManager.getZoneForIndex(indexB);
          return za.kname().compareTo(zb.kname()) < 0;
        }
      );
    }

  private:
    // disable copy constructor and assignment operator
    ZoneSorterByName(const ZoneSorterByName&) = delete;
    ZoneSorterByName& operator=(const ZoneSorterByName&) = delete;

  private:
    const ZM& mZoneManager;
};

}

#endif
