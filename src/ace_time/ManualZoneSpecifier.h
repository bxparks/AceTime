#ifndef ACE_TIME_MANUAL_ZONE_SPECIFIER_H
#define ACE_TIME_MANUAL_ZONE_SPECIFIER_H

#include <string.h> // strcmp()
#include "TimeOffset.h"
#include "ZoneSpecifier.h"

namespace ace_time {

/**
 * An implementation of ZoneSpecifier which allows the user to manually adjust
 * the UTC offset and the DST flag. Unlike BasicZoneSpecifier and
 * ExtendedZoneSpecifier, this class is mutable and copyable to allow the
 * application to detect changes to the timeOffset made by the user.
 */
class ManualZoneSpecifier: public ZoneSpecifier {
  public:
    /**
     * Constructor for a time zone with an offset from UTC that does not change
     * with epochSeconds. The internall isDst flag is set to 'false' initially,
     * and can be changed using the isDst(bool) mutator.
     *
     * Of the 5 parameters in the constructor, only stdOffset and isDst are
     * mutable after construction. The mutators are expected to be called from
     * applications that allow the user to change the UTC offset and isDst
     * flags during runtime. The others are not exposed to be mutable because
     * it seems unrealistic to expect the user to know the standard and DST
     * timezone abbreviations.
     *
     * @param stdOffset base offset of the zone, can be changed using
     *        the stdOffset(TimeOffset) mutator (default: 00:00)
     * @param isDst true if DST shfit is active (default: false)
     * @param stdAbbrev time zone abbreviation during normal time. Cannot be
     *        nullptr. Cannot be changed after construction. (default: "")
     * @param dstAbbrev time zone abbreviation during DST time.
     *        Cannot be nullptr. Cannot be changed after construction.
     *        (default: "").
     * @param deltaOffset additional UTC offset during DST time.
     *        Cannot be changed after construction. (default: +01:00).
     */
    explicit ManualZoneSpecifier(
        TimeOffset stdOffset = TimeOffset(),
        bool isDst = false,
        const char* stdAbbrev = "",
        const char* dstAbbrev = "",
        TimeOffset deltaOffset = TimeOffset::forHour(1)):
      ZoneSpecifier(kTypeManual),
      mStdOffset(stdOffset),
      mIsDst(isDst),
      mStdAbbrev(stdAbbrev),
      mDstAbbrev(dstAbbrev),
      mDeltaOffset(deltaOffset) {}

    /** Default copy constructor. */
    ManualZoneSpecifier(const ManualZoneSpecifier&) = default;

    /** Default assignment operator. */
    ManualZoneSpecifier& operator=(const ManualZoneSpecifier&) = default;

    /** Get the standard UTC offset. */
    TimeOffset stdOffset() const { return mStdOffset; }

    /** Get the current isDst flag. */
    bool isDst() const { return mIsDst; }

    /** Get the standard abbreviation. */
    const char* stdAbbrev() const { return mStdAbbrev; }

    /** Get the DST abbreviation. */
    const char* dstAbbrev() const { return mDstAbbrev; }

    /** Get the DST delta offset. */
    TimeOffset deltaOffset() const { return mDeltaOffset; }

    /**
     * Set the standard UTC offset. This can be used by applications that allow
     * the user to select a particular UTC offset.
     */
    void stdOffset(TimeOffset offset) { mStdOffset = offset; }

    /**
     * Set the current isDst flag. This is expected to be used by applications
     * that allow the user to manually select the DST flag.
     */
    void isDst(bool isDst) { mIsDst = isDst; }

    TimeOffset getTimeOffset(acetime_t /*epochSeconds*/) const override {
      return mIsDst
        ? TimeOffset::forOffsetCode(
            // Note: Use toOffsetCode() because TimeOffset is currently
            // implemented using OffsetCodes. If that changes to minutes,
            // then this should use toMinutes().
            mStdOffset.toOffsetCode() + mDeltaOffset.toOffsetCode())
        : mStdOffset;
    }

    TimeOffset getDeltaOffset(acetime_t /*epochSeconds*/) const override {
      return mIsDst ? mDeltaOffset : TimeOffset();
    }

    const char* getAbbrev(acetime_t /*epochSeconds*/) const override {
      return mIsDst ? mDstAbbrev : mStdAbbrev;
    }

    TimeOffset getTimeOffsetForDateTime(const LocalDateTime&) const override {
      return getTimeOffset(0);
    }

    void printTo(Print& printer) const override;

  private:
    bool equals(const ZoneSpecifier& other) const override {
      const auto& that = (const ManualZoneSpecifier&) other;
      // These parameters are ordered in decreasing expected probability of
      // being different from the other.
      return isDst() == that.isDst()
          && stdOffset() == that.stdOffset()
          && deltaOffset() == that.deltaOffset()
          && strcmp(stdAbbrev(), that.stdAbbrev()) == 0
          && strcmp(dstAbbrev(), that.dstAbbrev()) == 0;
    }

    /** Offset from UTC. */
    TimeOffset mStdOffset;

    /** Set to true if DST is enabled, when using ManualZoneSpecifier. */
    bool mIsDst;

    /** Time zone abbreviation for standard time, e.g. "PST". Not Nullable. */
    const char* mStdAbbrev;

    /** Time zone abbreviation for daylight time, e.g. "PDT". Not Nullable. */
    const char* mDstAbbrev;

    /** Additional offset to add to mStdOffset when observing DST. */
    TimeOffset mDeltaOffset;
};

}

#endif
