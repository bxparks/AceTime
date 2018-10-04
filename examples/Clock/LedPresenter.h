#ifndef CLOCK_LED_PRESENTER_H
#define CLOCK_LED_PRESENTER_H

#include <AceSegment.h>
#include "LedDisplay.h"
#include "RenderingInfo.h"
#include "Presenter.h"
#include "config.h"

#if DISPLAY_TYPE == DISPLAY_TYPE_LED

using namespace ace_segment;

class LedPresenter: public Presenter {
  public:
    LedPresenter(const LedDisplay& display):
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

    void setMode(uint8_t mode) { mRenderingInfo.mode = mode; }

    void setDateTime(const DateTime& dateTime) {
      mRenderingInfo.dateTime = dateTime;
    }

    void setSuppressBlink(bool suppressBlink) {
      mRenderingInfo.suppressBlink = suppressBlink;
    }

    void setBlinkShowState(bool blinkShowState) {
      mRenderingInfo.blinkShowState = blinkShowState;
    }

  private:
    void clearDisplay() override { mDisplay.renderer->clear(); }

    void displayData() override {
      setBlinkStyle();

      const DateTime& dateTime = mRenderingInfo.dateTime;
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
          mDisplay.clockWriter->writeClock(20, dateTime.year());
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
    const LedDisplay& mDisplay;
};

#endif

#endif
