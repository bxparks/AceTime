#ifndef WORLD_CLOCK_CONFIG_H
#define WORLD_CLOCK_CONFIG_H

//------------------------------------------------------------------
// Configuration parameters.
//------------------------------------------------------------------

#define ENABLE_SERIAL 0

#define EEPROM_SIZE 32

#define TIME_SOURCE_TYPE_NONE 0
#define TIME_SOURCE_TYPE_DS3231 1
#define TIME_SOURCE_TYPE_NTP 2
#define TIME_SOURCE_TYPE_BOTH 3

#ifndef AUNITER
  #define AUNITER_WORLD_CLOCK
  #warning Using default AUNITER_WORLD_CLOCK
#endif

#if defined(AUNITER_WORLD_CLOCK)
  #define MODE_BUTTON_PIN 8
  #define CHANGE_BUTTON_PIN 9
  #define TIME_SOURCE_TYPE TIME_SOURCE_TYPE_DS3231
  #define OLED_REMAP false
  #define OLED_CS0_PIN 18
  #define OLED_CS1_PIN 19
  #define OLED_CS2_PIN 20
  #define OLED_RST_PIN 4
  #define OLED_DC_PIN 10
#else
  #error Unknown AUNITER environment
#endif

#endif
