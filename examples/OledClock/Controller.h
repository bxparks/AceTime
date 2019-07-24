#ifndef OLED_CLOCK_CONTROLLER_H
#define OLED_CLOCK_CONTROLLER_H

#include <AceTime.h>
#include "config.h"
#include "ClockInfo.h"
#include "RenderingInfo.h"
#include "StoredInfo.h"
#include "PersistentStore.h"
#include "Presenter.h"

using namespace ace_time;
using namespace ace_time::common;
using namespace ace_time::clock;

/**
 * Class responsible for rendering the RenderingInfo to the indicated display.
 * Different subclasses output to different types of displays. In an MVC
 * architecture, this would be the Controller. The Model would be the various
 * member variables in this class. The View layer are the various Presenter
 * classes.
 */
class Controller {
  public:
    static const uint16_t kStoredInfoEepromAddress = 0;

    static const int16_t kDefaultOffsetMinutes = -8*60; // UTC-08:00

    // Number of minutes to use for a DST offset.
    static const int16_t kDstOffsetMinutes = 60;

    /**
     * Constructor.
     * @param persistentStore stores objects into the EEPROM with CRC
     * @param timeKeeper source of the current time
     * @param presenter renders the date and time info to the screen
     */
    Controller(PersistentStore& persistentStore, TimeKeeper& timeKeeper,
            Presenter& presenter):
        mPersistentStore(persistentStore),
        mTimeKeeper(timeKeeper),
        mPresenter(presenter)
      #if TIME_ZONE_TYPE == TIME_ZONE_TYPE_BASIC \
          || TIME_ZONE_TYPE == TIME_ZONE_TYPE_EXTENDED
        , mZoneManager(kZoneRegistrySize, kZoneRegistry)
      #endif
    {}

    void setup() {
      // Restore from EEPROM to retrieve time zone.
      StoredInfo storedInfo;
      bool isValid = mPersistentStore.readStoredInfo(storedInfo);
    #if FORCE_INITIALIZE
      setupClockInfo();
    #else
      if (isValid) {
        restoreClockInfo(mClockInfo, storedInfo);
      } else {
        setupClockInfo();
      }
    #endif

      // Retrieve current time from TimeKeeper and set the current clockInfo.
      updateDateTime();
    }

    /**
     * This should be called every 0.1s to support blinking mode and to avoid
     * noticeable drift against the RTC which has a 1 second resolution.
     */
    void update() {
      if (mMode == MODE_UNKNOWN) return;
      if (mIsPreparingToSleep) return;
      updateDateTime();
      updateBlinkState();
      updateRenderingInfo();
      mPresenter.display();
    }

    void modeButtonPress() {
      #if ENABLE_SERIAL == 1
        SERIAL_PORT_MONITOR.println(F("modeButtonPress()"));
      #endif
      switch (mMode) {
        // Cycle through the 3 main screens.
        case MODE_DATE_TIME:
          mMode = MODE_TIME_ZONE;
          break;
        case MODE_TIME_ZONE:
          mMode = MODE_ABOUT;
          break;
        case MODE_ABOUT:
          mMode = MODE_DATE_TIME;
          break;

        // Cycle through the various changeable date time fields.
        case MODE_CHANGE_YEAR:
          mMode = MODE_CHANGE_MONTH;
          break;
        case MODE_CHANGE_MONTH:
          mMode = MODE_CHANGE_DAY;
          break;
        case MODE_CHANGE_DAY:
          mMode = MODE_CHANGE_HOUR;
          break;
        case MODE_CHANGE_HOUR:
          mMode = MODE_CHANGE_MINUTE;
          break;
        case MODE_CHANGE_MINUTE:
          mMode = MODE_CHANGE_SECOND;
          break;
        case MODE_CHANGE_SECOND:
          mMode = MODE_CHANGE_YEAR;
          break;

      #if TIME_ZONE_TYPE == TIME_ZONE_TYPE_MANUAL
        // kTypeManual
        case MODE_CHANGE_TIME_ZONE_OFFSET:
          mMode = MODE_CHANGE_TIME_ZONE_DST;
          break;
        case MODE_CHANGE_TIME_ZONE_DST:
          mMode = MODE_CHANGE_TIME_ZONE_OFFSET;
          break;
      #else
        // kTypeBasic or kTypeExtended
        case MODE_CHANGE_TIME_ZONE_NAME:
          mMode = MODE_CHANGE_TIME_ZONE_NAME;
          break;
      #endif
      }
    }

