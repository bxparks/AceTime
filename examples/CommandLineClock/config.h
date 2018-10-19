#ifndef COMMAND_LINE_CLOCK_CONFIG_H
#define COMMAND_LINE_CLOCK_CONFIG_H

//---------------------------------------------------------------------------
// Configuration parameters.
//---------------------------------------------------------------------------

#ifndef AUNITER
  #warning Using ESP8266 environment to verify compilation of NTP code
  #define AUNITER_ESP8266
#endif

#if defined(AUNITER_NANO)
  #define USE_DS3231
#elif defined(AUNITER_MICRO)
  #define USE_DS3231
#elif defined(AUNITER_ESP8266)
  #define USE_NTP
#elif defined(AUNITER_ESP32)
  #define USE_NTP
#elif defined(AUNITER_MICRO_MINDER)
  #define USE_DS3231
#elif defined(AUNITER_MINI_MINDER)
  #define USE_DS3231
#elif defined(AUNITER_ESP_MINDER)
  #define USE_NTP
#elif defined(AUNITER_ESP_MINDER2)
  #define USE_NTP
#elif defined(AUNITER_ESP32_MINDER)
  #define USE_NTP
#else
  #define USE_SYSTEM
#endif

// Determine how to sync the SystemTimeKeeper.
#define SYNC_TYPE_MANUAL 0
#define SYNC_TYPE_COROUTINE 1
#define SYNC_TYPE SYNC_TYPE_MANUAL

#endif
