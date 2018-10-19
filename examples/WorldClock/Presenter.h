#ifndef WORLD_CLOCK_PRESENTER_H
#define WORLD_CLOCK_PRESENTER_H

#include <SSD1306Ascii.h>
#include <AceTime.h>
#include "RenderingInfo.h"
#include "config.h"

using namespace ace_time;
using namespace ace_time::common;

/**
 * Class that knows how to render a specific Mode on the OLED display.
 */
class Presenter {
  public:
    /** Constructor. */
    Presenter(SSD1306Ascii& oled):
        mOled(oled) {}

    void display() {
      if (needsClear()) {
        clearDisplay();
      }
      if (needsUpdate()) {
        displayData();
      }

      mPrevRenderingInfo = mRenderingInfo;
    }

    void setMode(uint8_t mode) {
      mRenderingInfo.mode = mode;
    }

    void setNow(uint32_t now) {
      mRenderingInfo.now = now;
    }

    void setClockInfo(const ClockInfo& clockInfo) {
      mRenderingInfo.clockInfo = clockInfo;
    }

    void setSuppressBlink(bool suppressBlink) {
      mRenderingInfo.suppressBlink = suppressBlink;
    }

    void setBlinkShowState(bool blinkShowState) {
      mRenderingInfo.blinkShowState = blinkShowState;
    }

  private:
    // Disable copy-constructor and assignment operator
    Presenter(const Presenter&) = delete;
    Presenter& operator=(const Presenter&) = delete;

    void clearDisplay() { mOled.clear(); }

    void displayData() {
      mOled.home();

      switch (mRenderingInfo.mode) {
        case MODE_DATE_TIME:
          displayDateTime();
          break;

        case MODE_CHANGE_YEAR:
        case MODE_CHANGE_MONTH:
        case MODE_CHANGE_DAY:
        case MODE_CHANGE_HOUR:
        case MODE_CHANGE_MINUTE:
        case MODE_CHANGE_SECOND:
          displayChangeableDateTime();
          break;

        case MODE_CLOCK_INFO:
        case MODE_CHANGE_HOUR_MODE:
        case MODE_CHANGE_BLINKING_COLON:
        case MODE_CHANGE_TIME_ZONE_HOUR:
        case MODE_CHANGE_TIME_ZONE_MINUTE:
        case MODE_CHANGE_TIME_ZONE_DST:
          displayClockInfo();
          break;
      }
    }

    void displayDateTime() const {
      const DateTime dateTime(mRenderingInfo.now,
          mRenderingInfo.clockInfo.timeZone);
      mOled.setFont(fixed_bold10x15);

      mOled.set2X();

      // time
      uint8_t hour = dateTime.hour();
      if (mRenderingInfo.clockInfo.hourMode == ClockInfo::kTwelve) {
        if (hour == 0) {
          hour = 12;
        } else if (hour > 12) {
          hour -= 12;
        }
        printPad2(mOled, hour, ' ');
      } else {
        printPad2(mOled, hour);
      }
      if (!mRenderingInfo.clockInfo.blinkingColon
          || shouldShowFor(MODE_DATE_TIME)) {
        mOled.print(':');
      } else {
        mOled.print(' ');
      }
      printPad2(mOled, dateTime.minute());

      // AM/PM indicator
      mOled.set1X();
      if (mRenderingInfo.clockInfo.hourMode == ClockInfo::kTwelve) {
        mOled.print((dateTime.hour() < 12) ? 'A' : 'P');
      }

      // weekDay, month/day, AM/PM
      // "Thu 10/18 P"
      mOled.println();
      mOled.println();

      mOled.print(DateStrings().weekDayShortString(dateTime.dayOfWeek()));
      mOled.print(' ');
      printPad2(mOled, dateTime.month(), ' ');
      mOled.print('/');
      printPad2(mOled, dateTime.day(), '0');
      mOled.print(' ');
      mOled.clearToEOL();

      // place name
      mOled.println();
      mOled.print(mRenderingInfo.clockInfo.name);
      mOled.clearToEOL();
    }

