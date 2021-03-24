# Benchmarks

<a name="CPU"></a>
## CPU

The [AutoBenchmark.ino](examples/AutoBenchmark/) program measures the
amount of CPU cycles taken by some of the more expensive methods. Here is a
summary of the elapsed time for the `ZonedDateTime::forEpochSeconds()` method
using a `BasicZoneManager` that already has its cache populated for the given
`TimeZone`:
```
----------------------------+--------+
Board or CPU                | micros |
----------------------------+--------+
ATmega328P 16 MHz (Nano)    |  618.0 |
SAMD21 48 MHz               |   71.8 |
STM32F1 72 MHz              |   10.4 |
ESP8266 80 MHz              |   26.5 |
ESP32 240 MHz               |    2.5 |
Teensy 3.2 96 MHz           |    6.4 |
----------------------------+--------+
```

<a name="Memory"></a>
## Memory

Here is a quick summary of the amount of static RAM consumed by various
classes (more details at [examples/AutoBenchmark](examples/AutoBenchmark):

**8-bit processors**

```
sizeof(LocalDate): 3
sizeof(LocalTime): 3
sizeof(LocalDateTime): 6
sizeof(TimeOffset): 2
sizeof(OffsetDateTime): 8
sizeof(BasicZoneProcessor): 116
sizeof(ExtendedZoneProcessor): 456
sizeof(BasicZoneManager<1>): 129
sizeof(ExtendedZoneManager<1>): 469
sizeof(TimeZone): 5
sizeof(ZonedDateTime): 13
sizeof(TimePeriod): 4
sizeof(DS3231Clock): 3
sizeof(SystemClockLoop): 45
sizeof(SystemClockCoroutine): 56
```

**32-bit processors**
```
sizeof(LocalDate): 3
sizeof(LocalTime): 3
sizeof(LocalDateTime): 6
sizeof(TimeOffset): 2
sizeof(OffsetDateTime): 8
sizeof(BasicZoneProcessor): 164
sizeof(ExtendedZoneProcessor): 540
sizeof(BasicZoneManager<1>): 188
sizeof(ExtendedZoneManager<1>): 564
sizeof(TimeZone): 12
sizeof(ZonedDateTime): 20
sizeof(TimePeriod): 4
sizeof(NtpClock): 88 (ESP8266), 116 (ESP32)
sizeof(SystemClockLoop): 56
sizeof(SystemClockCoroutine): 80
```

The [MemoryBenchmark](examples/MemoryBenchmark) program gives a more
comprehensive answer to the amount of memory taken by this library.
Here is a short summary for an 8-bit microcontroller (e.g. Arduino Nano):

* Using the `TimeZone` class with a `BasicZoneProcessor` for one timezone takes
  about 6 kB of flash memory and 177 bytes of static RAM.
* Using 2 timezones with `BasicZoneProcessor increases the consumption to
  about 7 kB of flash and 183 bytes of RAM.
* Loading the entire `zonedb::` ZoneInfo Database consumes 21 kB bytes of flash
  and 573 bytes of RAM.
* Adding the `SystemClock` to the `TimeZone` and `BasicZoneProcessor` with one
  timezone consumes 9 kB bytes of flash and 324 bytes of RAM.

These numbers indicate that the AceTime library is useful even on a limited
8-bit controller with only 30-32 kB of flash and 2 kB of RAM. As a concrete
example, the
[WorldClock](https://github.com/bxparks/clocks/tree/master/WorldClock) program
contains 3 OLED displays over SPI, 2 buttons, one DS3231 chip, and 3 timezones
using AceTime, and these all fit inside a Arduino Pro Micro limit of 30 kB flash
and 2.5 kB of RAM.


