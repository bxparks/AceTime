#ifndef ACE_TIME_MANUAL_ZONE_SPECIFIER_H
#define ACE_TIME_MANUAL_ZONE_SPECIFIER_H

#include <string.h> // strcmp()
#include "UtcOffset.h"
#include "ZoneSpecifier.h"

namespace ace_time {

/**
 * An implementation of ZoneSpecifier which allows the user to manually adjust
 * the UTC offset and the DST flag. Unlike BasicZoneSpecifier and
 * ExtendedZoneSpecifier, this class is mutable and copyable to allow the
 * application to detect changes to the utcOffset made by the user.
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
     * with epochSeconds. The internall isDst flag is set to 'false' initially,
     * and can be changed using the isDst(bool) mutator.
     *
     * @param stdOffset base offset of the zone (required), can be changed using
     *        the stdOffset(UtcOffset) mutator.
     * @param deltaOffset additional UTC offset during DST time (required),
     *        cannot be changed after construction
     * @param stdAbbrev time zone abbreviation during normal time (default ""),
     *        cannot be changed after construction
     * @param dstAbbrev time zone abbreviation during DST time (default ""),
     *        cannot be changed after construction
     */
    explicit ManualZoneSpecifier(UtcOffset stdOffset, UtcOffset deltaOffset,
        const char* stdAbbrev = "", const char* dstAbbrev = ""):
      ZoneSpecifier(kTypeManual),
      mStdOffset(stdOffset),
      mDeltaOffset(deltaOffset),
      mStdAbbrev(stdAbbrev),
      mDstAbbrev(dstAbbrev) {}

    /** Default copy constructor. */
    ManualZoneSpecifier(const ManualZoneSpecifier&) = default;

    /** Default assignment operator. */
    ManualZoneSpecifier& operator=(const ManualZoneSpecifier&) = default;

    /** Singleton instance of a UTC ZoneSpecifier. */
    static const ManualZoneSpecifier sUtcZoneSpecifier;

    /** Get the standard UTC offset. */
    UtcOffset stdOffset() const { return mStdOffset; }

    /** Get the standard abbreviation. */
    const char* stdAbbrev() const { return mStdAbbrev; }

    /** Get the DST delta offset. */
    UtcOffset deltaOffset() const { return mDeltaOffset; }

    /** Get the DST abbreviation. */
    const char* dstAbbrev() const { return mDstAbbrev; }

    /** Get the current isDst flag. */
    bool isDst() const { return mIsDst; }

    /**
     * Set the standard UTC offset. There are currently 2 use-cases for this:
     *
     * 1) ZonedDateTime:;forDateString() uses this to convert the string
     * representation of the UTC offset and store it in a ManualZoneSpecifier
     * through this method.
     *
     * 2) This can be used by applications that allow the user to select a
     * particular UTC offset. It seems unrealistic to expect the user to know
     * the standard and DST timezone abbreviations, so I have not exposed
     * methods to change those fields.
     */
    void stdOffset(UtcOffset offset) { mStdOffset = offset; }

    /**
     * Set the current isDst flag. This is expected to be used by applications
     * that allow the user to manually select the DST flag.
     */
    void isDst(bool isDst) { mIsDst = isDst; }

    UtcOffset getUtcOffset(acetime_t /*epochSeconds*/) const override {
      return mIsDst
        ? UtcOffset::forOffsetCode(mStdOffset.code() + mDeltaOffset.code())
        : mStdOffset;
    }

    UtcOffset getDeltaOffset(acetime_t /*epochSeconds*/) const override {
      return mIsDst ? mDeltaOffset : UtcOffset();
    }

    const char* getAbbrev(acetime_t /*epochSeconds*/) const override {
      return mIsDst ? mDstAbbrev : mStdAbbrev;
    }

    UtcOffset getUtcOffsetForDateTime(const LocalDateTime&) const override {
      return getUtcOffset(0);
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
