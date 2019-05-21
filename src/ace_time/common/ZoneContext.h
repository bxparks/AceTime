#ifndef ACE_TIME_COMMON_ZONE_CONTEXT_H
#define ACE_TIME_COMMON_ZONE_CONTEXT_H

namespace ace_time {
namespace common {

/**
 * Metadata about the zone database. A ZoneInfo struct will contain a pointer
 * to this.
 */
struct ZoneContext {
  /** Start year of the zone files. */
  int16_t startYear;

  /** Until year of the zone files. */
  int16_t untilYear;

  /** Epoch year. Currently always 2000 but could change in the future. */
  //uint16_t epoch_year;
};

}
}

#endif
