#ifndef CLOCK_PRESENTER_H
#define CLOCK_PRESENTER_H

class Presenter {
  public:
    /** Constructor. */
    Presenter() {}

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

    void setDateTime(const DateTime& dateTime) {
      mRenderingInfo.dateTime = dateTime;
    }

    void setTimeZone(const TimeZone& timeZone) {
      mRenderingInfo.timeZone = timeZone;
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

  protected:
    virtual void clearDisplay() = 0;
    virtual void displayData() = 0;

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
          || mRenderingInfo.timeZone != mPrevRenderingInfo.timeZone
          || mRenderingInfo.hourMode != mPrevRenderingInfo.hourMode;
    }

    RenderingInfo mRenderingInfo;
    RenderingInfo mPrevRenderingInfo;

  private:
    // Disable copy-constructor and assignment operator
    Presenter(const Presenter&) = delete;
    Presenter& operator=(const Presenter&) = delete;
    
};

#endif
