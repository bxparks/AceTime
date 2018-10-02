#ifndef CLOCK_OLED_CLOCK_H
#define CLOCK_OLED_CLOCK_H

#include <SSD1306AsciiWire.h>
#include <AceTime.h>
#include <ace_time/hw/CrcEeprom.h>
#include "Clock.h"
#include "OledPresenter.h"
#include "config.h"

#if DISPLAY_TYPE == DISPLAY_TYPE_OLED

class OledClock: public Clock {
  public:
    /** Constructor. */
    OledClock(TimeKeeper& timeKeeper, hw::CrcEeprom& crcEeprom,
            OledPresenter& presenter):
        Clock(timeKeeper, crcEeprom, presenter) {
      mMode = MODE_HOUR_MINUTE;
    }

    virtual void modeButtonPress() override {
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

    virtual void modeButtonLongPress() override {
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

    virtual void changeButtonPress() override {
      switch (mMode) {
        case MODE_CHANGE_HOUR:
          mSuppressBlink = true;
          mChangingDateTime.incrementHour();
          break;
        case MODE_CHANGE_MINUTE:
          mSuppressBlink = true;
          mChangingDateTime.incrementMinute();
          break;
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
      }

      // Update the display right away to prevent jitters in the display when
      // the button is triggering RepeatPressed events.
      update();
    }

    virtual void changeButtonRepeatPress() override {
      changeButtonPress();
    }

    virtual void changeButtonRelease() override {
      switch (mMode) {
        case MODE_CHANGE_YEAR:
        case MODE_CHANGE_MONTH:
        case MODE_CHANGE_DAY:
        case MODE_CHANGE_HOUR:
        case MODE_CHANGE_MINUTE:
        case MODE_CHANGE_TIME_ZONE_HOUR:
        case MODE_CHANGE_TIME_ZONE_MINUTE:
          mSuppressBlink = false;
          break;
      }
    }
};

#endif

#endif
