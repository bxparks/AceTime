/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#ifndef ACE_TIME_ZONEINFO_COMPAT_H
#define ACE_TIME_ZONEINFO_COMPAT_H

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
#include <WString.h> // FPSTR(), __FlashStringHelper

/** Determine if zonedb and zonedbx zoneinfo files are placed in PROGMEM. */
#define ACE_TIME_USE_PROGMEM 1
#if ACE_TIME_USE_PROGMEM
  #define ACE_TIME_PROGMEM PROGMEM
#else
  #define ACE_TIME_PROGMEM
#endif

// Some 3rd party Arduino cores does not define FPSTR(). And unfortunately, when
// they do, sometimes it's wrong, so we sometimes have to clobber it below.
#if ! defined(FPSTR)
  #define FPSTR(p) (reinterpret_cast<const __FlashStringHelper *>(p))
#endif

// Include the correct pgmspace.h depending on architecture.
#if defined(ARDUINO_ARCH_AVR)
  #include <avr/pgmspace.h>

// Seeeduino SAMD21 Core does not define an identifier for the following, so
// they are not supported:
//  * Wio lite MG126
//  * Wio GPS Board
//  * Wio LTE CAT.1
//
#elif defined(SEEED_XIAO_M0) \
  || defined(SEEEDUINO_ZERO) \
  || defined(SEEED_FEMTO_M0) \
  || defined(SEEEDUINO_LORAWAN) \
  || defined(SEEED_WIO_TERMINAL) \
  || defined(SEEED_GROVE_UI_WIRELESS)

  #include <avr/pgmspace.h>

  // Seeeduino (as of 1.8.3) provides an incorrect definition of FPSTR()
  // so we have to clobber it.
  #undef FPSTR
  #define FPSTR(p) (reinterpret_cast<const __FlashStringHelper *>(p))

#elif defined(ARDUINO_ARCH_SAMD)
  #include <avr/pgmspace.h>

  // Set this to 1 to clobber SERIAL_PORT_MONITOR to SerialUSB on
  // an original Arduino Zero when using the Native port. See USER_GUIDE.md for
  // more info.
  #define ACE_TIME_CLOBBER_SERIAL_PORT_MONITOR 0
  #if ACE_TIME_CLOBBER_SERIAL_PORT_MONITOR && defined(ARDUINO_SAMD_ZERO)
    #undef SERIAL_PORT_MONITOR
    #define SERIAL_PORT_MONITOR SerialUSB
  #endif

#elif defined(ARDUINO_ARCH_STM32)
  #include <avr/pgmspace.h>

#elif defined(ARDUINO_ARCH_ESP8266)
  #include <pgmspace.h>

#elif defined(ARDUINO_ARCH_ESP32)
  #include <pgmspace.h>

  #if ! defined(SERIAL_PORT_MONITOR)
    #define SERIAL_PORT_MONITOR Serial
  #endif

#elif defined(TEENSYDUINO)
  #include <avr/pgmspace.h>

#elif defined(EPOXY_DUINO)
  #include <pgmspace.h>

#else
  #warning Untested platform. AceTime may still work...

  #include <avr/pgmspace.h>

#endif

#endif // ACE_TIME_ZONEINFO_COMPAT_H
