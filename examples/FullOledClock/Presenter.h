#ifndef FULL_OLED_CLOCK_PRESERNTER_H
#define FULL_OLED_CLOCK_PRESERNTER_H

#include <AceTime.h>
#include <SSD1306AsciiWire.h>
#include "config.h"
#include "StoredInfo.h"
#include "RenderingInfo.h"
#include "Presenter.h"

using namespace ace_time;
using ace_time::common::printPad2;
using ace_time::common::DateStrings;

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

    void setDateTime(const ZonedDateTime& dateTime) {
      mRenderingInfo.dateTime = dateTime;
    }

    void setTimeZone(const ace_time::ManualZoneSpecifier& zoneSpecifier) {
      mRenderingInfo.zoneSpecifier = zoneSpecifier;
    }

    void setHourMode(uint8_t hourMode) {
      mRenderingInfo.hourMode = hourMode;
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
          || mRenderingInfo.dateTime != mPrevRenderingInfo.dateTime
          || mRenderingInfo.zoneSpecifier != mPrevRenderingInfo.zoneSpecifier
          || mRenderingInfo.hourMode != mPrevRenderingInfo.hourMode;
    }

    void clearDisplay() { mOled.clear(); }

    void displayData() {
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
      int8_t sign;
      uint8_t hour;
      uint8_t minute;
      const ManualZoneSpecifier& zoneSpecifier = mRenderingInfo.zoneSpecifier;
      zoneSpecifier.stdOffset().toHourMinute(sign, hour, minute);

      mOled.print("UTC");
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

    RenderingInfo mRenderingInfo;
    RenderingInfo mPrevRenderingInfo;
    SSD1306Ascii& mOled;
};

#endif
