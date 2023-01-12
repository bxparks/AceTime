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

    /**
     * The epochSeconds or LocalDateTime was not found because it was outside
     * the range of the zoneinfo database (too far past, or too far in the
     * future).
     */
    static const uint8_t kTypeNotFound = 0;

    /**
     * Find by epochSeconds matches a single LocalDateTime. Find by
     * LocalDateTime matches a single epochSeconds.
     */
    static const uint8_t kTypeExact = 1;

    /**
     * Find by LocalDateTime occurs in a gap and does not match any
     * epochSeconds. Find by epochSeconds will never return this because since
     * it will always match either a single LocalDateTime or match nothing.
     */
    static const uint8_t kTypeGap = 2;

    /**
     * Find by LocalDateTime matches 2 possible epochSeconds, which is
     * disambguiated by the LocalDateTime::fold input parameter. Find by
     * epochSeconds matches a LocalDateTime that can occur twice, and is
     * disambiguated by the OffsetDateTime::fold or ZonedDateTime::fold output
     * parameter.
     */
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

    /** Indicates that the LocalDateTime or epochSeconds was not found. */
    bool isError() const {
      return mStdOffsetMinutes == kInvalidMinutes;
    }

    uint8_t type() const { return mType; }

    /** STD offset of the resulting OffsetDateTime. */
    TimeOffset stdOffset() const {
      return TimeOffset::forMinutes(mStdOffsetMinutes);
    }

    /** DST offset of the resulting OffsetDateTime. */
    TimeOffset dstOffset() const {
      return TimeOffset::forMinutes(mDstOffsetMinutes);
    }

    /**
     * The local string buffer containing the timezone abbreviation (e.g.
     * "PST", "PDT") used at the given LocalDateTime or epochSeconds. This
     * buffer is a local copy, it is safe to use after calling other timezone
     * related functions.
     */
    const char* abbrev() const { return mAbbrev; }

  private:
    int16_t mStdOffsetMinutes = kInvalidMinutes;
    int16_t mDstOffsetMinutes = kInvalidMinutes;
    uint8_t mType = kTypeNotFound;
    char mAbbrev[internal::kAbbrevSize] = "";
};

}

#endif
