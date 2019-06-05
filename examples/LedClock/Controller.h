#ifndef LED_CLOCK_CONTROLLER_H
#define LED_CLOCK_CONTROLLER_H

#include <AceTime.h>
#include <AceSegment.h>
#include "config.h"
#include "ClockInfo.h"
#include "Presenter.h"
#include "StoredInfo.h"

using namespace ace_segment;
using namespace ace_time;
using namespace ace_time::common;
using namespace ace_time::provider;

class Controller {
  public:
    static const uint16_t kStoredInfoEepromAddress = 0;

    static const int16_t kDefaultOffsetMinutes = -8*60; // UTC-08:00

    /** Constructor. */
    Controller(TimeKeeper& timeKeeper, hw::CrcEeprom& crcEeprom,
          Presenter& presenter):
        mTimeKeeper(timeKeeper),
        mCrcEeprom(crcEeprom),
        mPresenter(presenter) {
      mMode = MODE_HOUR_MINUTE;
    }

    void setup() {
      // Restore from EEPROM to retrieve time zone.
      StoredInfo storedInfo;
      bool isValid = mCrcEeprom.readWithCrc(kStoredInfoEepromAddress,
          &storedInfo, sizeof(StoredInfo));
      if (isValid) {
        mClockInfo.zoneSpecifier = ManualZoneSpecifier(
            TimeOffset::forMinutes(storedInfo.offsetMinutes), false);
        mClockInfo.zoneSpecifier.isDst(storedInfo.isDst);
        mClockInfo.hourMode = storedInfo.hourMode;
      } else {
        mClockInfo.zoneSpecifier = ManualZoneSpecifier(
            TimeOffset::forMinutes(kDefaultOffsetMinutes), false);
        mClockInfo.zoneSpecifier.isDst(false);
        mClockInfo.hourMode = StoredInfo::kTwentyFour;
      }

      // Retrieve current time from TimeKeeper and set the current clockInfo.
      acetime_t nowSeconds = mTimeKeeper.getNow();
      mClockInfo.dateTime = ZonedDateTime::forEpochSeconds(
          nowSeconds, TimeZone::forZoneSpecifier(&mClockInfo.zoneSpecifier));
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
      switch (mMode) {
        case MODE_HOUR_MINUTE:
          mMode = MODE_MINUTE_SECOND;
          break;
        case MODE_MINUTE_SECOND:
          mMode = MODE_YEAR;
          break;
        case MODE_YEAR:
          mMode = MODE_MONTH;
          break;
        case MODE_MONTH:
          mMode = MODE_DAY;
          break;
        case MODE_DAY:
          mMode = MODE_WEEKDAY;
          break;
        case MODE_WEEKDAY:
          mMode = MODE_HOUR_MINUTE;
          break;
        case MODE_CHANGE_HOUR:
          mMode = MODE_CHANGE_MINUTE;
          break;
        case MODE_CHANGE_MINUTE:
          mMode = MODE_CHANGE_YEAR;
          break;
        case MODE_CHANGE_YEAR:
          mMode = MODE_CHANGE_MONTH;
          break;
        case MODE_CHANGE_MONTH:
          mMode = MODE_CHANGE_DAY;
          break;
        case MODE_CHANGE_DAY:
          mMode = MODE_CHANGE_HOUR;
          break;
      }
    }

    void modeButtonLongPress() {
      switch (mMode) {
        case MODE_HOUR_MINUTE:
          mChangingClockInfo = mClockInfo;
          mSecondFieldCleared = false;
          mMode = MODE_CHANGE_HOUR;
          break;
        case MODE_MINUTE_SECOND:
          mChangingClockInfo = mClockInfo;
          mSecondFieldCleared = false;
          mMode = MODE_CHANGE_MINUTE;
          break;

        case MODE_YEAR:
          mChangingClockInfo = mClockInfo;
          mSecondFieldCleared = false;
          mMode = MODE_CHANGE_YEAR;
          break;
        case MODE_MONTH:
          mChangingClockInfo = mClockInfo;
          mSecondFieldCleared = false;
          mMode = MODE_CHANGE_MONTH;
          break;
        case MODE_DAY:
          mChangingClockInfo = mClockInfo;
          mSecondFieldCleared = false;
          mMode = MODE_CHANGE_DAY;
          break;

        case MODE_CHANGE_YEAR:
          saveDateTime();
          mMode = MODE_YEAR;
          break;
        case MODE_CHANGE_MONTH:
          saveDateTime();
          mMode = MODE_MONTH;
          break;
        case MODE_CHANGE_DAY:
          saveDateTime();
          mMode = MODE_DAY;
          break;
        case MODE_CHANGE_HOUR:
          saveDateTime();
          mMode = MODE_HOUR_MINUTE;
          break;
        case MODE_CHANGE_MINUTE:
          saveDateTime();
          mMode = MODE_HOUR_MINUTE;
          break;
      }
    }

