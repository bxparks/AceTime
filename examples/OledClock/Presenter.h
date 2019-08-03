#ifndef OLED_CLOCK_PRESERNTER_H
#define OLED_CLOCK_PRESERNTER_H

#include <AceTime.h>
#include <SSD1306AsciiWire.h>
#include "config.h"
#include "StoredInfo.h"
#include "ClockInfo.h"
#include "RenderingInfo.h"
#include "Presenter.h"

using namespace ace_time;
using ace_time::common::printPad2;

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

    void setRenderingInfo(uint8_t mode, bool suppressBlink, bool blinkShowState,
        const ClockInfo& clockInfo) {
      mRenderingInfo.mode = mode;
      mRenderingInfo.suppressBlink = suppressBlink;
      mRenderingInfo.blinkShowState = blinkShowState;
      mRenderingInfo.hourMode = clockInfo.hourMode;
      mRenderingInfo.timeZone = clockInfo.timeZone;
      mRenderingInfo.dateTime = clockInfo.dateTime;
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
          || mRenderingInfo.hourMode != mPrevRenderingInfo.hourMode
          || mRenderingInfo.timeZone != mPrevRenderingInfo.timeZone
          || mRenderingInfo.dateTime != mPrevRenderingInfo.dateTime;
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
      #if TIME_ZONE_TYPE == TIME_ZONE_TYPE_MANUAL
        case MODE_CHANGE_TIME_ZONE_OFFSET:
        case MODE_CHANGE_TIME_ZONE_DST:
      #else
        case MODE_CHANGE_TIME_ZONE_NAME:
      #endif
          displayTimeZone();
          break;

        case MODE_ABOUT:
          displayAbout();
          break;
      }
    }

    void displayDateTime() const {
    #if ENABLE_SERIAL == 1
      SERIAL_PORT_MONITOR.println(F("displayDateTime()"));
    #endif
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
    #if ENABLE_SERIAL
      SERIAL_PORT_MONITOR.println(F("displayTimeZone()"));
    #endif
      mOled.setFont(fixed_bold10x15);

      // Don't use F() strings for short strings <= 4 characters. Seems to
      // increase flash memory, while saving only a few bytes of RAM.

      // Display the timezone using the TimeZoneData, not the dateTime, since
      // dateTime will contain a TimeZone, which points to the (singular)
      // Controller::mZoneProcessor, which will contain the old timeZone.
      auto& tz = mRenderingInfo.timeZone;
      mOled.print("TZ: ");
      const __FlashStringHelper* typeString;
      switch (tz.getType()) {
        case TimeZone::kTypeManual:
          typeString = F("manual");
          break;
        case TimeZone::kTypeBasic:
          typeString = F("basic");
          break;
        case TimeZone::kTypeExtended:
          typeString = F("extd");
          break;
        case TimeZone::kTypeBasicManaged:
          typeString = F("bas-man");
          break;
        case TimeZone::kTypeExtendedManaged:
          typeString = F("extd-man");
          break;
        default:
          typeString = F("unknown");
      }
      mOled.print(typeString);
      mOled.clearToEOL();

      switch (tz.getType()) {
      #if TIME_ZONE_TYPE == TIME_ZONE_TYPE_MANUAL
        case TimeZone::kTypeManual:
          mOled.println();
          mOled.print("UTC");
          if (shouldShowFor(MODE_CHANGE_TIME_ZONE_OFFSET)) {
            auto offset = TimeOffset::forOffsetCode(tz.getStdOffsetCode());
            offset.printTo(mOled);
          }
          mOled.clearToEOL();

          mOled.println();
          mOled.print("DST: ");
          if (shouldShowFor(MODE_CHANGE_TIME_ZONE_DST)) {
            mOled.print((tz.getDstOffsetCode() != 0) ? "on " : "off");
          }
          mOled.clearToEOL();
          break;

      #else
        case TimeZone::kTypeBasic:
        case TimeZone::kTypeExtended:
        case TimeZone::kTypeBasicManaged:
        case TimeZone::kTypeExtendedManaged:
          // Print name of timezone
          mOled.println();
          if (shouldShowFor(MODE_CHANGE_TIME_ZONE_NAME)) {
            tz.printShortTo(mOled);
          }
          mOled.clearToEOL();

          // Clear the DST: {on|off} line from a previous screen
          mOled.println();
          mOled.clearToEOL();
          break;
      #endif

        default:
          mOled.println();
          mOled.print(F("<unknown>"));
          mOled.clearToEOL();
          mOled.println();
          mOled.clearToEOL();
          break;
      }
    }

    void displayAbout() const {
    #if ENABLE_SERIAL == 1
      SERIAL_PORT_MONITOR.println(F("displayAbout()"));
    #endif
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

    SSD1306Ascii& mOled;

    RenderingInfo mRenderingInfo;
    RenderingInfo mPrevRenderingInfo;
};

#endif
