# WorldClock

This is a USB-powered clock that supports 3 timezones using the AceTime library.
It contains:

* (1) Arduino Pro Micro controller
* (3) SSD1306 OLED displays on SPI
* (1) DS3231 RTC chip on I2C
* (2) buttons on GPIO pins

## Schematic

Here is the rough schematic of this example:
```
              5V
              / \
              | |
Pro Micro     R R
+-----+       | |        DS3231
|     |       | |        +--------+
|  SCL|-------+----------|SCL     |
|  SDA|---------+--------|SDA     |
|     |                  +--------+
|     |
|     |        +--------------------+----------------+
|     |        | +------------------.-+--------------.-+
|     |        | | +----------------.-.-+------------.-.-+
|     |        | | |     OLED (SPI) | | | OLED (SPI) | | | OLED (SPI)
|     |        | | |     +--------+ | | | +--------+ | | | +--------+
| MOSI|--------+-.-.-----|MOSI    | +-.-.-|MOSI    | +-.-.-|MOSI    |
|  SCK|----------+-.-----|SCK     |   +-.-|SCK     |   +-.-|SCK     |
|  D10|------------+-----|D/C     |     +-|D/C     |     +-|D/C     |
|  D18|------------------|CS      |    .--|CS      |    .--|CS      |
|  D19|---------\        +--------+   /   +--------+   /   +--------+
|  D20|--------\ --------------------/                /
|     |         -------------------------------------/
|     |
|   D8|----S1---+
|   D9|----S2---+
+-----+         |
                |
               GND

R = 10k Ohms
S1, S2 = momentary buttons
D8, D8 = GPIO pins for switches
D10 = Data or Command selector
D18, D19, D20 = Chip Select for SPI displays
MOSI, SCL = SPI output and clock
SCL, SDA = I2C pins
```

## Photo

![WorldClock](WorldClock.jpg)

## Installation and User Guide

I don't have the time to write a documentation for this, but structure of the
code for WorldClock is very similar to [OledClock](../OldClock) so the notes
there will be very close to how the WorldClock works.
