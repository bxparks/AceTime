# FullOledClock

Version: 2019-06-04
Status: I think it works, need to verify

This program was split off from the original `example/Clock` directory. This
version supports a 128x64 OLED display using the I2C interface. The OLED display
can be rotated 180 degrees using the `OLED_REMAP` parameter in config.h.

The time source can be one of the following:
    * TIME_SOURCE_TYPE_NONE: use the system millis()
    * TIME_SOURCE_TYPE_DS3231: use a DS3231 RTC chip
    * TIME_SOURCE_TYPE_NTP: use an NTP server (ESP8266 or ESP32 only)
    * TIME_SOURCE_TYPE_BOTH: use an NTP server with DS3231 as backup