    void displayChangeableDateTime() const {
      const DateTime dateTime(mRenderingInfo.now,
          mRenderingInfo.clockInfo.timeZone);
      mOled.setFont(fixed_bold10x15);
      mOled.set1X();

      // date
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
      mOled.clearToEOL();

      // time
      mOled.println();
      if (shouldShowFor(MODE_CHANGE_HOUR)) {
        uint8_t hour = dateTime.hour();
        if (mRenderingInfo.clockInfo.hourMode == ClockInfo::kTwelve) {
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
      if (mRenderingInfo.clockInfo.hourMode == ClockInfo::kTwelve) {
        mOled.print((dateTime.hour() < 12) ? "AM" : "PM");
      }
      mOled.clearToEOL();

      // week day
      mOled.println();
      mOled.print(DateStrings().weekDayLongString(dateTime.dayOfWeek()));
      mOled.clearToEOL();

      // place name
      mOled.println();
      mOled.print(mRenderingInfo.clockInfo.name);
      mOled.clearToEOL();
    }

    void displayClockInfo() const {
      const ClockInfo& clockInfo = mRenderingInfo.clockInfo;
      const TimeZone& timeZone = clockInfo.timeZone;
      int8_t sign;
      uint8_t hour;
      uint8_t minute;
      timeZone.extractStandardHourMinute(sign, hour, minute);

      mOled.print(FF("12/24: "));
      if (shouldShowFor(MODE_CHANGE_HOUR_MODE)) {
        mOled.print(mRenderingInfo.clockInfo.hourMode == ClockInfo::kTwelve
            ? "12" : "24");
      } else {
        mOled.print("  ");
      }

      mOled.println();
      mOled.print(FF("Blink: "));
      if (shouldShowFor(MODE_CHANGE_BLINKING_COLON)) {
        mOled.print(clockInfo.blinkingColon ? "on " : "off");
      } else {
        mOled.print("   ");
      }

      mOled.println();
      mOled.print(FF("UTC"));
      if (shouldShowFor(MODE_CHANGE_TIME_ZONE_HOUR)) {
        mOled.print((sign < 0) ? '-' : '+');
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
      mOled.print(FF("DST: "));
      if (shouldShowFor(MODE_CHANGE_TIME_ZONE_DST)) {
        mOled.print(timeZone.isDst() ? "on " : "off");
      } else {
        mOled.print("   ");
      }
    }

    /**
     * True if the display should actually show the data. If the clock is in
     * "blinking" mode, then this will return false in accordance with the
     * mBlinkShowState.
     */
    bool shouldShowFor(uint8_t mode) const {
      return mode != mRenderingInfo.mode
          || mRenderingInfo.suppressBlink
          || mRenderingInfo.blinkShowState;
    }

    /** The display needs to be cleared before rendering. */
    bool needsClear() const {
      return mRenderingInfo.mode != mPrevRenderingInfo.mode;
    }

    /** The display needs to be updated because something changed. */
    bool needsUpdate() const {
      return mRenderingInfo.mode != mPrevRenderingInfo.mode
          || mRenderingInfo.suppressBlink != mPrevRenderingInfo.suppressBlink
          || (!mRenderingInfo.suppressBlink
              && (mRenderingInfo.blinkShowState
                  != mPrevRenderingInfo.blinkShowState))
          || mRenderingInfo.now != mPrevRenderingInfo.now
          || mRenderingInfo.clockInfo.timeZone
              != mPrevRenderingInfo.clockInfo.timeZone
          || mRenderingInfo.clockInfo.hourMode
              != mPrevRenderingInfo.clockInfo.hourMode;
    }

  private:
    SSD1306Ascii& mOled;

    RenderingInfo mRenderingInfo;
    RenderingInfo mPrevRenderingInfo;

};

#endif
