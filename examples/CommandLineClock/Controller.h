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
        , mBasicZoneManager(kBasicZoneRegistrySize, kBasicZoneRegistry)
      #endif
      #if ENABLE_TIME_ZONE_TYPE_EXTENDED
        , mExtendedZoneManager(
            kExtendedZoneRegistrySize, kExtendedZoneRegistry)
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
        setBasicTimeZoneForIndex();
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
    void setBasicTimeZoneForIndex(uint16_t zoneIndex = 0) {
      mZoneIndex = zoneIndex;
      mTimeZone = mBasicZoneManager.createForZoneIndex(zoneIndex);
      if (mTimeZone.isError()) {
        mTimeZone = mBasicZoneManager.createForZoneInfo(
            &zonedb::kZoneAmerica_Los_Angeles);
        mZoneIndex = 0;
      }
      preserveInfo();
    }
  #endif

  #if ENABLE_TIME_ZONE_TYPE_EXTENDED
    /** Set the time zone to America/Los_Angeles using ExtendedZoneSpecifier. */
    void setExtendedTimeZoneForIndex(uint16_t zoneIndex = 0) {
      mZoneIndex = zoneIndex;
      mTimeZone = mExtendedZoneManager.createForZoneIndex(zoneIndex);
      if (mTimeZone.isError()) {
        mTimeZone = mExtendedZoneManager.createForZoneInfo(
            &zonedbx::kZoneAmerica_Los_Angeles);
        mZoneIndex = 0;
      }
      preserveInfo();
    }
  #endif

  #if ENABLE_TIME_ZONE_TYPE_BASIC
    void setBasicTimeZoneForId(uint32_t zoneId) {
      mZoneIndex = mBasicZoneManager.indexForZoneId(zoneId);
      preserveInfo();
    }
  #endif

  #if ENABLE_TIME_ZONE_TYPE_EXTENDED
    void setExtendedTimeZoneForId(uint32_t zoneId) {
      mZoneIndex = mExtendedZoneManager.indexForZoneId(zoneId);
      preserveInfo();
    }
  #endif

    /** Return the current time zone. */
    TimeZone& getTimeZone() { return mTimeZone; }

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
      const BasicZoneRegistrar& registrar = mBasicZoneManager.getRegistrar();
      for (uint16_t i = 0; i < registrar.registrySize(); i++) {
        printer.print('[');
        printer.print(i);
        printer.print(']');
        printer.print(' ');
        const basic::ZoneInfo* zoneInfo = registrar.getZoneInfoForIndex(i);
        printer.println(BasicZone(zoneInfo).name());
      }
    }
  #endif

  #if ENABLE_TIME_ZONE_TYPE_EXTENDED
    /** Print list of supported zones. */
    void printExtendedZonesTo(Print& printer) const {
      const ExtendedZoneRegistrar& registrar =
          mExtendedZoneManager.getRegistrar();
      for (uint16_t i = 0; i < registrar.registrySize(); i++) {
        printer.print('[');
        printer.print(i);
        printer.print(']');
        printer.print(' ');
        const extended::ZoneInfo* zoneInfo = registrar.getZoneInfoForIndex(i);
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
      mStoredInfo.timeZoneType = mTimeZone.getType();
      mStoredInfo.stdOffsetCode =
          mTimeZone.getStdOffset().toOffsetCode();
      mStoredInfo.dstOffsetCode =
          mTimeZone.getDstOffset().toOffsetCode();
      mStoredInfo.zoneId = mTimeZone.getZoneId();
      return mPersistentStore.writeStoredInfo(mStoredInfo);
    }

    void restoreInfo() {
      switch (mStoredInfo.timeZoneType) {
        case TimeZone::kTypeManaged:
        case TimeZone::kTypeBasic:
        case TimeZone::kTypeExtended:
      #if ENABLE_TIME_ZONE_TYPE_BASIC
          setBasicTimeZoneForId(mStoredInfo.zoneId);
      #elif ENABLE_TIME_ZONE_TYPE_EXTENDED
          setExtendedTimeZoneForId(mStoredInfo.zoneId);
      #else
          setManualTimeZone(TimeOffset::forHour(-8), TimeOffset())
      #endif
          break;
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
    BasicZoneManager<1> mBasicZoneManager;;
  #endif
  #if ENABLE_TIME_ZONE_TYPE_EXTENDED
    ExtendedZoneManager<1> mExtendedZoneManager;;
  #endif
    uint16_t mZoneIndex = 0;

    StoredInfo mStoredInfo;
    bool mIsStoredInfoValid = false;
};

#endif
