#ifndef LED_CLOCK_PRESENTER_H
#define LED_CLOCK_PRESENTER_H

#include <AceTime.h>
#include <AceSegment.h>
#include "config.h"
#include "LedDisplay.h"
#include "RenderingInfo.h"

#if DISPLAY_TYPE == DISPLAY_TYPE_LED

using namespace ace_segment;
using namespace ace_time;
using ace_time::common::DateStrings;

class Presenter {
  public:
    Presenter(const LedDisplay& display):
        mDisplay(display) {}

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

    void setDateTime(const ace_time::ZonedDateTime& dateTime) {
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

    void clearDisplay() { mDisplay.renderer->clear(); }

    void displayData() {
      setBlinkStyle();

      const ZonedDateTime& dateTime = mRenderingInfo.dateTime;
      switch (mRenderingInfo.mode) {
        case MODE_HOUR_MINUTE:
        case MODE_CHANGE_HOUR:
        case MODE_CHANGE_MINUTE:
          mDisplay.clockWriter->writeClock(dateTime.hour(), dateTime.minute());
          break;
        case MODE_MINUTE_SECOND:
          mDisplay.clockWriter->writeCharAt(0, ClockWriter::kSpace);
          mDisplay.clockWriter->writeCharAt(1, ClockWriter::kSpace);
          mDisplay.clockWriter->writeDecimalAt(2, dateTime.second());
          mDisplay.clockWriter->writeColon(true);
          break;
        case MODE_YEAR:
        case MODE_CHANGE_YEAR:
          mDisplay.clockWriter->writeClock(20, dateTime.yearTiny());
          mDisplay.clockWriter->writeColon(false);
          break;
        case MODE_MONTH:
        case MODE_CHANGE_MONTH:
          mDisplay.clockWriter->writeDecimalAt(0, dateTime.month());
          mDisplay.clockWriter->writeColon(false);
          mDisplay.clockWriter->writeCharAt(2, ClockWriter::kSpace);
          mDisplay.clockWriter->writeCharAt(3, ClockWriter::kSpace);
          break;
        case MODE_DAY:
        case MODE_CHANGE_DAY:
          mDisplay.clockWriter->writeDecimalAt(0, dateTime.day());
          mDisplay.clockWriter->writeColon(false);
          mDisplay.clockWriter->writeCharAt(2, ClockWriter::kSpace);
          mDisplay.clockWriter->writeCharAt(3, ClockWriter::kSpace);
          break;
        case MODE_WEEKDAY:
          mDisplay.stringWriter->writeStringAt(
              0, DateStrings().weekDayShortString(dateTime.dayOfWeek()),
              true /* padRight */);
          break;
      }
    }

    void setBlinkStyle() {
      switch (mRenderingInfo.mode) {
        case MODE_CHANGE_HOUR:
          mDisplay.clockWriter->writeStyleAt(0, LedDisplay::BLINK_STYLE);
          mDisplay.clockWriter->writeStyleAt(1, LedDisplay::BLINK_STYLE);
          mDisplay.clockWriter->writeStyleAt(2, 0);
          mDisplay.clockWriter->writeStyleAt(3, 0);
          break;
        case MODE_CHANGE_MINUTE:
          mDisplay.clockWriter->writeStyleAt(0, 0);
          mDisplay.clockWriter->writeStyleAt(1, 0);
          mDisplay.clockWriter->writeStyleAt(2, LedDisplay::BLINK_STYLE);
          mDisplay.clockWriter->writeStyleAt(3, LedDisplay::BLINK_STYLE);
          break;
        case MODE_CHANGE_YEAR:
        case MODE_CHANGE_MONTH:
        case MODE_CHANGE_DAY:
          mDisplay.clockWriter->writeStyleAt(0, LedDisplay::BLINK_STYLE);
          mDisplay.clockWriter->writeStyleAt(1, LedDisplay::BLINK_STYLE);
          mDisplay.clockWriter->writeStyleAt(2, LedDisplay::BLINK_STYLE);
          mDisplay.clockWriter->writeStyleAt(3, LedDisplay::BLINK_STYLE);
          break;
      default:
        mDisplay.clockWriter->writeStyleAt(0, 0);
        mDisplay.clockWriter->writeStyleAt(1, 0);
        mDisplay.clockWriter->writeStyleAt(2, 0);
        mDisplay.clockWriter->writeStyleAt(3, 0);
      }
    }

  private:
    // Disable copy-constructor and assignment operator
    Presenter(const Presenter&) = delete;
    Presenter& operator=(const Presenter&) = delete;

    const LedDisplay& mDisplay;
    RenderingInfo mRenderingInfo;
    RenderingInfo mPrevRenderingInfo;

};

#endif

#endif
