#ifndef WORLD_CLOCK_CONFIG_H
#define WORLD_CLOCK_CONFIG_H

//---------------------------------------------------------------------------
// Compensate for buggy F() implementation in ESP8266.
//---------------------------------------------------------------------------

#if defined(ESP8266)
  #define FF(x) (x)
#else
  #define FF(x) F(x)
#endif

//------------------------------------------------------------------
// Configuration parameters.
//------------------------------------------------------------------

#define ENABLE_SERIAL 0

#define TIME_SOURCE_TYPE_NONE 0
#define TIME_SOURCE_TYPE_DS3231 1
#define TIME_SOURCE_TYPE_NTP 2
#define TIME_SOURCE_TYPE_BOTH 3

// This program should compile for most target environments, including
// AVR, ESP8266, and ESP32. The parameters below are for a Pro Micro
// with 3 OLED displays.
#define MODE_BUTTON_PIN 8
#define CHANGE_BUTTON_PIN 9
#define TIME_SOURCE_TYPE TIME_SOURCE_TYPE_DS3231
#define OLED_REMAP false
#define OLED_CS0_PIN 18
#define OLED_CS1_PIN 19
#define OLED_CS2_PIN 20
#define OLED_RST_PIN 4
#define OLED_DC_PIN 10

// Whether to use ManualTimeZone or AutoTimeZone
#define TIME_ZONE_TYPE_MANUAL 0
#define TIME_ZONE_TYPE_BASIC 1
#define TIME_ZONE_TYPE_EXTENDED 2
#define TIME_ZONE_TYPE TIME_ZONE_TYPE_BASIC

//------------------------------------------------------------------
// Rendering modes.
//------------------------------------------------------------------

static const uint8_t MODE_UNKNOWN = 0; // uninitialized
static const uint8_t MODE_DATE_TIME = 1;
static const uint8_t MODE_CLOCK_INFO = 2;
static const uint8_t MODE_CHANGE_YEAR = 3;
static const uint8_t MODE_CHANGE_MONTH = 4;
static const uint8_t MODE_CHANGE_DAY = 5;
static const uint8_t MODE_CHANGE_HOUR = 6;
static const uint8_t MODE_CHANGE_MINUTE = 7;
static const uint8_t MODE_CHANGE_SECOND = 8;
#if TIME_ZONE_TYPE == TIME_ZONE_TYPE_MANUAL
static const uint8_t MODE_CHANGE_TIME_ZONE_DST0 = 9;
static const uint8_t MODE_CHANGE_TIME_ZONE_DST1 = 10;
static const uint8_t MODE_CHANGE_TIME_ZONE_DST2 = 11;
#endif
static const uint8_t MODE_CHANGE_HOUR_MODE = 12;
static const uint8_t MODE_CHANGE_BLINKING_COLON = 13;

#endif
