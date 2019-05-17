#ifndef CLOCK_LED_CLOCK_H
#define CLOCK_LED_CLOCK_H

#include <AceTime.h>
#include <AceSegment.h>
#include "Clock.h"
#include "LedPresenter.h"
#include "config.h"

#if DISPLAY_TYPE == DISPLAY_TYPE_LED

using namespace ace_segment;

class LedClock: public Clock {
  public:
    /** Constructor. */
    LedClock(TimeKeeper& timeKeeper, CrcEeprom& crcEeprom,
          LedPresenter& presenter):
        Clock(timeKeeper, crcEeprom, presenter) {
      mMode = MODE_HOUR_MINUTE;
    }

    void modeButtonPress() override {
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

    void modeButtonLongPress() override {
      switch (mMode) {
        case MODE_HOUR_MINUTE:
          mChangingDateTime = mCurrentDateTime;
          mSecondFieldCleared = false;
          mMode = MODE_CHANGE_HOUR;
          break;
        case MODE_MINUTE_SECOND:
          mChangingDateTime = mCurrentDateTime;
          mSecondFieldCleared = false;
          mMode = MODE_CHANGE_MINUTE;
          break;

        case MODE_YEAR:
          mChangingDateTime = mCurrentDateTime;
          mSecondFieldCleared = false;
          mMode = MODE_CHANGE_YEAR;
          break;
        case MODE_MONTH:
          mChangingDateTime = mCurrentDateTime;
          mSecondFieldCleared = false;
          mMode = MODE_CHANGE_MONTH;
          break;
        case MODE_DAY:
          mChangingDateTime = mCurrentDateTime;
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

    void changeButtonPress() override {
      switch (mMode) {
        case MODE_CHANGE_HOUR:
          mSuppressBlink = true;
          date_time_mutation::incrementHour(mChangingDateTime);
          break;
        case MODE_CHANGE_MINUTE:
          mSuppressBlink = true;
          date_time_mutation::incrementMinute(mChangingDateTime);
          break;
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
      }

      // Update the display right away to prevent jitters in the display when
      // the button is triggering RepeatPressed events.
      update();
    }

    void changeButtonRepeatPress() override {
      changeButtonPress();
    }

    void changeButtonRelease() override {
      switch (mMode) {
        case MODE_CHANGE_YEAR:
        case MODE_CHANGE_MONTH:
        case MODE_CHANGE_DAY:
        case MODE_CHANGE_HOUR:
        case MODE_CHANGE_MINUTE:
        case MODE_CHANGE_SECOND:
        case MODE_CHANGE_TIME_ZONE_HOUR:
        case MODE_CHANGE_TIME_ZONE_MINUTE:
          mSuppressBlink = false;
          break;
      }
    }
};

#endif

#endif
