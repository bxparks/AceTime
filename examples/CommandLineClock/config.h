#ifndef COMMAND_LINE_CLOCK_CONFIG_H
#define COMMAND_LINE_CLOCK_CONFIG_H

//---------------------------------------------------------------------------
// Configuration parameters.
//---------------------------------------------------------------------------

#define TIME_SOURCE_TYPE_NONE 0
#define TIME_SOURCE_TYPE_DS3231 1
#define TIME_SOURCE_TYPE_NTP 2
#define TIME_SOURCE_TYPE_BOTH 3

#ifndef AUNITER
  #warning Using AUNITER_NANO
  #define AUNITER_NANO
#endif

#if defined(AUNITER_NANO)
  #define TIME_SOURCE_TYPE TIME_SOURCE_TYPE_DS3231
#elif defined(AUNITER_MICRO)
  #define TIME_SOURCE_TYPE TIME_SOURCE_TYPE_DS3231
#elif defined(AUNITER_ESP8266)
  #define TIME_SOURCE_TYPE TIME_SOURCE_TYPE_NTP
#elif defined(AUNITER_ESP32)
  #define TIME_SOURCE_TYPE TIME_SOURCE_TYPE_NTP
#elif defined(AUNITER_MICRO_MINDER)
  #define TIME_SOURCE_TYPE TIME_SOURCE_TYPE_DS3231
#elif defined(AUNITER_MINI_MINDER)
  #define TIME_SOURCE_TYPE TIME_SOURCE_TYPE_DS3231
#elif defined(AUNITER_ESP_MINDER)
  #define TIME_SOURCE_TYPE TIME_SOURCE_TYPE_NTP
#elif defined(AUNITER_ESP_MINDER2)
  #define TIME_SOURCE_TYPE TIME_SOURCE_TYPE_NTP
#elif defined(AUNITER_ESP32_MINDER)
  #define TIME_SOURCE_TYPE TIME_SOURCE_TYPE_NTP
#else
  #define TIME_SOURCE_TYPE TIME_SOURCE_TYPE_NONE
#endif

// Determine how to sync the SystemTimeKeeper.
#define SYNC_TYPE_MANUAL 0
#define SYNC_TYPE_COROUTINE 1
#define SYNC_TYPE SYNC_TYPE_MANUAL

#endif
