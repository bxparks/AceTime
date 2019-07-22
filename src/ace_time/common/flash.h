/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#ifndef ACE_TIME_COMMON_FLASH_H
#define ACE_TIME_COMMON_FLASH_H

#include <stdint.h>
#include <string.h>

/** Determine if zonedb and zonedbx zoneinfo files are placed in PROGMEM. */
#define ACE_TIME_USE_PROGMEM 1
#if ACE_TIME_USE_PROGMEM
  #define ACE_TIME_PROGMEM PROGMEM
#else
  #define ACE_TIME_PROGMEM
#endif

// Include the correct pgmspace.h depending on architecture. Define a
// consistent acetime_strcmp_P() which can be passed as a function pointer
// into the ZoneManager template class.
#if defined(ARDUINO_ARCH_AVR)
  #include <avr/pgmspace.h>
  #define FPSTR(p) (reinterpret_cast<const __FlashStringHelper *>(p))
  #define acetime_strcmp_P strcmp_P

#elif defined(ARDUINO_ARCH_SAMD)
  #include <avr/pgmspace.h>
  #define FPSTR(p) (reinterpret_cast<const __FlashStringHelper *>(p))

  // strcmp_P(a,b) is defined to be strcmp(a,b), but we need a function
  // pointer, so map it directly to strcmp()
  #define acetime_strcmp_P strcmp

  // The Arduino Zero using "Native USB Port" uses SerialUSB, but
  // SERIAL_PORT_MONITOR continues to point to Serial, which causes nothing to
  // appear on the Serial Monitor. Clobber SERIAL_PORT_MONITOR to point to
  // SerialUSB. This is the correct setting on the Chinese SAMD21 M0 Mini
  // clones which claim to be compatible with the Arduino Zero.
  #if defined(ARDUINO_SAMD_ZERO)
    #warning Setting SERIAL_PORT_MONITOR to SerialUSB
    #undef SERIAL_PORT_MONITOR
    #define SERIAL_PORT_MONITOR SerialUSB
  #endif

#elif defined(TEENSYDUINO)
  #include <avr/pgmspace.h>
  #define FPSTR(p) (reinterpret_cast<const __FlashStringHelper *>(p))
  // Teensyduino defines strcmp_P(a, b) as strcmp(a,b), which cannot be
  // passed as a function pointer, so we have to use strcmp() directly.
  #define acetime_strcmp_P strcmp

#elif defined(ESP8266)
  #include <pgmspace.h>

  // ESP8266 2.5.2 defines strcmp_P() as a macro function, but we need a real
  // function.
  inline int acetime_strcmp_P(const char* str1, const char* str2P) {
    return strcmp_P((str1), (str2P));
  }

  // ESP8266 2.5.2 doesn't have these so provide our own implementation.
  extern "C" {
    const char* strchr_P(const char* s, int c);
    const char* strrchr_P(const char* s, int c);
  }

#elif defined(ESP32)
  #include <pgmspace.h>
  // Fix incorrect definition of FPSTR in ESP32 1.0.2. See
  // https://github.com/espressif/arduino-esp32/issues/1371
  #undef FPSTR
  #define FPSTR(p) (reinterpret_cast<const __FlashStringHelper *>(p))
  #define acetime_strcmp_P strcmp_P

  // ESP32 1.0.2 doesn't have these so provide our own implementation.
  extern "C" {
    const char* strchr_P(const char* s, int c);
    const char* strrchr_P(const char* s, int c);
  }

#elif defined(__linux__) or defined(__APPLE__)
  #include <pgmspace.h>
  #define FPSTR(p) (reinterpret_cast<const __FlashStringHelper *>(p))
  #define acetime_strcmp_P strcmp_P
  #define SERIAL_PORT_MONITOR Serial

#else
  #error Unsupported platform
#endif

/**
 * Compare 2 strings in flash memory. None of the various strXxx_P() functions
 * work when both strings are in flash memory.
 */
int acetime_strcmp_PP(const char* a, const char* b);

#endif
