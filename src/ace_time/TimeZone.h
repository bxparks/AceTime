#ifndef ACE_TIME_TIME_ZONE_H
#define ACE_TIME_TIME_ZONE_H

#include <stdint.h>
#include "UtcOffset.h"
#include "ZoneAgent.h"

class Print;

namespace ace_time {

namespace internal {

/** Fields needed by TimeZone::kTypeManual. */
struct ManualZone {
  /** Offset from UTC. */
  UtcOffset utcOffset;

  /** Indicate whether Daylight Saving Time is in effect. */
  bool isDst;

  /** Time zone abbreviation for standard time, e.g. "PST". Nullable. */
  const char* stdAbbrev;

  /** Time zone abbreviation for daylight time, e.g. "PDT". Nullable. */
  const char* dstAbbrev;
};

/** Fields needed by TimeZone::kTypeAuto. */
struct AutoZone {
  /** Manager of the time zone rules for the given ZoneInfo. */
  mutable ZoneAgent* zoneAgent;
};

}

/**
 * Class that describes a time zone. There are 2 types:
 *
 *    - kTypeManual represents an offset from UTC with a DST flag, both of
 *      which can be adjusted by the user. This type is mutable.
 *
 *    - kTypeAuto represents a time zone described by the TZ Database which
 *      contains rules about when the transition occurs from standard to DST
 *      modes. This type is immutable.
 *
 * This class should be treated as a value type and passed around by value or
 * by const reference. If the user wants to change the offset and DST of a
 * Manual TimeZone, copy the TimeZone by value and call one of the mutators
 * which are valid for kTypeManual.
 *
 * An alternative implementation would make TimeZone a base class of an
 * inheritance hierarchy with 2 subclasses (ManualTimeZone and AutoTimeZone).
 * However this means that the TimeZone object can no longer be passed around
 * by value, and the DateTime is forced to hold on to the TimeZone object using
 * a pointer. It then becomes very difficult to change the offset and DST
 * fields of the ManualTimeZone. Using a single TimeZone class and implementing
 * it as a value type simplifies a lot of code.
 *
 * The disadvantage of merging the Manual and Auto types is that for the Auto
 * type, the ZoneAgent needed to be split off as an external dependency to the
 * TimeZone.
 */
class TimeZone {
  public:
    static const uint8_t kTypeManual = 0;
    static const uint8_t kTypeAuto = 1;

    /** Factory method. Craete an auto TimeZone using the given ZoneAgent. */
    static TimeZone forZone(ZoneAgent* zoneAgent) {
      return TimeZone(zoneAgent);
    }

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
      // TODO: write better time zone parser
      parseFromOffsetString(ts, &offsetCode);
      return TimeZone::forUtcOffset(UtcOffset::forOffsetCode(offsetCode));
    }

    /** Default constructor creates the UTC time zone. */
    explicit TimeZone():
        mType(kTypeManual),
        mManual{UtcOffset(), false, "UTC", "UTC"} {}

    /** Return the type of TimeZone. */
    uint8_t getType() const { return mType; }

    /** Return the effective zone offset. */
    UtcOffset getUtcOffset(uint32_t epochSeconds) const {
      if (mType == kTypeAuto) {
        return mAuto.zoneAgent == nullptr
            ? UtcOffset()
            : mAuto.zoneAgent->getUtcOffset(epochSeconds);
      } else {
        return UtcOffset::forOffsetCode(
            mManual.utcOffset.toOffsetCode() + (mManual.isDst ? 4 : 0));
      }
    }

    /** Return true if the time zone observes DST at epochSeconds. */
    bool getDst(uint32_t epochSeconds) const {
      if (mType == kTypeAuto) {
        return mAuto.zoneAgent == nullptr
            ? false
            : mAuto.zoneAgent->isDst(epochSeconds);
      } else {
        return mManual.isDst;
      }
    }

    /** Return the abbreviation of the time zone. */
    const char* getAbbrev(uint32_t epochSeconds) const {
      if (mType == kTypeAuto) {
        return mAuto.zoneAgent == nullptr
            ? "" // TODO: Should this be "UTC"?
            : mAuto.zoneAgent->getAbbrev(epochSeconds);
      } else {
        const char* abbrev = mManual.isDst
            ? mManual.dstAbbrev
            : mManual.stdAbbrev;
        return (abbrev == nullptr) ? "" : abbrev;
      }
    }

    /** Print the human readable representation of the time zone. */
    void printTo(Print& printer) const;

    /** Return the base UTC offset. Valid only for kTypeManual. */
    UtcOffset utcOffset() const { return mManual.utcOffset; }

    /** Return a mutable base UTC offset. Valid only for kTypeManual. */
    UtcOffset& utcOffset() { return mManual.utcOffset; }

    /**
     * Set the base offset without regards to the DST setting. Valid only for
     * kTypeManual.
     */
    void utcOffset(UtcOffset utcOffset) {
      mManual.utcOffset = utcOffset;
    }

    /** Return the base isDst flag. Valid only for kTypeManual. */
    bool isDst() const { return mManual.isDst; }

    /** Set the base isDst flag. Valid only for kTypeManual. */
    void isDst(bool isDst) { mManual.isDst = isDst; }

    /**
     * Return the standard abbreviation. Nullable. Valid only for kTypeManual.
     */
    const char* stdAbbrev() const { return mManual.stdAbbrev; }

    /** Return the DST abbreviation. Nullable. Valid only for kTypeManual. */
    const char* dstAbbrev() const { return mManual.dstAbbrev; }

  private:
    friend bool operator==(const TimeZone& a, const TimeZone& b);

    /** Length of UTC offset string (e.g. "-07:00", "+01:30"). */
    static const uint8_t kUtcOffsetStringLength = 6;

    /** Convert offsetString to offsetCode. */
    static void parseFromOffsetString(const char* offsetString,
        uint8_t* offsetCode);

    /** Constructor for kTypeAuto */
    explicit TimeZone(ZoneAgent* zoneAgent):
        mType(kTypeAuto),
        mAuto{zoneAgent} {}

    /** Constructor for kTypeManual. */
    explicit TimeZone(UtcOffset utcOffset, bool isDst,
            const char* stdAbbrev, const char* dstAbbrev):
        mType(kTypeManual),
        mManual{utcOffset, isDst, stdAbbrev, dstAbbrev} {}

    /** Type of time zone. */
    uint8_t mType;

    /** Union of Auto and Manual time zone fields. */
    union {
      internal::ManualZone mManual;
      internal::AutoZone mAuto;
    };
};

inline bool operator==(const TimeZone& a, const TimeZone& b) {
  if (a.mType != b.mType) return false;
  if (a.mType == TimeZone::kTypeAuto) {
    return a.mAuto.zoneAgent->getZoneInfo() == b.mAuto.zoneAgent->getZoneInfo();
  } else {
    return a.mManual.utcOffset == b.mManual.utcOffset
        && a.mManual.isDst == b.mManual.isDst
        && a.mManual.stdAbbrev == b.mManual.stdAbbrev
        && a.mManual.dstAbbrev == b.mManual.dstAbbrev;
  }
}

inline bool operator!=(const TimeZone& a, const TimeZone& b) {
  return ! (a == b);
}

}

#endif