    void modeButtonLongPress() {
      #if ENABLE_SERIAL == 1
        SERIAL_PORT_MONITOR.println(F("modeButtonLongPress()"));
      #endif
      switch (mMode) {
        case MODE_DATE_TIME:
          mChangingClockInfo = mClockInfo;
          mSecondFieldCleared = false;
          mMode = MODE_CHANGE_YEAR;
          break;

        case MODE_CHANGE_YEAR:
        case MODE_CHANGE_MONTH:
        case MODE_CHANGE_DAY:
        case MODE_CHANGE_HOUR:
        case MODE_CHANGE_MINUTE:
        case MODE_CHANGE_SECOND:
          saveDateTime();
          mMode = MODE_DATE_TIME;
          break;

        case MODE_TIME_ZONE:
          mChangingClockInfo = mClockInfo;
        #if TIME_ZONE_TYPE == TIME_ZONE_TYPE_MANUAL
          mMode = MODE_CHANGE_TIME_ZONE_OFFSET;
        #else
          mMode = MODE_CHANGE_TIME_ZONE_NAME;
        #endif
          break;

      #if TIME_ZONE_TYPE == TIME_ZONE_TYPE_MANUAL
        case MODE_CHANGE_TIME_ZONE_OFFSET:
        case MODE_CHANGE_TIME_ZONE_DST:
      #else
        case MODE_CHANGE_TIME_ZONE_NAME:
      #endif
          saveClockInfo();
          mMode = MODE_TIME_ZONE;
          break;
      }
    }

    void changeButtonPress() {
      #if ENABLE_SERIAL == 1
        SERIAL_PORT_MONITOR.println(F("changeButtonPress()"));
      #endif
      switch (mMode) {
        // Switch 12/24 modes if in MODE_DATA_TIME
        case MODE_DATE_TIME:
          mClockInfo.hourMode ^= 0x1;
          preserveClockInfo(mClockInfo);
          break;

        case MODE_CHANGE_YEAR:
          mSuppressBlink = true;
          zoned_date_time_mutation::incrementYear(mChangingClockInfo.dateTime);
          break;
        case MODE_CHANGE_MONTH:
          mSuppressBlink = true;
          zoned_date_time_mutation::incrementMonth(mChangingClockInfo.dateTime);
          break;
        case MODE_CHANGE_DAY:
          mSuppressBlink = true;
          zoned_date_time_mutation::incrementDay(mChangingClockInfo.dateTime);
          break;
        case MODE_CHANGE_HOUR:
          mSuppressBlink = true;
          zoned_date_time_mutation::incrementHour(mChangingClockInfo.dateTime);
          break;
        case MODE_CHANGE_MINUTE:
          mSuppressBlink = true;
          zoned_date_time_mutation::incrementMinute(
              mChangingClockInfo.dateTime);
          break;
        case MODE_CHANGE_SECOND:
          mSuppressBlink = true;
          mChangingClockInfo.dateTime.second(0);
          mSecondFieldCleared = true;
          break;

      #if TIME_ZONE_TYPE == TIME_ZONE_TYPE_MANUAL
        case MODE_CHANGE_TIME_ZONE_OFFSET:
        {
          mSuppressBlink = true;
          auto offset = TimeOffset::forOffsetCode(
              mChangingClockInfo.timeZone.getStdOffsetCode());
          time_offset_mutation::increment15Minutes(offset);
          mChangingClockInfo.timeZone.setStdOffset(offset);
          break;
        }
        case MODE_CHANGE_TIME_ZONE_DST:
        {
          mSuppressBlink = true;
          uint8_t offsetCode = mChangingClockInfo.timeZone.getDstOffsetCode();
          offsetCode = (offsetCode == 0)
              ? TimeOffset::forMinutes(kDstOffsetMinutes).toOffsetCode()
              : 0;
          mChangingClockInfo.timeZone.setDstOffset(
              TimeOffset::forOffsetCode(offsetCode));
          break;
        }

      #else
        // Cycle through the zones in the registry
        case MODE_CHANGE_TIME_ZONE_NAME:
          mSuppressBlink = true;
          mZoneIndex++;
          if (mZoneIndex >= kZoneRegistrySize) {
            mZoneIndex = 0;
          }
          mChangingClockInfo.timeZone =
              mZoneManager.createForZoneIndex(mZoneIndex);
          mChangingClockInfo.dateTime =
              mChangingClockInfo.dateTime.convertToTimeZone(
                  mChangingClockInfo.timeZone);
          break;

      #endif
      }

      // Update the display right away to prevent jitters in the display when
      // the button is triggering RepeatPressed events.
      update();
    }

    void changeButtonRepeatPress() {
      changeButtonPress();
    }

