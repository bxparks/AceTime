/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#ifndef ACE_TIME_COMMON_COMPAT_H
#define ACE_TIME_COMMON_COMPAT_H

/**
 * @file compat.h
 *
 * Macros and definitions that provide a consistency layer among the various
 * Arduino boards for compatibility. Most of this is caused by inconsistent,
 * incomplete, or sometimes incorrect emulation of the AVR PROGMEM macro and
 * its related str*_P() functions in <avr/pgmspace.h>.
 */

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

  // Set this to 1 to clobber SERIAL_PORT_MONITOR to SerialUSB on
  // an original Arduino Zero when using the Native port. See USER_GUIDE.md for
  // more info.
  #define ACE_TIME_CLOBBER_SERIAL_PORT_MONITOR 0
  #if ACE_TIME_CLOBBER_SERIAL_PORT_MONITOR && defined(ARDUINO_SAMD_ZERO)
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

  // ESP32 does not define SERIAL_PORT_MONITOR. Define it unless another
  // library has already defined it.
  #if ! defined(SERIAL_PORT_MONITOR)
    #define SERIAL_PORT_MONITOR Serial
  #endif

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
