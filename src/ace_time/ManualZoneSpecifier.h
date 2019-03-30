#ifndef ACE_TIME_MANUAL_ZONE_SPECIFIER_H
#define ACE_TIME_MANUAL_ZONE_SPECIFIER_H

#include <string.h> // strcmp()
#include "UtcOffset.h"
#include "ZoneSpecifier.h"

namespace ace_time {

/**
 * An implementation of ZoneSpecifier which allows the user to manually adjust
 * the UTC offset and the DST flag. Unlike other implementations of
 * ZoneSpecifier, this class is mutable and is expected to be copied during
 * normal operation.
 */
class ManualZoneSpecifier: public ZoneSpecifier {
  public:
    /**
     * Default constructor describes the UTC+00:00 time zone with no DST.
     * The abbreviations are set to "UTC".
     */
    explicit ManualZoneSpecifier():
      ZoneSpecifier(kTypeManual),
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
      ZoneSpecifier(kTypeManual),
      mStdOffset(stdOffset),
      mDeltaOffset(deltaOffset),
      mStdAbbrev(stdAbbrev),
      mDstAbbrev(dstAbbrev) {}

    /** Default copy constructor. */
    ManualZoneSpecifier(const ManualZoneSpecifier& that) = default;

    /** Default assignment operator. */
    ManualZoneSpecifier& operator=(const ManualZoneSpecifier& that) = default;

    /** Singleton instance of a UTC ZoneSpecifier. */
    static ManualZoneSpecifier sUtcZoneSpecifier;

    UtcOffset stdOffset() const { return mStdOffset; }

    const char* stdAbbrev() const { return mStdAbbrev; }

    UtcOffset deltaOffset() const { return mDeltaOffset; }

    const char* dstAbbrev() const { return mDstAbbrev; }

    /** Return the base isDst flag. Valid only for ManualZoneSpecifier. */
    bool isDst() const { return mIsDst; }

    /** Return the reference to stdOffset to allow changing it. */
    UtcOffset& stdOffset() { return mStdOffset; }

    /** Set the base isDst flag. Valid only for ManualZoneSpecifier. */
    void isDst(bool isDst) { mIsDst = isDst; }

    UtcOffset getUtcOffset(acetime_t /*epochSeconds*/) const override {
      return mIsDst
        ? UtcOffset::forOffsetCode(mStdOffset.code() + mDeltaOffset.code())
        : mStdOffset;
    }

    /** Return the DST delta offset after accounting for mIsDst flag. */
    UtcOffset getDeltaOffset(acetime_t /*epochSeconds*/) const {
      return mIsDst ? mDeltaOffset : UtcOffset();
    }

    const char* getAbbrev(acetime_t /*epochSeconds*/) const override {
      return mIsDst ? mDstAbbrev : mStdAbbrev;
    }

    void printTo(Print& printer) const override;

  private:
    bool equals(const ZoneSpecifier& other) const override {
      const auto& that = (const ManualZoneSpecifier&) other;
      return isDst() == that.isDst()
          && stdOffset() == that.stdOffset()
          && deltaOffset() == that.deltaOffset()
          && strcmp(stdAbbrev(), that.stdAbbrev()) == 0
          && strcmp(dstAbbrev(), that.dstAbbrev()) == 0;
    }

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

}

#endif
