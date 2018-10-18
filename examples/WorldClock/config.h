#ifndef WORLD_CLOCK_CONFIG_H
#define WORLD_CLOCK_CONFIG_H

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

//------------------------------------------------------------------
// Rendering modes.
//------------------------------------------------------------------

static const uint8_t MODE_UNKNOWN = 0; // uninitialized
static const uint8_t MODE_DATE_TIME = 1;
static const uint8_t MODE_CLOCK_INFO = 8;
static const uint8_t MODE_CHANGE_YEAR = 10;
static const uint8_t MODE_CHANGE_MONTH = 11;
static const uint8_t MODE_CHANGE_DAY = 12;
static const uint8_t MODE_CHANGE_HOUR = 13;
static const uint8_t MODE_CHANGE_MINUTE = 14;
static const uint8_t MODE_CHANGE_SECOND = 15;
static const uint8_t MODE_CHANGE_TIME_ZONE_HOUR = 16;
static const uint8_t MODE_CHANGE_TIME_ZONE_MINUTE = 17;
static const uint8_t MODE_CHANGE_TIME_ZONE_DST = 18;
static const uint8_t MODE_CHANGE_HOUR_MODE = 19;

#endif
