#ifndef OLED_CLOCK_CONTROLLER_H
#define OLED_CLOCK_CONTROLLER_H

#include <AceTime.h>
#include <ace_time/hw/CrcEeprom.h>
#include "config.h"
#include "ClockInfo.h"
#include "RenderingInfo.h"
#include "StoredInfo.h"
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

    /**
     * Constructor.
     * @param timeKeeper source of the current time
     * @param crcEeprom stores objects into the EEPROM with CRC
     * @param presenter renders the date and time info to the screen
     */
    Controller(TimeKeeper& timeKeeper, hw::CrcEeprom& crcEeprom,
            Presenter& presenter):
        mTimeKeeper(timeKeeper),
        mCrcEeprom(crcEeprom),
        mPresenter(presenter)
      #if TIME_ZONE_TYPE == TIME_ZONE_TYPE_BASIC \
          || TIME_ZONE_TYPE == TIME_ZONE_TYPE_EXTENDED
        , mZoneManager(kZoneRegistrySize, kZoneRegistry)
      #endif
    {}

    void setup() {
      // Restore from EEPROM to retrieve time zone.
      StoredInfo storedInfo;
      bool isValid = mCrcEeprom.readWithCrc(kStoredInfoEepromAddress,
          &storedInfo, sizeof(StoredInfo));
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
        Serial.println(F("modeButtonPress()"));
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
        Serial.println(F("modeButtonLongPress()"));
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
        Serial.println(F("changeButtonPress()"));
      #endif
      switch (mMode) {
        // Switch 12/24 modes if in MODE_DATA_TIME
        case MODE_DATE_TIME:
          mClockInfo.hourMode ^= 0x1;
          preserveClockInfo(mCrcEeprom, mClockInfo);
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
          mSuppressBlink = true;
          {
            auto offset = TimeOffset::forOffsetCode(
                mChangingClockInfo.timeZoneData.stdOffsetCode);
            time_offset_mutation::increment15Minutes(offset);
            mChangingClockInfo.timeZoneData.stdOffsetCode =
                offset.toOffsetCode();
          }
          break;
        case MODE_CHANGE_TIME_ZONE_DST:
          mSuppressBlink = true;
          uint8_t offsetCode = mChangingClockInfo.timeZoneData.dstOffsetCode;
          offsetCode = (offsetCode == 0)
              ? TimeOffset::forMinutes(DST_OFFSET_MINUTES).toOffsetCode()
              : 0;
          mChangingClockInfo.timeZoneData.dstOffsetCode = offsetCode;
          break;

      #else
        // Cycle through the zones in the registry
        case MODE_CHANGE_TIME_ZONE_NAME:
          mSuppressBlink = true;
          mChangingClockInfo.zoneIndex++;
          if (mChangingClockInfo.zoneIndex >= kZoneRegistrySize) {
            mChangingClockInfo.zoneIndex = 0;
          }
        #if TIME_ZONE_TYPE == TIME_ZONE_TYPE_BASIC
          mChangingClockInfo.timeZoneData.basicZoneInfo =
              mZoneManager.getZoneInfo(mChangingClockInfo.zoneIndex);
        #else
          mChangingClockInfo.timeZoneData.extendedZoneInfo =
              mZoneManager.getZoneInfo(mChangingClockInfo.zoneIndex);
        #endif
          {
            auto tz = getTimeZoneForClockInfo(mChangingClockInfo.timeZoneData);
            mChangingClockInfo.dateTime =
                mChangingClockInfo.dateTime.convertToTimeZone(tz);
          }
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

      auto tz = getTimeZoneForClockInfo(mClockInfo.timeZoneData);
      mClockInfo.dateTime = ZonedDateTime::forEpochSeconds(
          mTimeKeeper.getNow(), tz);

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

    TimeZone getTimeZoneForClockInfo(const TimeZoneData& timeZoneData) {
  #if TIME_ZONE_TYPE == TIME_ZONE_TYPE_MANUAL
      auto tz = TimeZone::forTimeZoneData(timeZoneData, nullptr, nullptr);
  #elif TIME_ZONE_TYPE == TIME_ZONE_TYPE_BASIC
      auto tz = TimeZone::forTimeZoneData(timeZoneData,
          &mZoneSpecifier, nullptr);
  #else
      auto tz = TimeZone::forTimeZoneData(timeZoneData,
          nullptr, &mZoneSpecifier);
  #endif
      return tz;
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
      mClockInfo.hourMode = mChangingClockInfo.hourMode;
      mClockInfo.timeZoneData = mChangingClockInfo.timeZoneData;
      mClockInfo.zoneIndex = mChangingClockInfo.zoneIndex;
      mClockInfo.dateTime = mClockInfo.dateTime;
      preserveClockInfo(mCrcEeprom, mClockInfo);
    }

    /** Save the clock info into EEPROM. */
    static void preserveClockInfo(hw::CrcEeprom& crcEeprom,
        const ClockInfo& clockInfo) {
      StoredInfo storedInfo;
      storedInfo.hourMode = clockInfo.hourMode;
      storedInfo.type = clockInfo.timeZoneData.type;
      switch (clockInfo.timeZoneData.type) {
        case TimeZoneData::kTypeManual:
          storedInfo.stdOffsetCode = clockInfo.timeZoneData.stdOffsetCode;
          storedInfo.dstOffsetCode = clockInfo.timeZoneData.dstOffsetCode;
          break;
        case TimeZoneData::kTypeBasic:
          storedInfo.zoneIndex = clockInfo.zoneIndex;
          break;
        case TimeZoneData::kTypeExtended:
          storedInfo.zoneIndex = clockInfo.zoneIndex;
          break;
      }

      crcEeprom.writeWithCrc(kStoredInfoEepromAddress, &storedInfo,
          sizeof(StoredInfo));
    }

    /** Restore clockInfo from storedInfo. */
    void restoreClockInfo(ClockInfo& clockInfo,
        const StoredInfo& storedInfo) {
      clockInfo.hourMode = storedInfo.hourMode;
      clockInfo.timeZoneData.type = storedInfo.type;
      switch (storedInfo.type) {
      #if TIME_ZONE_TYPE == TIME_ZONE_TYPE_MANUAL
        case TimeZoneData::kTypeManual:
          clockInfo.timeZoneData.stdOffsetCode = storedInfo.stdOffsetCode;
          clockInfo.timeZoneData.dstOffsetCode = storedInfo.dstOffsetCode;
          break;
      #elif TIME_ZONE_TYPE == TIME_ZONE_TYPE_BASIC
        case TimeZoneData::kTypeBasic:
          clockInfo.zoneIndex = storedInfo.zoneIndex;
          clockInfo.timeZoneData.basicZoneInfo =
              mZoneManager.getZoneInfo(storedInfo.zoneIndex);
          break;
      #elif TIME_ZONE_TYPE == TIME_ZONE_TYPE_EXTENDED
        case TimeZoneData::kTypeExtended:
          clockInfo.zoneIndex = storedInfo.zoneIndex;
          clockInfo.timeZoneData.extendedZoneInfo =
              mZoneManager.getZoneInfo(storedInfo.zoneIndex);
          break;
      #endif
        default:
        #if TIME_ZONE_TYPE == TIME_ZONE_TYPE_MANUAL
          clockInfo.timeZoneData.type = TimeZoneData::kTypeManual;
          clockInfo.timeZoneData.stdOffsetCode = kDefaultOffsetMinutes / 15;
          clockInfo.timeZoneData.dstOffsetCode = 0;
        #elif TIME_ZONE_TYPE == TIME_ZONE_TYPE_BASIC
          clockInfo.zoneIndex = 0;
          clockInfo.timeZoneData.type = TimeZoneData::kTypeBasic;
          clockInfo.timeZoneData.basicZoneInfo =
              mZoneManager.getZoneInfo(clockInfo.zoneIndex);
        #elif TIME_ZONE_TYPE == TIME_ZONE_TYPE_EXTENDED
          clockInfo.zoneIndex = 0;
          clockInfo.timeZoneData.type = TimeZoneData::kTypeExtended;
          clockInfo.timeZoneData.extendedZoneInfo =
              mZoneManager.getZoneInfo(clockInfo.zoneIndex);
        #endif
      }
    }

    /** Set up the initial ClockInfo states. */
    void setupClockInfo() {
      StoredInfo storedInfo;
      storedInfo.hourMode = StoredInfo::kTwentyFour;

    #if TIME_ZONE_TYPE == TIME_ZONE_TYPE_MANUAL
      storedInfo.type = TimeZoneData::kTypeManual;
      storedInfo.stdOffsetCode = kDefaultOffsetMinutes / 15;
      storedInfo.dstOffsetCode = 0;
    #elif TIME_ZONE_TYPE == TIME_ZONE_TYPE_BASIC
      storedInfo.type = TimeZoneData::kTypeBasic;
      storedInfo.zoneIndex = 0;
    #elif TIME_ZONE_TYPE == TIME_ZONE_TYPE_EXTENDED
      storedInfo.type = TimeZoneData::kTypeExtended;
      storedInfo.zoneIndex = 0;
    #endif

      restoreClockInfo(mClockInfo, storedInfo);
    }

    TimeKeeper& mTimeKeeper;
    hw::CrcEeprom& mCrcEeprom;
    Presenter& mPresenter;

  #if TIME_ZONE_TYPE == TIME_ZONE_TYPE_BASIC
    BasicZoneManager mZoneManager;
    BasicZoneSpecifier mZoneSpecifier;
  #elif TIME_ZONE_TYPE == TIME_ZONE_TYPE_EXTENDED
    ExtendedZoneManager mZoneManager;
    ExtendedZoneSpecifier mZoneSpecifier;
  #endif

    ClockInfo mClockInfo; // current clock
    ClockInfo mChangingClockInfo; // the target clock

    uint8_t mMode = MODE_DATE_TIME; // current mode

    bool mSecondFieldCleared;
    bool mSuppressBlink; // true if blinking should be suppressed

    bool mBlinkShowState = true; // true means actually show
    uint16_t mBlinkCycleStartMillis = 0; // millis since blink cycle start
    bool mIsPreparingToSleep = false;
};

#endif
