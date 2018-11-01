#ifndef ACE_TIME_TIME_ZONE_H
#define ACE_TIME_TIME_ZONE_H

#include <stdint.h>
#include "ZoneOffset.h"
#include "ZoneInfo.h"
#include "ZoneManager.h"

class Print;

namespace ace_time {

/**
 * Class that describes a time zone. There are 2 subtypes:
 * kTypeFixed represents a fixed offset from UTC, with an optional DST flag.
 * kTypeAuto represents a time zone described by the TZ Database which
 * contains rules about when the transition occurs from standard to DST modes.
 */
class TimeZone {
  public:
    static const uint8_t kTypeFixed = 0;
    static const uint8_t kTypeAuto = 1;

    /** Factory method. Create from ZoneOffset. */
    static TimeZone forZoneOffset(ZoneOffset zoneOffset,
        bool isDst = false, const char* abbrev = nullptr) {
      return TimeZone(zoneOffset, isDst, abbrev);
    }

    /** Factory method. Create from time zone string. */
    static TimeZone forOffsetString(const char* ts) {
      uint8_t offsetCode;
      bool isDst;
      // TODO: write better time zone parser
      parseFromOffsetString(ts, &offsetCode, &isDst);
      return TimeZone::forZoneOffset(
          ZoneOffset::forOffsetCode(offsetCode), isDst);
    }

    /** Factory method. Create from ZoneInfo. */
    static TimeZone forZone(const ZoneInfo* zoneInfo) {
      return TimeZone(zoneInfo);
    }

    /** Default constructor. */
    TimeZone():
        mType(kTypeFixed),
        mZoneOffset(),
        mIsDst(false),
        mAbbrev(nullptr),
        mZoneManager(nullptr) {}

    /** Return the type of TimeZone. */
    uint8_t getType() const { return mType; }

    /** Return the DST mode setting. */
    bool isDst(uint32_t epochSeconds) const {
      if (mType == kTypeFixed) {
        return mIsDst;
      } else {
        return mZoneManager.isDst(epochSeconds);
      }
    }

    /** Return the effective zone offset. */
    ZoneOffset getZoneOffset(uint32_t epochSeconds) const {
      if (mType == kTypeFixed) {
        return ZoneOffset::forOffsetCode(
            mZoneOffset.toOffsetCode() + (mIsDst ? 4 : 0));
      } else {
        return mZoneManager.getZoneOffset(epochSeconds);
      }
    }

    /** Return the abbreviation of the time zone. */
    const char* getAbbrev(uint32_t epochSeconds) const {
      if (mType == kTypeFixed) {
        return mAbbrev == nullptr ? "" : mAbbrev;
      } else {
        return mZoneManager.getAbbrev(epochSeconds);
      }
    }

    /** Return the standard offset without regards to the DST setting. */
    const ZoneOffset& getStandardZoneOffset() const { return mZoneOffset; }

    /** Return the standard offset without regards to the DST setting. */
    ZoneOffset& getStandardZoneOffset() { return mZoneOffset; }

    /** Set the standdardoffset. */
    void setStandardZoneOffset(ZoneOffset zoneOffset) {
      mZoneOffset = zoneOffset;
    }

    /** Return the standard isDst flag. */
    bool getStandardDst() const { return mIsDst; }

    /** Set the standard isDst flag. */
    void setStandardDst(bool isDst) { mIsDst = isDst; }

    /** Return the standard abbreviation. */
    const char* getStandardAbbrev() const { return mAbbrev; }

    /** Set the standard abbreviation. */
    void setStandardAbbrev(const char* abbrev) { mAbbrev = abbrev; }

    /** Print the human readable representation of the time zone. */
    void printTo(Print& printer) const;

  private:
    friend bool operator==(const TimeZone& a, const TimeZone& b);
    friend bool operator!=(const TimeZone& a, const TimeZone& b);

    /** Length of UTC offset string (e.g. "-07:00", "+01:30"). */
    static const uint8_t kTimeZoneStringLength = 6;

    /** Constructor for kTypeFixed. */
    explicit TimeZone(ZoneOffset zoneOffset, bool isDst, const char* abbrev):
        mType(kTypeFixed),
        mZoneOffset(zoneOffset),
        mIsDst(isDst),
        mAbbrev(abbrev),
        mZoneManager(nullptr) {}

    /** Constructor for kTypeAuto. */
    explicit TimeZone(const ZoneInfo* zoneInfo):
        mType(kTypeAuto),
        mZoneOffset(),
        mIsDst(false),
        mAbbrev(nullptr),
        mZoneManager(zoneInfo) {}

    /** Set time zone from the given UTC offset string. */
    static void parseFromOffsetString(const char* offsetString,
        uint8_t* offsetCode, bool* isDst);

    /** Type of time zone. */
    uint8_t mType;

    /** Offset from UTC. */
    ZoneOffset mZoneOffset;

    /** Indicate whether Daylight Saving Time is in effect. */
    bool mIsDst;

    /** Time zone abbreviation, e.g. "PDT". Nullable. */
    const char* mAbbrev;

    /** Manager of the time zone rules for the given ZoneInfo. */
    mutable ZoneManager mZoneManager;
};

inline bool operator==(const TimeZone& a, const TimeZone& b) {
  if (a.mType != b.mType) return false;

  if (a.mType == TimeZone::kTypeFixed) {
    return a.mZoneOffset == b.mZoneOffset
        && a.mIsDst == b.mIsDst
        && a.mAbbrev == b.mAbbrev;
  } else {
    return a.mZoneManager.getZoneInfo() == b.mZoneManager.getZoneInfo();
  }
}

inline bool operator!=(const TimeZone& a, const TimeZone& b) {
  return ! (a == b);
}

}

#endif
