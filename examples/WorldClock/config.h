#ifndef WORLD_CLOCK_CONFIG_H
#define WORLD_CLOCK_CONFIG_H

//------------------------------------------------------------------
// Configuration parameters.
//------------------------------------------------------------------

#define ENABLE_SERIAL 0

// This program should compile for most target environments, including AVR,
// ESP8266, and ESP32. The parameters below are for the specific device that I
// built which has a Pro Micro with 3 OLED displays using SPI, a DS3231 over
// I2C, and 2 buttons.
#define MODE_BUTTON_PIN 8
#define CHANGE_BUTTON_PIN 9
#define TIME_SOURCE_TYPE TIME_SOURCE_TYPE_DS3231
#define OLED_REMAP false
#define OLED_CS0_PIN 18
#define OLED_CS1_PIN 19
#define OLED_CS2_PIN 20
#define OLED_RST_PIN 4
#define OLED_DC_PIN 10

// Whether to use Manual TimeZone, Basic TimeZone or Extended TimeZone.
#define TIME_ZONE_TYPE_MANUAL 0
#define TIME_ZONE_TYPE_BASIC 1
#define TIME_ZONE_TYPE_EXTENDED 2
#define TIME_ZONE_TYPE TIME_ZONE_TYPE_BASIC

//------------------------------------------------------------------
// Rendering modes.
//------------------------------------------------------------------

const uint8_t MODE_UNKNOWN = 0; // uninitialized
const uint8_t MODE_DATE_TIME = 1;
const uint8_t MODE_CLOCK_INFO = 2;
const uint8_t MODE_ABOUT = 3;

const uint8_t MODE_CHANGE_YEAR = 10;
const uint8_t MODE_CHANGE_MONTH = 11;
const uint8_t MODE_CHANGE_DAY = 12;
const uint8_t MODE_CHANGE_HOUR = 13;
const uint8_t MODE_CHANGE_MINUTE = 14;
const uint8_t MODE_CHANGE_SECOND = 15;

const uint8_t MODE_CHANGE_HOUR_MODE = 20;
const uint8_t MODE_CHANGE_BLINKING_COLON = 21;

#if TIME_ZONE_TYPE == TIME_ZONE_TYPE_MANUAL
const uint8_t MODE_CHANGE_TIME_ZONE_DST0 = 30;
const uint8_t MODE_CHANGE_TIME_ZONE_DST1 = 31;
const uint8_t MODE_CHANGE_TIME_ZONE_DST2 = 32;
#endif

#endif
