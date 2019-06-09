#include "LedDisplay.h"

#if LED_MODULE_TYPE == LED_MODULE_DIRECT
  const uint8_t LedDisplay::DIGIT_PINS[LedDisplay::NUM_DIGITS] = 
      LED_DIGIT_PINS;
  const uint8_t LedDisplay::SEGMENT_PINS[LedDisplay::NUM_SEGMENTS] =
      LED_SEGMENT_PINS;
#elif LED_MODULE_TYPE == LED_MODULE_SEGMENT_SERIAL
  const uint8_t LedDisplay::DIGIT_PINS[LedDisplay::NUM_DIGITS] = 
      LED_DIGIT_PINS;
#endif
