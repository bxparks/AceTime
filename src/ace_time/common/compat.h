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

// Include the correct pgmspace.h depending on architecture.
#if defined(ARDUINO_ARCH_AVR)
  #include <avr/pgmspace.h>
  #define FPSTR(p) (reinterpret_cast<const __FlashStringHelper *>(p))

#elif defined(ARDUINO_ARCH_SAMD)
  #include <avr/pgmspace.h>
  #define FPSTR(p) (reinterpret_cast<const __FlashStringHelper *>(p))

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

#elif defined(ESP8266)
  #include <pgmspace.h>

#elif defined(ESP32)
  #include <pgmspace.h>

  // ESP32 does not define SERIAL_PORT_MONITOR. Define it unless another
  // library has already defined it.
  #if ! defined(SERIAL_PORT_MONITOR)
    #define SERIAL_PORT_MONITOR Serial
  #endif

#elif defined(EPOXY_DUINO)
  #include <pgmspace.h>

#elif defined(ARDUINO_ARCH_STM32)
  #include <avr/pgmspace.h>
  #define FPSTR(p) (reinterpret_cast<const __FlashStringHelper *>(p))

  #undef SERIAL_PORT_MONITOR
  #define SERIAL_PORT_MONITOR Serial

#else
  #error Unsupported platform
#endif

#endif
