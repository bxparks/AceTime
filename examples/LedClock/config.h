#ifndef LED_CLOCK_CONFIG_H
#define LED_CLOCK_CONFIG_H

//------------------------------------------------------------------
// Configuration parameters.
//------------------------------------------------------------------

#define ENABLE_SERIAL 0

#define EEPROM_SIZE 32

// Options that determine the source of time.
#define TIME_SOURCE_TYPE_NONE 0
#define TIME_SOURCE_TYPE_DS3231 1
#define TIME_SOURCE_TYPE_NTP 2
#define TIME_SOURCE_TYPE_BOTH 3

// Options that determine that type of LED display.
#define LED_MODULE_DIRECT 0
#define LED_MODULE_SERIAL 1
#define LED_MODULE_SPI 2

#ifndef AUNITER
  #define AUNITER_MICRO_MINDER
  #warning Using default AUNITER_MICRO_MINDER
#endif

#if defined(AUNITER_NANO)
  #define MODE_BUTTON_PIN 2
  #define CHANGE_BUTTON_PIN 3
  #define TIME_SOURCE_TYPE TIME_SOURCE_TYPE_DS3231
  #define LED_MODULE_TYPE LED_MODULE_DIRECT
#elif defined(AUNITER_MICRO) || defined(AUNITER_MICRO_MINDER)
  #define MODE_BUTTON_PIN 8
  #define CHANGE_BUTTON_PIN 9
  #define TIME_SOURCE_TYPE TIME_SOURCE_TYPE_DS3231
  #define LED_MODULE_TYPE LED_MODULE_SERIAL
#elif defined(AUNITER_MINI_MINDER)
  #define MODE_BUTTON_PIN 2
  #define CHANGE_BUTTON_PIN 3
  #define TIME_SOURCE_TYPE TIME_SOURCE_TYPE_DS3231
  #define LED_MODULE_TYPE LED_MODULE_SERIAL
#elif defined(AUNITER_ESP8266) || defined(AUNITER_ESP_MINDER)
  #define MODE_BUTTON_PIN D4
  #define CHANGE_BUTTON_PIN D3
  #define TIME_SOURCE_TYPE TIME_SOURCE_TYPE_NTP
  #define LED_MODULE_TYPE LED_MODULE_SERIAL
#elif defined(AUNITER_ESP_MINDER2)
  #define MODE_BUTTON_PIN D4
  #define CHANGE_BUTTON_PIN D3
  #define TIME_SOURCE_TYPE TIME_SOURCE_TYPE_NTP
  #define LED_MODULE_TYPE LED_MODULE_SERIAL
#elif defined(AUNITER_ESP32)
  #define MODE_BUTTON_PIN 4
  #define CHANGE_BUTTON_PIN 3
  #define TIME_SOURCE_TYPE TIME_SOURCE_TYPE_NTP
  #define LED_MODULE_TYPE LED_MODULE_SERIAL
#else
  #error Unknown AUNITER environment
#endif

//------------------------------------------------------------------
// Button state transition nodes.
//------------------------------------------------------------------

static const uint8_t MODE_UNKNOWN = 0; // uninitialized
static const uint8_t MODE_DATE_TIME = 1;
static const uint8_t MODE_HOUR_MINUTE = 2;
static const uint8_t MODE_MINUTE_SECOND = 3;
static const uint8_t MODE_YEAR = 4;
static const uint8_t MODE_MONTH = 5;
static const uint8_t MODE_DAY = 6;
static const uint8_t MODE_WEEKDAY = 7;
static const uint8_t MODE_TIME_ZONE = 8;

static const uint8_t MODE_CHANGE_YEAR = 10;
static const uint8_t MODE_CHANGE_MONTH = 11;
static const uint8_t MODE_CHANGE_DAY = 12;
static const uint8_t MODE_CHANGE_HOUR = 13;
static const uint8_t MODE_CHANGE_MINUTE = 14;
static const uint8_t MODE_CHANGE_SECOND = 15;

static const uint8_t MODE_CHANGE_TIME_ZONE_OFFSET = 20;
static const uint8_t MODE_CHANGE_TIME_ZONE_DST = 21;
static const uint8_t MODE_CHANGE_HOUR_MODE = 22;

#endif
