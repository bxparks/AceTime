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
 * enough information so that it can be reconstructed using a ZoneManager.
 * The data structure can be stored persistently then read back.
 * TimeZone::forTimeZoneData() factory method.
 */
struct TimeZoneData {
  static const uint8_t kTypeError = 0;
  static const uint8_t kTypeManual = 1;
  static const uint8_t kTypeZoneId = 2;

  uint8_t type;

  union {
    /** Used for kTypeManual. */
    struct {
      int8_t stdOffsetCode;
      int8_t dstOffsetCode;
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
      return (a.stdOffsetCode == b.stdOffsetCode)
          && (a.dstOffsetCode == b.dstOffsetCode);
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
