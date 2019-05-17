#ifndef CLOCK_LED_DISPLAY_H
#define CLOCK_LED_DISPLAY_H

#include <AceSegment.h>

using namespace ace_segment;

/**
 * Helper class that encapsulates the logic of setting up and using a 7-segment
 * LED display.
 */
class LedDisplay {
  public:
    static const uint8_t DRIVER_TYPE_DIRECT = 0;
    static const uint8_t DRIVER_TYPE_SERIAL = 1;

    static const uint8_t FRAMES_PER_SECOND = 60;
    static const uint8_t NUM_SUBFIELDS = 1;
    static const uint8_t BLINK_STYLE = 1;
    static const uint16_t BLINK_DURATION_MILLIS = 1000;

    static const uint8_t NUM_DIGITS = 4;
    static const uint8_t NUM_SEGMENTS = 8;

    // Transistors on the digits or segments which do NOT have the resistors.
    static const bool USE_TRANSISTORS = true;

    // Common Cathode or Anode
    static const bool COMMON_CATHODE = true;

    static const uint16_t STATS_RESET_INTERVAL = 1200;

    /**
     * Constructor. This creates a number of objects on the heap. It is assumed
     * that the LedDisplay object is created once at the beginning of the
     * program and never deleted.
     */
    LedDisplay(uint8_t driverType, const uint8_t* digitPins,
        const uint8_t* segmentPins) {
      hardware = new Hardware();

      // Create the Driver.
      if (driverType == DRIVER_TYPE_DIRECT) {
        driver = new SplitDirectDigitDriver(
            hardware, dimmablePatterns,
            COMMON_CATHODE, USE_TRANSISTORS,
            false /* transistorsOnSegments */, NUM_DIGITS, NUM_SEGMENTS,
            NUM_SUBFIELDS, digitPins, segmentPins);
      } else if (driverType == DRIVER_TYPE_SERIAL) {
        driver = new SplitSerialDigitDriver(
            hardware, dimmablePatterns,
            COMMON_CATHODE, USE_TRANSISTORS,
            false /* transistorsOnSegments */, NUM_DIGITS, NUM_SEGMENTS,
            NUM_SUBFIELDS, digitPins, segmentPins[0], segmentPins[1],
            segmentPins[2]);
      }
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
