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
      mMode = MODE_DATE_TIME;
    }

    virtual void modeButtonPress() override {
      switch (mMode) {
        case MODE_DATE_TIME:
          mMode = MODE_WEEKDAY;
          break;
        case MODE_WEEKDAY:
          mMode = MODE_TIME_ZONE;
          break;
        case MODE_TIME_ZONE:
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

        case MODE_CHANGE_TIME_ZONE_HOUR:
          mMode = MODE_CHANGE_TIME_ZONE_MINUTE;
          break;
        case MODE_CHANGE_TIME_ZONE_MINUTE:
          mMode = MODE_CHANGE_TIME_ZONE_DST;
          break;
        case MODE_CHANGE_TIME_ZONE_DST:
          mMode = MODE_CHANGE_TIME_ZONE_HOUR;
          break;
      }
    }

    virtual void modeButtonLongPress() override {
      switch (mMode) {
        case MODE_DATE_TIME:
          mChangingDateTime = mCurrentDateTime;
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
          mChangingDateTime.timeZone(mTimeZone);
          mMode = MODE_CHANGE_TIME_ZONE_HOUR;
          break;

        case MODE_CHANGE_TIME_ZONE_HOUR:
        case MODE_CHANGE_TIME_ZONE_MINUTE:
        case MODE_CHANGE_TIME_ZONE_DST:
          saveTimeZone();
          mMode = MODE_TIME_ZONE;
          break;
      }
    }

    virtual void changeButtonPress() override {
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

        case MODE_CHANGE_TIME_ZONE_HOUR:
          mSuppressBlink = true;
          mChangingDateTime.timeZone().incrementHour();
          break;
        case MODE_CHANGE_TIME_ZONE_MINUTE:
          mSuppressBlink = true;
          mChangingDateTime.timeZone().increment15Minutes();
          break;
        case MODE_CHANGE_TIME_ZONE_DST:
          mSuppressBlink = true;
          mChangingDateTime.timeZone().isDst(
              !mChangingDateTime.timeZone().isDst());
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
        case MODE_CHANGE_SECOND:
        case MODE_CHANGE_TIME_ZONE_HOUR:
        case MODE_CHANGE_TIME_ZONE_MINUTE:
        case MODE_CHANGE_TIME_ZONE_DST:
          mSuppressBlink = false;
          break;
      }
    }
};

#endif

#endif
