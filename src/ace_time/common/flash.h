/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#ifndef ACE_TIME_COMMON_FLASH_H
#define ACE_TIME_COMMON_FLASH_H

/** Use PROGMEM for BasicZoneSpecifier. */
#define ACE_TIME_USE_BASIC_PROGMEM 1

/** Use PROGMEM for ExtendedZoneSpecifier. */
#define ACE_TIME_USE_EXTENDED_PROGMEM 1

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

// Include the correct pgmspace.h depending on architecture. Define a
// consistent acetime_strcmp_P() which can be passed as a funcdtion pointer
// into the ZoneManager template class.
#if defined(__AVR__)
  #include <avr/pgmspace.h>
  #define FPSTR(p) (reinterpret_cast<const __FlashStringHelper *>(p))
  #define acetime_strcmp_P strcmp_P
#elif defined(TEENSYDUINO)
  #include <avr/pgmspace.h>
  #define FPSTR(p) (reinterpret_cast<const __FlashStringHelper *>(p))
  // Teensyduino defines strcmp_P(a, b) as strcmp(a,b), which cannot be
  // passed as a function pointer, so we have to use strcmp() directly.
  #define acetime_strcmp_P strcmp
#elif defined(ESP8266)
  #include <pgmspace.h>
  // ESP8266 2.5.2 defines strcmp_P() as a macro, so we have to provide a real
  // function.
  inline int acetime_strcmp_P(const char* str1, const char* str2P) {
    return strcmp_P((str1), (str2P));
  }
#elif defined(ESP32)
  #include <pgmspace.h>
  // Fix incorrect definition of FPSTR in ESP32, see
  // https://github.com/espressif/arduino-esp32/issues/1371
  #undef FPSTR
  #define FPSTR(p) (reinterpret_cast<const __FlashStringHelper *>(p))
  #define acetime_strcmp_P strcmp_P
#elif defined(__linux__) or defined(__APPLE__)
  #include <pgmspace.h>
  #define FPSTR(p) (reinterpret_cast<const __FlashStringHelper *>(p))
  #define acetime_strcmp_P strcmp_P
#else
  #error Unsupported platform
#endif

/**
 * Compare 2 strings in flash memory. None of the various strXxx_P() functions
 * work when both strings are in flash memory.
 */
inline int acetime_strcmp_PP(const char* a, const char* b) {
  if (a == b) { return 0; }
  if (a == nullptr) { return -1; }
  if (b == nullptr) { return 1; }

  while (true) {
    uint8_t ca = pgm_read_byte(a);
    uint8_t cb = pgm_read_byte(b);
    if (ca != cb) return (int) ca - (int) cb;
    if (ca == '\0') return 0;
    a++;
    b++;
  }
}

#endif
