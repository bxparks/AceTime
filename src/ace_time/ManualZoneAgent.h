#ifndef ACE_TIME_MANUAL_ZONE_AGENT_H
#define ACE_TIME_MANUAL_ZONE_AGENT_H

#include "UtcOffset.h"
#include "ZoneAgent.h"

namespace ace_time {

class ManualZoneAgent: public ZoneAgent {
  public:
    explicit ManualZoneAgent():
      mStdOffset(),
      mStdAbbrev("UTC"),
      mDeltaOffset(),
      mDstAbbrev("UTC") {}

    explicit ManualZoneAgent(UtcOffset stdOffset,
        const char* stdAbbrev = "",
        UtcOffset deltaOffset = UtcOffset(),
        const char* dstAbbrev = ""):
      mStdOffset(stdOffset),
      mStdAbbrev(stdAbbrev),
      mDeltaOffset(deltaOffset),
      mDstAbbrev(dstAbbrev) {}

    UtcOffset stdOffset() const { return mStdOffset; }

    const char* stdAbbrev() const { return mStdAbbrev; }

    UtcOffset deltaOffset() const { return mDeltaOffset; }

    const char* dstAbbrev() const { return mDstAbbrev; }

    uint8_t getType() const override { return kTypeManual; }

    UtcOffset getUtcOffset(bool isDst) override {
      return isDst
        ? UtcOffset::forOffsetCode(mStdOffset.code() + mDeltaOffset.code())
        : mStdOffset;
    }

    UtcOffset getDeltaOffset(bool isDst) override {
      return isDst ? mDeltaOffset : UtcOffset();
    }

    const char* getAbbrev(bool isDst) override {
      return isDst ? mDstAbbrev : mStdAbbrev;
    }

  private:
    /** Offset from UTC. */
    UtcOffset mStdOffset;

    /** Time zone abbreviation for standard time, e.g. "PST". Nullable. */
    const char* mStdAbbrev;

    /** Additional offset when observing DST. */
    UtcOffset mDeltaOffset;

    /** Time zone abbreviation for daylight time, e.g. "PDT". Nullable. */
    const char* mDstAbbrev;
};

}

#endif