    void changeButtonRelease() {
      switch (mMode) {
        case MODE_CHANGE_YEAR:
        case MODE_CHANGE_MONTH:
        case MODE_CHANGE_DAY:
        case MODE_CHANGE_HOUR:
        case MODE_CHANGE_MINUTE:
        case MODE_CHANGE_SECOND:
      #if TIME_ZONE_TYPE == TIME_ZONE_TYPE_MANUAL
        case MODE_CHANGE_TIME_ZONE_OFFSET:
        case MODE_CHANGE_TIME_ZONE_DST:
      #else
        case MODE_CHANGE_TIME_ZONE_NAME:
      #endif
          mSuppressBlink = false;
          break;
      }
    }

  private:
  #if TIME_ZONE_TYPE == TIME_ZONE_TYPE_BASIC
    static const basic::ZoneInfo* const kZoneRegistry[];
  #elif TIME_ZONE_TYPE == TIME_ZONE_TYPE_EXTENDED
    static const extended::ZoneInfo* const kZoneRegistry[];
  #endif
    static const uint16_t kZoneRegistrySize;

    void updateDateTime() {
      // TODO: It might be possible to track just the epochSeconds instead of
      // converting it to a ZonedDateTime at each iteration.

      mClockInfo.dateTime = ZonedDateTime::forEpochSeconds(
          mTimeKeeper.getNow(), mClockInfo.timeZone);

      // If in CHANGE mode, and the 'second' field has not been cleared,
      // update the mChangingDateTime.second field with the current second.
      switch (mMode) {
        case MODE_CHANGE_YEAR:
        case MODE_CHANGE_MONTH:
        case MODE_CHANGE_DAY:
        case MODE_CHANGE_HOUR:
        case MODE_CHANGE_MINUTE:
        case MODE_CHANGE_SECOND:
          if (!mSecondFieldCleared) {
            mChangingClockInfo.dateTime.second(mClockInfo.dateTime.second());
          }
          break;
      }
    }

    void updateBlinkState () {
      uint16_t now = millis();
      uint16_t duration = now - mBlinkCycleStartMillis;
      if (duration < 500) {
        mBlinkShowState = true;
      } else if (duration < 1000) {
        mBlinkShowState = false;
      } else {
        mBlinkCycleStartMillis = now;
      }
    }

    void updateRenderingInfo() {
      switch (mMode) {
        case MODE_DATE_TIME:
        case MODE_TIME_ZONE:
        case MODE_ABOUT:
          mPresenter.setRenderingInfo(mMode, mSuppressBlink, mBlinkShowState,
              mClockInfo);
          break;

        case MODE_CHANGE_YEAR:
        case MODE_CHANGE_MONTH:
        case MODE_CHANGE_DAY:
        case MODE_CHANGE_HOUR:
        case MODE_CHANGE_MINUTE:
        case MODE_CHANGE_SECOND:
      #if TIME_ZONE_TYPE == TIME_ZONE_TYPE_MANUAL
        case MODE_CHANGE_TIME_ZONE_OFFSET:
        case MODE_CHANGE_TIME_ZONE_DST:
      #else
        case MODE_CHANGE_TIME_ZONE_NAME:
      #endif
          mPresenter.setRenderingInfo(mMode, mSuppressBlink, mBlinkShowState,
              mChangingClockInfo);
          break;
      }
    }

    /** Save the current UTC dateTime to the RTC. */
    void saveDateTime() {
      mTimeKeeper.setNow(mChangingClockInfo.dateTime.toEpochSeconds());
    }

    /** Transfer info from ChangingClockInfo to ClockInfo. */
    void saveClockInfo() {
    #if ENABLE_SERIAL == 1
      SERIAL_PORT_MONITOR.println(F("saveClockInfo()"));
    #endif
      mClockInfo = mChangingClockInfo;
      preserveClockInfo(mClockInfo);
    }

    /** Save the clock info into EEPROM. */
    void preserveClockInfo(const ClockInfo& clockInfo) {
    #if ENABLE_SERIAL == 1
      SERIAL_PORT_MONITOR.println(F("preserveClockInfo()"));
    #endif
      StoredInfo storedInfo;
      storedInfo.hourMode = clockInfo.hourMode;
      storedInfo.timeZoneData = clockInfo.timeZone.toTimeZoneData();
      mPersistentStore.writeStoredInfo(storedInfo);
    }

