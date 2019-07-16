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

struct TimeZoneData {
  static const uint8_t kTypeFixed = 0;
  static const uint8_t kTypeManual = ZoneSpecifier::kTypeManual;
  static const uint8_t kTypeBasic = ZoneSpecifier::kTypeBasic;
  static const uint8_t kTypeExtended = ZoneSpecifier::kTypeExtended;

  uint8_t type;
  union {
    int8_t offsetCode;
    struct {
      int8_t stdOffsetCode;
      bool isDst;
    };
    const basic::ZoneInfo* basicZoneInfo;
    const extended::ZoneInfo* extendedZoneInfo;
  };
};

inline bool operator==(const TimeZoneData& a, const TimeZoneData& b) {
  if (a.type != b.type) return false;
  switch (a.type) {
    case TimeZoneData::kTypeFixed:
      return (a.offsetCode == b.offsetCode);
    case TimeZoneData::kTypeManual:
      return a.stdOffsetCode == b.stdOffsetCode && a.isDst == b.isDst;
    case TimeZoneData::kTypeBasic:
      return a.basicZoneInfo == b.basicZoneInfo;
    case TimeZoneData::kTypeExtended:
      return a.extendedZoneInfo == b.extendedZoneInfo;
    default:
      return false;
  }
}

inline bool operator!=(const TimeZoneData& a, const TimeZoneData& b) {
  return ! (a == b);
}

}

#endif
