# AceTime

[![AUnit Tests](https://github.com/bxparks/AceTime/actions/workflows/aunit_tests.yml/badge.svg)](https://github.com/bxparks/AceTime/actions/workflows/aunit_tests.yml)
[![Validation Tests](https://github.com/bxparks/AceTime/actions/workflows/validation.yml/badge.svg)](https://github.com/bxparks/AceTime/actions/workflows/validation.yml)

**Breaking Changes in v1.8.0**: Three breaking changes were made in v1.8.0 to
reduce the maintenance complexity of the library, and to reduce the flash memory
consumption of client applications. See the [Migrating to
v1.8.0](MIGRATING.md#MigratingToVersion180) document.

The AceTime library provides Date, Time, and TimeZone classes which can convert
"epoch seconds" (seconds from the AceTime Epoch of 2000-01-01T00:00:00 UTC) to
human-readable local date and time fields. Those classes can also convert local
date and time between different time zones, properly accounting for all DST
transitions from the year 2000 until 2050. The ZoneInfo Database is extracted
from the [IANA TZ database](https://www.iana.org/time-zones). Different subsets
of the ZoneInfo Database can be compiled into the application to reduce flash
memory size.

The companion library [AceTimeClock](https://github.com/bxparks/AceTimeClock)
provides Clock classes to retrieve the time from more accurate sources, such as
an [NTP](https://en.wikipedia.org/wiki/Network_Time_Protocol) server, or a
[DS3231
RTC](https://www.maximintegrated.com/en/products/analog/real-time-clocks/DS3231.html)
chip. A special version of the `Clock` class called the `SystemClock` provides a
fast and accurate "epoch seconds" across all Arduino compatible systems. This
"epoch seconds" can be given to the classes in this library to convert it into
human readable components in different timezones.

The primordial motivation for creating the AceTime library was to build a
digital clock with an OLED or LED display, that would show the date and time of
multiple timezones at the same time, while adjusting for any DST changes in the
selected timezones automatically. Another major goal of the library is to keep
the resource (flash and RAM) consumption as small as practical, to allow
substantial portion of this library to run inside the 32kB of flash and 2kB of
RAM limits of an Arduino Nano or a SparkFun Pro Micro dev board. To meet that
goal, this library does not perform any dynamic allocation of memory. Everything
it needs is allocated statically.

This library can be an alternative to the Arduino Time
(https://github.com/PaulStoffregen/Time) and Arduino Timezone
(https://github.com/JChristensen/Timezone) libraries.

**Version**: 1.8.1 (2021-10-18, TZDB version 2021d)

**Changelog**: [CHANGELOG.md](CHANGELOG.md)

**See Also**:

* AceTimeClock (https://github.com/bxparks/AceTimeClock)

## Table of Contents

* [Installation](#Installation)
    * [Source Code](#SourceCode)
    * [Dependencies](#Dependencies)
* [Documentation](#Documentation)
    * [HelloDateTime](#HelloDateTime)
    * [HelloZoneManager](#HelloZoneManager)
    * [WorldClock](#WorldClock)
* [User Guide](#UserGuide)
* [Validation](#Validation)
* [Resource Consumption](#ResourceConsumption)
    * [Size Of Classes](#SizeOfClasses)
    * [Zone DB Size](#ZoneDbSize)
    * [Flash And Static Memory](#FlashAndStaticMemory)
    * [CPU Usage](#CPUUsage)
* [System Requirements](#SystemRequirements)
    * [Hardware](#Hardware)
    * [Tool Chain](#ToolChain)
    * [Operating System](#OperatingSystem)
* [Motivation and Design Considerations](#Motivation)
* [Comparison to Other Time Libraries](#Comparisons)
    * [Arduino Time Library](#ArduinoTimeLibrary)
    * [C Time Library](#CLibrary)
    * [ezTime](#EzTime)
    * [Micro Time Zone](#MicroTimeZone)
    * [Java Time, Joda-Time, Noda Time](#JavaTime)
    * [Howard Hinnant Date Library](#HinnantDate)
    * [Google cctz](#Cctz)
* [License](#License)
* [Feedback and Support](#FeedbackAndSupport)
* [Authors](#Authors)

<a name="Installation"></a>
## Installation

The latest stable release is available in the Arduino Library Manager in the
IDE. Search for "AceTime". Click install. The Library Manager should
automatically install AceTime and its the dependent library:

* AceTime (https://github.com/bxparks/AceTime)
* AceCommon (https://github.com/bxparks/AceCommon)

The development version can be installed by cloning the above repos manually.
You can copy over the contents to the `./libraries` directory used by the
Arduino IDE. (The result is a set of directories named `./libraries/AceTime` and
`./libraries/AceCommon`). Or you can create symlinks from `./libraries` to these
directories. Or you can `git clone` directly into the `./libraries` directory.

The `develop` branch contains the latest development.
The `master` branch contains the stable releases.

<a name="SourceCode"></a>
### Source Code

The source files are organized as follows:

* `src/AceTime.h` - main header file
* `src/ace_time/` - date and time classes (`ace_time::` namespace)
    * `src/ace_time/common/` - shared classes and utilities
    * `src/ace_time/internal/` - internal classes (`ace_time::basic`,
      `ace_time::extended` namespaces)
    * `src/ace_time/testing/` - files used in unit tests (`ace_time::testing`
      namespace)
    * `src/ace_time/zonedb/` - files generated from TZ Database for
      `BasicZoneProcessor` (`ace_time::zonedb` namespace)
    * `src/ace_time/zonedbx/` - files generated from TZ Database for
      `ExtendedZoneProcessor` (`ace_time::zonedbx` namespace)
* `tests/` - unit tests using [AUnit](https://github.com/bxparks/AUnit)
* `examples/` - example programs and benchmarks
    * Simple
        * [examples/HelloDateTime](examples/HelloDateTime)
            * Simple demo of `ZonedDateTime` class
        * [examples/HelloZoneManager](examples/HelloZoneManager)
            * Simple demo of `BasicZoneManager` class
    * Benchmarks
        * [examples/MemoryBenchmark](examples/MemoryBenchmark)
            * determine flash and static memory consumption of various classes
        * [examples/AutoBenchmark](examples/AutoBenchmark)
            * determine CPU usage of various features
        * [examples/ComparisonBenchmark](examples/ComparisonBenchmark)
            * compare AceTime with
            [Arduino Time Lib](https://github.com/PaulStoffregen/Time)
    * Debugging
        * [examples/DebugZoneProcessor](examples/DebugZoneProcessor)
            * Command-line debugging tool for ExtenedZoneProcessor using the
            EpoxyDuino environment

<a name="Dependencies"></a>
### Dependencies

The AceTime library depends on the following library:

* AceCommon (https://github.com/bxparks/AceCommon)

Various programs in the `examples/` directory have one or more of the following
external dependencies. The comment section near the top of the `*.ino` file will
usually have more precise dependency information:

* AceTimeClock (https://github.com/bxparks/AceTimeClock)
* AceRoutine (https://github.com/bxparks/AceRoutine)
* Arduino Time Lib (https://github.com/PaulStoffregen/Time)
* Arduino Timezone (https://github.com/JChristensen/Timezone)

If you want to run the unit tests or validation tests using a Linux or MacOS
machine, you need:

* AUnit (https://github.com/bxparks/AUnit)
* EpoxyDuino (https://github.com/bxparks/EpoxyDuino)

<a name="Documentation"></a>
## Documentation

* [README.md](README.md): this file
* [USER_GUIDE.md](USER_GUIDE.md): the AceTime User Guide
* [DEVELOPER.md](DEVELOPER.md): internal details for developers of AceTime
  library
* [MIGRATING.md](MIGRATING.md): migrating to versions with breaking changes
* [Doxygen docs](https://bxparks.github.io/AceTime/html) hosted on GitHub Pages
* [docs/comparisons.md](docs/comparisons.md) comparisons to other date, time and
  timezone libraries

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

// Create a BasicZoneManager with the entire TZ Database of ZONE entries. Use
// kZoneAndLinkRegistrySize and kZoneAndLinkRegistry to include LINK entries as
// well, at the cost of additional flash consumption. Cache size of 3 means that
// it can support 3 concurrent timezones without performance penalties.
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

<a name="WorldClock"></a>
### WorldClock

Here is a photo of the WorldClock
(https://github.com/bxparks/clocks/tree/master/WorldClock) that supports 3
OLED displays with 3 timezones, and automatically adjusts the DST transitions
for all 3 zones:

![WorldClock](https://github.com/bxparks/clocks/blob/master/WorldClock/WorldClock.jpg)

<a name="UserGuide"></a>
## User Guide

The full documentation of the following classes are given in the
[USER_GUIDE.md](USER_GUIDE.md):

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
    * two sets of timezone data are provided (Basic and Extended) because
      2 slightly different algorithms for handling timezone data are provided
    * ZoneInfo (opaque reference to a timezone)
        * `ace_time::basic::ZoneInfo` (258 zones and 193 links, as of 2021c)
            * `ace_time::zonedb::kZoneAfrica_Abidjan`
            * `ace_time::zonedb::kZoneAfrica_Accra`
            * ...
            * `ace_time::zonedb::kZonePacific_Wake`
            * `ace_time::zonedb::kZonePacific_Wallis`
        * `ace_time::extended::ZoneInfo` (377 zones and 217 links, as of 2021c)
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

<a name="Validation"></a>
## Validation

The details of how the Date, Time and TimeZone classes are validated are given
in [AceTimeValidation](https://github.com/bxparks/AceTimeValidation).

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

<a name="ResourceConsumption"></a>
## Resource Consumption

<a name="SizeOfClasses"></a>
### SizeOf Classes

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
```

<a name="ZoneDbSize"></a>
### Zone DB Size

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

An example of more complex application is the
WorldClock (https://github.com/bxparks/clocks/tree/master/WorldClock)
which has 3 OLED displays over SPI, 3 timezones using `BasicZoneProcessor`, a
`SystemClock` synchronized to a DS3231 chip on I2C, and 2 buttons with
debouncing and event dispatching provided by the AceButton
(https://github.com/bxparks/AceButton) library. This application consumes about
24 kB, well inside the 28 kB flash limit of a SparkFun Pro Micro controller.

This library does not perform dynamic allocation of memory so that it can be
used in small microcontroller environments. In other words, it does not call the
`new` operator nor the `malloc()` function, and it does not use the Arduino
`String` class. Everything it needs is allocated statically at initialization
time.

<a name="FlashAndStaticMemory"></a>
### Flash And Static Memory

[MemoryBenchmark](examples/MemoryBenchmark/) was used to determine the
size of the library for various microcontrollers (Arduino Nano to ESP32). Here
are 2 samples:

Arduino Nano

```
+---------------------------------------------------------------------+
| Functionality                          |  flash/  ram |       delta |
|----------------------------------------+--------------+-------------|
| Baseline                               |    474/   11 |     0/    0 |
|----------------------------------------+--------------+-------------|
| LocalDateTime                          |    820/   13 |   346/    2 |
| ZonedDateTime                          |   1302/   13 |   828/    2 |
| Manual ZoneManager                     |   1546/   13 |  1072/    2 |
| Basic TimeZone (1 zone)                |   6174/  199 |  5700/  188 |
| Basic TimeZone (2 zones)               |   6678/  203 |  6204/  192 |
| BasicZoneManager (1 zone)              |   7572/  217 |  7098/  206 |
| BasicZoneManager (zones)               |  20264/  593 | 19790/  582 |
| BasicZoneManager (zones+fat links)     |  24592/  593 | 24118/  582 |
| BasicLinkManager                       |   2404/   19 |  1930/    8 |
| Extended TimeZone (1 zone)             |   9182/  233 |  8708/  222 |
| Extended TimeZone (2 zones)            |   9826/  237 |  9352/  226 |
| ExtendedZoneManager (1 zone)           |  10564/  251 | 10090/  240 |
| ExtendedZoneManager (zones)            |  32298/  735 | 31824/  724 |
| ExtendedZoneManager (zones+fat links)  |  37178/  735 | 36704/  724 |
| ExtendedLinkManager                    |   2596/   19 |  2122/    8 |
+---------------------------------------------------------------------+
```

ESP8266:

```
+---------------------------------------------------------------------+
| Functionality                          |  flash/  ram |       delta |
|----------------------------------------+--------------+-------------|
| Baseline                               | 260089/27892 |     0/    0 |
|----------------------------------------+--------------+-------------|
| LocalDateTime                          | 260429/27896 |   340/    4 |
| ZonedDateTime                          | 260541/27896 |   452/    4 |
| Manual ZoneManager                     | 260557/27896 |   468/    4 |
| Basic TimeZone (1 zone)                | 266165/28472 |  6076/  580 |
| Basic TimeZone (2 zones)               | 266549/28472 |  6460/  580 |
| BasicZoneManager (1 zone)              | 267141/28472 |  7052/  580 |
| BasicZoneManager (zones)               | 284469/28472 | 24380/  580 |
| BasicZoneManager (zones+fat links)     | 291173/28472 | 31084/  580 |
| BasicLinkManager                       | 261961/27892 |  1872/    0 |
| Extended TimeZone (1 zone)             | 268357/28616 |  8268/  724 |
| Extended TimeZone (2 zones)            | 268741/28616 |  8652/  724 |
| ExtendedZoneManager (1 zone)           | 269317/28616 |  9228/  724 |
| ExtendedZoneManager (zones)            | 299177/28620 | 39088/  728 |
| ExtendedZoneManager (zones+fat links)  | 306745/28620 | 46656/  728 |
| ExtendedLinkManager                    | 262153/27892 |  2064/    0 |
+---------------------------------------------------------------------+
```

<a name="CPUUsage"></a>
### CPU Usage

[AutoBenchmark](examples/AutoBenchmark/) was used to determine the
CPU time consume by various features of the classes in this library. Two samples
are shown below:

Arduino Nano

```
+--------------------------------------------------+----------+
| Method                                           |   micros |
|--------------------------------------------------+----------|
| EmptyLoop                                        |    4.000 |
|--------------------------------------------------+----------|
| LocalDate::forEpochDays()                        |  221.000 |
| LocalDate::toEpochDays()                         |   56.000 |
| LocalDate::dayOfWeek()                           |   48.000 |
| OffsetDateTime::forEpochSeconds()                |  325.000 |
| OffsetDateTime::toEpochSeconds()                 |   84.000 |
| ZonedDateTime::toEpochSeconds()                  |   85.000 |
| ZonedDateTime::toEpochDays()                     |   71.000 |
| ZonedDateTime::forEpochSeconds(UTC)              |  339.000 |
| ZonedDateTime::forEpochSeconds(Basic_nocache)    | 1186.000 |
| ZonedDateTime::forEpochSeconds(Basic_cached)     |  615.000 |
| ZonedDateTime::forEpochSeconds(Extended_nocache) | 2137.000 |
| ZonedDateTime::forEpochSeconds(Extended_cached)  |  616.000 |
| BasicZoneManager::createForZoneName(binary)      |  119.000 |
| BasicZoneManager::createForZoneId(binary)        |   48.000 |
| BasicZoneManager::createForZoneId(linear)        |  307.000 |
+--------------------------------------------------+----------+
Iterations_per_run: 1000
```

ESP8266

```
+--------------------------------------------------+----------+
| Method                                           |   micros |
|--------------------------------------------------+----------|
| EmptyLoop                                        |    4.800 |
|--------------------------------------------------+----------|
| LocalDate::forEpochDays()                        |    7.900 |
| LocalDate::toEpochDays()                         |    3.800 |
| LocalDate::dayOfWeek()                           |    3.600 |
| OffsetDateTime::forEpochSeconds()                |   12.100 |
| OffsetDateTime::toEpochSeconds()                 |    6.700 |
| ZonedDateTime::toEpochSeconds()                  |    6.900 |
| ZonedDateTime::toEpochDays()                     |    5.800 |
| ZonedDateTime::forEpochSeconds(UTC)              |   12.900 |
| ZonedDateTime::forEpochSeconds(Basic_nocache)    |   95.400 |
| ZonedDateTime::forEpochSeconds(Basic_cached)     |   26.000 |
| ZonedDateTime::forEpochSeconds(Extended_nocache) |  189.600 |
| ZonedDateTime::forEpochSeconds(Extended_cached)  |   25.800 |
| BasicZoneManager::createForZoneName(binary)      |   14.800 |
| BasicZoneManager::createForZoneId(binary)        |    6.500 |
| BasicZoneManager::createForZoneId(linear)        |   44.200 |
+--------------------------------------------------+----------+
Iterations_per_run: 10000
```

<a name="SystemRequirements"></a>
## System Requirements

<a name="Hardware"></a>
### Hardware

This library has Tier 1 support on the following boards:

* Arduino Nano (16 MHz ATmega328P)
* SparkFun Pro Micro (16 MHz ATmega32U4)
* SAMD21 M0 Mini (48 MHz ARM Cortex-M0+)
* STM32 Blue Pill (STM32F103C8, 72 MHz ARM Cortex-M3)
* NodeMCU 1.0 (ESP-12E module, 80 MHz ESP8266)
* WeMos D1 Mini (ESP-12E module, 80 MHz ESP8266)
* ESP32 dev board (ESP-WROOM-32 module, 240 MHz dual core Tensilica LX6)
* Teensy 3.2 (96 MHz ARM Cortex-M4)

Tier 2 support can be expected on the following boards, mostly because I don't
test these as often:

* ATtiny85 (8 MHz ATtiny85)
* Arduino Pro Mini (16 MHz ATmega328P)
* Mini Mega 2560 (Arduino Mega 2560 compatible, 16 MHz ATmega2560)
* Teensy LC (48 MHz ARM Cortex-M0+)

The following boards are *not* supported:

* Any platform using the ArduinoCore-API
  (https://github.com/arduino/ArduinoCore-api), such as:
    * megaAVR (e.g. Nano Every)
    * SAMD21 boards w/ `arduino:samd` version >= 1.8.10 (e.g. Nano 33 IoT,
      MKRZero, etc)
    * Raspberry Pi Pico RP2040

<a name="ToolChain"></a>
### Tool Chain

This library was developed and tested using:

* [Arduino IDE 1.8.16](https://www.arduino.cc/en/Main/Software)
* [Arduino CLI 0.19.2](https://arduino.github.io/arduino-cli)
* [SpenceKonde ATTinyCore 1.5.2](https://github.com/SpenceKonde/ATTinyCore)
* [Arduino AVR Boards 1.8.3](https://github.com/arduino/ArduinoCore-avr)
* [Arduino SAMD Boards 1.8.9](https://github.com/arduino/ArduinoCore-samd)
* [SparkFun AVR Boards 1.1.13](https://github.com/sparkfun/Arduino_Boards)
* [SparkFun SAMD Boards 1.8.4](https://github.com/sparkfun/Arduino_Boards)
* [STM32duino 2.0.0](https://github.com/stm32duino/Arduino_Core_STM32)
* [ESP8266 Arduino 3.0.2](https://github.com/esp8266/Arduino)
* [ESP32 Arduino 1.0.6](https://github.com/espressif/arduino-esp32)
* [Teensyduino 1.55](https://www.pjrc.com/teensy/td_download.html)

This library is *not* compatible with:

* Any platform using the
  [ArduinoCore-API](https://github.com/arduino/ArduinoCore-api), for example:
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

<a name="Motivation"></a>
## Motivation and Design Considerations

In the beginning, I created a digital clock using an Arduino Nano board, a small
OLED display, and a DS3231 RTC chip. Everything worked, it was great. Then I
wanted the clock to figure out the Daylight Saving Time (DST) automatically. And
I wanted to create a clock that could display multiple timezones. Thus began my
journey down the rabbit hole of
[timezones](https://en.wikipedia.org/wiki/Time_zone).

In full-featured operating systems (e.g. Linux, MacOS, Windows) and languages
with timezone library support (e.g. Java, Python, JavaScript, C#, Go), the user
has the ability to specify the Daylight Saving time (DST) transitions using 2
ways:
* [POSIX
format](https://www.gnu.org/software/libc/manual/html_node/TZ-Variable.html)
which encodes the DST transitions into a string (e.g.
`EST+5EDT,M3.2.0/2,M11.1.0/2`) that can be parsed programmatically, or
* a reference to a [TZ Database](https://www.iana.org/time-zones) entry
(e.g. `America/Los_Angeles` or `Europe/London`) which identifies a set of time
transition rules for the given timezone.

The problem with the POSIX format is that it is somewhat difficult for a human
to understand, and the programmer must manually update this string when a
timezone changes its DST transition rules. Also, there is no historical
information in the POSIX string, so date and time written in the past cannot be
accurately expressed. The problem with the TZ Database is that most
implementations are too large to fit inside most Arduino environments. The
Arduino libraries that I am aware of use the POSIX format (e.g.
[ropg/ezTime](https://github.com/ropg/ezTime) or
[JChristensen/Timezone](https://github.com/JChristensen/Timezone)) for
simplicity and smaller memory footprint.

The AceTime library uses the TZ Database. When new versions of the database are
released (several times a year), I can regenerate the zone files, recompile the
application, and it will instantly use the new transition rules, without the
developer needing to create a new POSIX string. To address the memory constraint
problem, the AceTime library is designed to load only of the smallest subset of
the TZ Database that is required to support the selected timezones (1 to 4 have
been extensively tested). Dynamic lookup of the time zone is possible using the
`ZoneManager`, and the app develop can customize it with the list of zones that
are compiled into the app. On microcontrollers with more than about 32kB of
flash memory (e.g. ESP8266, ESP32, Teensy 3.2) and depending on the size of the
rest of the application, it may be possible to load the entire IANA TZ database.
This will allow the end-user to select the timezone dynamically, just like on
the big-iron machines.

The AceTime library is inspired by and borrows from:
* [Java 11 Time](https://docs.oracle.com/en/java/javase/11/docs/api/java.base/java/time/package-summary.html)
* [Micro Time Zone](https://github.com/evq/utz)
* [Arduino Timezone](https://github.com/JChristensen/Timezone)
* [Arduino Time](https://github.com/PaulStoffregen/Time)
* [Joda-Time](https://www.joda.org/joda-time/)
* [Noda Time](https://nodatime.org/)
* [Python datetime](https://docs.python.org/3/library/datetime.html)
* [Python pytz](https://pypi.org/project/pytz/)
* [ezTime](https://github.com/ropg/ezTime)

The names and API of AceTime classes are heavily borrowed from the [Java JDK 11
java.time](https://docs.oracle.com/en/java/javase/11/docs/api/java.base/java/time/package-summary.html)
package. Some important differences come from the fact that in Java, most
objects are reference objects and created on the heap. To allow AceTime to work
on an Arduino chip with only 2kB of RAM and 32kB of flash, the AceTime C++
classes perform *no* heap allocations (i.e. no calls to `operator new()` or
`malloc()`). Many of the smaller classes in the library are expected to be used
as "value objects", in other words, created on the stack and copied by value.
Fortunately, the C++ compilers are extremely good at optimizing away unnecessary
copies of these small objects. It is not possible to remove all complex memory
allocations when dealing with the TZ Database. In the AceTime library, I managed
to move most of the complex memory handling logic into the `ZoneProcessor` class
hierarchy. These are relatively large objects which are meant to be opaque
to the application developer, created statically at start-up time of
the application, and never deleted during the lifetime of the application.

The [Arduino Time Library](https://github.com/PaulStoffregen/Time) uses a set of
C functions similar to the [traditional C/Unix library
methods](http://www.catb.org/esr/time-programming/) (e.g `makeTime()` and
`breaktime()`). It also uses the Unix epoch of 1970-01-01T00:00:00Z and a
`int32_t` type as its `time_t` to track the number of seconds since the epoch.
That means that the largest date it can handle is 2038-01-19T03:14:07Z. AceTime
uses an epoch that starts on 2000-01-01T00:00:00Z using the same `int32_t` as
its `ace_time::acetime_t`, which means that maximum date increases to
2068-01-19T03:14:07Z. AceTime is also quite a bit faster than the Arduino Time
Library (although in most cases, performance of the Time Library is not an
issue): AceTime is **2-5X** faster on an ATmega328P, **3-5X** faster on the
ESP8266, **7-8X** faster on the ESP32, and **7-8X** faster on the Teensy ARM
processor.

AceTime aims to be the smallest library that can run on the basic Arduino
platform (e.g. Nano with 32kB flash and 2kB of RAM) that fully supports all
timezones in the TZ Database at compile-time. Memory constraints of the smallest
Arduino boards may limit the number of timezones supported by a single program
at runtime to 1-3 timezones. The library also aims to be as portable as
possible, and supports AVR microcontrollers, as well as ESP8266, ESP32 and
Teensy microcontrollers.

<a name="Comparisons"></a>
## Comparisons to Other Time Libraries

<a name="ArduinoTimeLibrary"></a>
### Arduino Time Library

The AceTime library can be substantially faster than the equivalent methods in
the [Arduino Time Library](https://github.com/PaulStoffregen/Time). The
[ComparisonBenchmark.ino](examples/ComparisonBenchmark/) program compares the
CPU run time of `LocalDateTime::forEpochSeconds()` and
`LocalDateTime::toEpochSeconds()` with the equivalent `breakTime()` and
`makeTime()` functions of the Arduino Time Library. Details are given in the
[ComparisonBenchmark/README.md](examples/ComparisonBenchmark/README.md) file
in that folder, but here is a summary of the roundtrip times for various boards
(in microseconds):

```
----------------------------+---------+----------+
Board or CPU                | AceTime | Time Lib |
----------------------------+---------+----------+
ATmega328P 16MHz (Nano)     | 337.500 |  931.500 |
ESP8266 80MHz               |  19.700 |   68.200 |
ESP32 240MHz                |   1.180 |    9.380 |
Teensy 3.2 96MHz            |   2.750 |   22.390 |
----------------------------+---------+----------+
```

<a name="CLibrary"></a>
### C Time Library (time.h)

Some version of the standard Unix/C library `<time.h>` is available in *some*
Arduino platforms, but not others:

* The [AVR libc time
  library](https://www.nongnu.org/avr-libc/user-manual/group__avr__time.html)
    * contains methods such as `gmtime()` to convert `time_t` integer into date
      time components `struct tm`,
    * and a non-standard `mk_gmtime()` to convert components into a `time_t`
      integer
    * the `time_t` integer is unsigned, and starts at 2000-01-01T00:00:00 UTC
    * no support for timezones
    * the `time()` value does *not* auto-increment. The `system_tick()` function
      must be manually called, probably in an ISR (interrupt service routine).
* The SAMD21 and Teensy platforms do not seem to have a `<time.h>` library.
* The ESP8266 and ESP32 have a `<time.h>` library.
    * contains some rudimentary support for POSIX formatted timezones.
    * does not have the equivalent of the (non-standard) `mk_gmtime()` AVR
      function.
    * unknown, not researched:
        * does the `time()` value auto-increment?
        * is the source of `time()` the same as `millis()` or a different RTC?

These libraries are all based upon the [traditional C/Unix library
methods](http://www.catb.org/esr/time-programming/) which can be difficult to
understand.

<a name="EzTime"></a>
### ezTime

The [ezTime](https://github.com/ropg/ezTime) is a library that seems to be
composed of 2 parts: A client library that runs on the microcontroller and a
server part that provides a translation from the timezone name to the POSIX DST
transition string. Unfortunately, this means that the controller must have
network access for this library to work. I wanted to create a library that was
self-contained and could run on an Arduino Nano with just an RTC chip without a
network shield.

<a name="MicroTimeZone"></a>
### Micro Time Zone

The [Micro Time Zone](https://github.com/evq/utz) is a pure-C library
that can compile on the Arduino platform. It contains a limited subset of the TZ
Database encoded as C structs and determines the DST transitions using the
encoded structs. It supports roughly of 45 zones with just a 3kB tzinfo
database. The initial versions of AceTime, particularly the `BasicZoneProcessor`
class was directly inspired by this library. It would be interesting to run this
library to the same set of "validation" unit tests that checks the AceTime logic
and see how accurate this library is. One problem with Micro Time Zone library
is that it loads the entire tzinfo database for all 45 time zones, even if only
one zone is used. Therefore, the AceTime library will consume less flash memory
for the database part if only a handful of zones are used. But the AceTime
library contains more algorithmic code so will consume more flash memory. It is
not entirely clear which library is smaller for 1-3 time zones. (This may be an
interesting investigation the future.)

<a name="JavaTime"></a>
### Java Time, Joda-Time, Noda Time

The names and functionality of most the date and time classes (`LocalTime`,
`LocalDate`, `LocalDateTime`, `OffsetDateTime`, and `ZonedDateTime`) were
inspired by the architecture of the [Java 11
java.time](https://docs.oracle.com/en/java/javase/11/docs/api/java.base/java/time/package-summary.html)
package. However, there were many parts of the `java.time` package that were not
appropriate for a microcontroller environment with small memory. Those were
implemented in other ways. There were other parts that seemed better implemented
by [Joda-Time](https://www.joda.org/joda-time/) or [Noda
Time](https://nodatime.org/)). I picked the ones that I liked.

Those other libraries (`java.time`, Joda-Time, and Noda Time) all provide
substantially more fine-grained class hierarchies to provider better
type-safety. For example, those libraries just mentioned provided an `Instant`
class, a `Duration` class, an `Interval` class. The `java.time` package also
provides other fine-grained classes such as `OffsetTime`, `OffsetDate`, `Year`,
`Month`, `MonthDay` classes. For the AceTime library, I decided to avoid
providing too many classes. The API of the library is already too large, I did
not want to make them larger than necessary.

<a name="HinnantDate"></a>
### Howard Hinnant Date Library

The [date](https://github.com/HowardHinnant/date) package by Howard Hinnant is
based upon the `<chrono>` standard library and consists of several libraries of
which `date.h` and `tz.h` are comparable to AceTime. Modified versions of these
libraries were voted into the C++20 standard.

Unfortunately these libraries are not suitable for an Arduino microcontroller
environment because:

* The libraries depend extensively on 64-bit integers which are
  impractical on 8-bit microcontrollers with only 32kB of flash memory.
* The `tz.h` library has the option of downloading the TZ Database files over
  the network using `libcurl` to the OS filesystem then parsing the files, or
  using the native Zoneinfo entries on the host OS. Neither options are
  practical on small microcontrollers. The raw TZ Database files consume about
  1MB in gzip'ed format, which are not suitable for a 32kB Arduino
  microcontroller.
* The libraries has dependencies on other libraries such as `<iostream>` and
  `<chrono>` which don't exist on most Arduino platforms.
* The libraries are heavily templatized to provide maximum flexibility
  and type-safety. But this makes the libraries incredibly hard to understand
  and cumbersome to use for the simple use cases targeted by the AceTime
  library.

The Hinnant date libraries were invaluable for writing the
[BasicHinnantDateTest](https://github.com/bxparks/AceTimeValidation/tree/master/BasicHinnantDateTest)
and
[ExtendedHinnantDateTest](https://github.com/bxparks/AceTimeValidation/tree/master/ExtendedHinnantDateTest)
validation tests which compare the AceTime algorithms to the Hinnant Date
algorithms. For all times zones between the years 2000 until 2050, the AceTime
UTC offsets (`TimeZone::getUtcOffset()`), timezone abbreviations
(`TimeZone::getAbbrev()`), and epochSecond conversion to date components
(`ZonedDateTime::fromEpochSeconds()`) match the results from the Hinnant Date
libraries.

<a name="Cctz"></a>
### Google cctz

The [cctz](https://github.com/google/cctz) library from Google is also based on
the `<chrono>` library. I have not looked at this library closely because I
assumed that it would *not* fit inside an Arduino controller. Hopefully I will
get some time to take a closer look in the future.

<a name="License"></a>
## License

[MIT License](https://opensource.org/licenses/MIT)

<a name="FeedbackAndSupport"></a>
## Feedback and Support

If you have any questions, comments and other support questions about how to
use this library, please use the
[GitHub Discussions](https://github.com/bxparks/AceTime/discussions)
for this project. If you have bug reports or feature requests, please file a
ticket in [GitHub Issues](https://github.com/bxparks/AceTime/issues).
I'd love to hear about how this software and its documentation can be improved.
I can't promise that I will incorporate everything, but I will give your ideas
serious consideration.

Please refrain from emailing me directly unless the content is sensitive. The
problem with email is that I cannot reference the email conversation when other
people ask similar questions later.

<a name="Authors"></a>
## Authors

* Created by Brian T. Park (brian@xparks.net).
* Support an existing WiFi connection in `NtpClock` by denis-stepanov@
  [#24](https://github.com/bxparks/AceTime/issues/24).
* Support for STM32RTC through the `ace_time::clock::StmRtcClock` class
  by Anatoli Arkhipenko (arkhipenko@)
  [#39](https://github.com/bxparks/AceTime/pull/39).
