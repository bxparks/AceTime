/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#ifndef ACE_TIME_TIME_ZONE_DATA_H
#define ACE_TIME_TIME_ZONE_DATA_H

#include <stdint.h>
#include "ZoneSpecifier.h"

namespace ace_time {

namespace basic {
class ZoneInfo;
}
namespace extended {
class ZoneInfo;
}

/**
 * Data structure that captures the internal state of a TimeZone object with
 * enough information that is equivalent to doing a deep-copy of the TimeZone,
 * including its underlying ZoneSpecifier object. It may be used to compare
 * different TimeZone objects, without having to keep multiple copies
 * of the ZoneSpecifier object which can consume a significant amount of RAM.
 *
 * The data structure is intended to be used within the application in-memory,
 * since it holds a pointer to a ZoneInfo object. However, a serialization data
 * structure can be created from the fields in this data structure. A TimeZone
 * object may be recreated with the information in this struct (see
 * TimeZone::forTimeZoneData() factory method.)
 */
struct TimeZoneData {
  static const uint8_t kTypeError = 0;
  static const uint8_t kTypeManual = 1;
  static const uint8_t kTypeBasic = ZoneSpecifier::kTypeBasic;
  static const uint8_t kTypeExtended = ZoneSpecifier::kTypeExtended;
  static const uint8_t kTypeManaged = kTypeExtended + 1;

  uint8_t type;

  union {
    /** Used for kTypeManual. */
    struct {
      int8_t stdOffsetCode;
      int8_t dstOffsetCode;
    };

    /** Used for kTypeBasic and kTypeExtended. */
    const void* zoneInfo;

    /** Used for kTypeManaged. */
    uint32_t zoneId;
  };
};

inline bool operator==(const TimeZoneData& a, const TimeZoneData& b) {
  if (a.type != b.type) return false;
  switch (a.type) {
    case TimeZoneData::kTypeManual:
      return (a.stdOffsetCode == b.stdOffsetCode)
          && (a.dstOffsetCode == b.dstOffsetCode);
    case TimeZoneData::kTypeBasic:
    case TimeZoneData::kTypeExtended:
      return (a.zoneInfo == b.zoneInfo);
    case TimeZoneData::kTypeManaged:
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
