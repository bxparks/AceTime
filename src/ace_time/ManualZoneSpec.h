#ifndef ACE_TIME_MANUAL_ZONE_SPEC_H
#define ACE_TIME_MANUAL_ZONE_SPEC_H

#include "UtcOffset.h"
#include "ZoneSpec.h"

namespace ace_time {

class ManualZoneSpec: public ZoneSpec {
  public:
    /**
     * Default constructor describes the UTC+00:00 time zone with no DST.
     * The abbreviations are set to "UTC".
     */
    explicit ManualZoneSpec():
      mStdOffset(),
      mStdAbbrev("UTC"),
      mDeltaOffset(),
      mDstAbbrev("UTC") {}

    /**
     * Constructor for a time zone with an offset from UTC that does not change
     * with epochSeconds. The offset can change when the isDst flag is set.
     *
     * @param stdOffset base offset of the zone (required)
     * @param stdAbbrev time zone abbreviation during normal time (default "")
     * @param deltaOffset additional UTC offset during DST time (default 0)
     * @param dstAbbrev time zone abbreviation during DST time (default "")
     */
    explicit ManualZoneSpec(UtcOffset stdOffset,
        const char* stdAbbrev = "",
        UtcOffset deltaOffset = UtcOffset(),
        const char* dstAbbrev = ""):
      mStdOffset(stdOffset),
      mStdAbbrev(stdAbbrev),
      mDeltaOffset(deltaOffset),
      mDstAbbrev(dstAbbrev) {}

    /** Singleton instance of a UTC ZoneSpec. */
    static ManualZoneSpec sUtcZoneSpec;

    UtcOffset stdOffset() const { return mStdOffset; }

    const char* stdAbbrev() const { return mStdAbbrev; }

    UtcOffset deltaOffset() const { return mDeltaOffset; }

    const char* dstAbbrev() const { return mDstAbbrev; }

    uint8_t getType() const override { return kTypeManual; }

    /** Return the UTC offset after accounting for isDst flag. */
    UtcOffset getUtcOffset(bool isDst) {
      return isDst
        ? UtcOffset::forOffsetCode(mStdOffset.code() + mDeltaOffset.code())
        : mStdOffset;
    }

    /** Return the DST delta offset after accounting for isDst flag. */
    UtcOffset getDeltaOffset(bool isDst) {
      return isDst ? mDeltaOffset : UtcOffset();
    }

    /** Return the time zone abbreviation after accounting for isDst flag. */
    const char* getAbbrev(bool isDst) {
      return isDst ? mDstAbbrev : mStdAbbrev;
    }

  private:
    /** Offset from UTC. */
    UtcOffset mStdOffset;

    /** Time zone abbreviation for standard time, e.g. "PST". Not Nullable. */
    const char* mStdAbbrev;

    /** Additional offset to add to mStdOffset when observing DST. */
    UtcOffset mDeltaOffset;

    /** Time zone abbreviation for daylight time, e.g. "PDT". Not Nullable. */
    const char* mDstAbbrev;
};

}

#endif
