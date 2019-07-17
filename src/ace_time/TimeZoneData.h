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
  static const uint8_t kTypeFixed = 0;
  static const uint8_t kTypeManual = ZoneSpecifier::kTypeManual;
  static const uint8_t kTypeBasic = ZoneSpecifier::kTypeBasic;
  static const uint8_t kTypeExtended = ZoneSpecifier::kTypeExtended;

  uint8_t type;

  union {
    /** Used for kTypeFixed. */
    int8_t offsetCode;

    /** Used for kTypeManual. */
    struct {
      int8_t stdOffsetCode;
      bool isDst;
    };

    /** Used for kTypeBasic. */
    const basic::ZoneInfo* basicZoneInfo;

    /** Used for kTypeExtended. */
    const extended::ZoneInfo* extendedZoneInfo;
  };
};

inline bool operator==(const TimeZoneData& a, const TimeZoneData& b) {
  if (a.type != b.type) return false;
  switch (a.type) {
    case TimeZoneData::kTypeFixed:
      return (a.offsetCode == b.offsetCode);
    case TimeZoneData::kTypeManual:
      return (a.stdOffsetCode == b.stdOffsetCode) && (a.isDst == b.isDst);
    case TimeZoneData::kTypeBasic:
      return (a.basicZoneInfo == b.basicZoneInfo);
    case TimeZoneData::kTypeExtended:
      return (a.extendedZoneInfo == b.extendedZoneInfo);
    default:
      return false;
  }
}

inline bool operator!=(const TimeZoneData& a, const TimeZoneData& b) {
  return ! (a == b);
}

}

#endif
