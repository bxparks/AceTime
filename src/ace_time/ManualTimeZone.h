#ifndef ACE_TIME_MANUAL_TIME_ZONE_H
#define ACE_TIME_MANUAL_TIME_ZONE_H

#include <stdint.h>
#include "UtcOffset.h"
#include "TimeZone.h"

class Print;

namespace ace_time {

/**
 * Class that represents a fixed offset from UTC, with an optional DST flag.
 * The class is designed to be mutable to allow the user to configure the UTC
 * offset and DST flag. But a single instance can be shared among multiple
 * DateTime instances. Therefore, if users are allowed to configure the time
 * zone interactively, it is recommended that 2 instances of ManualTimeZone are
 * used. One which is configured by the user, and the other which is assigned
 * to the internal clock.
 */
class ManualTimeZone: public TimeZone {
  public:
    /** Default UTC TimeZone instance. */
    static const ManualTimeZone sUtc;

    /**
     * Factory method. Create from UtcOffset.
     *
     * @param utcOffset offset from UTC
     * @param isDst true if DST is in effect
     * @param stdAbbrev abbreviation during standard time (e.g. "PST")
     * @param dstAbbrev abbreviation during DST time (e.g. "PDT")
     */
    static ManualTimeZone forUtcOffset(UtcOffset utcOffset,
        bool isDst = false, const char* stdAbbrev = nullptr,
        const char* dstAbbrev = nullptr) {
      return ManualTimeZone(utcOffset, isDst, stdAbbrev, dstAbbrev);
    }

    /** Factory method. Create from time zone string. */
    static ManualTimeZone forOffsetString(const char* ts) {
      uint8_t offsetCode;
      // TODO: write better time zone parser
      parseFromOffsetString(ts, &offsetCode);
      return ManualTimeZone::forUtcOffset(UtcOffset::forOffsetCode(offsetCode));
    }

    /** Default constructor creates the UTC time zone. */
    explicit ManualTimeZone():
        TimeZone(kTypeManual),
        mUtcOffset(),
        mIsDst(false),
        mStdAbbrev(nullptr),
        mDstAbbrev(nullptr) {}

    UtcOffset getUtcOffset(uint32_t /*epochSeconds*/) const override {
      return UtcOffset::forOffsetCode(
          mUtcOffset.toOffsetCode() + (mIsDst ? 4 : 0));
    }

    const char* getAbbrev(uint32_t /*epochSeconds*/) const override {
      const char* abbrev = mIsDst ? mDstAbbrev : mStdAbbrev;
      return (abbrev == nullptr) ? "" : abbrev;
    }

    bool getDst(uint32_t /*epochSeconds*/) const override {
      return mIsDst;
    }

    void printTo(Print& printer) const override;

    /** Return a read-only base UTC offset. */
    UtcOffset utcOffset() const { return mUtcOffset; }

    /** Return a mutable base UTC offset. */
    UtcOffset& utcOffset() { return mUtcOffset; }

    /** Set the base offset without regards to the DST setting. */
    void utcOffset(UtcOffset utcOffset) {
      mUtcOffset = utcOffset;
    }

    /** Return the base isDst flag. */
    bool isDst() const { return mIsDst; }

    /** Set the base isDst flag. */
    void isDst(bool isDst) { mIsDst = isDst; }

    /** Return the standard abbreviation. Nullable. */
    const char* stdAbbrev() const { return mStdAbbrev; }

    /** Return the DST abbreviation. Nullable. */
    const char* dstAbbrev() const { return mDstAbbrev; }

  private:
    /** Length of UTC offset string (e.g. "-07:00", "+01:30"). */
    static const uint8_t kUtcOffsetStringLength = 6;

    /** Set time zone from the given UTC offset string. */
    static void parseFromOffsetString(const char* offsetString,
        uint8_t* offsetCode);

    /** Constructor for kTypeManual. */
    explicit ManualTimeZone(UtcOffset utcOffset, bool isDst,
            const char* stdAbbrev, const char* dstAbbrev):
        TimeZone(kTypeManual),
        mUtcOffset(utcOffset),
        mIsDst(isDst),
        mStdAbbrev(stdAbbrev),
        mDstAbbrev(dstAbbrev) {}

    bool equals(const TimeZone& that) const override {
      const ManualTimeZone& other = static_cast<const ManualTimeZone&>(that);
      return mUtcOffset == other.mUtcOffset
          && mIsDst == other.mIsDst
          && mStdAbbrev == other.mStdAbbrev
          && mDstAbbrev == other.mDstAbbrev;
    }

    /** Offset from UTC. */
    UtcOffset mUtcOffset;

    /** Indicate whether Daylight Saving Time is in effect. */
    bool mIsDst;

    /** Time zone abbreviation for standard time, e.g. "PST". Nullable. */
    const char* mStdAbbrev;

    /** Time zone abbreviation for daylight time, e.g. "PDT". Nullable. */
    const char* mDstAbbrev;
};

}

#endif
