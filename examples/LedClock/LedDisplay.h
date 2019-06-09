#ifndef LED_CLOCK_DISPLAY_H
#define LED_CLOCK_DISPLAY_H

#include <AceSegment.h>
#include "config.h"

using namespace ace_segment;

/**
 * Helper class that encapsulates the logic of setting up and using a 7-segment
 * LED display.
 */
class LedDisplay {
  public:
    static const uint8_t NUM_DIGITS = 4;
    static const uint8_t NUM_SEGMENTS = 8;

    static const uint8_t DIGIT_PINS[NUM_DIGITS];

    #if LED_MODULE_TYPE == LED_MODULE_DIRECT
      static const uint8_t SEGMENT_PINS[NUM_SEGMENTS];
    #elif LED_MODULE_TYPE == LED_MODULE_SERIAL
      static const uint8_t LATCH_PIN = 10; // ST_CP on 74HC595
      static const uint8_t DATA_PIN = MOSI; // DS on 74HC595
      static const uint8_t CLOCK_PIN = SCK; // SH_CP on 74HC595
    #else
      #error Unsupported LED_MODULE_TYPE
    #endif

    static const uint8_t FRAMES_PER_SECOND = 60;
    static const uint8_t NUM_SUBFIELDS = 1;
    static const uint8_t BLINK_STYLE = 1;
    static const uint16_t BLINK_DURATION_MILLIS = 1000;

    static const uint16_t STATS_RESET_INTERVAL = 1200;

    /**
     * Constructor. This creates a number of objects on the heap. It is assumed
     * that the LedDisplay object is created once at the beginning of the
     * program and never deleted.
     */
    LedDisplay() {
      hardware = new Hardware();

      // Create the Driver.
      #if LED_MODULE_TYPE == LED_MODULE_DIRECT
        driver = new SplitDirectDigitDriver(
            hardware, dimmablePatterns,
            true /*commonCathode*/, true /*useTransitors*/,
            false /* transistorsOnSegments */, NUM_DIGITS, NUM_SEGMENTS,
            NUM_SUBFIELDS, DIGIT_PINS, segmentPins);
      #elif LED_MODULE_TYPE == LED_MODULE_SERIAL
        driver = new SplitSerialDigitDriver(
            hardware, dimmablePatterns,
            true /*commonCathode*/, true /*useTransitors*/,
            false /* transistorsOnSegments */, NUM_DIGITS, NUM_SEGMENTS,
            NUM_SUBFIELDS, DIGIT_PINS, LATCH_PIN, DATA_PIN, CLOCK_PIN);
      #else
        #error Unsupported LED_MODULE_TYPE
      #endif
      driver->configure();

      // Create the blink styler
      blinkStyler = new BlinkStyler(FRAMES_PER_SECOND, BLINK_DURATION_MILLIS);
      styleTable = new StyleTable();
      styleTable->setStyler(BLINK_STYLE, blinkStyler);

      // Create the Renderer.
      renderer = new Renderer(hardware, driver, styledPatterns, styleTable,
          NUM_DIGITS, FRAMES_PER_SECOND, STATS_RESET_INTERVAL);
      renderer->configure();

      clockWriter = new ClockWriter(renderer);
      charWriter = new CharWriter(renderer);
      stringWriter = new StringWriter(charWriter);
    }

    void renderFieldWhenReady() const { renderer->renderFieldWhenReady(); }

    void renderField() const { renderer->renderField(); }

    uint16_t getFieldsPerSecond() const {
      return renderer->getFieldsPerSecond();
    }

    Renderer* renderer;
    ClockWriter* clockWriter;
    StringWriter* stringWriter;

  private:
    DimmablePattern dimmablePatterns[NUM_DIGITS];
    StyledPattern styledPatterns[NUM_DIGITS];
    Hardware* hardware;
    Driver* driver;
    BlinkStyler* blinkStyler;
    StyleTable* styleTable;
    CharWriter* charWriter;
};

#endif
