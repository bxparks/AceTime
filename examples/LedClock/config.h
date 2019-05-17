#ifndef CLOCK_CONFIG_H
#define CLOCK_CONFIG_H

//------------------------------------------------------------------
// Configuration parameters.
//------------------------------------------------------------------

#define ENABLE_SERIAL 0

#define EEPROM_SIZE 32

#define DISPLAY_TYPE_LED 0
#define DISPLAY_TYPE_OLED 1
#define DISPLAY_TYPE_FULL_OLED 2

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
  #define DISPLAY_TYPE DISPLAY_TYPE_FULL_OLED
  #define OLED_REMAP false
#elif defined(AUNITER_MICRO) || defined(AUNITER_MICRO_MINDER)
  #define MODE_BUTTON_PIN 8
  #define CHANGE_BUTTON_PIN 9
  #define TIME_SOURCE_TYPE TIME_SOURCE_TYPE_DS3231
  #define DISPLAY_TYPE DISPLAY_TYPE_FULL_OLED
  #define OLED_REMAP true
#elif defined(AUNITER_MINI_MINDER)
  #define MODE_BUTTON_PIN 2
  #define CHANGE_BUTTON_PIN 3
  #define TIME_SOURCE_TYPE TIME_SOURCE_TYPE_DS3231
  #define DISPLAY_TYPE DISPLAY_TYPE_FULL_OLED
  #define OLED_REMAP false
#elif defined(AUNITER_ESP8266) || defined(AUNITER_ESP_MINDER)
  #define MODE_BUTTON_PIN D4
  #define CHANGE_BUTTON_PIN D3
  #define TIME_SOURCE_TYPE TIME_SOURCE_TYPE_NTP
  #define DISPLAY_TYPE DISPLAY_TYPE_FULL_OLED
  #define OLED_REMAP false
#elif defined(AUNITER_ESP_MINDER2)
  #define MODE_BUTTON_PIN D4
  #define CHANGE_BUTTON_PIN D3
  #define TIME_SOURCE_TYPE TIME_SOURCE_TYPE_NTP
  #define DISPLAY_TYPE DISPLAY_TYPE_FULL_OLED
  #define OLED_REMAP true
#elif defined(AUNITER_ESP32)
  #define MODE_BUTTON_PIN 4
  #define CHANGE_BUTTON_PIN 3
  #define TIME_SOURCE_TYPE TIME_SOURCE_TYPE_NTP
  #define DISPLAY_TYPE DISPLAY_TYPE_FULL_OLED
  #define OLED_REMAP true
#else
  #error Unknown AUNITER environment
#endif

#endif
