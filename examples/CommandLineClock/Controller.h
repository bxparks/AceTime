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
        mSystemTimeKeeper(systemTimeKeeper),
        mBasicZoneSpecifier(&zonedb::kZoneAmerica_Los_Angeles) {}

    void setup() {
      mIsStoredInfoValid = mPersistentStore.readStoredInfo(mStoredInfo);

      if (mIsStoredInfoValid) {
        if (mStoredInfo.timeZoneType == TimeZone::kTypeZoneSpecifier) {
          setTimeZone();
        } else {
          setTimeZone(UtcOffset::forMinutes(mStoredInfo.offsetMinutes),
              mStoredInfo.isDst);
        }
      } else {
        setTimeZone();
      }

      // TODO: Set the ssid and password to some initial blank state, so that
      // the user can be notified that they need to be provided.
    }

    /** Set the time zone using the given offset. */
    void setTimeZone(UtcOffset utcOffset, bool isDst) {
      mManualZoneSpecifier = ManualZoneSpecifier(utcOffset, isDst);
      mTimeZone = TimeZone(&mManualZoneSpecifier);
      preserveInfo();
    }

    // TODO: If we want the user to be able to select from a menu of zones,
    // then setTimeZone() needs to take a ZoneInfo as an argument, and we need
    // to enable the assignment operator() for BasicZoneSpecifier or
    // ExtendedZoneSpecifier. Doable but makes those classes consume more
    // program memory.
    /** Set the time zone to America/Los_Angeles. */
    void setTimeZone() {
      mTimeZone = TimeZone(&mBasicZoneSpecifier);
      preserveInfo();
    }

    /** Return the current time zone. */
    const TimeZone& getTimeZone() const { return mTimeZone; }

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
    void setNow(acetime_t now) {
      mSystemTimeKeeper.setNow(now);
    }

    /** Return the current time from the system time keeper. */
    ZonedDateTime getNow() const {
      return ZonedDateTime::forEpochSeconds(
          mSystemTimeKeeper.getNow(), mTimeZone);
    }

    /** Return true if the initial setup() retrieved a valid storedInfo. */
    bool isStoredInfoValid() const { return mIsStoredInfoValid; }

    /** Return the stored info. */
    const StoredInfo& getStoredInfo() const { return mStoredInfo; }

    /** Return DST mode. */
    bool isDst() const { return mManualZoneSpecifier.isDst(); }

    /** Set DST on or off */
    void setDst(bool status) {
      mManualZoneSpecifier.isDst(status);
      preserveInfo();
    }

    /**
     * Implement the 'modify' command, which copies the current dateTime to
     * mChangingDateTime.
     */
    void modifyDateTime() {
      ZonedDateTime dt = getNow();
      mChangingDateTime = dt;
      mInModifyMode = true;
    }

    bool inModifyMode() const { return mInModifyMode; }

    /** Return reference to mChangingDateTime. */
    ZonedDateTime& getChangingDateTime() { return mChangingDateTime; }

    /** Save the current mChangingDateTime to system time. */
    void saveDateTime() {
      setNow(mChangingDateTime.toEpochSeconds());
      mInModifyMode = false;
    }

  private:
    uint16_t preserveInfo() {
      mIsStoredInfoValid = true;
      mStoredInfo.timeZoneType = mTimeZone.getType();
      mStoredInfo.offsetMinutes = mManualZoneSpecifier.stdOffset().toMinutes();
      mStoredInfo.isDst = mManualZoneSpecifier.isDst();
      return mPersistentStore.writeStoredInfo(mStoredInfo);
    }

    PersistentStore& mPersistentStore;
    TimeKeeper& mSystemTimeKeeper;
    ZonedDateTime mChangingDateTime;
    TimeZone mTimeZone;
    BasicZoneSpecifier mBasicZoneSpecifier;
    ManualZoneSpecifier mManualZoneSpecifier;

    StoredInfo mStoredInfo;
    bool mIsStoredInfoValid = false;
    bool mInModifyMode = false;
};

#endif