    /** Restore clockInfo from storedInfo. */
    void restoreClockInfo(ClockInfo& clockInfo, const StoredInfo& storedInfo) {
    #if ENABLE_SERIAL == 1
      SERIAL_PORT_MONITOR.println(F("restoreClockInfo()"));
      SERIAL_PORT_MONITOR.print(F("hourMode: ")); SERIAL_PORT_MONITOR.println(storedInfo.hourMode);
      SERIAL_PORT_MONITOR.print(F("type: ")); SERIAL_PORT_MONITOR.println(storedInfo.timeZoneData.type);
      if (storedInfo.type == TimeZoneData::kTypeManual) {
        SERIAL_PORT_MONITOR.print(F("std: "));
        SERIAL_PORT_MONITOR.println(storedInfo.timeZoneData.stdOffsetCode);
        SERIAL_PORT_MONITOR.print(F("dst: "));
        SERIAL_PORT_MONITOR.println(storedInfo.timeZoneData.dstOffsetCode);
      } else if (storedInfo.type == TimeZoneData::kTypeZoneId) {
        SERIAL_PORT_MONITOR.print(F("zoneId: 0x")); SERIAL_PORT_MONITOR.println(storedInfo.zoneId, 16);
      } else {
        SERIAL_PORT_MONITOR.println(F("<error>"));
      }
    #endif
      clockInfo.hourMode = storedInfo.hourMode;

    #if TIME_ZONE_TYPE == TIME_ZONE_TYPE_MANUAL
      if (storedInfo.timeZoneData.type == TimeZoneData::kTypeManual) {
        clockInfo.timeZone = TimeZone::forTimeOffset(
            TimeOffset::forOffsetCode(storedInfo.stdOffsetCode),
            TimeOffset::forOffsetCode(storedInfo.dstOffsetCode));
      } else {
        clockInfo.timeZone = TimeZone::forTimeOffset(
            TimeOffset::forMinutes(kDefaultOffsetMinutes),
            TimeOffset());
      }
    #elif TIME_ZONE_TYPE == TIME_ZONE_TYPE_BASIC \
        || TIME_ZONE_TYPE == TIME_ZONE_TYPE_EXTENDED
      if (storedInfo.timeZoneData.type == TimeZoneData::kTypeZoneId) {
        clockInfo.timeZone = mZoneManager.createForTimeZoneData(
            storedInfo.timeZoneData);
        if (! clockInfo.timeZone.isError()) {
          mZoneIndex = mZoneManager.indexForZoneId(
              storedInfo.timeZoneData.zoneId);
        } else {
          clockInfo.timeZone = mZoneManager.createForZoneIndex(0);
          mZoneIndex = 0;
        }
      } else {
        clockInfo.timeZone = mZoneManager.createForZoneIndex(0);
        mZoneIndex = 0;
      }
    #endif

    }

    /** Set up the initial ClockInfo states. */
    void setupClockInfo() {
      StoredInfo storedInfo;
      storedInfo.hourMode = StoredInfo::kTwentyFour;

    #if TIME_ZONE_TYPE == TIME_ZONE_TYPE_MANUAL
      storedInfo.timeZoneData.type = TimeZone::kTypeManual;
      storedInfo.timeZoneData.stdOffsetCode =
          TimeOffset::forMinues(kDefaultOffsetMinutes).toOffsetCode();
      storedInfo.timeZoneData.dstOffsetCode = 0;
    #elif TIME_ZONE_TYPE == TIME_ZONE_TYPE_BASIC
      storedInfo.timeZoneData.type = TimeZoneData::kTypeZoneId;
      storedInfo.timeZoneData.zoneId =
          BasicZone(&zonedb::kZoneAmerica_Los_Angeles).zoneId();
    #elif TIME_ZONE_TYPE == TIME_ZONE_TYPE_EXTENDED
      storedInfo.timeZoneData.type = TimeZoneData::kTypeZoneId;
      storedInfo.timeZoneData.zoneId =
          ExtendedZone(&zonedbx::kZoneAmerica_Los_Angeles).zoneId();
    #endif

      restoreClockInfo(mClockInfo, storedInfo);
    }

    PersistentStore& mPersistentStore;
    TimeKeeper& mTimeKeeper;
    Presenter& mPresenter;

  #if TIME_ZONE_TYPE == TIME_ZONE_TYPE_BASIC
    BasicZoneManager<2> mZoneManager;;
  #elif TIME_ZONE_TYPE == TIME_ZONE_TYPE_EXTENDED
    ExtendedZoneManager<2> mZoneManager;;
  #endif

    ClockInfo mClockInfo; // current clock
    ClockInfo mChangingClockInfo; // the target clock

    uint8_t mMode = MODE_DATE_TIME; // current mode

    /**
     * Zone index into the ZoneRegistry. Defined if TIME_ZONE_TYPE is BASIC or
     * EXTENDED.
     */
    uint16_t mZoneIndex;

    bool mSecondFieldCleared;
    bool mSuppressBlink; // true if blinking should be suppressed

    bool mBlinkShowState = true; // true means actually show
    uint16_t mBlinkCycleStartMillis = 0; // millis since blink cycle start
    bool mIsPreparingToSleep = false;
};

#endif
