#ifndef CLOCK_OLED_PRESENTER_H
#define CLOCK_OLED_PRESENTER_H

#include <SSD1306AsciiWire.h>
#include <AceTime.h>
#include "RenderingInfo.h"
#include "Presenter.h"
#include "config.h"

#if DISPLAY_TYPE == DISPLAY_TYPE_OLED

using namespace ace_time;
using namespace ace_time::common;

class OledPresenter: public Presenter {
  public:
    OledPresenter(SSD1306Ascii& oled):
        mOled(oled) {}

  private:
    void clearDisplay() override { mOled.clear(); }

    void displayData() override {
      mOled.home();
      mOled.setFont(lcd5x7);
      mOled.set2X();

      switch (mRenderingInfo.mode) {
        case MODE_DATE_TIME:
        case MODE_CHANGE_YEAR:
        case MODE_CHANGE_MONTH:
        case MODE_CHANGE_DAY:
        case MODE_CHANGE_HOUR:
        case MODE_CHANGE_MINUTE:
        case MODE_CHANGE_SECOND:
          displayDateTime();
          break;

        case MODE_WEEKDAY:
          displayWeekDay();
          break;

        case MODE_TIME_ZONE:
        case MODE_CHANGE_TIME_ZONE_HOUR:
        case MODE_CHANGE_TIME_ZONE_MINUTE:
        case MODE_CHANGE_TIME_ZONE_DST:
          displayTimeZone();
          break;
      }
    }

    void displayDateTime() const {
      const ZonedDateTime& dateTime = mRenderingInfo.dateTime;

      // date
      if (shouldShowFor(MODE_CHANGE_YEAR)) {
        mOled.print(dateTime.year());
      } else {
        mOled.print("    ");
      }
      mOled.print('-');
      if (shouldShowFor(MODE_CHANGE_MONTH)) {
        printPad2(mOled, dateTime.month());
      } else {
        mOled.print("  ");
      }
      mOled.print('-');
      if (shouldShowFor(MODE_CHANGE_DAY)) {
        printPad2(mOled, dateTime.day());
      } else {
        mOled.print("  ");
      }
      mOled.clearToEOL();
      mOled.println();

      // time
      if (shouldShowFor(MODE_CHANGE_HOUR)) {
        printPad2(mOled, dateTime.hour());
      } else {
        mOled.print("  ");
      }
      mOled.print(':');
      if (shouldShowFor(MODE_CHANGE_MINUTE)) {
        printPad2(mOled, dateTime.minute());
      } else {
        mOled.print("  ");
      }
      mOled.print(':');
      if (shouldShowFor(MODE_CHANGE_SECOND)) {
        printPad2(mOled, dateTime.second());
      } else {
        mOled.print("  ");
      }
      mOled.clearToEOL();
      mOled.println();
    }

    void displayWeekDay() const {
      const ZonedDateTime& dateTime = mRenderingInfo.dateTime;
      mOled.print(DateStrings().weekDayLongString(dateTime.dayOfWeek()));
      mOled.clearToEOL();
    }

    void displayTimeZone() const {
      const TimeZone& timeZone = mRenderingInfo.dateTime.timeZone();
      const TimeOffset timeOffset = timeZone.getStandardTimeOffset();
      int8_t hour;
      uint8_t minute;
      timeOffset.toHourMinute(hour, minute);

      mOled.print("UTC");
      if (shouldShowFor(MODE_CHANGE_TIME_ZONE_HOUR)) {
        mOled.print((hour < 0) ? '-' : '+');
        if (hour < 0) hour = -hour;
        printPad2(mOled, hour);
      } else {
        mOled.print("   ");
      }
      mOled.print(':');
      if (shouldShowFor(MODE_CHANGE_TIME_ZONE_MINUTE)) {
        printPad2(mOled, minute);
      } else {
        mOled.print("  ");
      }
      mOled.println();
      mOled.print("DST: ");
      if (shouldShowFor(MODE_CHANGE_TIME_ZONE_DST)) {
        mOled.print(timeZone.getStandardDst() ? "on " : "off");
      } else {
        mOled.print("   ");
      }
    }

  private:
    SSD1306Ascii& mOled;
};

#endif

#endif
