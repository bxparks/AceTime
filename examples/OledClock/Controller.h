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
        mPresenter(presenter) {
    }

    void setup() {
      // Restore from EEPROM to retrieve time zone.
    #if 0
      StoredInfo storedInfo;
      bool isValid = mCrcEeprom.readWithCrc(kStoredInfoEepromAddress,
          &storedInfo, sizeof(StoredInfo));
      if (isValid) {
        restoreInfo(storedInfo);
      } else {
        setupInfo();
      }
    #else
      setupInfo();
    #endif

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

        // Cycle through the various changeable time zone fields.
        case MODE_CHANGE_TIME_ZONE_TYPE:
          mMode = MODE_CHANGE_TIME_ZONE_OFFSET;
          break;
        case MODE_CHANGE_TIME_ZONE_OFFSET:
          mMode = MODE_CHANGE_TIME_ZONE_DST;
          break;
        case MODE_CHANGE_TIME_ZONE_DST:
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
        case MODE_CHANGE_TIME_ZONE_TYPE:
          mSuppressBlink = true;
          {
            if (mChangingClockInfo.timeZoneType == TimeZone::kTypeManual) {
              mChangingClockInfo.timeZoneType = TimeZone::kTypeBasic;
            } else {
              mChangingClockInfo.timeZoneType = TimeZone::kTypeManual;
            }
          }
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
      TimeZone tz = TimeZone::forZoneSpecifier(
          (mClockInfo.timeZoneType == TimeZone::kTypeManual)
              ? (ZoneSpecifier*) &mClockInfo.manualZspec
              : (ZoneSpecifier*) &mClockInfo.basicZspec);
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
          mPresenter.setRenderingInfo(mMode, mSuppressBlink, mBlinkShowState,
              mChangingClockInfo);
          break;
      }
    }

    /** Save the current UTC dateTime to the RTC. */
    void saveDateTime() {
      mTimeKeeper.setNow(mChangingClockInfo.dateTime.toEpochSeconds());
    }

    /** Save the time zone from Changing to current. */
    void saveClockInfo() {
      mClockInfo.timeZoneType = mChangingClockInfo.timeZoneType;
      mClockInfo.hourMode = mChangingClockInfo.hourMode;
      mClockInfo.manualZspec = mChangingClockInfo.manualZspec;
      mClockInfo.basicZspec = mChangingClockInfo.basicZspec;
      mClockInfo.dateTime = mClockInfo.dateTime.convertToTimeZone(
          mChangingClockInfo.dateTime.timeZone());
      preserveInfo();
    }

    /** Save the current clock info info to EEPROM. */
    void preserveInfo() {
      StoredInfo storedInfo;
      storedInfo.timeZoneType = mClockInfo.timeZoneType;
      storedInfo.offsetMinutes =
          mClockInfo.manualZspec.stdOffset().toMinutes();
      storedInfo.isDst = mClockInfo.manualZspec.isDst();
      storedInfo.hourMode = mClockInfo.hourMode;
      // TODO: Save basicZoneSpecifier info

      mCrcEeprom.writeWithCrc(kStoredInfoEepromAddress, &storedInfo,
          sizeof(StoredInfo));
    }

    /** Restore from the EEPROM. */
    void restoreInfo(const StoredInfo& storedInfo) {
      mClockInfo.manualZspec = ManualZoneSpecifier(
          TimeOffset::forMinutes(storedInfo.offsetMinutes), false);
      mClockInfo.manualZspec.isDst(storedInfo.isDst);
      // TODO: Restore basicZoneSpecifier info
      mClockInfo.hourMode = storedInfo.hourMode;
    }

    void setupInfo() {
      mClockInfo.timeZoneType = TimeZone::kTypeManual;
      mClockInfo.manualZspec = ManualZoneSpecifier(
          TimeOffset::forMinutes(kDefaultOffsetMinutes), false);
      mClockInfo.manualZspec.isDst(false);
      mClockInfo.basicZspec.setZoneInfo(&zonedb::kZoneAmerica_Los_Angeles);
      mClockInfo.hourMode = StoredInfo::kTwentyFour;
    }

    TimeKeeper& mTimeKeeper;
    hw::CrcEeprom& mCrcEeprom;
    Presenter& mPresenter;
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
