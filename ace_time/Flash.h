#ifndef ACE_TIME_FLASH_H
#define ACE_TIME_FLASH_H

class __FlashStringHelper;

#if defined(__AVR__) || defined(__arm__)
  #include <avr/pgmspace.h>
#elif defined(ESP8266) || defined(ESP32)
  #include <pgmspace.h>
#else
  #error Unsupported platform
#endif

#endif
