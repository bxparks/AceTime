/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#ifndef ACE_TIME_COMMON_FLASH_H
#define ACE_TIME_COMMON_FLASH_H

#if defined(__AVR__) || defined(__arm__)
  #include <avr/pgmspace.h>
#elif defined(ESP8266) || defined(ESP32)
  #include <pgmspace.h>
#elif defined(__linux__) or defined(__APPLE__)
  #include <pgmspace.h>
#else
  #error Unsupported platform
#endif

#endif
