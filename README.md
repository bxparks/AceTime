# AceTime

[![AUnit Tests](https://github.com/bxparks/AceTime/actions/workflows/aunit_tests.yml/badge.svg)](https://github.com/bxparks/AceTime/actions/workflows/aunit_tests.yml)
[![Python Tools](https://github.com/bxparks/AceTime/actions/workflows/python_tools.yml/badge.svg)](https://github.com/bxparks/AceTime/actions/workflows/python_tools.yml)
[![Validation Tests](https://github.com/bxparks/AceTime/actions/workflows/validation.yml/badge.svg)](https://github.com/bxparks/AceTime/actions/workflows/validation.yml)

The AceTime library provides Date, Time, and TimeZone classes which can convert
"epoch seconds" to human-readable local date and time fields. Those classes can
also convert local date and time between different time zones, properly
accounting for all DST transitions from the year 2000 until 2050. The ZoneInfo
Database is extracted from the [IANA TZ
database](https://www.iana.org/time-zones). Different subsets of the ZoneInfo
Database can be compiled into the application to reduce flash memory size.

The library also provides Clock classes to retrieve the time from more
accurate sources (such as an
[NTP](https://en.wikipedia.org/wiki/Network_Time_Protocol) server, or
a [DS3231 RTC](https://www.maximintegrated.com/en/products/analog/real-time-clocks/DS3231.html)
chip. A special version of the `Clock` class called the `SystemClock` provides a
fast and accurate "epoch seconds" across all Arduino compatible systems. This
"epoch seconds" can be given to the Date, Time and TimeZone classes to display
the current date and time in any desired timezone.

The primordial motivation for creating the AceTime library was to build a
digital clock with an OLED or LED display, that would show the date and time of
multiple timezones at the same time, while adjusting for any DST changes in the
selected timezone automatically. Another major goal of the library is to keep
the resource (flash and RAM) consumption as small as practical, to allow
substantial portion of this library to run inside the 32kB of flash and 2kB of
RAM limits of an Arduino Nano or a SparkFun Pro Micro dev board. To meet that
goal, this library does not perform any dynamic allocation of memory. Everything
it needs is allocated statically.

This library can be an alternative to the Arduino Time
(https://github.com/PaulStoffregen/Time) and Arduino Timezone
(https://github.com/JChristensen/Timezone) libraries.

**Version**: 1.6+ (2021-03-14, TZ DB version 2021a)

**Changelog**: [CHANGELOG.md](CHANGELOG.md)

**IMPORTANT CHANGE for v1.2**: This library now depends on the the "AceCommon"
library for some of its low-level routines. See the
[docs/installation.md](docs/installation.md) for
installation instructions.

## Table of Contents

* [Overview](#Overview)
    * [Date, Time, TimeZone](#DateTimeAndTimeZone)
    * [Clocks](#Clocks)
    * [MemoryUsage](#MemoryUsage)
    * [CPU Usage](#CPUUsage)
    * [Validation](#Validation)
* [Quick Examples](#QuickExamples)
    * [HelloDateTime](#HelloDateTime)
    * [HelloZoneManager](#HelloZoneManager)
    * [HelloSystemClock](#HelloSystemClock)
    * [WorldClock](#WorldClock)
* [Installation](#Installation)
* [User Guides](#UserGuides)
    * [docs/date_time_timezone.md](docs/date_time_timezone.md)
    * [docs/clock_system_clock.md](docs/clock_system_clock.md)
    * [Doxygen docs](https://bxparks.github.io/AceTime/html) hosted on GitHub
      Pages
* [Benchmarks](#Benchmarks)
* [Comparisons](#Comparisons)
* [System Requirements](#SystemRequirements)
    * [Hardware](#Hardware)
    * [Tool Chain](#ToolChain)
    * [Operating System](#OperatingSystem)
* [License](#License)
* [Feedback and Support](#Feedback)
* [Authors](#Authors)

<a name="Overview"></a>
## Overview

The classes in this library can be grouped into roughly 2 parts:

* Date, time and time zone classes
    * convert epoch seconds to human readable date and time fields,
    * support all time zones in the
      [IANA TZ Database](https://www.iana.org/time-zones)
    * calculate DST transitions on all timezones
    * convert local date and time from one timezone to another
* Clock classes
    * provide access to more accurate external time sources
    * provide a fast access to an auto-updating "Epoch Seconds" on all Arduino
      platforms

The Date, Time and TimeZone classes can be used independently of the Clock
classes. The Clock classes are only minimally coupled to the Date, Time,
TimeZone classes, depending on only the `acetime_t` type (aliased to
`int32_t`) which represents the "epoch seconds". The AceTime "Epoch" is defined
to be 2000-01-01 00:00:00 UTC.

When the library was first created, it seemed convenient to include the 2 parts
described above in a single library. As this library evolved and accumulated
more features, it might have been better for the 2 parts to be separated into
independent Arduino libraries. However, I am resisting the temptation to make
this change in order to preserve backwards compatibility for existing users of
this library. The change that I have done is substantially rewrite this
`README.md` and the accompanying documentation so that the 2 parts of this
library are more isolated and decoupled from each other. Hopefully this will
make it easier to understand how to use this library.

<a name="DateTimeAndTimeZone"></a>
### Date, Time, and TimeZone

The Date, Time, and TimeZone classes provide an abstraction layer to make it
easier to use and manipulate date and time fields, in different time zones.

The documentation of these classes are given in
[docs/date_time_timezone.md](docs/date_time_timezone.md):

* date and time classes and types
    * `ace_time::acetime_t`
    * `ace_time::DateStrings`
    * `ace_time::LocalTime`
    * `ace_time::LocalDate`
    * `ace_time::LocalDateTime`
    * `ace_time::TimeOffset`
    * `ace_time::OffsetDateTime`
    * `ace_time::TimePeriod`
    * mutation helpers
        * `ace_time::local_date_mutation::`
        * `ace_time::time_offset_mutation::`
        * `ace_time::time_period_mutation::`
        * `ace_time::zoned_date_time_mutation::`
* timezone classes
    * `ace_time::ZoneProcessor`
        * `ace_time::BasicZoneProcessor`
        * `ace_time::ExtendedZoneProcessor`
    * `ace_time::TimeZone`
    * `ace_time::ZonedDateTime`
    * `ace_time::ZoneManager`
        * `ace_time::BasicZoneManager`
        * `ace_time::ExtendedZoneManager`
        * `ace_time::ManualZoneManager`
* ZoneInfo Database
    * programmatically generated from the IANA TZ Database files
    * ZoneInfo (opaque reference to a timezone)
        * `ace_time::basic::ZoneInfo` (266 zones and 183 links, as of 2021a)
            * `ace_time::zonedb::kZoneAfrica_Abidjan`
            * `ace_time::zonedb::kZoneAfrica_Accra`
            * ...
            * `ace_time::zonedb::kZonePacific_Wake`
            * `ace_time::zonedb::kZonePacific_Wallis`
        * `ace_time::extended::ZoneInfo` (386 zones and 207 links, as of 2021a)
            * `ace_time::zonedbx::kZoneAfrica_Abidjan`
            * `ace_time::zonedbx::kZoneAfrica_Accra`
            * ...
            * `ace_time::zonedbx::kZonePacific_Wake`
            * `ace_time::zonedbx::kZonePacific_Wallis`
    * ZoneId
        * unique and stable `uint32_t` identifiers for each timezone
        * Basic
            * `ace_time::zonedb::kZoneIdAfrica_Abidjan`
            * `ace_time::zonedb::kZoneIdAfrica_Accra`
            * ...
            * `ace_time::zonedb::kZoneIdPacific_Wake`
            * `ace_time::zonedb::kZoneIdPacific_Wallis`
        * Extended
            * `ace_time::zonedbx::kZoneIdAfrica_Abidjan`
            * `ace_time::zonedbx::kZoneIdAfrica_Accra`
            * ...
            * `ace_time::zonedbx::kZoneIdPacific_Wake`
            * `ace_time::zonedbx::kZoneIdPacific_Wallis`

<a name="Clocks"></a>
### Clocks

The main purpose of the `Clock` class hierarchy is to provide a 32-bit signed
integer (`acetime_t` typedef'ed to `int32_t`) that represents the number of
seconds since a fixed point in the past called the "Epoch".

The documentation of these classes are given in
[docs/clock_system_clock.md](docs/clock_system_clock.md):

* `ace_time::clock::Clock`
    * `ace_time::clock::DS3231Clock`
    * `ace_time::clock::NtpClock`
    * `ace_time::clock::StmRtcClock`
    * `ace_time::clock::Stm32F1Clock`
    * `ace_time::clock::UnixClock`
    * `ace_time::clock::SystemClock`
        * `ace_time::clock::SystemClockCoroutine`
        * `ace_time::clock::SystemClockLoop`

Different subclasses of `Clock` provide access to different hardware or network
devices that provide a source of time.

The `SystemClock` subclass is a special class whose purpose is to provide an
`epochSeconds` integer increments by one every second. It then allows fast
access to this auto-incrementing integer. By fast, I mean that it should allow
sampling at least as fast as 10 times per second, but ideally much faster than
1000 times a second.

The `SystemClockRoutine` and `SystemClockLoop` classes provide a mechanism to
synchronize the `epochSeconds` of the `SystemClock` with a more accurate
`referenceClock` provided by one of the other `Clock` subclasses.

<a name="MemoryUsage"></a>
### Memory Usage

This library does not perform dynamic allocation of memory so that it can be
used in small microcontroller environments. In other words, it does not call the
`new` operator nor the `malloc()` function, and it does not use the Arduino
`String` class. Everything it needs is allocated statically at initialization
time.

The ZoneInfo Database entries are stored in flash memory (using the `PROGMEM`
compiler directive) if the microcontroller allows it (e.g. AVR, ESP8266) so that
they do not consume static RAM. The
[examples/MemoryBenchmark](examples/MemoryBenchmark/) program shows the flash
memory consumption for the ZoneInfo data files are:

* `BasicZoneProcessor`
    * 266 Zones
        * 13 kB (8-bit processor)
        * 17 kB (32-bit processor)
    * 266 Zones and 183 Links
        * 22 kB (8-bit processor)
        * 27 kB (32-bit processor)
* `ExtendedZoneProcessor`
    * 386 Zones
        * 22 kB (8-bit processor)
        * 30 kB (32-bit processor)
    * 386 Zones and 207 Links
        * 30 kB (8-bit processor)
        * 37 kB (32-bit processor)

Normally a small application will use only a small number of timezones. The
AceTime library with one timezone using the `BasicZoneProcessor` and the
`SystemClock` consumes:

* 9-10 kB of flash and 4-500 bytes of RAM on an 8-bit AVR processors,
* 6-23 kB of flash and 900-1800 bytes of RAM on a 32-bit processors.

An example of more complex application is the
WorldClock (https://github.com/bxparks/clocks/tree/master/WorldClock)
which has 3 OLED displays over SPI, 3 timezones using `BasicZoneProcessor`, a
`SystemClock` synchronized to a DS3231 chip on I2C, and 2 buttons with
debouncing and event dispatching provided by the AceButton
(https://github.com/bxparks/AceButton) library. This application consumes about
24 kB, well inside the 28 kB flash limit of an Arduino Pro Micro controller.

<a name="CPUUsage"></a>
### CPU Usage

Conversion from date-time components (year, month, day, etc) to epochSeconds
(`ZonedDateTime::toEpochSeconds()`) takes about:

* ~90 microseconds on an 8-bit AVR,
* ~17 microseconds on a SAMD21,
* ~4 microseconds on an STM32 Blue Pill,
* ~7 microseconds on an ESP8266,
* ~1.4 microseconds on an ESP32,
* ~0.4 microseconds on a Teensy 3.2.

Conversion from an epochSeconds to date-time components including timezone
(`ZonedDateTime::forEpochSeconds()`) takes (assuming cache hits):

* ~600 microseconds on an 8-bit AVR,
* ~70 microseconds on an SAMD21,
* ~10-11 microseconds on an STM32 Blue Pill,
* ~27 microseconds on an ESP8266,
* ~2.5 microseconds on an ESP32,
* ~5-6 microseconds on a Teensy 3.2.

The creation of a TimeZone from its zoneName or its zoneId using a
`BasicZoneManager` configured with a custom ZoneRegistry with 85 zones takes:

* 36-400 microseconds, for an 8-bit AVR,
* 4-14 microseconds, for a SAMD21
* 3-18 microseconds for an STM32 Blue Pill,
* 7-50 microseconds for an ESP8266,
* 0.6-3 microseconds for an ESP32,
* 3-10 microseconds for a Teensy 3.2.

<a name="Validation"></a>
### Validation

The details of how the Date, Time and TimeZone classes are validated are given
in [docs/validation.md](docs/validation.md).

The ZoneInfo Database and the algorithms in this library have been validated to
match the UTC offsets calculated using 5 other date/time libraries written in
different programming languages:

* the Python pytz (https://pypi.org/project/pytz/) library from the year 2000
  until 2037 (inclusive),
* the Python  dateutil (https://pypi.org/project/python-dateutil/) library from
  the year 2000 until 2037 (inclusive),
* the Java JDK 11
  [java.time](https://docs.oracle.com/en/java/javase/11/docs/api/java.base/java/time/package-summary.html)
  library from year 2000 to 2049 (inclusive),
* the C++11/14/17 Hinnant date (https://github.com/HowardHinnant/date) library
  from year 1974 to 2049 (inclusive).
* the C# Noda Time (https://nodatime.org) library from 1974 to 2049 (inclusive)

<a name="QuickExamples"></a>
## Quick Examples

<a name="HelloDateTime"></a>
### HelloDateTime

Here is a simple program (see [examples/HelloDateTime](examples/HelloDateTime))
which demonstrates how to create and manipulate date and times in different time
zones:

```C++
#include <AceTime.h>

using namespace ace_time;

// ZoneProcessor instances should be created statically at initialization time.
static BasicZoneProcessor pacificProcessor;
static BasicZoneProcessor londonProcessor;

void setup() {
  delay(1000);
  Serial.begin(115200);
  while (!Serial); // Wait until Serial is ready - Leonardo/Micro

  auto pacificTz = TimeZone::forZoneInfo(&zonedb::kZoneAmerica_Los_Angeles,
        &pacificProcessor);
  auto londonTz = TimeZone::forZoneInfo(&zonedb::kZoneEurope_London,
        &londonProcessor);

  // Create from components. 2019-03-10T03:00:00 is just after DST change in
  // Los Angeles (2am goes to 3am).
  auto startTime = ZonedDateTime::forComponents(
      2019, 3, 10, 3, 0, 0, pacificTz);

  Serial.print(F("Epoch Seconds: "));
  acetime_t epochSeconds = startTime.toEpochSeconds();
  Serial.println(epochSeconds);

  Serial.print(F("Unix Seconds: "));
  acetime_t unixSeconds = startTime.toUnixSeconds();
  Serial.println(unixSeconds);

  Serial.println(F("=== Los_Angeles"));
  auto pacificTime = ZonedDateTime::forEpochSeconds(epochSeconds, pacificTz);
  Serial.print(F("Time: "));
  pacificTime.printTo(Serial);
  Serial.println();

  Serial.print(F("Day of Week: "));
  Serial.println(
      DateStrings().dayOfWeekLongString(pacificTime.dayOfWeek()));

  // Print info about UTC offset
  TimeOffset offset = pacificTime.timeOffset();
  Serial.print(F("Total UTC Offset: "));
  offset.printTo(Serial);
  Serial.println();

  // Print info about the current time zone
  Serial.print(F("Zone: "));
  pacificTz.printTo(Serial);
  Serial.println();

  // Print the current time zone abbreviation, e.g. "PST" or "PDT"
  Serial.print(F("Abbreviation: "));
  Serial.print(pacificTz.getAbbrev(epochSeconds));
  Serial.println();

  // Create from epoch seconds. London is still on standard time.
  auto londonTime = ZonedDateTime::forEpochSeconds(epochSeconds, londonTz);

  Serial.println(F("=== London"));
  Serial.print(F("Time: "));
  londonTime.printTo(Serial);
  Serial.println();

  // Print info about the current time zone
  Serial.print(F("Zone: "));
  londonTz.printTo(Serial);
  Serial.println();

  // Print the current time zone abbreviation, e.g. "PST" or "PDT"
  Serial.print(F("Abbreviation: "));
  Serial.print(londonTz.getAbbrev(epochSeconds));
  Serial.println();

  Serial.println(F("=== Compare ZonedDateTime"));
  Serial.print(F("pacificTime.compareTo(londonTime): "));
  Serial.println(pacificTime.compareTo(londonTime));
  Serial.print(F("pacificTime == londonTime: "));
  Serial.println((pacificTime == londonTime) ? "true" : "false");
}

void loop() {
}
```

Running this should produce the following on the Serial port:
```
Epoch Seconds: 605527200
Unix Seconds: 1552212000
=== Los Angeles
Time: 2019-03-10T03:00:00-07:00[America/Los_Angeles]
Day of Week: Sunday
Total UTC Offset: -07:00
Zone: America/Los_Angeles
Abbreviation: PDT
=== London
Time: 2019-03-10T10:00:00+00:00[Europe/London]
Zone: Europe/London
Abbreviation: GMT
=== Compare ZonedDateTime
pacificTime.compareTo(londonTime): 0
pacificTime == londonTime: false
```

<a name="HelloZoneManager"></a>
### HelloZoneManager

The [examples/HelloZoneManager](examples/HelloZoneManager) example shows how to
load the entire ZoneInfo Database into a `BasicZoneManager`, then create 3 time
zones using 3 different ways: `createForZoneInfo()`, `createForZoneName()`, and
`createForZoneId()`.

```C++
#include <AceTime.h>

using namespace ace_time;

// Create a BasicZoneManager with the entire ZoneInfo Database.
static const int CACHE_SIZE = 3;
static BasicZoneManager<CACHE_SIZE> manager(
  zonedb::kZoneRegistrySize, zonedb::kZoneRegistry);

void setup() {
  Serial.begin(115200);
  while (!Serial); // Wait Serial is ready - Leonardo/Micro

  // Create Los Angeles by ZoneInfo
  auto pacificTz = manager.createForZoneInfo(&zonedb::kZoneAmerica_Los_Angeles);
  auto pacificTime = ZonedDateTime::forComponents(
      2019, 3, 10, 3, 0, 0, pacificTz);
  pacificTime.printTo(Serial);
  Serial.println();

  // Create London by ZoneName
  auto londonTz = manager.createForZoneName("Europe/London");
  auto londonTime = pacificTime.convertToTimeZone(londonTz);
  londonTime.printTo(Serial);
  Serial.println();

  // Create Sydney by ZoneId
  auto sydneyTz = manager.createForZoneId(zonedb::kZoneIdAustralia_Sydney);
  auto sydneyTime = pacificTime.convertToTimeZone(sydneyTz);
  sydneyTime.printTo(Serial);
  Serial.println();
}

void loop() {
}
```

This consumes about 25kB of flash, which means that it can run on an
Arduino Nano or Micro . It produces the following output:
```
2019-03-10T03:00:00-07:00[America/Los_Angeles]
2019-03-10T10:00:00+00:00[Europe/London]
2019-03-10T21:00:00+11:00[Australia/Sydney]
```

<a name="HelloSystemClock"></a>
### HelloSystemClock

This is the example code for using the `SystemClock` taken from
[examples/HelloSystemClock](examples/HelloSystemClock).

```C++
#include <AceTime.h>

using namespace ace_time;
using namespace ace_time::clock;

// ZoneProcessor instances should be created statically at initialization time.
static BasicZoneProcessor pacificProcessor;

static SystemClockLoop systemClock(nullptr /*reference*/, nullptr /*backup*/);

void setup() {
  delay(1000);
  Serial.begin(115200);
  while (!Serial); // Wait until Serial is ready - Leonardo/Micro

  systemClock.setup();

  // Creating timezones is cheap, so we can create them on the fly as needed.
  auto pacificTz = TimeZone::forZoneInfo(&zonedb::kZoneAmerica_Los_Angeles,
      &pacificProcessor);

  // Set the SystemClock using these components.
  auto pacificTime = ZonedDateTime::forComponents(
      2019, 6, 17, 19, 50, 0, pacificTz);
  systemClock.setNow(pacificTime.toEpochSeconds());
}

void printCurrentTime() {
  acetime_t now = systemClock.getNow();

  // Create a time
  auto pacificTz = TimeZone::forZoneInfo(&zonedb::kZoneAmerica_Los_Angeles,
      &pacificProcessor);
  auto pacificTime = ZonedDateTime::forEpochSeconds(now, pacificTz);
  pacificTime.printTo(Serial);
  Serial.println();
}

// Do NOT use delay() here.
void loop() {
  static acetime_t prevNow = systemClock.getNow();
  systemClock.loop();
  acetime_t now = systemClock.getNow();
  if (now - prevNow >= 2) {
    printCurrentTime();
    prevNow = now;
  }
}
```

This will start by setting the SystemClock to 2019-06-17T19:50:00-07:00,
then printing the system time every 2 seconds:
```
2019-06-17T19:50:00-07:00[America/Los_Angeles]
2019-06-17T19:50:02-07:00[America/Los_Angeles]
2019-06-17T19:50:04-07:00[America/Los_Angeles]
...
```

<a name="WorldClock"></a>
### WorldClock

Here is a photo of the WorldClock
(https://github.com/bxparks/clocks/tree/master/WorldClock) that supports 3
OLED displays with 3 timezones, and automatically adjusts the DST transitions
for all 3 zones:

![WorldClock](https://github.com/bxparks/clocks/blob/master/WorldClock/WorldClock.jpg)

<a name="Installation"></a>
## Installation

See [docs/installation.md](docs/installation.md).

<a name="UserGuides"></a>
## User Guides

* [README.md](README.md)
    * this file
* [docs/date_time_timezone.md](docs/date_time_timezone.md)
    * Date and Time classes
    * TimeZone classes
    * ZoneInfo Database
    * Mutations
    * Error Handling
    * Motivation and Design Considerations
    * Bugs and Limitations
* [docs/clock_system_clock.md](docs/clock_system_clock.md)
    * Clock
    * NTP Clock, DS3231 Clock, STM32 RTC Clock, STM32F1 Clock
    * SystemClock, SystemClockLoop, SystemClockCoroutine
* [Doxygen docs](https://bxparks.github.io/AceTime/html) hosted on GitHub Pages

<a name="Benchmarks"></a>
## Benchmarks

See [docs/benchmarks.md](docs/benchmarks.md).

<a name="Comparisons"></a>
## Comparisons

See [docs/comparisons.md](docs/comparisons.md).

<a name="SystemRequirements"></a>
## System Requirements

<a name="Hardware"></a>
### Hardware

The library is tested on the following boards:

* Arduino Nano clone (16 MHz ATmega328P)
* SparkFun Pro Micro clone (16 MHz ATmega32U4)
* SAMD21 M0 Mini (48 MHz ARM Cortex-M0+)
* STM32 Blue Pill (STM32F103C8, 72 MHz ARM Cortex-M3)
* NodeMCU 1.0 (ESP-12E module, 80 MHz ESP8266)
* WeMos D1 Mini (ESP-12E module, 80 MHz ESP8266)
* ESP32 dev board (ESP-WROOM-32 module, 240 MHz dual core Tensilica LX6)
* Teensy 3.2 (96 MHz ARM Cortex-M4)

I will occasionally test on the following hardware as a sanity check:

* Mini Mega 2560 (Arduino Mega 2560 compatible, 16 MHz ATmega2560)
* Teensy LC (48 MHz ARM Cortex-M0+)

The following boards are *not* supported:

* megaAVR (e.g. Nano Every)
* SAMD21 boards w/ `arduino:samd` version >= 1.8.10 (e.g. MKRZero)

<a name="ToolChain"></a>
### Tool Chain

This library was developed and tested using:

* [Arduino IDE 1.8.13](https://www.arduino.cc/en/Main/Software)
* [Arduino CLI 0.14.0](https://arduino.github.io/arduino-cli)
* [Arduino AVR Boards 1.8.3](https://github.com/arduino/ArduinoCore-avr)
* [Arduino SAMD Boards 1.8.9](https://github.com/arduino/ArduinoCore-samd)
* [SparkFun AVR Boards 1.1.13](https://github.com/sparkfun/Arduino_Boards)
* [SparkFun SAMD Boards 1.8.1](https://github.com/sparkfun/Arduino_Boards)
* [STM32duino 1.9.0](https://github.com/stm32duino/Arduino_Core_STM32)
* [ESP8266 Arduino 2.7.4](https://github.com/esp8266/Arduino)
* [ESP32 Arduino 1.0.4](https://github.com/espressif/arduino-esp32)
* [Teensydino 1.53](https://www.pjrc.com/teensy/td_download.html)

This library is *not* compatible with:
* [Arduino megaAVR](https://github.com/arduino/ArduinoCore-megaavr/)
* [MegaCoreX](https://github.com/MCUdude/MegaCoreX)
* [Arduino SAMD Boards >=1.8.10](https://github.com/arduino/ArduinoCore-samd)

It should work with [PlatformIO](https://platformio.org/) but I have
not tested it.

The library works on Linux or MacOS (using both g++ and clang++ compilers) using
the EpoxyDuino (https://github.com/bxparks/EpoxyDuino) emulation layer.

<a name="OperatingSystem"></a>
### Operating System

I use Ubuntu 18.04 and 20.04 for the vast majority of my development. I expect
that the library will work fine under MacOS and Windows, but I have not tested
them.

<a name="License"></a>
## License

[MIT License](https://opensource.org/licenses/MIT)

<a name="Feedback"></a>
## Feedback and Support

If you find this library useful, consider starring this project on GitHub. The
stars will let me prioritize the more popular libraries over the less popular
ones.

If you have any questions, comments, bug reports, or feature requests, please
file a GitHub ticket instead of emailing me unless the content is sensitive.
(The problem with email is that I cannot reference the email conversation when
other people ask similar questions later.) I'd love to hear about how this
software and its documentation can be improved. I can't promise that I will
incorporate everything, but I will give your ideas serious consideration.

<a name="Authors"></a>
## Authors

* Created by Brian T. Park (brian@xparks.net).
* Support an existing WiFi connection in `NtpClock` by denis-stepanov@
  [#24](https://github.com/bxparks/AceTime/issues/24).
* Support for STM32RTC through the `ace_time::clock::StmRtcClock` class
  by Anatoli Arkhipenko (arkhipenko@)
  [#39](https://github.com/bxparks/AceTime/pull/39).
