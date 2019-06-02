#ifndef FULL_OLED_CLOCK_CONFIG_H
#define FULL_OLED_CLOCK_CONFIG_H

//------------------------------------------------------------------
// Configuration parameters.
//------------------------------------------------------------------

const uint8_t MODE_UNKNOWN = 0; // uninitialized
const uint8_t MODE_DATE_TIME = 1;
const uint8_t MODE_TIME_ZONE = 2;
const uint8_t MODE_ABOUT = 3;

const uint8_t MODE_CHANGE_YEAR = 10;
const uint8_t MODE_CHANGE_MONTH = 11;
const uint8_t MODE_CHANGE_DAY = 12;
const uint8_t MODE_CHANGE_HOUR = 13;
const uint8_t MODE_CHANGE_MINUTE = 14;
const uint8_t MODE_CHANGE_SECOND = 15;

const uint8_t MODE_CHANGE_TIME_ZONE_OFFSET = 20;
const uint8_t MODE_CHANGE_TIME_ZONE_DST = 21;
const uint8_t MODE_CHANGE_HOUR_MODE = 22;

#define CLOCK_VERSION_STRING "0.1"

#define ENABLE_SERIAL 0

#define EEPROM_SIZE 32

#define TIME_SOURCE_TYPE_NONE 0
#define TIME_SOURCE_TYPE_DS3231 1
#define TIME_SOURCE_TYPE_NTP 2
#define TIME_SOURCE_TYPE_BOTH 3

#ifndef AUNITER
  #define AUNITER_MICRO_MINDER
  #warning Using default AUNITER_MICRO_MINDER
#endif

#if defined(AUNITER_NANO)
  #define MODE_BUTTON_PIN 2
  #define CHANGE_BUTTON_PIN 3
  #define TIME_SOURCE_TYPE TIME_SOURCE_TYPE_DS3231
  #define OLED_REMAP false
#elif defined(AUNITER_MICRO) || defined(AUNITER_MICRO_MINDER)
  #define MODE_BUTTON_PIN 8
  #define CHANGE_BUTTON_PIN 9
  #define TIME_SOURCE_TYPE TIME_SOURCE_TYPE_DS3231
  #define OLED_REMAP true
#elif defined(AUNITER_MINI_MINDER)
  #define MODE_BUTTON_PIN 2
  #define CHANGE_BUTTON_PIN 3
  #define TIME_SOURCE_TYPE TIME_SOURCE_TYPE_DS3231
  #define OLED_REMAP false
#elif defined(AUNITER_ESP8266) || defined(AUNITER_ESP_MINDER)
  #define MODE_BUTTON_PIN D4
  #define CHANGE_BUTTON_PIN D3
  #define TIME_SOURCE_TYPE TIME_SOURCE_TYPE_NTP
  #define OLED_REMAP false
#elif defined(AUNITER_ESP_MINDER2)
  #define MODE_BUTTON_PIN D4
  #define CHANGE_BUTTON_PIN D3
  #define TIME_SOURCE_TYPE TIME_SOURCE_TYPE_NTP
  #define OLED_REMAP true
#elif defined(AUNITER_ESP32)
  #define MODE_BUTTON_PIN 4
  #define CHANGE_BUTTON_PIN 3
  #define TIME_SOURCE_TYPE TIME_SOURCE_TYPE_NTP
  #define OLED_REMAP true
#else
  #error Unknown AUNITER environment
#endif

#endif
