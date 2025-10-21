/*
 * MIT License
 * Copyright (c) 2023 Brian T. Park
 */

#ifndef ACE_TIME_ZONED_EXTRA_H
#define ACE_TIME_ZONED_EXTRA_H

#include <string.h> // strncpy()
#include <stdint.h>
#include "common/common.h" // acetime_t, kAbbrevSize
#include "TimeOffset.h"

namespace ace_time {

class TimeZone;
class PlainDateTime;

class ZonedExtra {
  public:
    /** Size of char buffer needed to hold the largest abbreviation. */
    static const uint8_t kAbbrevSize = ace_time::kAbbrevSize;

    /**
     * The epochSeconds or PlainDateTime was not found because it was outside
     * the range of the zoneinfo database (too far past, or too far in the
     * future).
     */
    static const uint8_t kTypeNotFound = 0;

    /**
     * The given PlainDateTime matches a single epochSeconds.
     * The given epochSeconds matches a single PlainDateTime.
     */
    static const uint8_t kTypeExact = 1;

    /**
     * The given PlainDateTime occurs in a gap and does not match any
     * epochSeconds.
     *
     * A given epochSeconds will never return this because it will always match
     * a unique PlainDateTime.
     */
    static const uint8_t kTypeGap = 2;

    /**
     * The given PlainDateTime matches 2 possible epochSeconds, which is
     * disambiguated by the 'disambiguate' parameter in the lookup function.
     *
     * A look up using epochSeconds will never return this because it will
     * always match a unique PlainDateTime.
     */
    static const uint8_t kTypeOverlap = 3;

    /** Return an instance that indicates an error. */
    static ZonedExtra forError() {
      return ZonedExtra();
    }

    /**
     * Return an instance for the given PlainDateTime and TimeZone.
     * If you already have a ZonedDateTime, then the PlainDateTime can be
     * retrieved using ZonedDateTime::plainDateTime().
     */
    static ZonedExtra forComponents(
        int16_t year, uint8_t month, uint8_t day,
        uint8_t hour, uint8_t minute, uint8_t second,
        const TimeZone& tz,
        Disambiguate disambiguate = Disambiguate::kCompatible);

    /** Return an instance for the given epochSeconds and TimeZone. */
    static ZonedExtra forEpochSeconds(
        acetime_t epochSeconds,
        const TimeZone& tz);

    /**
     * Return an instance for the given PlainDateTime and TimeZone.
     * If you already have a ZonedDateTime, then the PlainDateTime can be
     * retrieved using ZonedDateTime::plainDateTime().
     */
    static ZonedExtra forPlainDateTime(
        const PlainDateTime& pdt,
        const TimeZone& tz,
        Disambiguate disambiguate = Disambiguate::kCompatible);

    /** Backwards compatible version of forPlainDateTime(). */
    ACE_TIME_DEPRECATED
    static ZonedExtra forLocalDateTime(
        const PlainDateTime& pdt,
        const TimeZone& tz,
        Disambiguate disambiguate = Disambiguate::kCompatible) {
      return forPlainDateTime(pdt, tz, disambiguate);
    }

    /** Constructor */
    explicit ZonedExtra() {}

    /** Constructor */
    explicit ZonedExtra(
        uint8_t type,
        int32_t stdOffsetSeconds,
        int32_t dstOffsetSeconds,
        int32_t reqStdOffsetSeconds,
        int32_t reqDstOffsetSeconds,
        const char* abbrev)
      : mStdOffsetSeconds(stdOffsetSeconds)
      , mDstOffsetSeconds(dstOffsetSeconds)
      , mReqStdOffsetSeconds(reqStdOffsetSeconds)
      , mReqDstOffsetSeconds(reqDstOffsetSeconds)
      , mType(type)
    {
      strncpy(mAbbrev, abbrev, kAbbrevSize - 1);
      mAbbrev[kAbbrevSize - 1] = '\0';
    }

    /** Indicates that the PlainDateTime or epochSeconds was not found. */
    bool isError() const {
      return mStdOffsetSeconds == kInvalidSeconds;
    }

    uint8_t type() const { return mType; }

    /** STD offset of the resulting OffsetDateTime. */
    TimeOffset stdOffset() const {
      return TimeOffset::forSeconds(mStdOffsetSeconds);
    }

    /** DST offset of the resulting OffsetDateTime. */
    TimeOffset dstOffset() const {
      return TimeOffset::forSeconds(mDstOffsetSeconds);
    }

    /**
     * The total time offset (stdOffset + dstOffset). This will be the same
     * value as `ZonedDateTime::timeOffset()` when a ZonedDataTime is created
     * using `ZonedDateTime::forComponents()` or
     * `ZonedDateTime::forEpochSeconds()`.
     */
    TimeOffset timeOffset() const {
      return TimeOffset::forSeconds(mStdOffsetSeconds + mDstOffsetSeconds);
    }

    /**
     * STD offset of the requested epochSeconds or PlainDateTime.
     * This will be different from stdOffset only for kTypeGap.
     */
    TimeOffset reqStdOffset() const {
      return TimeOffset::forSeconds(mReqStdOffsetSeconds);
    }

    /**
     * DST offset of the requested epochSeconds or PlainDateTime.
     * This will be different from stdOffset only for kTypeGap.
     */
    TimeOffset reqDstOffset() const {
      return TimeOffset::forSeconds(mReqDstOffsetSeconds);
    }

    /**
     * The total time offset of the requested epochSeconds of PlainDateTime,
     * (reqStdOffset + reqDstOffset). This value becomes lost when a
     * ZonedDateTime is created using `ZonedDateTime::forComponents()` during a
     * DST gap, because it was used to convert the given PlainDateTime to an
     * epochSeconds, before the epochSeconds was renormalized back into a
     * ZonedDateTime. The ZonedExtra object provided access to this UTC offset.
     */
    TimeOffset reqTimeOffset() const {
      return TimeOffset::forSeconds(
          mReqStdOffsetSeconds + mReqDstOffsetSeconds);
    }

    /**
     * Returns the pointer to the local string buffer containing the timezone
     * abbreviation (e.g. "PST", "PDT") used at the given PlainDateTime or
     * epochSeconds. This pointer is safe to use as long as this object is
     * alive.
     */
    const char* abbrev() const { return mAbbrev; }

  private:
    static const int32_t kInvalidSeconds = INT32_MIN;

    int32_t mStdOffsetSeconds = kInvalidSeconds;
    int32_t mDstOffsetSeconds = kInvalidSeconds;
    int32_t mReqStdOffsetSeconds = kInvalidSeconds;
    int32_t mReqDstOffsetSeconds = kInvalidSeconds;
    uint8_t mType = kTypeNotFound;
    char mAbbrev[kAbbrevSize] = "";
};

}

#endif
