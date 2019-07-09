/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#ifndef ACE_TIME_COMMON_FLASH_H
#define ACE_TIME_COMMON_FLASH_H

/** Use PROGMEM for BasicZoneSpecifier. */
#define ACE_TIME_USE_PROGMEM_BASIC 1

/** Use PROGMEM for ExtendedZoneSpecifier. */
#define ACE_TIME_USE_PROGMEM_EXTENDED 0

#if ACE_TIME_USE_PROGMEM_BASIC
  #define ACE_TIME_BASIC_PROGMEM PROGMEM
#else
  #define ACE_TIME_BASIC_PROGMEM
#endif

#if ACE_TIME_USE_PROGMEM_EXTENDED
  #define ACE_TIME_EXTENDED_PROGMEM PROGMEM
#else
  #define ACE_TIME_EXTENDED_PROGMEM
#endif

// Include the correct pgmspace.h depending on architecture
#if defined(__AVR__) || defined(TEENSYDUINO)
  #include <avr/pgmspace.h>
#elif defined(ESP8266) || defined(ESP32)
  #include <pgmspace.h>
#elif defined(__linux__) or defined(__APPLE__)
  #include <pgmspace.h>
#else
  #error Unsupported platform
#endif

#endif
