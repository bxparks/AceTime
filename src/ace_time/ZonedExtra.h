/*
 * MIT License
 * Copyright (c) 2023 Brian T. Park
 */

#ifndef ACE_TIME_ZONED_EXTRA_H
#define ACE_TIME_ZONED_EXTRA_H

#include <string.h> // memcpy()
#include <stdint.h>
#include "internal/common.h" // kAbbrevSize

namespace ace_time {

class ZonedExtra {
  public:
    static const int16_t kInvalidMinutes = INT16_MIN;

    static const uint8_t kTypeNotFound = 0;
    static const uint8_t kTypeExact = 1;
    static const uint8_t kTypeGap = 2;
    static const uint8_t kTypeOverlap = 3;

    static ZonedExtra forError() {
      return ZonedExtra();
    }

    /** Consructor */
    explicit ZonedExtra() {}

    /** Consructor */
    explicit ZonedExtra(
        uint8_t type,
        int16_t stdOffsetMinutes,
        int16_t dstOffsetMinutes,
        const char* abbrev)
      : mStdOffsetMinutes(stdOffsetMinutes)
      , mDstOffsetMinutes(dstOffsetMinutes)
      , mType(type)
    {
      memcpy(mAbbrev, abbrev, internal::kAbbrevSize);
      mAbbrev[internal::kAbbrevSize - 1] = '\0';
    }

    bool isError() const {
      return mStdOffsetMinutes == kInvalidMinutes;
    }

    TimeOffset stdOffset() const {
      return TimeOffset::forMinutes(mStdOffsetMinutes);
    }

    TimeOffset dstOffset() const {
      return TimeOffset::forMinutes(mDstOffsetMinutes);
    }

    const char* abbrev() const { return mAbbrev; }

  private:
    int16_t mStdOffsetMinutes = kInvalidMinutes;
    int16_t mDstOffsetMinutes = kInvalidMinutes;
    uint8_t mType = kTypeNotFound;
    char mAbbrev[internal::kAbbrevSize] = "";
};

}

#endif
