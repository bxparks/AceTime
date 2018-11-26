#ifndef ACE_TIME_TIME_ZONE_H
#define ACE_TIME_TIME_ZONE_H

#include <stdint.h>
#include "common/ZoneInfo.h"
#include "UtcOffset.h"
#include "ZoneManager.h"

class Print;

namespace ace_time {

/**
 * Class that describes a time zone. There are 2 subtypes:
 *
 *    - kTypeFixed represents a fixed offset from UTC, with an optional DST
 *    flag.
 *    - kTypeAuto represents a time zone described by the TZ Database which
 *    contains rules about when the transition occurs from standard to DST
 *    modes.
 *
 * This class is designed to be logically immutable (an internal cache is
 * mutable but hidden from the calling code). The application should create
 * a single instance of TimeZone for each time zone needed, and reuse the
 * same instance for multiple DateTime objects.
 */
class TimeZone {
  public:
    static const uint8_t kTypeFixed = 0;
    static const uint8_t kTypeAuto = 1;

    /** Default UTC TimeZone instance. */
    static const TimeZone sUtc;

    /**
     * Factory method. Create from UtcOffset.
     *
     * @param utcOffset offset from UTC
     * @param isDst true if DST is in effect
     * @param stdAbbrev abbreviation during standard time (e.g. "PST")
     * @param dstAbbrev abbreviation during DST time (e.g. "PDT")
     */
    static TimeZone forUtcOffset(UtcOffset utcOffset,
        bool isDst = false, const char* stdAbbrev = nullptr,
        const char* dstAbbrev = nullptr) {
      return TimeZone(utcOffset, isDst, stdAbbrev, dstAbbrev);
    }

    /** Factory method. Create from time zone string. */
    static TimeZone forOffsetString(const char* ts) {
      uint8_t offsetCode;
      bool isDst;
      // TODO: write better time zone parser
      parseFromOffsetString(ts, &offsetCode, &isDst);
      return TimeZone::forUtcOffset(
          UtcOffset::forOffsetCode(offsetCode), isDst);
    }

    /** Factory method. Create from ZoneInfo. */
    static TimeZone forZone(const common::ZoneInfo* zoneInfo) {
      return TimeZone(zoneInfo);
    }

    /** Default constructor creates the UTC time zone. */
    explicit TimeZone():
        mType(kTypeFixed),
        mUtcOffset(),
        mIsDst(false),
        mStdAbbrev(nullptr),
        mDstAbbrev(nullptr),
        mZoneManager(nullptr) {}

    /** Return the type of TimeZone. */
    uint8_t getType() const { return mType; }

    /** Return the effective zone offset. */
    UtcOffset getUtcOffset(uint32_t epochSeconds) const {
      if (mType == kTypeFixed) {
        return getUtcOffset();
      } else {
        return mZoneManager.getUtcOffset(epochSeconds);
      }
    }

    /** Return the abbreviation of the time zone. */
    const char* getAbbrev(uint32_t epochSeconds) const {
      if (mType == kTypeFixed) {
        return getAbbrev();
      } else {
        return mZoneManager.getAbbrev(epochSeconds);
      }
    }

    /** Return the base offset without regards to the DST setting. */
    const UtcOffset& getBaseUtcOffset() const { return mUtcOffset; }

    /** Set the base offset without regards to the DST setting. */
    void setBaseUtcOffset(UtcOffset utcOffset) {
      mUtcOffset = utcOffset;
    }

    /** Return the base isDst flag. */
    bool getBaseDst() const { return mIsDst; }

    /** Set the base isDst flag. */
    void setBaseDst(bool isDst) { mIsDst = isDst; }

    /** Return the standard abbreviation. Nullable. */
    const char* getStdAbbrev() const { return mStdAbbrev; }

    /** Return the DST abbreviation. Nullable. */
    const char* getDstAbbrev() const { return mDstAbbrev; }

    /** Print the human readable representation of the time zone. */
    void printTo(Print& printer) const;

  private:
    friend bool operator==(const TimeZone& a, const TimeZone& b);
    friend bool operator!=(const TimeZone& a, const TimeZone& b);

    /** Length of UTC offset string (e.g. "-07:00", "+01:30"). */
    static const uint8_t kUtcOffsetStringLength = 6;

    /** Constructor for kTypeFixed. */
    explicit TimeZone(UtcOffset utcOffset, bool isDst,
            const char* stdAbbrev, const char* dstAbbrev):
        mType(kTypeFixed),
        mUtcOffset(utcOffset),
        mIsDst(isDst),
        mStdAbbrev(stdAbbrev),
        mDstAbbrev(dstAbbrev),
        mZoneManager(nullptr) {}

    /** Constructor for kTypeAuto. */
    explicit TimeZone(const common::ZoneInfo* zoneInfo):
        mType(kTypeAuto),
        mUtcOffset(),
        mIsDst(false),
        mStdAbbrev(nullptr),
        mDstAbbrev(nullptr),
        mZoneManager(zoneInfo) {}

    /** Set time zone from the given UTC offset string. */
    static void parseFromOffsetString(const char* offsetString,
        uint8_t* offsetCode, bool* isDst);

    /**
     * Return the abbreviation depending on the isDst flag.
     * Return empty string if nullptr.
     */
    const char* getAbbrev() const {
      const char* abbrev = mIsDst ? mDstAbbrev : mStdAbbrev;
      return (abbrev == nullptr) ? "" : abbrev;
    }

    /** Return the effective zone offset for kTypeFixed. */
    UtcOffset getUtcOffset() const {
      return UtcOffset::forOffsetCode(
          mUtcOffset.toOffsetCode() + (mIsDst ? 4 : 0));
    }

    /** Type of time zone. */
    uint8_t mType;

    /** Offset from UTC. */
    UtcOffset mUtcOffset;

    /** Indicate whether Daylight Saving Time is in effect. */
    bool mIsDst;

    /** Time zone abbreviation for standard time, e.g. "PST". Nullable. */
    const char* mStdAbbrev;

    /** Time zone abbreviation for daylight time, e.g. "PDT". Nullable. */
    const char* mDstAbbrev;

    /** Manager of the time zone rules for the given ZoneInfo. */
    mutable ZoneManager mZoneManager;
};

inline bool operator==(const TimeZone& a, const TimeZone& b) {
  if (a.mType != b.mType) return false;

  if (a.mType == TimeZone::kTypeFixed) {
    return a.mUtcOffset == b.mUtcOffset
        && a.mIsDst == b.mIsDst
        && a.mStdAbbrev == b.mStdAbbrev
        && a.mDstAbbrev == b.mDstAbbrev;
  } else {
    return a.mZoneManager.getZoneInfo() == b.mZoneManager.getZoneInfo();
  }
}

inline bool operator!=(const TimeZone& a, const TimeZone& b) {
  return ! (a == b);
}

}

#endif
