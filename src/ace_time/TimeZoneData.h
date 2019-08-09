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
 */
struct TimeZoneData {
  static const uint8_t kTypeError = 0;
  static const uint8_t kTypeManual = 1;
  static const uint8_t kTypeZoneId = 2;

  uint8_t type;

  union {
    /**
     * Used for kTypeManual. Use minutes instead of TimeOffsetCode (i.e.
     * 15-minute increments in the off chance that a future version of this
     * library supports timezones that shift by one-minute increments.
     */
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
    default:
      return false;
  }
}

inline bool operator!=(const TimeZoneData& a, const TimeZoneData& b) {
  return ! (a == b);
}

}

#endif
