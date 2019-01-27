#ifndef ACE_TIME_MANUAL_ZONE_SPECIFIER_H
#define ACE_TIME_MANUAL_ZONE_SPECIFIER_H

#include <string.h> // strcmp()
#include "UtcOffset.h"
#include "ZoneSpecifier.h"

namespace ace_time {

class ManualZoneSpecifier: public ZoneSpecifier {
  public:
    /**
     * Default constructor describes the UTC+00:00 time zone with no DST.
     * The abbreviations are set to "UTC".
     */
    explicit ManualZoneSpecifier():
      mStdOffset(),
      mDeltaOffset(),
      mStdAbbrev("UTC"),
      mDstAbbrev("UTC") {}

    /**
     * Constructor for a time zone with an offset from UTC that does not change
     * with epochSeconds. The offset can change when the isDst flag is set.
     *
     * @param stdOffset base offset of the zone (required)
     * @param deltaOffset additional UTC offset during DST time
     * @param stdAbbrev time zone abbreviation during normal time (default "")
     * @param dstAbbrev time zone abbreviation during DST time (default "")
     */
    explicit ManualZoneSpecifier(UtcOffset stdOffset,
        UtcOffset deltaOffset,
        const char* stdAbbrev = "",
        const char* dstAbbrev = ""):
      mStdOffset(stdOffset),
      mDeltaOffset(deltaOffset),
      mStdAbbrev(stdAbbrev),
      mDstAbbrev(dstAbbrev) {}

    /** Singleton instance of a UTC ZoneSpecifier. */
    static ManualZoneSpecifier sUtcZoneSpecifier;

    UtcOffset stdOffset() const { return mStdOffset; }

    UtcOffset& stdOffset() { return mStdOffset; }

    const char* stdAbbrev() const { return mStdAbbrev; }

    UtcOffset deltaOffset() const { return mDeltaOffset; }

    const char* dstAbbrev() const { return mDstAbbrev; }

    /** Return the base isDst flag. Valid only for AutoZoneSpecifier. */
    bool isDst() const { return mIsDst; }

    /** Set the base isDst flag. Valid only for ManualZoneSpecifier. */
    void isDst(bool isDst) { mIsDst = isDst; }

    uint8_t getType() const override { return kTypeManual; }

    /** Return the UTC offset after accounting for mIsDst flag. */
    UtcOffset getUtcOffset() {
      return mIsDst
        ? UtcOffset::forOffsetCode(mStdOffset.code() + mDeltaOffset.code())
        : mStdOffset;
    }

    /** Return the DST delta offset after accounting for mIsDst flag. */
    UtcOffset getDeltaOffset() {
      return mIsDst ? mDeltaOffset : UtcOffset();
    }

    /** Return the time zone abbreviation after accounting for mIsDst flag. */
    const char* getAbbrev() {
      return mIsDst ? mDstAbbrev : mStdAbbrev;
    }

  private:
    friend bool operator==(const ManualZoneSpecifier& a,
        const ManualZoneSpecifier& b);

    /** Offset from UTC. */
    UtcOffset mStdOffset;

    /** Additional offset to add to mStdOffset when observing DST. */
    UtcOffset mDeltaOffset;

    /** Time zone abbreviation for standard time, e.g. "PST". Not Nullable. */
    const char* mStdAbbrev;

    /** Time zone abbreviation for daylight time, e.g. "PDT". Not Nullable. */
    const char* mDstAbbrev;

    /** Set to true if DST is enabled, when using ManualZoneSpecifier. */
    bool mIsDst = false;
};

inline bool operator==(const ManualZoneSpecifier& a,
    const ManualZoneSpecifier& b) {
  return a.isDst() == b.isDst()
      && a.stdOffset() == b.stdOffset()
      && a.deltaOffset() == b.deltaOffset()
      && strcmp(a.stdAbbrev(), b.stdAbbrev()) == 0
      && strcmp(a.dstAbbrev(), b.dstAbbrev()) == 0;
}

inline bool operator!=(const ManualZoneSpecifier& a,
    const ManualZoneSpecifier& b) {
  return ! (a == b);
}

}

#endif
