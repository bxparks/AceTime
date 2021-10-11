# AceTime

[![AUnit Tests](https://github.com/bxparks/AceTime/actions/workflows/aunit_tests.yml/badge.svg)](https://github.com/bxparks/AceTime/actions/workflows/aunit_tests.yml)
[![Validation Tests](https://github.com/bxparks/AceTime/actions/workflows/validation.yml/badge.svg)](https://github.com/bxparks/AceTime/actions/workflows/validation.yml)

**Breaking Change in v1.8**: Two major changes were made in v1.8 to reduce
reduce the maintenance complexity of the library, and to reduce the flash
memory consumption of downstream apps.

1) The `SystemClock` and other clock classes was moved to
   [AceTimeClock](https://github.com/bxparks/AceTimeClock).
2) The `DS3231Clock` was converted into a template class to replace a direct
   dependency to the I2C `<Wire.h>` library to an indirect dependency to the
   [AceWire](https://github.com/bxparks/AceWire) library.

The migration guide to v1.8 is given in the [Migration for
v1.8](#MigrationForVersion18) subsection below.

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

**Version**: 1.8 (2021-10-08, TZDB version 2021c)

**Changelog**: [CHANGELOG.md](CHANGELOG.md)

**See Also**:

* AceTimeClock (https://github.com/bxparks/AceTimeClock)
* AceTimePython (https://github.com/bxparks/AceTimePython)

## Table of Contents

* [Installation](#Installation)
    * [Source Code](#SourceCode)
    * [Dependencies](#Dependencies)
* [Documentation](#Documentation)
    * [HelloDateTime](#HelloDateTime)
    * [HelloZoneManager](#HelloZoneManager)
    * [WorldClock](#WorldClock)
* [User Guide](#UserGuide)
* [Migration for v1.8](#MigrationForVersion18)
    * [Migrating to AceTimeClock](#MigratingToAceTimeClock)
    * [Migrating the DS3231Clock](#MigratingTheDS3231Clock)
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
* [License](#License)
* [Feedback and Support](#FeedbackAndSupport)
* [Authors](#Authors)

<a name="Installation"></a>
## Installation

The latest stable release is available in the Arduino Library Manager in the
IDE. Search for "AceTime". Click install. The Library Manager should
automatically install the dependent library:

* AceCommon (https://github.com/bxparks/AceCommon)

The development version can be installed by cloning the 2 git repos:

* AceTime (https://github.com/bxparks/AceTime)
* AceCommon (https://github.com/bxparks/AceCommon)

You can copy over the contents to the `./libraries` directory used by the
Arduino IDE. (The result is directories named `./libraries/AceTime` and
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
* [Doxygen docs](https://bxparks.github.io/AceTime/html) hosted on GitHub Pages
* [DEVELOPER.md](DEVELOPER.md) for developers of the AceTime library itself
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

<a name="MigrationFor1.8"></a>
## Migration for v1.8

In AceTime v1.8, the `ace_time::clock` classes were moved into the new
AceTimeClock library. At the same time, the `DS3231Clock` class was also
converted into a template class, replacing its direct dependency to the
pre-installed `<Wire.h>` library with an indirect dependency to the
[AceWire](https://github.com/bxparks/AceWire) library.


<a name="MigratingToAceTimeClock"></a>
### Migrating to AceTimeClock

For AceTime v1.8, the clock classes under the `ace_time::clock` namespace have
been moved to the `AceTimeClock` library. To help backwards compatibility,
the namespace of the clock classes remain in the `ace_time::clock` namespace.

To migrate your old code, install `AceTimeClock` using the Arduino Library
Manager. (See [AceTimeClock
Installation](https://github.com/bxparks/AceTimeClock#Installation) for more
details). Then you need to update your code to add the `<AceTimeClock.h>` header
file, in addition to the `<AceTime.h>` header.

For each instance of `<AceTime.h>`, add the `<AceTimeClock.h>` as well. So
replace:

```C++
#include <AceTime.h>
using namespace ace_time;
using namespace ace_time::clock;
```
with

```C++
#include <AceTime.h>
#include <AceTimeClock.h>
using namespace ace_time;
using namespace ace_time::clock;
```

<a name="MigratingTheDS3231Clock"></a>
### Migrating the DS3231Clock

For AceTime v1.8, the `DS3231Clock` class was converted into a template class to
replace a direct dependency to the `<Wire.h>` library with an indirect
dependency to to the [AceWire](https://github.com/bxparks/AceWire) library.
There were 2 primary motivation for the change.

One, simply including the `<Wire.h>` header file increases the flash memory
usage by ~1300 bytes on AVR, *even* if the `Wire` object is never used. The
`DS3231Clock` class is the *only* class in the AceTime library that depended on
the `<Wire.h>`. So any application that pulled in `<AceTime.h>` for the time
zone classes, but did not use the clock classes, would suffers the increased
flash usage of the `<Wire.h>` library.

Two, the `TwoWire` class from `<Wire.h>` cannot be used polymorphically
(see [SoftwareWire#28](https://github.com/Testato/SoftwareWire/issues/28) for
more details.) If a library uses the `<Wire.h>` directly, it is impossible to
replace the `Wire` object with a different I2C implementation (for example,
one of the ones in this [Overview of Arduino I2C
libraries](https://github.com/Testato/SoftwareWire/wiki/Arduino-I2C-libraries).
The `<AceWire.h>` library solves this problem by using compile-time polymorphism
using C++ templates.

Here is the migration process. For all occurrences of the `DS3231Clock` class
like this:

```C++
#include <AceTimeClock.h>
using namespace ace_time;
using namespace ace_time::clock;
...
DS3231Clock dsClock;
```

Replace that with a template instance of the `DS3231Clock<T>` class:

```C++
#include <AceTimeClock.h>
#include <AceWire.h> // TwoWireInterface
#include <Wire.h> // TwoWire, Wire
using namespace ace_time;
using namespace ace_time::clock;

using WireInterface = ace_wire::TwoWireInterface<TwoWire>;
WireInterface wireInterface(Wire);
DS3231Clock<WireInterface> dsClock(wireInterface);
```

Here is another example where the `SimpleWireInterface` class is used instead of
the `TwoWireInterface` shown above:

```C++
#include <AceTimeClock.h>
#include <AceWire.h>
using namespace ace_time;

const uint8_t SCL_PIN = SCL;
const uint8_t SDA_PIN = SDA;
const uint8_t DELAY_MICROS = 4;
using WireInterface = ace_wire::SimpleWireInterface;
WireInterface wireInterface(SDA_PIN, SCL_PIN, DELAY_MICROS);
DS3231Clock<WireInterface> dsClock(wireInterface);
```

According to the benchmarks at
[AceWire/MemoryBenchmark](https://github.com/bxparks/AceWire/tree/develop/examples/MemoryBenchmark),
using `SimpleWireInterface` instead of the `TwoWire` class reduces flash
consumption by 1500 bytes on an AVR processor. The flash consumption can be
reduced by 2000 bytes if the "fast" version `SimpleWireFastInterface` is used
instead.

Regardless of the specific I2C implementation selected, no changes to
`DS3231Clock<T>` templatized class are required.

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
| BasicZoneManager (1 zone)              |   7900/  219 |  7426/  208 |
| BasicZoneManager (zones)               |  20594/  595 | 20120/  584 |
| BasicZoneManager (zones+thin links)    |  22208/  595 | 21734/  584 |
| BasicZoneManager (zones+fat links)     |  24922/  595 | 24448/  584 |
| Extended TimeZone (1 zone)             |   9182/  233 |  8708/  222 |
| Extended TimeZone (2 zones)            |   9826/  237 |  9352/  226 |
| ExtendedZoneManager (1 zone)           |  10868/  253 | 10394/  242 |
| ExtendedZoneManager (zones)            |  32604/  737 | 32130/  726 |
| ExtendedZoneManager (zones+thin links) |  34406/  737 | 33932/  726 |
| ExtendedZoneManager (zones+fat links)  |  37484/  737 | 37010/  726 |
+---------------------------------------------------------------------+
```

ESP8266:

```
+---------------------------------------------------------------------+
| Functionality                          |  flash/  ram |       delta |
|----------------------------------------+--------------+-------------|
| Baseline                               | 260089/27892 |     0/    0 |
|----------------------------------------+--------------+-------------|
| LocalDateTime                          | 262597/27972 |  2508/   80 |
| ZonedDateTime                          | 262709/27972 |  2620/   80 |
| Manual ZoneManager                     | 262725/27972 |  2636/   80 |
| Basic TimeZone (1 zone)                | 268253/28548 |  8164/  656 |
| Basic TimeZone (2 zones)               | 268637/28548 |  8548/  656 |
| BasicZoneManager (1 zone)              | 269453/28548 |  9364/  656 |
| BasicZoneManager (zones)               | 286781/28548 | 26692/  656 |
| BasicZoneManager (zones+thin links)    | 288365/28548 | 28276/  656 |
| BasicZoneManager (zones+fat links)     | 293485/28548 | 33396/  656 |
| Extended TimeZone (1 zone)             | 270445/28692 | 10356/  800 |
| Extended TimeZone (2 zones)            | 270829/28692 | 10740/  800 |
| ExtendedZoneManager (1 zone)           | 271789/28692 | 11700/  800 |
| ExtendedZoneManager (zones)            | 301601/28688 | 41512/  796 |
| ExtendedZoneManager (zones+thin links) | 303345/28688 | 43256/  796 |
| ExtendedZoneManager (zones+fat links)  | 309169/28688 | 49080/  796 |
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
| EmptyLoop                                        |      4.0 |
|--------------------------------------------------+----------|
| LocalDate::forEpochDays()                        |    218.0 |
| LocalDate::toEpochDays()                         |     56.0 |
| LocalDate::dayOfWeek()                           |     49.0 |
| OffsetDateTime::forEpochSeconds()                |    323.0 |
| OffsetDateTime::toEpochSeconds()                 |     86.0 |
| ZonedDateTime::toEpochSeconds()                  |     83.0 |
| ZonedDateTime::toEpochDays()                     |     72.0 |
| ZonedDateTime::forEpochSeconds(UTC)              |    338.0 |
| ZonedDateTime::forEpochSeconds(Basic_nocache)    |   1188.0 |
| ZonedDateTime::forEpochSeconds(Basic_cached)     |    617.0 |
| ZonedDateTime::forEpochSeconds(Extended_nocache) |   2139.0 |
| ZonedDateTime::forEpochSeconds(Extended_cached)  |    616.0 |
| BasicZoneManager::createForZoneName(binary)      |    121.0 |
| BasicZoneManager::createForZoneId(binary)        |     48.0 |
| BasicZoneManager::createForZoneId(linear)        |    307.0 |
| BasicZoneManager::createForZoneId(link)          |     83.0 |
+--------------------------------------------------+----------+
Iterations_per_run: 1000
```

ESP8266

```
+--------------------------------------------------+----------+
| Method                                           |   micros |
|--------------------------------------------------+----------|
| EmptyLoop                                        |      4.9 |
|--------------------------------------------------+----------|
| LocalDate::forEpochDays()                        |      7.9 |
| LocalDate::toEpochDays()                         |      3.8 |
| LocalDate::dayOfWeek()                           |      3.8 |
| OffsetDateTime::forEpochSeconds()                |     12.3 |
| OffsetDateTime::toEpochSeconds()                 |      6.8 |
| ZonedDateTime::toEpochSeconds()                  |      6.8 |
| ZonedDateTime::toEpochDays()                     |      5.8 |
| ZonedDateTime::forEpochSeconds(UTC)              |     13.0 |
| ZonedDateTime::forEpochSeconds(Basic_nocache)    |     95.4 |
| ZonedDateTime::forEpochSeconds(Basic_cached)     |     25.9 |
| ZonedDateTime::forEpochSeconds(Extended_nocache) |    189.5 |
| ZonedDateTime::forEpochSeconds(Extended_cached)  |     25.9 |
| BasicZoneManager::createForZoneName(binary)      |     14.8 |
| BasicZoneManager::createForZoneId(binary)        |      6.5 |
| BasicZoneManager::createForZoneId(linear)        |     44.0 |
| BasicZoneManager::createForZoneId(link)          |     11.5 |
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
