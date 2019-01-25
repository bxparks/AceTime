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
        mMode(MODE_UNKNOWN) {}

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
          mMode = MODE_CHANGE_TIME_ZONE_DST0;
          break;
        case MODE_CHANGE_TIME_ZONE_DST0:
          mMode = MODE_CHANGE_TIME_ZONE_DST1;
          break;
        case MODE_CHANGE_TIME_ZONE_DST1:
          mMode = MODE_CHANGE_TIME_ZONE_DST2;
          break;
        case MODE_CHANGE_TIME_ZONE_DST2:
#endif
          mMode = MODE_CHANGE_HOUR_MODE;
          break;
      }
    }

    void modeButtonLongPress() {
      switch (mMode) {
        case MODE_DATE_TIME:
          mChangingDateTime = ZonedDateTime::forEpochSeconds(
              mTimeKeeper.getNow(), mClockInfo0.timeZone);
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
          mMode = MODE_CHANGE_HOUR_MODE;
          break;

        case MODE_CHANGE_HOUR_MODE:
        case MODE_CHANGE_BLINKING_COLON:
#if TIME_ZONE_TYPE == TIME_ZONE_TYPE_MANUAL
        case MODE_CHANGE_TIME_ZONE_DST0:
        case MODE_CHANGE_TIME_ZONE_DST1:
        case MODE_CHANGE_TIME_ZONE_DST2:
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
          date_time_mutation::incrementYear(mChangingDateTime);
          break;
        case MODE_CHANGE_MONTH:
          mSuppressBlink = true;
          date_time_mutation::incrementMonth(mChangingDateTime);
          break;
        case MODE_CHANGE_DAY:
          mSuppressBlink = true;
          date_time_mutation::incrementDay(mChangingDateTime);
          break;
        case MODE_CHANGE_HOUR:
          mSuppressBlink = true;
          date_time_mutation::incrementHour(mChangingDateTime);
          break;
        case MODE_CHANGE_MINUTE:
          mSuppressBlink = true;
          date_time_mutation::incrementMinute(mChangingDateTime);
          break;
        case MODE_CHANGE_SECOND:
          mSuppressBlink = true;
          mChangingDateTime.second(0);
          mSecondFieldCleared = true;
          break;

        case MODE_CHANGE_HOUR_MODE:
          mSuppressBlink = true;
          mClockInfo0.hourMode = 1 - mClockInfo0.hourMode;
          mClockInfo1.hourMode = 1 - mClockInfo1.hourMode;
          mClockInfo2.hourMode = 1 - mClockInfo2.hourMode;
          break;
        case MODE_CHANGE_BLINKING_COLON:
          mSuppressBlink = true;
          mClockInfo0.blinkingColon = !mClockInfo0.blinkingColon;
          mClockInfo1.blinkingColon = !mClockInfo1.blinkingColon;
          mClockInfo2.blinkingColon = !mClockInfo2.blinkingColon;
          break;
#if TIME_ZONE_TYPE == TIME_ZONE_TYPE_MANUAL
        case MODE_CHANGE_TIME_ZONE_DST0:
          mSuppressBlink = true;
          mClockInfo0.timeZone.isDst(!mClockInfo0.timeZone.isDst());
          break;

        case MODE_CHANGE_TIME_ZONE_DST1:
          mSuppressBlink = true;
          mClockInfo1.timeZone.isDst(!mClockInfo1.timeZone.isDst());
          break;

        case MODE_CHANGE_TIME_ZONE_DST2:
          mSuppressBlink = true;
          mClockInfo2.timeZone.isDst(!mClockInfo2.timeZone.isDst());
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
        case MODE_CHANGE_TIME_ZONE_DST0:
        case MODE_CHANGE_TIME_ZONE_DST1:
        case MODE_CHANGE_TIME_ZONE_DST2:
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
            ZonedDateTime dt = ZonedDateTime::forEpochSeconds(
                mTimeKeeper.getNow());
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
      switch (mMode) {
        case MODE_DATE_TIME:
        case MODE_CLOCK_INFO: {
          acetime_t now = mTimeKeeper.getNow();
          mPresenter0.update(mMode, now, mBlinkShowState, mSuppressBlink,
              mClockInfo0);
          mPresenter1.update(mMode, now, mBlinkShowState, mSuppressBlink,
              mClockInfo1);
          mPresenter2.update(mMode, now, mBlinkShowState, mSuppressBlink,
              mClockInfo2);
          break;
        }

        case MODE_CHANGE_YEAR:
        case MODE_CHANGE_MONTH:
        case MODE_CHANGE_DAY:
        case MODE_CHANGE_HOUR:
        case MODE_CHANGE_MINUTE:
        case MODE_CHANGE_SECOND:
        case MODE_CHANGE_HOUR_MODE:
        case MODE_CHANGE_BLINKING_COLON: {
          acetime_t now = mChangingDateTime.toEpochSeconds();
          mPresenter0.update(mMode, now, mBlinkShowState, mSuppressBlink,
              mClockInfo0);
          mPresenter1.update(mMode, now, mBlinkShowState, true, mClockInfo1);
          mPresenter2.update(mMode, now, mBlinkShowState, true, mClockInfo2);
          break;
        }

#if TIME_ZONE_TYPE == TIME_ZONE_TYPE_MANUAL
        case MODE_CHANGE_TIME_ZONE_DST0:
          updateChangingDst(0);
          break;
        case MODE_CHANGE_TIME_ZONE_DST1:
          updateChangingDst(1);
          break;
        case MODE_CHANGE_TIME_ZONE_DST2:
          updateChangingDst(2);
          break;
#endif
      }
    }

    void updateChangingDst(uint8_t clockId) {
      acetime_t now = mChangingDateTime.toEpochSeconds();
      mPresenter0.update(mMode, now, mBlinkShowState, clockId!=0, mClockInfo0);
      mPresenter1.update(mMode, now, mBlinkShowState, clockId!=1, mClockInfo1);
      mPresenter2.update(mMode, now, mBlinkShowState, clockId!=2, mClockInfo2);
    }

    /** Save the current UTC ZonedDateTime to the RTC. */
    void saveDateTime() {
      mTimeKeeper.setNow(mChangingDateTime.toEpochSeconds());
    }

    void saveClockInfo() {
      preserveInfo();
    }

    /** Read the UTC ZonedDateTime from RTC and convert to current time zone. */
    void readDateTime(ZonedDateTime* dateTime) {
      acetime_t now = mTimeKeeper.getNow();
      *dateTime = ZonedDateTime::forEpochSeconds(now, mClockInfo0.timeZone);
    }

    void preserveInfo() {
      StoredInfo storedInfo;

      // Create hourMode and blinkingColon from clock0. The others will be
      // identical.
      storedInfo.hourMode = mClockInfo0.hourMode;
      storedInfo.blinkingColon = mClockInfo0.blinkingColon;

#if TIME_ZONE_TYPE == TIME_ZONE_TYPE_MANUAL
      storedInfo.isDst0 = mClockInfo0.timeZone.isDst();
      storedInfo.isDst1 = mClockInfo1.timeZone.isDst();
      storedInfo.isDst2 = mClockInfo2.timeZone.isDst();
#endif

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
    ZonedDateTime mChangingDateTime; // source of now() in "Change" modes

    bool mSecondFieldCleared;
    bool mSuppressBlink; // true if blinking should be suppressed

    bool mBlinkShowState = true; // true means actually show
    uint16_t mBlinkCycleStartMillis = 0; // millis since blink cycle start
};

#endif
