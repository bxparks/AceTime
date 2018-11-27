#ifndef WORLD_CLOCK_CONTROLLER_H
#define WORLD_CLOCK_CONTROLLER_H

#include <AceTime.h>
#include <ace_time/hw/CrcEeprom.h>
#include <ace_time/common/DateStrings.h>
#include "RenderingInfo.h"
#include "StoredInfo.h"
#include "Presenter.h"

using namespace ace_time;
using namespace ace_time::common;
using namespace ace_time::provider;

/**
 * Maintains the internal state of the world clock, handling button inputs,
 * and calling out to the Presenter to display the clock. In an MVC
 * architecture, this would be the Controller. The Model would be the various
 * member variables in thic class. The View layer is the Presenter class.
 */
class Controller {
  public:
    static const uint16_t kStoredInfoEepromAddress = 0;

    /**
     * Constructor.
     * @param timeKeeper source of the current time
     * @param crcEeprom stores objects into the EEPROM with CRC
     * @param presenter renders the date and time info to the screen
     */
    Controller(TimeKeeper& timeKeeper, hw::CrcEeprom& crcEeprom,
            Presenter& presenter0, Presenter& presenter1,
            Presenter& presenter2):
        mTimeKeeper(timeKeeper),
        mCrcEeprom(crcEeprom),
        mPresenter0(presenter0),
        mPresenter1(presenter1),
        mPresenter2(presenter2),
        mMode(MODE_DATE_TIME) {}

    /** Initialize the controller with the various time zones of each clock. */
    void setup();

    /**
     * This should be called every 0.1s to support blinking mode and to avoid
     * noticeable drift against the RTC which has a 1 second resolution.
     */
    void update() {
      if (mMode == MODE_UNKNOWN) return;
      updateDateTime();
      updateBlinkState();
      updateRenderingInfo();

      mPresenter0.display();
      mPresenter1.display();
      mPresenter2.display();
    }

    void modeButtonPress() {
      switch (mMode) {
        case MODE_DATE_TIME:
          mMode = MODE_CLOCK_INFO;
          break;
        case MODE_CLOCK_INFO:
          mMode = MODE_DATE_TIME;
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
        case MODE_CHANGE_HOUR:
          mMode = MODE_CHANGE_MINUTE;
          break;
        case MODE_CHANGE_MINUTE:
          mMode = MODE_CHANGE_SECOND;
          break;
        case MODE_CHANGE_SECOND:
          mMode = MODE_CHANGE_YEAR;
          break;

        case MODE_CHANGE_HOUR_MODE:
          mMode = MODE_CHANGE_BLINKING_COLON;
          break;
        case MODE_CHANGE_BLINKING_COLON:
#if TIME_ZONE_TYPE == TIME_ZONE_TYPE_MANUAL
          mMode = MODE_CHANGE_TIME_ZONE_DST;
          break;
        case MODE_CHANGE_TIME_ZONE_DST:
#endif
          mMode = MODE_CHANGE_HOUR_MODE;
          break;
      }
    }

    void modeButtonLongPress() {
      switch (mMode) {
        case MODE_DATE_TIME:
          mChangingDateTime = DateTime::forEpochSeconds(
              mTimeKeeper.getNow(), &mClockInfo0.timeZone);
          mChangingClockInfo = mClockInfo0;
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

        case MODE_CLOCK_INFO:
          mChangingClockInfo = mClockInfo0;
          mMode = MODE_CHANGE_HOUR_MODE;
          break;

        case MODE_CHANGE_HOUR_MODE:
        case MODE_CHANGE_BLINKING_COLON:
#if TIME_ZONE_TYPE == TIME_ZONE_TYPE_MANUAL
        case MODE_CHANGE_TIME_ZONE_DST:
#endif
          saveClockInfo();
          mMode = MODE_CLOCK_INFO;
          break;
      }
    }

