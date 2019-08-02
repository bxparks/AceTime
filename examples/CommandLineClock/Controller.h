#ifndef COMMAND_LINE_CLOCK_CONTROLLER_H
#define COMMAND_LINE_CLOCK_CONTROLLER_H

#include <AceRoutine.h>
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
        SERIAL_PORT_MONITOR.println(F("Found valid EEPROM info"));
        restoreInfo(mStoredInfo);
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
    /** Set the time zone to America/Los_Angeles using BasicZoneProcessor. */
    void setBasicTimeZoneForIndex(uint16_t zoneIndex = 0) {
      SERIAL_PORT_MONITOR.print(F("setBasicTimeZoneForIndex(): "));
      SERIAL_PORT_MONITOR.println(zoneIndex);
      mTimeZone = mBasicZoneManager.createForZoneIndex(zoneIndex);
      validateAndSaveTimeZone();
    }

    void setBasicTimeZoneForId(uint32_t zoneId) {
      mTimeZone = mBasicZoneManager.createForZoneId(zoneId);
      validateAndSaveTimeZone();
    }
  #endif

  #if ENABLE_TIME_ZONE_TYPE_EXTENDED
    /** Set the time zone to America/Los_Angeles using ExtendedZoneProcessor. */
    void setExtendedTimeZoneForIndex(uint16_t zoneIndex = 0) {
      SERIAL_PORT_MONITOR.print(F("setExtendedTimeZoneForIndex(): "));
      SERIAL_PORT_MONITOR.println(zoneIndex);
      mTimeZone = mExtendedZoneManager.createForZoneIndex(zoneIndex);
      validateAndSaveTimeZone();
    }

    void setExtendedTimeZoneForId(uint32_t zoneId) {
      mTimeZone = mExtendedZoneManager.createForZoneId(zoneId);
      validateAndSaveTimeZone();
    }
  #endif

    /** Return the current time zone. */
    TimeZone& getTimeZone() { return mTimeZone; }

  #if TIME_SOURCE_TYPE == TIME_SOURCE_TYPE_NTP
    /** Set the wifi credentials and setup the NtpClock. */
    void setWiFi(const char* ssid, const char* password) {
      strncpy(mStoredInfo.ssid, ssid, StoredInfo::kSsidMaxLength);
      mStoredInfo.ssid[StoredInfo::kSsidMaxLength - 1] = '\0';
      strncpy(mStoredInfo.password, password, StoredInfo::kPasswordMaxLength);
      mStoredInfo.password[StoredInfo::kPasswordMaxLength - 1] = '\0';
      preserveInfo();
    }
  #endif

    /** Set the current time of the system clock. */
    void setNow(acetime_t now) {
      mSystemClock.setNow(now);
    }

    /** Return the current time from the system clock. */
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
    void forceSync() {
      mSystemClock.forceSync();
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
    static const basic::ZoneInfo* const kBasicZoneRegistry[] ACE_TIME_PROGMEM;
    static const uint16_t kBasicZoneRegistrySize;
  #endif
  #if ENABLE_TIME_ZONE_TYPE_EXTENDED
    static const extended::ZoneInfo* const kExtendedZoneRegistry[]
        ACE_TIME_PROGMEM;
    static const uint16_t kExtendedZoneRegistrySize;
  #endif

    void validateAndSaveTimeZone() {
      if (mTimeZone.isError()) {
        mTimeZone = mBasicZoneManager.createForZoneInfo(
            &zonedb::kZoneAmerica_Los_Angeles);
      }
      preserveInfo();
    }

    uint16_t preserveInfo() {
      SERIAL_PORT_MONITOR.println(F("preserveInfo()"));
      mIsStoredInfoValid = true;
      mStoredInfo.timeZoneType = mTimeZone.getType();
      mStoredInfo.stdOffsetCode =
          mTimeZone.getStdOffset().toOffsetCode();
      mStoredInfo.dstOffsetCode =
          mTimeZone.getDstOffset().toOffsetCode();
      mStoredInfo.zoneId = mTimeZone.getZoneId();
      return mPersistentStore.writeStoredInfo(mStoredInfo);
    }

    void restoreInfo(const StoredInfo& storedInfo) {
      SERIAL_PORT_MONITOR.print(F("restoreInfo(): "));
      SERIAL_PORT_MONITOR.println(storedInfo.timeZoneType);
      switch (storedInfo.timeZoneType) {
        case TimeZone::kTypeBasic:
        case TimeZone::kTypeExtended:
        case TimeZone::kTypeBasicManaged:
        case TimeZone::kTypeExtendedManaged:
      #if ENABLE_TIME_ZONE_TYPE_BASIC
          setBasicTimeZoneForId(storedInfo.zoneId);
      #elif ENABLE_TIME_ZONE_TYPE_EXTENDED
          setExtendedTimeZoneForId(storedInfo.zoneId);
      #else
          setManualTimeZone(TimeOffset::forHour(-8), TimeOffset());
      #endif
          break;
        case TimeZone::kTypeManual:
          setManualTimeZone(
              TimeOffset::forOffsetCode(storedInfo.stdOffsetCode),
              TimeOffset::forOffsetCode(storedInfo.dstOffsetCode));
          break;
        default:
          SERIAL_PORT_MONITOR.print(F("restoreInfo(): Setting UTC timezone"));
          setManualTimeZone(TimeOffset(), TimeOffset());
      }
    }

    PersistentStore& mPersistentStore;
    SystemClock& mSystemClock;

  #if ENABLE_TIME_ZONE_TYPE_BASIC
    BasicZoneManager<1> mBasicZoneManager;;
  #endif
  #if ENABLE_TIME_ZONE_TYPE_EXTENDED
    ExtendedZoneManager<1> mExtendedZoneManager;;
  #endif

    TimeZone mTimeZone;
    StoredInfo mStoredInfo;
    bool mIsStoredInfoValid = false;
};

#endif
