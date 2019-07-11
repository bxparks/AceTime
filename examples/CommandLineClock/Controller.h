#ifndef COMMAND_LINE_CLOCK_CONTROLLER_H
#define COMMAND_LINE_CLOCK_CONTROLLER_H

#include <AceTime.h>
#include "config.h"
#include "PersistentStore.h"
#include "StoredInfo.h"

using namespace ace_time;
using namespace ace_time::clock;

class Controller {
  public:
    Controller(PersistentStore& persistentStore, SystemClock& systemClock):
        mPersistentStore(persistentStore),
        mSystemClock(systemClock),
        mZoneManager(kZoneRegistry, kZoneRegistrySize),
      #if TIME_ZONE_TYPE == TIME_ZONE_TYPE_BASIC
        mBasicZoneSpecifier(&zonedb::kZoneAmerica_Los_Angeles)
      #elif TIME_ZONE_TYPE == TIME_ZONE_TYPE_EXTENDED
        mExtendedZoneSpecifier(&zonedbx::kZoneAmerica_Los_Angeles)
      #endif
    {}

    void setup() {
      mIsStoredInfoValid = mPersistentStore.readStoredInfo(mStoredInfo);

      if (mIsStoredInfoValid) {
        restoreInfo();
      } else {
        setBasicTimeZone();
      }

      // TODO: Set the ssid and password to some initial blank state, so that
      // the user can be notified that they need to be provided.
    }

    /** Set the time zone to the given offset using Fixed type. */
    void setFixedTimeZone(TimeOffset timeOffset) {
      mTimeZone = TimeZone::forTimeOffset(timeOffset);
      preserveInfo();
    }

    /** Set the time zone to given offset using ManualZoneSpecifier. */
    void setManualTimeZone(TimeOffset timeOffset, bool isDst) {
      mManualZoneSpecifier = ManualZoneSpecifier(timeOffset, isDst);
      mTimeZone = TimeZone::forZoneSpecifier(&mManualZoneSpecifier);
      preserveInfo();
    }

    /** Set the time zone to America/Los_Angeles using BasicZoneSpecifier. */
    void setBasicTimeZone() {
  #if TIME_ZONE_TYPE == TIME_ZONE_TYPE_BASIC
      mTimeZone = TimeZone::forZoneSpecifier(&mBasicZoneSpecifier);
      preserveInfo();
  #endif
    }

    /** Set the time zone to America/Los_Angeles using ExtendedZoneSpecifier. */
    void setExtendedTimeZone() {
  #if TIME_ZONE_TYPE == TIME_ZONE_TYPE_EXTENDED
      mTimeZone = TimeZone::forZoneSpecifier(&mExtendedZoneSpecifier);
      preserveInfo();
  #endif
    }

    /** Set the DST setting of ManualZoneSpecifier. */
    void setDst(bool isDst) {
      mTimeZone.isDst(isDst);
      preserveInfo();
    }

    /** Return the current time zone. */
    const TimeZone& getTimeZone() const { return mTimeZone; }

#if TIME_SOURCE_TYPE == TIME_SOURCE_TYPE_NTP
    /** Set the wifi credentials and setup the NtpTimeProvider. */
    void setWiFi(const char* ssid, const char* password) {
      strncpy(mStoredInfo.ssid, ssid, StoredInfo::kSsidMaxLength);
      mStoredInfo.ssid[StoredInfo::kSsidMaxLength - 1] = '\0';
      strncpy(mStoredInfo.password, password, StoredInfo::kPasswordMaxLength);
      mStoredInfo.password[StoredInfo::kPasswordMaxLength - 1] = '\0';
      preserveInfo();
    }
#endif

    /** Set the current time of the system time keeper. */
    void setNow(acetime_t now) {
      mSystemClock.setNow(now);
    }

    /** Return the current time from the system time keeper. */
    ZonedDateTime getCurrentDateTime() const {
      return ZonedDateTime::forEpochSeconds(
          mSystemClock.getNow(), mTimeZone);
    }

    /** Return true if the initial setup() retrieved a valid storedInfo. */
    bool isStoredInfoValid() const { return mIsStoredInfoValid; }

    /** Return the stored info. */
    const StoredInfo& getStoredInfo() const { return mStoredInfo; }

    /** Return DST mode. */
    bool isDst() const { return mTimeZone.isDst(); }

    /** Force SystemClock to sync. */
    void sync() {
      mSystemClock.setup();
    }

    /** Print list of supported zones. */
    void printZonesTo(Print& printer) const {
      uint16_t registrySize = mZoneManager.registrySize();
      for (uint16_t i = 0; i < registrySize; i++) {
      #if TIME_ZONE_TYPE == TIME_ZONE_TYPE_BASIC
        const basic::ZoneInfo* zoneInfo = mZoneManager.getZoneInfo(i);
        printer.println(BasicZoneInfo(zoneInfo).name());
      #elif TIME_ZONE_TYPE == TIME_ZONE_TYPE_EXTENDED
        const extended::ZoneInfo* zoneInfo = mZoneManager.getZoneInfo(i);
        printer.println(ExtendedZoneInfo(zoneInfo).name());
      #endif
      }
    }

  private:
  #if TIME_ZONE_TYPE == TIME_ZONE_TYPE_BASIC
    static const basic::ZoneInfo* const kZoneRegistry[];
    static const uint16_t kZoneRegistrySize;
  #elif TIME_ZONE_TYPE == TIME_ZONE_TYPE_EXTENDED
    static const extended::ZoneInfo* const kZoneRegistry[];
    static const uint16_t kZoneRegistrySize;
  #endif

    uint16_t preserveInfo() {
      mIsStoredInfoValid = true;
      mStoredInfo.timeZoneType = mTimeZone.getType();
      mStoredInfo.offsetMinutes = mManualZoneSpecifier.stdOffset().toMinutes();
      mStoredInfo.isDst = mManualZoneSpecifier.isDst();
      return mPersistentStore.writeStoredInfo(mStoredInfo);
    }

    void restoreInfo() {
      if (mStoredInfo.timeZoneType == TimeZone::kTypeFixed) {
        setFixedTimeZone(TimeOffset::forMinutes(mStoredInfo.offsetMinutes));
      } else if (mStoredInfo.timeZoneType == TimeZone::kTypeManual) {
        setManualTimeZone(TimeOffset::forMinutes(mStoredInfo.offsetMinutes),
            mStoredInfo.isDst);
      } else if (mStoredInfo.timeZoneType == TimeZone::kTypeBasic) {
        setBasicTimeZone();
      } else if (mStoredInfo.timeZoneType == TimeZone::kTypeExtended) {
        setExtendedTimeZone();
      }
    }

    PersistentStore& mPersistentStore;
    SystemClock& mSystemClock;
    TimeZone mTimeZone;
    ManualZoneSpecifier mManualZoneSpecifier;

  #if TIME_ZONE_TYPE == TIME_ZONE_TYPE_BASIC
    BasicZoneManager mZoneManager;
    BasicZoneSpecifier mBasicZoneSpecifier;
  #elif TIME_ZONE_TYPE == TIME_ZONE_TYPE_EXTENDED
    ExtendedZoneManager mZoneManager;
    ExtendedZoneSpecifier mExtendedZoneSpecifier;
  #endif

    StoredInfo mStoredInfo;
    bool mIsStoredInfoValid = false;
};

#endif
