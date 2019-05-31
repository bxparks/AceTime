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
 *
 * Note: Don't use F() macro for the short strings in this class. It causes the
 * flash/ram to increase from (27748/1535) to (27820/1519). In other words, we
 * increase program size by 72 bytes, to save 16 bytes of RAM. For the
 * WorldClock app, flash memory is more precious than RAM.
 */
class Presenter {
  public:
    /** Constructor. */
    Presenter(SSD1306Ascii& oled):
        mOled(oled) {}

    void display() {
      if (mRenderingInfo.mode == MODE_UNKNOWN) {
        clearDisplay();
        return;
      }

      if (needsClear()) {
        clearDisplay();
      }
      if (needsUpdate()) {
        displayData();
      }
    }

    void update(uint8_t mode, acetime_t now, bool blinkShowState,
        bool suppressBlink, const ClockInfo& clockInfo) {
      mPrevRenderingInfo = mRenderingInfo;

      mRenderingInfo.mode = mode;
      mRenderingInfo.now = now;
      mRenderingInfo.suppressBlink = suppressBlink;
      mRenderingInfo.blinkShowState = blinkShowState;

      mRenderingInfo.name = clockInfo.name;
      mRenderingInfo.hourMode = clockInfo.hourMode;
      mRenderingInfo.blinkingColon = clockInfo.blinkingColon;
      mRenderingInfo.timeZone = clockInfo.timeZone;
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

        case MODE_ABOUT:
          displayAbout();
          break;

        case MODE_CLOCK_INFO:
        case MODE_CHANGE_HOUR_MODE:
        case MODE_CHANGE_BLINKING_COLON:
#if TIME_ZONE_TYPE == TIME_ZONE_TYPE_MANUAL
        case MODE_CHANGE_TIME_ZONE_DST0:
        case MODE_CHANGE_TIME_ZONE_DST1:
        case MODE_CHANGE_TIME_ZONE_DST2:
#endif
          displayClockInfo();
          break;

        case MODE_CHANGE_YEAR:
        case MODE_CHANGE_MONTH:
        case MODE_CHANGE_DAY:
        case MODE_CHANGE_HOUR:
        case MODE_CHANGE_MINUTE:
        case MODE_CHANGE_SECOND:
          displayChangeableDateTime();
          break;
      }
    }

    void displayDateTime() const {
      const ZonedDateTime dateTime = ZonedDateTime::forEpochSeconds(
          mRenderingInfo.now, mRenderingInfo.timeZone);

      mOled.setFont(fixed_bold10x15);
      mOled.set2X();

      // time
      uint8_t hour = dateTime.hour();
      if (mRenderingInfo.hourMode == ClockInfo::kTwelve) {
        if (hour == 0) {
          hour = 12;
        } else if (hour > 12) {
          hour -= 12;
        }
        printPad2(mOled, hour, ' ');
      } else {
        printPad2(mOled, hour);
      }
      if (!mRenderingInfo.blinkingColon || shouldShowFor(MODE_DATE_TIME)) {
        mOled.print(':');
      } else {
        mOled.print(' ');
      }
      printPad2(mOled, dateTime.minute());

      // AM/PM indicator
      mOled.set1X();
      if (mRenderingInfo.hourMode == ClockInfo::kTwelve) {
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
      acetime_t epochSeconds = dateTime.toEpochSeconds();
      dateTime.timeZone().printAbbrevTo(mOled, epochSeconds);
      mOled.print(' ');
      mOled.print('(');
      mOled.print(mRenderingInfo.name);
      mOled.print(')');
      mOled.clearToEOL();
    }

    void displayChangeableDateTime() const {
      const ZonedDateTime dateTime = ZonedDateTime::forEpochSeconds(
          mRenderingInfo.now, mRenderingInfo.timeZone);

      mOled.setFont(fixed_bold10x15);
      mOled.set1X();

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

      // time
      mOled.println();
      if (shouldShowFor(MODE_CHANGE_HOUR)) {
        uint8_t hour = dateTime.hour();
        if (mRenderingInfo.hourMode == ClockInfo::kTwelve) {
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
      if (mRenderingInfo.hourMode == ClockInfo::kTwelve) {
        mOled.print((dateTime.hour() < 12) ? "AM" : "PM");
      }
      mOled.clearToEOL();

      // week day
      mOled.println();
      mOled.print(DateStrings().weekDayLongString(dateTime.dayOfWeek()));
      mOled.clearToEOL();

      // abbreviation and place name
      mOled.println();
      dateTime.timeZone().printAbbrevTo(mOled, mRenderingInfo.now);
      mOled.print(' ');
      mOled.print('(');
      mOled.print(mRenderingInfo.name);
      mOled.print(')');
      mOled.clearToEOL();
    }

    void displayClockInfo() const {
      mOled.print("12/24: ");
      if (shouldShowFor(MODE_CHANGE_HOUR_MODE)) {
        mOled.print(mRenderingInfo.hourMode == ClockInfo::kTwelve
            ? "12" : "24");
      } else {
        mOled.print("  ");
      }

      mOled.println();
      mOled.print("Blink: ");
      if (shouldShowFor(MODE_CHANGE_BLINKING_COLON)) {
        mOled.print(mRenderingInfo.blinkingColon ? "on " : "off");
      } else {
        mOled.print("   ");
      }

      // Extract time zone info.
#if TIME_ZONE_TYPE == TIME_ZONE_TYPE_MANUAL
      const TimeZone& timeZone = mRenderingInfo.timeZone;
      TimeOffset timeOffset = timeZone.getUtcOffset(0);
      int8_t hour;
      uint8_t minute;
      timeOffset.toHourMinute(hour, minute);

      mOled.println();
      mOled.print("UTC");
      mOled.print((hour < 0) ? '-' : '+');
      if (hour < 0) hour = -hour;
      printPad2(mOled, hour);
      mOled.print(':');
      printPad2(mOled, minute);

      mOled.println();
      mOled.print("DST: ");
      if (shouldShowFor(MODE_CHANGE_TIME_ZONE_DST0)
          && shouldShowFor(MODE_CHANGE_TIME_ZONE_DST1)
          && shouldShowFor(MODE_CHANGE_TIME_ZONE_DST2)) {
        mOled.print(timeZone.isDst() ? "on " : "off");
      } else {
        mOled.print("   ");
      }
#endif
    }

    void displayAbout() const;

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
          || mRenderingInfo.now != mPrevRenderingInfo.now
          || mRenderingInfo.suppressBlink != mPrevRenderingInfo.suppressBlink
          || (!mRenderingInfo.suppressBlink
              && (mRenderingInfo.blinkShowState
                  != mPrevRenderingInfo.blinkShowState))
          || mRenderingInfo.hourMode != mPrevRenderingInfo.hourMode
          || mRenderingInfo.blinkingColon != mPrevRenderingInfo.blinkingColon
          || mRenderingInfo.name != mPrevRenderingInfo.name
          || mRenderingInfo.timeZone != mPrevRenderingInfo.timeZone;
    }

  private:
    SSD1306Ascii& mOled;

    RenderingInfo mRenderingInfo;
    RenderingInfo mPrevRenderingInfo;

};

#endif
