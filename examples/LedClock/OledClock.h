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

    void modeButtonPress() override {
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

        case MODE_CHANGE_TIME_ZONE_OFFSET:
          mMode = MODE_CHANGE_TIME_ZONE_DST;
          break;
        case MODE_CHANGE_TIME_ZONE_DST:
          mMode = MODE_CHANGE_TIME_ZONE_OFFSET;
          break;
      }
    }

    void modeButtonLongPress() override {
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
          saveClockInfo();
          mMode = MODE_DATE_TIME;
          break;

        case MODE_TIME_ZONE:
          mChangingDateTime.timeZone(mTimeZone);
          mMode = MODE_CHANGE_TIME_ZONE_OFFSET;
          break;

        case MODE_CHANGE_TIME_ZONE_OFFSET:
        case MODE_CHANGE_TIME_ZONE_DST:
          saveTimeZone();
          mMode = MODE_TIME_ZONE;
          break;
      }
    }

    void changeButtonPress() override {
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

        case MODE_CHANGE_TIME_ZONE_OFFSET:
          mSuppressBlink = true;
          mChangingDateTime.timeZone().getStandardTimeOffset()
              .increment15Minutes();
          break;
        case MODE_CHANGE_TIME_ZONE_DST:
          mSuppressBlink = true;
          mChangingDateTime.timeZone().setStandardDst(
              !mChangingDateTime.timeZone().getStandardDst());
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
        case MODE_CHANGE_TIME_ZONE_OFFSET:
        case MODE_CHANGE_TIME_ZONE_DST:
          mSuppressBlink = false;
          break;
      }
    }
};

#endif

#endif