    void changeButtonPress() {
      switch (mMode) {
        case MODE_CHANGE_YEAR:
          mSuppressBlink = true;
          mChangingDateTime.incrementYear();
          break;
        case MODE_CHANGE_MONTH:
          mSuppressBlink = true;
          mChangingDateTime.incrementMonth();
          break;
        case MODE_CHANGE_DAY:
          mSuppressBlink = true;
          mChangingDateTime.incrementDay();
          break;
        case MODE_CHANGE_HOUR:
          mSuppressBlink = true;
          mChangingDateTime.incrementHour();
          break;
        case MODE_CHANGE_MINUTE:
          mSuppressBlink = true;
          mChangingDateTime.incrementMinute();
          break;
        case MODE_CHANGE_SECOND:
          mSuppressBlink = true;
          mChangingDateTime.second(0);
          mSecondFieldCleared = true;
          break;

        case MODE_CHANGE_HOUR_MODE:
          mSuppressBlink = true;
          mChangingClockInfo.hourMode = 1 - mChangingClockInfo.hourMode;
          break;
        case MODE_CHANGE_BLINKING_COLON:
          mSuppressBlink = true;
          mChangingClockInfo.blinkingColon = !mChangingClockInfo.blinkingColon;
          break;
#if TIME_ZONE_TYPE == TIME_ZONE_TYPE_MANUAL
        case MODE_CHANGE_TIME_ZONE_DST:
          mSuppressBlink = true;
          mChangingClockInfo.timeZone.isDst(
              !mChangingClockInfo.timeZone.isDst());
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
        case MODE_CHANGE_HOUR_MODE:
        case MODE_CHANGE_BLINKING_COLON:
#if TIME_ZONE_TYPE == TIME_ZONE_TYPE_MANUAL
        case MODE_CHANGE_TIME_ZONE_DST:
#endif
          mSuppressBlink = false;
          break;
      }
    }
  protected:
    void updateDateTime() {
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
            DateTime dt = DateTime::forEpochSeconds(mTimeKeeper.getNow());
            mChangingDateTime.second(dt.second());
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
      updatePresenter(mPresenter0, mClockInfo0);
      updatePresenter(mPresenter1, mClockInfo1);
      updatePresenter(mPresenter2, mClockInfo2);
    }

    void updatePresenter(Presenter& presenter, ClockInfo& clockInfo) {
      presenter.setMode(mMode);
      presenter.setSuppressBlink(mSuppressBlink);
      presenter.setBlinkShowState(mBlinkShowState);

      switch (mMode) {
        case MODE_DATE_TIME:
        case MODE_CLOCK_INFO:
          presenter.setNow(mTimeKeeper.getNow());
          presenter.setClockInfo(clockInfo);
          break;

        case MODE_CHANGE_YEAR:
        case MODE_CHANGE_MONTH:
        case MODE_CHANGE_DAY:
        case MODE_CHANGE_HOUR:
        case MODE_CHANGE_MINUTE:
        case MODE_CHANGE_SECOND:
        case MODE_CHANGE_HOUR_MODE:
        case MODE_CHANGE_BLINKING_COLON:
#if TIME_ZONE_TYPE == TIME_ZONE_TYPE_MANUAL
        case MODE_CHANGE_TIME_ZONE_DST:
#endif
        {
          presenter.setNow(mChangingDateTime.toEpochSeconds());
          presenter.setClockInfo(mChangingClockInfo);
          break;
        }
      }
    }

    /** Save the current UTC DateTime to the RTC. */
    void saveDateTime() {
      mTimeKeeper.setNow(mChangingDateTime.toEpochSeconds());
    }

    void saveClockInfo() {
      mClockInfo0 = mChangingClockInfo;

      mClockInfo1.hourMode = mChangingClockInfo.hourMode;
      mClockInfo1.blinkingColon = mChangingClockInfo.blinkingColon;

      mClockInfo2.hourMode = mChangingClockInfo.hourMode;
      mClockInfo2.blinkingColon = mChangingClockInfo.blinkingColon;

#if TIME_ZONE_TYPE == TIME_ZONE_TYPE_MANUAL
      mClockInfo1.timeZone.isDst(mChangingClockInfo.timeZone.isDst());
      mClockInfo2.timeZone.isDst(mChangingClockInfo.timeZone.isDst());
#endif

      preserveInfo();
    }

    /** Read the UTC DateTime from RTC and convert to current time zone. */
    void readDateTime(DateTime* dateTime) {
      uint32_t now = mTimeKeeper.getNow();
      *dateTime = DateTime::forEpochSeconds(now, &mClockInfo0.timeZone);
    }

    void preserveInfo() {
      // Create StoreInfo from clock0. The others will be identical.
      // TODO: isDst should be saved for each clock
      StoredInfo storedInfo;
      storedInfo.isDst = mClockInfo0.timeZone.isDst();
      storedInfo.hourMode = mClockInfo0.hourMode;
      storedInfo.blinkingColon = mClockInfo0.blinkingColon;

      mCrcEeprom.writeWithCrc(kStoredInfoEepromAddress, &storedInfo,
          sizeof(StoredInfo));
    }

  private:
    // Disable copy-constructor and assignment operator
    Controller(const Controller&) = delete;
    Controller& operator=(const Controller&) = delete;

    TimeKeeper& mTimeKeeper;
    hw::CrcEeprom& mCrcEeprom;
    Presenter& mPresenter0;
    Presenter& mPresenter1;
    Presenter& mPresenter2;
    ClockInfo mClockInfo0;
    ClockInfo mClockInfo1;
    ClockInfo mClockInfo2;

    uint8_t mMode; // current mode
    DateTime mChangingDateTime; // DateTime set by user in "Change" modes
    ClockInfo mChangingClockInfo; // ClockInfo set by user in "Change" modes

    bool mSecondFieldCleared;
    bool mSuppressBlink; // true if blinking should be suppressed

    bool mBlinkShowState = true; // true means actually show
    uint16_t mBlinkCycleStartMillis = 0; // millis since blink cycle start
};

#endif
