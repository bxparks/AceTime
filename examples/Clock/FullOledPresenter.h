#ifndef CLOCK_FULL_OLED_PRESENTER_H
#define CLOCK_FULL_OLED_PRESENTER_H

#include <SSD1306AsciiWire.h>
#include <AceTime.h>
#include "RenderingInfo.h"
#include "Presenter.h"
#include "config.h"

#if DISPLAY_TYPE == DISPLAY_TYPE_FULL_OLED

using namespace ace_time;
using namespace ace_time::common;

class FullOledPresenter: public Presenter {
  public:
    FullOledPresenter(SSD1306Ascii& oled):
        mOled(oled) {}

  private:
    virtual void clearDisplay() override { mOled.clear(); }

    virtual void displayData() override {
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
          displayWeekday();
          break;

        case MODE_TIME_ZONE:
        case MODE_CHANGE_TIME_ZONE_HOUR:
        case MODE_CHANGE_TIME_ZONE_MINUTE:
          displayTimeZone();
          break;

        case MODE_TEMPERATURE:
          displayTemperature();
          break;
      }
    }

    void displayDateTime() const {
      const DateTime& dateTime = mRenderingInfo.dateTime;
      if (shouldShowFor(MODE_CHANGE_YEAR)) {
        mOled.print("20");
        printPad2(mOled, dateTime.year());
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
      } else{
        mOled.print("  ");
      }
      mOled.println();

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
    }

    void displayWeekday() const {
      const DateTime& dateTime = mRenderingInfo.dateTime;
      mOled.print(DateStrings().weekDayLongString(dateTime.dayOfWeek()));
      mOled.clearToEOL();
    }

    void displayTimeZone() const {
      const TimeZone& timeZone = mRenderingInfo.dateTime.timeZone();
      uint8_t hour;
      uint8_t minute;
      timeZone.extractHourMinute(hour, minute);

      mOled.print("UTC");
      if (shouldShowFor(MODE_CHANGE_TIME_ZONE_HOUR)) {
        mOled.print((timeZone.tzCode() < 0) ? '-' : '+');
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
    }

    void displayTemperature() const {
      const HardwareTemperature& temperature = mRenderingInfo.temperature;
      temperature.printTo(mOled);
      mOled.print('C');
    }

  private:
    SSD1306Ascii& mOled;
};

#endif

#endif
