#ifndef CLOCK_FULL_OLED_CLOCK_H
#define CLOCK_FULL_OLED_CLOCK_H

#include <SSD1306AsciiWire.h>
#include <AceTime.h>
#include <ace_time/hw/CrcEeprom.h>
#include "Clock.h"
#include "FullOledPresenter.h"
#include "config.h"

#if DISPLAY_TYPE == DISPLAY_TYPE_FULL_OLED

class FullOledClock: public Clock {
  public:
    /** Constructor. */
    FullOledClock(TimeKeeper& timeKeeper, hw::CrcEeprom& crcEeprom,
            FullOledPresenter& presenter):
        Clock(timeKeeper, crcEeprom, presenter) {
      mMode = MODE_DATE_TIME;
    }

    void modeButtonPress() override {
      switch (mMode) {
        case MODE_DATE_TIME:
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
          mMode = MODE_CHANGE_HOUR_MODE;
          break;
        case MODE_CHANGE_HOUR_MODE:
          mMode = MODE_CHANGE_TIME_ZONE_HOUR;
          break;
      }
    }

    void modeButtonLongPress() override {
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
          mMode = MODE_CHANGE_TIME_ZONE_HOUR;
          break;

        case MODE_CHANGE_TIME_ZONE_HOUR:
        case MODE_CHANGE_TIME_ZONE_MINUTE:
        case MODE_CHANGE_TIME_ZONE_DST:
        case MODE_CHANGE_HOUR_MODE:
          saveClockInfo();
          mMode = MODE_TIME_ZONE;
          break;
      }
    }

    void changeButtonPress() override {
      switch (mMode) {
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
        case MODE_CHANGE_HOUR:
          mSuppressBlink = true;
          date_time_mutation::incrementHour(mChangingClockInfo.dateTime);
          break;
        case MODE_CHANGE_MINUTE:
          mSuppressBlink = true;
          date_time_mutation::incrementMinute(mChangingClockInfo.dateTime);
          break;
        case MODE_CHANGE_SECOND:
          mSuppressBlink = true;
          mChangingClockInfo.dateTime.second(0);
          mSecondFieldCleared = true;
          break;

        case MODE_CHANGE_TIME_ZONE_HOUR:
          {
            mSuppressBlink = true;
            UtcOffset offset = mChangingClockInfo.zoneSpecifier.stdOffset();
            utc_offset_mutation::incrementHour(offset);
            mChangingClockInfo.zoneSpecifier.stdOffset(offset);
          }
          break;
        case MODE_CHANGE_TIME_ZONE_MINUTE:
          {
            mSuppressBlink = true;
            UtcOffset offset = mChangingClockInfo.zoneSpecifier.stdOffset();
            utc_offset_mutation::increment15Minutes(offset);
            mChangingClockInfo.zoneSpecifier.stdOffset(offset);
          }
          break;
        case MODE_CHANGE_TIME_ZONE_DST:
          mSuppressBlink = true;
          mChangingClockInfo.zoneSpecifier.isDst(
              !mChangingClockInfo.zoneSpecifier.isDst());
          break;
        case MODE_CHANGE_HOUR_MODE:
          mSuppressBlink = true;
          mChangingClockInfo.hourMode = 1 - mChangingClockInfo.hourMode;
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
        case MODE_CHANGE_TIME_ZONE_DST:
        case MODE_CHANGE_HOUR_MODE:
          mSuppressBlink = false;
          break;
      }
    }
};

#endif

#endif
