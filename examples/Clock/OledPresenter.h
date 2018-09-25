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
    virtual void clearDisplay() override { mOled.clear(); }

    virtual void displayData() override {
      mOled.home();
      mOled.setFont(lcdnums12x16);
      mOled.set2X();

      const DateTime& dateTime = mRenderingInfo.dateTime;
      const HardwareTemperature& temperature = mRenderingInfo.temperature;
      switch (mRenderingInfo.mode) {
        case MODE_HOUR_MINUTE:
          printPad2(mOled, dateTime.hour());
          mOled.print(':');
          printPad2(mOled, dateTime.minute());
          break;
        case MODE_CHANGE_HOUR:
          if (shouldShowFor(MODE_CHANGE_HOUR)) {
            printPad2(mOled, dateTime.hour());
          } else {
            mOled.print("  ");
          }
          mOled.print(':');
          printPad2(mOled, dateTime.minute());
          break;
        case MODE_CHANGE_MINUTE:
          printPad2(mOled, dateTime.hour());
          mOled.print(':');
          if (shouldShowFor(MODE_CHANGE_MINUTE)) {
            printPad2(mOled, dateTime.minute());
          } else {
            mOled.print("  ");
          }
          break;
        case MODE_MINUTE_SECOND:
          mOled.print("  :");
          printPad2(mOled, dateTime.second());
          break;
        case MODE_YEAR:
        case MODE_CHANGE_YEAR:
          if (shouldShowFor(MODE_CHANGE_YEAR)) {
            mOled.print("20");
            printPad2(mOled, dateTime.year());
          } else {
            mOled.print("    ");
          }
          break;
        case MODE_MONTH:
        case MODE_CHANGE_MONTH:
          if (shouldShowFor(MODE_CHANGE_MONTH)) {
            printPad2(mOled, dateTime.month());
          } else {
            mOled.print("  ");
          }
          break;
        case MODE_DAY:
        case MODE_CHANGE_DAY:
          if (shouldShowFor(MODE_CHANGE_DAY)) {
            printPad2(mOled, dateTime.day());
          } else {
            mOled.print("  ");
          }
          break;
        case MODE_WEEKDAY:
          mOled.setFont(Arial_bold_14);
          mOled.set2X();
          mOled.print(DateStrings().weekDayShortString(dateTime.dayOfWeek()));
          break;

        case MODE_TEMPERATURE:
          mOled.setFont(Arial_bold_14);
          mOled.set2X();
          temperature.printTo(mOled);
          mOled.print('C');
          break;
      }
    }

  private:
    SSD1306Ascii& mOled;
};

#endif

#endif
