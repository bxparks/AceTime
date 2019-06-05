# LedClock

Version: 2019-06-04
Status: Compiles, not verified to work.

This program was split off from the original `example/Clock` directory. This
version supports 4-segment LED clock displays. It supports multiple types of
4-segment LED displays, according to the `LED_MODULE_TYPE`:
    * `LED_MODULE_DIRECT`: both segment and digit pins driven by GPIO pins directly
    * `LED_MODULE_SERIAL`: segment pins driven by 74HC595
    * `LED_MODULE_SPI`: both segment and digit pins driven by 74HC595

The time source can be one of the following:
    * TIME_SOURCE_TYPE_NONE: use the system millis()
    * TIME_SOURCE_TYPE_DS3231: use a DS3231 RTC chip
    * TIME_SOURCE_TYPE_NTP: use an NTP server (ESP8266 or ESP32 only)
    * TIME_SOURCE_TYPE_BOTH: use an NTP server with DS3231 as backup
