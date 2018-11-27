#ifndef COMMAND_LINE_CLOCK_CONTROLLER_H
#define COMMAND_LINE_CLOCK_CONTROLLER_H

#include <AceTime.h>
#include "config.h"
#include "PersistentStore.h"
#include "StoredInfo.h"

using namespace ace_time;
using namespace ace_time::provider;

class Controller {
  public:
    Controller(PersistentStore& persistentStore, TimeKeeper& systemTimeKeeper):
        mPersistentStore(persistentStore),
        mSystemTimeKeeper(systemTimeKeeper) {}

    void setup() {
      mIsStoredInfoValid = mPersistentStore.readStoredInfo(mStoredInfo);
      mCurrentTimeZone = &mManualTimeZone;

      if (mIsStoredInfoValid) {
        if (mStoredInfo.timeZoneType == TimeZone::kTypeAuto) {
          AutoTimeZone tz = AutoTimeZone::forZone(&zonedb::kZoneLos_Angeles);
          setAutoTimeZone(tz);
        } else {
          ManualTimeZone tz = ManualTimeZone::forUtcOffset(
              UtcOffset::forOffsetCode(mStoredInfo.offsetCode),
              mStoredInfo.isDst);
          setManualTimeZone(tz);
        }
      } else {
        AutoTimeZone tz = AutoTimeZone::forZone(&zonedb::kZoneLos_Angeles);
        setAutoTimeZone(tz);
      }

      // TODO: Set the ssid and password to some initial blank state, so that
      // the user can be notified that they need to be provided.
    }

    /** Set the time zone of the clock and preserve it. */
    void setManualTimeZone(const ManualTimeZone& timeZone) {
      mManualTimeZone = timeZone;
      mCurrentTimeZone = &mManualTimeZone;
      preserveInfo();
    }

    /** Set the time zone of the clock and preserve it. */
    void setAutoTimeZone(const AutoTimeZone& timeZone) {
      mAutoTimeZone = timeZone;
      mCurrentTimeZone = &mAutoTimeZone;
      preserveInfo();
    }

    /** Return the current time zone. */
    const TimeZone* getTimeZone() const { return mCurrentTimeZone; }

#if TIME_SOURCE_TYPE == TIME_SOURCE_TYPE_NTP
    /**
     * Set the wifi credentials and setup the NtpTimeProvider.
     * Return the number of bytes written.
     */
    uint16_t setWiFi(const char* ssid, const char* password) {
      strncpy(mStoredInfo.ssid, ssid, StoredInfo::kSsidMaxLength);
      mStoredInfo.ssid[StoredInfo::kSsidMaxLength - 1] = '\0';
      strncpy(mStoredInfo.password, password, StoredInfo::kPasswordMaxLength);
      mStoredInfo.password[StoredInfo::kPasswordMaxLength - 1] = '\0';
      return preserveInfo();
    }
#endif

    /** Set the current time of the system time keeper. */
    void setNow(const DateTime& newDateTime) {
      uint32_t seconds = newDateTime.toEpochSeconds();
      mSystemTimeKeeper.setNow(seconds);
    }

    /** Return the current time from the system time keeper. */
    DateTime getNow() const {
      return DateTime::forEpochSeconds(
          mSystemTimeKeeper.getNow(), mCurrentTimeZone);
    }

    /** Return true if the initial setup() retrieved a valid storedInfo. */
    bool isStoredInfoValid() const { return mIsStoredInfoValid; }

    /** Return the stored info. */
    const StoredInfo& getStoredInfo() const { return mStoredInfo; }

    /** Return DST mode. */
    bool isDst() const { return mManualTimeZone.isDst(); }

    /** Set DST on or off */
    void setDst(bool status) {
      mManualTimeZone.isDst(status);
      preserveInfo();
    }

    /**
     * Implement the 'modify' command, which copies the current DateTime to
     * mChangingDateTime.
     */
    void modifyDateTime() {
      DateTime dt = getNow();
      mChangingDateTime = dt;
      mInModifyMode = true;
    }

    bool inModifyMode() const { return mInModifyMode; }

    /** Return reference to mChangingDateTime. */
    DateTime& getChangingDateTime() { return mChangingDateTime; }

    /** Save the current mChangingDateTime to system time. */
    void saveDateTime() {
      setNow(mChangingDateTime);
      mInModifyMode = false;
    }

  private:
    uint16_t preserveInfo() {
      mIsStoredInfoValid = true;
      mStoredInfo.timeZoneType = mCurrentTimeZone->getType();
      mStoredInfo.offsetCode = mManualTimeZone.utcOffset().toOffsetCode();
      mStoredInfo.isDst = mManualTimeZone.isDst();
      return mPersistentStore.writeStoredInfo(mStoredInfo);
    }

    PersistentStore& mPersistentStore;
    TimeKeeper& mSystemTimeKeeper;
    DateTime mChangingDateTime;
    ManualTimeZone mManualTimeZone;
    AutoTimeZone mAutoTimeZone;
    TimeZone* mCurrentTimeZone; // points to mManualTimeZone or mAutoTimeZone

    StoredInfo mStoredInfo;
    bool mIsStoredInfoValid = false;
    bool mInModifyMode = false;
};

#endif
