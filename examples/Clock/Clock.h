#ifndef CLOCK_CLOCK_H
#define CLOCK_CLOCK_H

#include <AceTime.h>
#include <ace_time/hw/CrcEeprom.h>
#include <ace_time/common/DateStrings.h>
#include "ClockInfo.h"
#include "RenderingInfo.h"
#include "StoredInfo.h"
#include "Presenter.h"

using namespace ace_time;
using namespace ace_time::common;
using namespace ace_time::provider;

/**
 * Class responsible for rendering the RenderingInfo to the indicated display.
 * Different subclasses output to different types of displays. In an MVC
 * architecture, this would be the Controller. The Model would be the various
 * member variables in this class. The View layer are the various Presenter
 * classes.
 */
class Clock {
  public:
    static const uint16_t kStoredInfoEepromAddress = 0;

    static const int16_t kDefaultOffsetMinutes = -8*60; // UTC-08:00

    /**
     * Constructor.
     * @param timeKeeper source of the current time
     * @param crcEeprom stores objects into the EEPROM with CRC
     * @param presenter renders the date and time info to the screen
     */
    Clock(TimeKeeper& timeKeeper, hw::CrcEeprom& crcEeprom,
            Presenter& presenter):
        mTimeKeeper(timeKeeper),
        mCrcEeprom(crcEeprom),
        mPresenter(presenter) {}

    void setup() {
      // Restore from EEPROM to retrieve time zone.
      StoredInfo storedInfo;
      bool isValid = mCrcEeprom.readWithCrc(kStoredInfoEepromAddress,
          &storedInfo, sizeof(StoredInfo));
      if (isValid) {
        mClockInfo.zoneSpecifier = ManualZoneSpecifier(
            UtcOffset::forMinutes(storedInfo.offsetMinutes),
            UtcOffset::forHour(1) /*deltaOffset*/);
        mClockInfo.zoneSpecifier.isDst(storedInfo.isDst);
        mClockInfo.hourMode = storedInfo.hourMode;
      } else {
        mClockInfo.zoneSpecifier = ManualZoneSpecifier(
            UtcOffset::forMinutes(kDefaultOffsetMinutes),
            UtcOffset::forHour(1) /*deltaOffset*/);
        mClockInfo.zoneSpecifier.isDst(false);
        mClockInfo.hourMode = StoredInfo::kTwentyFour;
      }

      // Retrieve current time from TimeKeeper and set the current clockInfo.
      acetime_t nowSeconds = mTimeKeeper.getNow();
      mClockInfo.dateTime = ZonedDateTime::forEpochSeconds(
          nowSeconds, TimeZone(&mClockInfo.zoneSpecifier));
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

    virtual void modeButtonPress() = 0;

    virtual void modeButtonLongPress() = 0;

    virtual void changeButtonPress() = 0;

    virtual void changeButtonRelease() = 0;

    virtual void changeButtonRepeatPress() = 0;

  protected:
    void updateDateTime() {
      // TODO: It might be possible to track just the epochSeconds instead of
      // converting it to a ZonedDateTime at each iteration.
      mClockInfo.dateTime = ZonedDateTime::forEpochSeconds(
          mTimeKeeper.getNow(), TimeZone(&mClockInfo.zoneSpecifier));

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
      mPresenter.setMode(mMode);
      mPresenter.setSuppressBlink(mSuppressBlink);
      mPresenter.setBlinkShowState(mBlinkShowState);

      switch (mMode) {
        case MODE_DATE_TIME:
#if DISPLAY_TYPE == DISPLAY_TYPE_LED || DISPLAY_TYPE == DISPLAY_TYPE_OLED
        case MODE_HOUR_MINUTE:
        case MODE_MINUTE_SECOND:
        case MODE_YEAR:
        case MODE_MONTH:
        case MODE_DAY:
#endif
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
        case MODE_CHANGE_TIME_ZONE_HOUR:
        case MODE_CHANGE_TIME_ZONE_MINUTE:
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
          TimeZone(&mClockInfo.zoneSpecifier));
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
