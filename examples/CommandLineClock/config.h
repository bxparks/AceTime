#ifndef COMMAND_LINE_CLOCK_CONFIG_H
#define COMMAND_LINE_CLOCK_CONFIG_H

//---------------------------------------------------------------------------
// Configuration parameters.
//---------------------------------------------------------------------------

#ifndef AUNITER
  #warning Using ESP_MINDER environment to check compile of NTP code
  #define AUNITER_ESP_MINDER
  #define AUNITER_SSID "ssid"
  #define AUNITER_PASSWORD "wifipassword"
#endif

#if defined(AUNITER_MICRO_MINDER)
  #define USE_DS3231
#elif defined(AUNITER_MINI_MINDER)
  #define USE_DS3231
#elif defined(AUNITER_ESP_MINDER)
  #define USE_NTP
#elif defined(AUNITER_ESP_MINDER2)
  #define USE_NTP
#else
  #define USE_SYSTEM
#endif

// Determine how to sync the SystemTimeKeeper.
#define SYNC_TYPE_MANUAL 0
#define SYNC_TYPE_COROUTINE 1
#define SYNC_TYPE SYNC_TYPE_COROUTINE

// Must be greater than (sizeof(StoredInfo) + 4).
#define EEPROM_SIZE 32

#endif
