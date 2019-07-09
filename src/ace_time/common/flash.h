/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#ifndef ACE_TIME_COMMON_FLASH_H
#define ACE_TIME_COMMON_FLASH_H

/** Use PROGMEM for BasicZoneSpecifier. */
#define ACE_TIME_USE_BASIC_PROGMEM 1

/** Use PROGMEM for ExtendedZoneSpecifier. */
#define ACE_TIME_USE_EXTENDED_PROGMEM 0

#if ACE_TIME_USE_BASIC_PROGMEM
  #define ACE_TIME_BASIC_PROGMEM PROGMEM
#else
  #define ACE_TIME_BASIC_PROGMEM
#endif

#if ACE_TIME_USE_EXTENDED_PROGMEM
  #define ACE_TIME_EXTENDED_PROGMEM PROGMEM
#else
  #define ACE_TIME_EXTENDED_PROGMEM
#endif

// Include the correct pgmspace.h depending on architecture
#if defined(__AVR__) || defined(TEENSYDUINO)
  #include <avr/pgmspace.h>
  #define FPSTR(p) (reinterpret_cast<const __FlashStringHelper *>(p))
#elif defined(ESP8266)
  #include <pgmspace.h>
#elif defined(ESP32)
  #include <pgmspace.h>
  // Fix incorrect definition of FPSTR in ESP32, see
  // https://github.com/espressif/arduino-esp32/issues/1371
  #undef FPSTR
  #define FPSTR(p) (reinterpret_cast<const __FlashStringHelper *>(p))
#elif defined(__linux__) or defined(__APPLE__)
  #include <pgmspace.h>
  #define FPSTR(p) (reinterpret_cast<const __FlashStringHelper *>(p))
#else
  #error Unsupported platform
#endif

#endif
