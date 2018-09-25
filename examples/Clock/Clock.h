#ifndef CLOCK_CLOCK_H
#define CLOCK_CLOCK_H

#include <AceTime.h>
#include <ace_time/hw/HardwareTemperature.h>
#include <ace_time/hw/CrcEeprom.h>
#include <ace_time/common/DateStrings.h>
#include "RenderingInfo.h"
#include "StoredInfo.h"
#include "Presenter.h"

using namespace ace_time;
using namespace ace_time::common;
using namespace ace_time::hw;

/**
 * Class responsible for rendering the RenderingInfo to the indicated display.
 * Different subclasses output to different types of displays. In an MVC
 * architecture, this would be the Controller. The Model would be the various
 * member variables in thic class. There is no clear separation of the View
 * class in the current implementation, it is somewhat implemented in the
 * various subclasses of Clock (i.e. OledClock, FullOledClock, and LedClock).
 * TODO: Extract those out into a proper View layer.
 */
class Clock {
  public:
    static const uint16_t kStoredInfoEepromAddress = 0;
    static const int8_t kDefaultTzCode = -28; // Pacific Daylight Time, -07:00

    /**
     * Constructor.
     * @param timeKeeper
     * @crcEeprom
     * @ds3231 nullable if the device does not have a DS3231 RTC chip
     */
    Clock(TimeKeeper& timeKeeper, CrcEeprom& crcEeprom, Presenter& presenter,
            const DS3231* ds3231):
        mTimeKeeper(timeKeeper),
        mCrcEeprom(crcEeprom),
        mPresenter(presenter),
        mDS3231(ds3231) {}

    void setup() {
      // Retrieve current time from TimeKeeper.
      uint32_t nowSeconds = mTimeKeeper.getNow();

      // Restore from EEPROM to retrieve time zone.
      StoredInfo storedInfo;
      bool isValid = mCrcEeprom.readWithCrc(kStoredInfoEepromAddress,
          &storedInfo, sizeof(StoredInfo));
      if (isValid) {
        mTimeZone = TimeZone(storedInfo.tzCode);
      } else {
        mTimeZone = TimeZone(kDefaultTzCode);
      }

      // Set the current date time using the mTimeZone.
      mCurrentDateTime = DateTime(nowSeconds, mTimeZone);
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
      mCurrentDateTime = DateTime(mTimeKeeper.getNow(), mTimeZone);

      // If in CHANGE mode, and the 'second' field has not been cleared,
      // update the mChangingDateTime.second field with the current second.
      switch (mMode) {
        case MODE_CHANGE_YEAR:
        case MODE_CHANGE_MONTH:
        case MODE_CHANGE_DAY:
        case MODE_CHANGE_HOUR:
        case MODE_CHANGE_MINUTE:
        case MODE_CHANGE_SECOND:
#if ENABLE_UNIFIED_DATE == 1
        case MODE_CHANGE_TIME_ZONE_HOUR:
        case MODE_CHANGE_TIME_ZONE_MINUTE:
#endif
          if (!mSecondFieldCleared) {
            mChangingDateTime.second(mCurrentDateTime.second());
          }
          break;
      }
    }

    /** Update the blinkShowState. */
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
          mPresenter.setDateTime(mCurrentDateTime);
          break;

        case MODE_CHANGE_YEAR:
        case MODE_CHANGE_MONTH:
        case MODE_CHANGE_DAY:
        case MODE_CHANGE_HOUR:
        case MODE_CHANGE_MINUTE:
        case MODE_CHANGE_SECOND:
        case MODE_CHANGE_TIME_ZONE_HOUR:
        case MODE_CHANGE_TIME_ZONE_MINUTE:
          mPresenter.setDateTime(mChangingDateTime);
          break;
      }
    }

    /** Save the current UTC DateTime to the RTC. */
    void saveDateTime() {
      mTimeKeeper.setNow(mChangingDateTime.toSecondsSinceEpoch());
    }

    void saveTimeZone() {
      mTimeZone = mChangingDateTime.timeZone();
      mCurrentDateTime = mCurrentDateTime.convertToTimeZone(mTimeZone);
      preserveInfo(); // save mTimeZone in EEPROM
    }

    /** Read the UTC DateTime from RTC and convert to current time zone. */
    void readDateTime(DateTime* dateTime) {
      uint32_t now = mTimeKeeper.getNow();
      *dateTime = DateTime(now, mTimeZone);
    }

    void readTemperature(HardwareTemperature* temperature) {
      if (mDS3231) {
        mDS3231->readTemperature(temperature);
      }
    }

    void preserveInfo() {
      StoredInfo storedInfo;
      storedInfo.tzCode = mTimeZone.tzCode();
      mCrcEeprom.writeWithCrc(kStoredInfoEepromAddress, &storedInfo,
          sizeof(StoredInfo));
    }

  protected:
    TimeKeeper& mTimeKeeper;
    CrcEeprom& mCrcEeprom;
    Presenter& mPresenter;
    const DS3231* const mDS3231;

    uint8_t mMode = MODE_UNKNOWN; // current mode
    TimeZone mTimeZone; // current time zone of clock
    DateTime mCurrentDateTime; // DateTime from the TimeKeeper
    DateTime mChangingDateTime; // DateTime set by user in "Change" modes
    bool mSecondFieldCleared;
    bool mSuppressBlink; // true if blinking should be suppressed

    bool mBlinkShowState = true; // true means actually show
    uint16_t mBlinkCycleStartMillis = 0; // millis since blink cycle start
    bool mIsPreparingToSleep = false;
};

#endif
