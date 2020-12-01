/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#ifndef ACE_TIME_TIME_ZONE_DATA_H
#define ACE_TIME_TIME_ZONE_DATA_H

#include <stdint.h>
#include "ZoneProcessor.h"

namespace ace_time {

/**
 * Data structure that captures the internal state of a TimeZone object with
 * enough information so that it can be serialized using
 * TimeZone::toTimeZoneData() then reconstructed using
 * ZoneManager::createForTimeZoneData(). This data structure is meant to a
 * simple and somewhat opaque serialization object. You should not rely on this
 * struct to remain stable, nor reach into its internal fields. No versioning
 * is provided for simplicity. If the internal format changes in the future,
 * the previous version will likely be incompatible with the new version of the
 * library. It is recommended to use a CRC check to detect version
 * incompatibility if this data structure is saved (e.g. EEPROM) and retrieved
 * later .
 *
 * For convenience, an array of TimeZoneData can be initializeed using
 * the usual initializer syntax. In other words, the following is allowed:
 *
 * @code
 *  TimeZoneData zones[3] = {
 *    {0, 0},
 *    {zonedb::kZoneIdAmerica_Los_Angeles},
 *    {}
 *  };
 * @endcode
 */
struct TimeZoneData {
  static const uint8_t kTypeError = 0;
  static const uint8_t kTypeManual = 1;
  static const uint8_t kTypeZoneId = 2;

  /**
   * Constructor for kTypeZoneId needed because C+11 does not have member
   * initialization, and cannot initialize the 'zoneId' component of the union.
   */
  TimeZoneData(uint32_t zid)
    : type(kTypeZoneId),
      zoneId(zid)
    {}

  /** Constructor for kTypeManual. */
  TimeZoneData(int16_t stdMinutes, int16_t dstMinutes)
    : type(kTypeManual),
      stdOffsetMinutes(stdMinutes),
      dstOffsetMinutes(dstMinutes)
    {}

  /** Default constructor gives kTypeError sentinel. */
  TimeZoneData()
    : type(kTypeError),
      zoneId(0)
    {}

  uint8_t type;

  union {
    /** Used for kTypeManual. */
    struct {
      int16_t stdOffsetMinutes;
      int16_t dstOffsetMinutes;
    };

    /**
     * All of kTypeBasic, kTypeExtended, kTypeBasicManaged,
     * kTypeExtendedManaged collapse down to a kTypeZoneId.
     */
    uint32_t zoneId;
  };
};

inline bool operator==(const TimeZoneData& a, const TimeZoneData& b) {
  if (a.type != b.type) return false;
  switch (a.type) {
    case TimeZoneData::kTypeManual:
      return (a.stdOffsetMinutes == b.stdOffsetMinutes)
          && (a.dstOffsetMinutes == b.dstOffsetMinutes);
    case TimeZoneData::kTypeZoneId:
      return (a.zoneId == b.zoneId);
    case TimeZoneData::kTypeError:
      return true;
    default:
      return false;
  }
}

inline bool operator!=(const TimeZoneData& a, const TimeZoneData& b) {
  return ! (a == b);
}

}

#endif
