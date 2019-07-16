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
        mPresenter(presenter),
        mBasicZoneManager(kBasicZoneRegistry, kBasicZoneRegistrySize) {}

    void setup() {
      // Restore from EEPROM to retrieve time zone.
      StoredInfo storedInfo;
      bool isValid = mCrcEeprom.readWithCrc(kStoredInfoEepromAddress,
          &storedInfo, sizeof(StoredInfo));
      if (isValid) {
        restoreInfo(storedInfo);
      } else {
        setupInfo();
      }

      // Retrieve current time from TimeKeeper and set the current clockInfo.
      acetime_t nowSeconds = mTimeKeeper.getNow();
      mClockInfo.dateTime = ZonedDateTime::forEpochSeconds(
          nowSeconds, TimeZone::forZoneSpecifier(&mClockInfo.manualZspec));
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

        // Transition from 'manual' (ManualZoneSpecifier) or 'auto' (i.e.
        // BasicZoneSpecifier)
        case MODE_CHANGE_TIME_ZONE_TYPE:
          if (mChangingClockInfo.timeZone.getType() == TimeZone::kTypeManual) {
            mMode = MODE_CHANGE_TIME_ZONE_OFFSET;
          } else {
            mMode = MODE_CHANGE_TIME_ZONE_NAME;
          }
          break;

        // ManualZoneSpecifier
        case MODE_CHANGE_TIME_ZONE_OFFSET:
          mMode = MODE_CHANGE_TIME_ZONE_DST;
          break;
        case MODE_CHANGE_TIME_ZONE_DST:
          mMode = MODE_CHANGE_TIME_ZONE_TYPE;
          break;

        // BasicZoneSpecifier
        case MODE_CHANGE_TIME_ZONE_NAME:
          mMode = MODE_CHANGE_TIME_ZONE_TYPE;
          break;
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
          mMode = MODE_CHANGE_TIME_ZONE_TYPE;
          break;

        case MODE_CHANGE_TIME_ZONE_TYPE:
        case MODE_CHANGE_TIME_ZONE_OFFSET:
        case MODE_CHANGE_TIME_ZONE_DST:
        case MODE_CHANGE_TIME_ZONE_NAME:
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
          preserveInfo();
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

        // Toggle the timezone
        case MODE_CHANGE_TIME_ZONE_TYPE:
          mSuppressBlink = true;
          mChangingClockInfo.timeZone = TimeZone::forZoneSpecifier(
              (mChangingClockInfo.timeZone.getType() == TimeZone::kTypeManual)
                  ? (ZoneSpecifier*) &mChangingClockInfo.basicZspec
                  : (ZoneSpecifier*) &mChangingClockInfo.manualZspec);
          mChangingClockInfo.dateTime =
              mChangingClockInfo.dateTime.convertToTimeZone(
                  mChangingClockInfo.timeZone);
          break;

        case MODE_CHANGE_TIME_ZONE_OFFSET:
          mSuppressBlink = true;
          {
            TimeOffset offset = mChangingClockInfo.manualZspec.stdOffset();
            time_offset_mutation::increment15Minutes(offset);
            mChangingClockInfo.manualZspec.stdOffset(offset);
          }
          break;
        case MODE_CHANGE_TIME_ZONE_DST:
          mSuppressBlink = true;
          mChangingClockInfo.manualZspec.isDst(
              !mChangingClockInfo.manualZspec.isDst());
          break;

        // Cycle through the zones in the registry
        case MODE_CHANGE_TIME_ZONE_NAME:
          mSuppressBlink = true;
          mChangingClockInfo.zoneIndex++;
          if (mChangingClockInfo.zoneIndex >= kBasicZoneRegistrySize) {
            mChangingClockInfo.zoneIndex = 0;
          }
          {
            const basic::ZoneInfo* zoneInfo =
                mBasicZoneManager.getZoneInfo(mChangingClockInfo.zoneIndex);
            mChangingClockInfo.basicZspec.setZoneInfo(zoneInfo);
            mChangingClockInfo.timeZone = TimeZone::forZoneSpecifier(
                &mChangingClockInfo.basicZspec);
            mChangingClockInfo.dateTime = mClockInfo.dateTime.convertToTimeZone(
                mChangingClockInfo.timeZone);
          }
          break;
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
        case MODE_CHANGE_TIME_ZONE_TYPE:
        case MODE_CHANGE_TIME_ZONE_OFFSET:
        case MODE_CHANGE_TIME_ZONE_DST:
        case MODE_CHANGE_TIME_ZONE_NAME:
          mSuppressBlink = false;
          break;
      }
    }

  private:
    static const basic::ZoneInfo* const kBasicZoneRegistry[];
    static const uint16_t kBasicZoneRegistrySize;

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
        case MODE_CHANGE_TIME_ZONE_TYPE:
        case MODE_CHANGE_TIME_ZONE_OFFSET:
        case MODE_CHANGE_TIME_ZONE_DST:
        case MODE_CHANGE_TIME_ZONE_NAME:
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
      mClockInfo.zoneIndex = mChangingClockInfo.zoneIndex;
      mClockInfo.manualZspec = mChangingClockInfo.manualZspec;
      mClockInfo.basicZspec = mChangingClockInfo.basicZspec;
      mClockInfo.dateTime = mClockInfo.dateTime;

      // Deep-copy the timeZone
      mClockInfo.timeZone = TimeZone::forZoneSpecifier(
          (mChangingClockInfo.timeZone.getType() == TimeZone::kTypeManual)
              ? (ZoneSpecifier*) &mClockInfo.manualZspec
              : (ZoneSpecifier*) &mClockInfo.basicZspec);
      mClockInfo.dateTime.timeZone(mClockInfo.timeZone);

      preserveInfo();
    }

    /** Save the current clock info info to EEPROM. */
    void preserveInfo() {
      StoredInfo storedInfo;
      storedInfo.hourMode = mClockInfo.hourMode;
      storedInfo.timeZoneType = mClockInfo.timeZone.getType();
      storedInfo.offsetMinutes =
          mClockInfo.manualZspec.stdOffset().toMinutes();
      storedInfo.isDst = mClockInfo.manualZspec.isDst();
      storedInfo.zoneIndex = mClockInfo.zoneIndex;

      mCrcEeprom.writeWithCrc(kStoredInfoEepromAddress, &storedInfo,
          sizeof(StoredInfo));
    }

    /** Restore from the EEPROM. */
    void restoreInfo(const StoredInfo& storedInfo) {
      mClockInfo.hourMode = storedInfo.hourMode;

      mClockInfo.manualZspec = ManualZoneSpecifier(
          TimeOffset::forMinutes(storedInfo.offsetMinutes), false);
      mClockInfo.manualZspec.isDst(storedInfo.isDst);
      mClockInfo.zoneIndex = storedInfo.zoneIndex;
      const basic::ZoneInfo* zoneInfo =
          mBasicZoneManager.getZoneInfo(mClockInfo.zoneIndex);
      mClockInfo.basicZspec.setZoneInfo(zoneInfo);

      mClockInfo.timeZone = TimeZone::forZoneSpecifier(
          (storedInfo.timeZoneType == TimeZone::kTypeManual)
              ? (ZoneSpecifier*) &mClockInfo.manualZspec
              : (ZoneSpecifier*) &mClockInfo.basicZspec);
    }

    /** Set up the initial ClockInfo states. */
    void setupInfo() {
      StoredInfo storedInfo;
      storedInfo.timeZoneType = TimeZone::kTypeManual;
      storedInfo.offsetMinutes = kDefaultOffsetMinutes;
      storedInfo.isDst = false;
      storedInfo.zoneIndex = 0;
      storedInfo.hourMode = StoredInfo::kTwentyFour;

      restoreInfo(storedInfo);
    }

    TimeKeeper& mTimeKeeper;
    hw::CrcEeprom& mCrcEeprom;
    Presenter& mPresenter;
    BasicZoneManager mBasicZoneManager;
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