    void changeButtonPress() {
      switch (mMode) {
        case MODE_CHANGE_HOUR:
          mSuppressBlink = true;
          date_time_mutation::incrementHour(mChangingClockInfo.dateTime);
          break;
        case MODE_CHANGE_MINUTE:
          mSuppressBlink = true;
          date_time_mutation::incrementMinute(mChangingClockInfo.dateTime);
          break;
        case MODE_CHANGE_YEAR:
          mSuppressBlink = true;
          date_time_mutation::incrementYear(mChangingClockInfo.dateTime);
          break;
        case MODE_CHANGE_MONTH:
          mSuppressBlink = true;
          date_time_mutation::incrementMonth(mChangingClockInfo.dateTime);
          break;
        case MODE_CHANGE_DAY:
          mSuppressBlink = true;
          date_time_mutation::incrementDay(mChangingClockInfo.dateTime);
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
        case MODE_CHANGE_TIME_ZONE_OFFSET:
          mSuppressBlink = false;
          break;
      }
    }

  private:
    void updateDateTime() {
      // TODO: It might be possible to track just the epochSeconds instead of
      // converting it to a ZonedDateTime at each iteration.
      mClockInfo.dateTime = ZonedDateTime::forEpochSeconds(
          mTimeKeeper.getNow(),
          TimeZone::forZoneSpecifier(&mClockInfo.zoneSpecifier));

      // If in CHANGE mode, and the 'second' field has not been cleared, update
      // the displayed time with the current second.
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
      mPresenter.setMode(mMode);
      mPresenter.setSuppressBlink(mSuppressBlink);
      mPresenter.setBlinkShowState(mBlinkShowState);

      switch (mMode) {
        case MODE_DATE_TIME:
        case MODE_HOUR_MINUTE:
        case MODE_MINUTE_SECOND:
        case MODE_YEAR:
        case MODE_MONTH:
        case MODE_DAY:
        case MODE_WEEKDAY:
        case MODE_TIME_ZONE:
          mPresenter.setDateTime(mClockInfo.dateTime);
          mPresenter.setTimeZone(mClockInfo.zoneSpecifier);
          mPresenter.setHourMode(mClockInfo.hourMode);
          break;

        case MODE_CHANGE_YEAR:
        case MODE_CHANGE_MONTH:
        case MODE_CHANGE_DAY:
        case MODE_CHANGE_HOUR:
        case MODE_CHANGE_MINUTE:
        case MODE_CHANGE_SECOND:
        case MODE_CHANGE_TIME_ZONE_OFFSET:
        case MODE_CHANGE_TIME_ZONE_DST:
        case MODE_CHANGE_HOUR_MODE:
          mPresenter.setDateTime(mChangingClockInfo.dateTime);
          mPresenter.setTimeZone(mChangingClockInfo.zoneSpecifier);
          mPresenter.setHourMode(mChangingClockInfo.hourMode);
          break;
      }
    }

    /** Save the current UTC dateTime to the RTC. */
    void saveDateTime() {
      mTimeKeeper.setNow(mChangingClockInfo.dateTime.toEpochSeconds());
    }

    /** Save the time zone from Changing to current. */
    void saveClockInfo() {
      mClockInfo.hourMode = mChangingClockInfo.hourMode;
      mClockInfo.zoneSpecifier = mChangingClockInfo.zoneSpecifier;
      mClockInfo.dateTime = mClockInfo.dateTime.convertToTimeZone(
          TimeZone::forZoneSpecifier(&mClockInfo.zoneSpecifier));
      preserveInfo();
    }

    /** Save the current clock info info to EEPROM. */
    void preserveInfo() {
      StoredInfo storedInfo;
      storedInfo.timeZoneType = mClockInfo.zoneSpecifier.getType();
      storedInfo.offsetMinutes =
          mClockInfo.zoneSpecifier.stdOffset().toMinutes();
      storedInfo.isDst = mClockInfo.zoneSpecifier.isDst();
      storedInfo.hourMode = mClockInfo.hourMode;

      mCrcEeprom.writeWithCrc(kStoredInfoEepromAddress, &storedInfo,
          sizeof(StoredInfo));
    }


    TimeKeeper& mTimeKeeper;
    hw::CrcEeprom& mCrcEeprom;
    Presenter& mPresenter;
    ClockInfo mClockInfo; // current clock
    ClockInfo mChangingClockInfo; // the target clock

    uint8_t mMode = MODE_UNKNOWN; // current mode

    bool mSecondFieldCleared;
    bool mSuppressBlink; // true if blinking should be suppressed

    bool mBlinkShowState = true; // true means actually show
    uint16_t mBlinkCycleStartMillis = 0; // millis since blink cycle start
    bool mIsPreparingToSleep = false;
};

#endif
