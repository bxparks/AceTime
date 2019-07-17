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
        mSystemClock(systemClock)
      #if ENABLE_TIME_ZONE_TYPE_BASIC
        , mBasicZoneManager(kBasicZoneRegistry, kBasicZoneRegistrySize)
        , mBasicZoneSpecifier(&zonedb::kZoneAmerica_Los_Angeles)
      #endif
      #if ENABLE_TIME_ZONE_TYPE_EXTENDED
        , mExtendedZoneManager(kExtendedZoneRegistry,
            kExtendedZoneRegistrySize)
        , mExtendedZoneSpecifier(&zonedbx::kZoneAmerica_Los_Angeles)
      #endif
    {}

    void setup() {
      mIsStoredInfoValid = mPersistentStore.readStoredInfo(mStoredInfo);

      if (mIsStoredInfoValid) {
        restoreInfo();
      } else {
#if TIME_SOURCE_TYPE == TIME_SOURCE_TYPE_NTP
        mStoredInfo.ssid[0] = '\0';
        mStoredInfo.password[0] = '\0';
#endif
        setBasicTimeZone();
      }
    }

    /** Set the time zone to the given offset using kTypeManual. */
    void setManualTimeZone(TimeOffset stdOffset, TimeOffset dstOffset) {
      mTimeZone = TimeZone::forTimeOffset(stdOffset, dstOffset);
      preserveInfo();
    }

    /** Set the DST setting of Manual TimeZone. */
    void setDst(bool isDst) {
      mTimeZone.setDstOffset(TimeOffset::forHour(isDst ? 1 : 0));
      preserveInfo();
    }

  #if ENABLE_TIME_ZONE_TYPE_BASIC
    /** Set the time zone to America/Los_Angeles using BasicZoneSpecifier. */
    void setBasicTimeZone(uint16_t zoneIndex = 0) {
      mZoneIndex = zoneIndex;
      const basic::ZoneInfo* zoneInfo =
          mBasicZoneManager.getZoneInfo(zoneIndex);
      if (zoneInfo == nullptr) zoneInfo = &zonedb::kZoneAmerica_Los_Angeles;
      mBasicZoneSpecifier.setZoneInfo(zoneInfo);
      mTimeZone = TimeZone::forZoneSpecifier(&mBasicZoneSpecifier);
      preserveInfo();
    }
  #endif

  #if ENABLE_TIME_ZONE_TYPE_EXTENDED
    /** Set the time zone to America/Los_Angeles using ExtendedZoneSpecifier. */
    void setExtendedTimeZone(uint16_t zoneIndex = 0) {
      mZoneIndex = zoneIndex;
      const extended::ZoneInfo* zoneInfo =
          mExtendedZoneManager.getZoneInfo(zoneIndex);
      if (zoneInfo == nullptr) zoneInfo = &zonedbx::kZoneAmerica_Los_Angeles;
      mExtendedZoneSpecifier.setZoneInfo(zoneInfo);
      mTimeZone = TimeZone::forZoneSpecifier(&mExtendedZoneSpecifier);
      preserveInfo();
    }
  #endif

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

  #if ENABLE_TIME_ZONE_TYPE_BASIC
    /** Print list of supported zones. */
    void printBasicZonesTo(Print& printer) const {
      uint16_t registrySize = mBasicZoneManager.registrySize();
      for (uint16_t i = 0; i < registrySize; i++) {
        printer.print('[');
        printer.print(i);
        printer.print(']');
        printer.print(' ');
        const basic::ZoneInfo* zoneInfo = mBasicZoneManager.getZoneInfo(i);
        printer.println(BasicZone(zoneInfo).name());
      }
    }
  #endif

  #if ENABLE_TIME_ZONE_TYPE_EXTENDED
    /** Print list of supported zones. */
    void printExtendedZonesTo(Print& printer) const {
      uint16_t registrySize = mExtendedZoneManager.registrySize();
      for (uint16_t i = 0; i < registrySize; i++) {
        printer.print('[');
        printer.print(i);
        printer.print(']');
        printer.print(' ');
        const extended::ZoneInfo* zoneInfo =
            mExtendedZoneManager.getZoneInfo(i);
        printer.println(ExtendedZone(zoneInfo).name());
      }
    }
  #endif

  private:
  #if ENABLE_TIME_ZONE_TYPE_BASIC
    static const basic::ZoneInfo* const kBasicZoneRegistry[]
        ACE_TIME_BASIC_PROGMEM;
    static const uint16_t kBasicZoneRegistrySize;
  #endif
  #if ENABLE_TIME_ZONE_TYPE_EXTENDED
    static const extended::ZoneInfo* const kExtendedZoneRegistry[]
        ACE_TIME_EXTENDED_PROGMEM;
    static const uint16_t kExtendedZoneRegistrySize;
  #endif

    uint16_t preserveInfo() {
      mIsStoredInfoValid = true;
      TimeZoneData tzd = mTimeZone.toTimeZoneData();
      mStoredInfo.timeZoneType = tzd.type;
      mStoredInfo.stdOffsetCode = tzd.stdOffsetCode;
      mStoredInfo.dstOffsetCode = tzd.dstOffsetCode;
      // TODO: Replace mZoneIndex with info from TimeZoneData
      mStoredInfo.zoneIndex = mZoneIndex;
      return mPersistentStore.writeStoredInfo(mStoredInfo);
    }

    void restoreInfo() {
      switch (mStoredInfo.timeZoneType) {
      #if ENABLE_TIME_ZONE_TYPE_BASIC
        case TimeZone::kTypeBasic:
          setBasicTimeZone(mStoredInfo.zoneIndex);
          break;
      #endif
      #if ENABLE_TIME_ZONE_TYPE_EXTENDED
        case TimeZone::kTypeExtended:
          setExtendedTimeZone(mStoredInfo.zoneIndex);
          break;
      #endif
        default:
          setManualTimeZone(
              TimeOffset::forOffsetCode(mStoredInfo.stdOffsetCode),
              TimeOffset::forOffsetCode(mStoredInfo.dstOffsetCode));
      }
    }

    PersistentStore& mPersistentStore;
    SystemClock& mSystemClock;
    TimeZone mTimeZone;

  #if ENABLE_TIME_ZONE_TYPE_BASIC
    BasicZoneManager mBasicZoneManager;
    BasicZoneSpecifier mBasicZoneSpecifier;
  #endif
  #if ENABLE_TIME_ZONE_TYPE_EXTENDED
    ExtendedZoneManager mExtendedZoneManager;
    ExtendedZoneSpecifier mExtendedZoneSpecifier;
  #endif
    uint16_t mZoneIndex = 0;

    StoredInfo mStoredInfo;
    bool mIsStoredInfoValid = false;
};

#endif
