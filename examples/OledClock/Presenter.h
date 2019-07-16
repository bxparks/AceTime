#ifndef OLED_CLOCK_PRESERNTER_H
#define OLED_CLOCK_PRESERNTER_H

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

    void setTimeZone(const ace_time::ManualZoneSpecifier& manualZspec) {
      mRenderingInfo.manualZspec = manualZspec;
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
          || mRenderingInfo.manualZspec != mPrevRenderingInfo.manualZspec
          || mRenderingInfo.hourMode != mPrevRenderingInfo.hourMode;
    }

    void clearDisplay() const { mOled.clear(); }

    void displayData() {
      mOled.home();

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
        case MODE_CHANGE_TIME_ZONE_TYPE:
        case MODE_CHANGE_TIME_ZONE_OFFSET:
        case MODE_CHANGE_TIME_ZONE_DST:
          displayTimeZone();
          break;

        case MODE_ABOUT:
          displayAbout();
          break;
      }
    }

    void displayDateTime() const {
      mOled.setFont(fixed_bold10x15);
      const ZonedDateTime& dateTime = mRenderingInfo.dateTime;
      if (dateTime.isError()) {
        mOled.println(F("9999-99-99"));
        mOled.println(F("99:99:99   "));
        mOled.println(F("Error     "));
        return;
      }

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
      mOled.print(DateStrings().dayOfWeekLongString(dateTime.dayOfWeek()));
      mOled.clearToEOL();
    }

    void displayTimeZone() const {
      mOled.setFont(fixed_bold10x15);

      const ManualZoneSpecifier& manualZspec = mRenderingInfo.manualZspec;
      TimeOffset offset = manualZspec.stdOffset();

      // Don't use F() strings for these short strings. Seems to increase
      // flash memory, while saving only a few bytes of RAM.

      mOled.print("TZ: ");
      if (shouldShowFor(MODE_CHANGE_TIME_ZONE_TYPE)) {
        const TimeZone& tz = mRenderingInfo.dateTime.timeZone();
        mOled.print((tz.getType() == TimeZone::kTypeManual)
            ? "manual" : "auto");
      } else {
        mOled.print("      "); // 6 spaces
      }

      mOled.println();
      mOled.print("UTC");
      if (shouldShowFor(MODE_CHANGE_TIME_ZONE_OFFSET)) {
        offset.printTo(mOled);
      } else {
        mOled.print("      "); // 6 spaces to span "+hh:mm"
      }

      mOled.println();
      mOled.print("DST: ");
      if (shouldShowFor(MODE_CHANGE_TIME_ZONE_DST)) {
        mOled.print(manualZspec.isDst() ? "on " : "off");
      } else {
        mOled.print("   ");
      }
    }

    void displayAbout() const {
      mOled.setFont(SystemFont5x7);

      // Use F() macros for these longer strings. Seems to save both
      // flash memory and RAM.
      mOled.print(F("OledClock: "));
      mOled.println(CLOCK_VERSION_STRING);
      mOled.print(F("TZ: "));
      mOled.println(zonedb::kTzDatabaseVersion);
      mOled.print(F("AceTime: "));
      mOled.print(ACE_TIME_VERSION_STRING);
    }

    RenderingInfo mRenderingInfo;
    RenderingInfo mPrevRenderingInfo;
    SSD1306Ascii& mOled;
};

#endif
