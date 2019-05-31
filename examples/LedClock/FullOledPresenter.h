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
    void clearDisplay() override { mOled.clear(); }

    void displayData() override {
      mOled.home();
      mOled.setFont(fixed_bold10x15);
      //mOled.set2X();

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

        case MODE_TIME_ZONE:
        case MODE_CHANGE_TIME_ZONE_HOUR:
        case MODE_CHANGE_TIME_ZONE_MINUTE:
        case MODE_CHANGE_TIME_ZONE_DST:
        case MODE_CHANGE_HOUR_MODE:
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
      } else{
        mOled.print("  ");
      }
      mOled.clearToEOL();
      mOled.println();

      // time
      if (shouldShowFor(MODE_CHANGE_HOUR)) {
        uint8_t hour = dateTime.hour();
        if (mRenderingInfo.hourMode == StoredInfo::kTwelve) {
          if (hour == 0) {
            hour = 12;
          } else if (hour > 12) {
            hour -= 12;
          }
          printPad2(mOled, hour, ' ');
        } else {
          printPad2(mOled, hour);
        }
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
      mOled.print(' ');
      if (mRenderingInfo.hourMode == StoredInfo::kTwelve) {
        mOled.print((dateTime.hour() < 12) ? "AM" : "PM");
      }
      mOled.clearToEOL();
      mOled.println();

      // week day
      mOled.print(DateStrings().weekDayLongString(dateTime.dayOfWeek()));
      mOled.clearToEOL();
    }

    void displayTimeZone() const {
      int8_t hour;
      uint8_t minute;
      const ManualZoneSpecifier& zoneSpecifier = mRenderingInfo.zoneSpecifier;
      zoneSpecifier.stdOffset().toHourMinute(hour, minute);

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
        mOled.print(zoneSpecifier.isDst() ? "on " : "off");
      } else {
        mOled.print("   ");
      }

      mOled.println();
      mOled.print("12/24: ");
      if (shouldShowFor(MODE_CHANGE_HOUR_MODE)) {
        mOled.print(mRenderingInfo.hourMode == StoredInfo::kTwelve
            ? "12" : "24");
      } else {
        mOled.print("  ");
      }
    }

  private:
    SSD1306Ascii& mOled;
};

#endif

#endif
