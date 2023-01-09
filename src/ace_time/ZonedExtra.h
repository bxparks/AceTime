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

    static ZonedExtra forError() {
      return ZonedExtra();
    }

    /** Consructor */
    explicit ZonedExtra() {}

    /** Consructor */
    explicit ZonedExtra(
        int16_t stdOffsetMinutes,
        int16_t dstOffsetMinutes,
        const char* abbrev)
      : mStdOffsetMinutes(stdOffsetMinutes)
      , mDstOffsetMinutes(dstOffsetMinutes)
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
    char mAbbrev[internal::kAbbrevSize] = "";
};

}

#endif
