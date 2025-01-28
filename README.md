# AceTime

[![AUnit Tests](https://github.com/bxparks/AceTime/actions/workflows/aunit_tests.yml/badge.svg)](https://github.com/bxparks/AceTime/actions/workflows/aunit_tests.yml)
[![Validation Tests](https://github.com/bxparks/AceTime/actions/workflows/validation.yml/badge.svg)](https://github.com/bxparks/AceTime/actions/workflows/validation.yml)

The AceTime library provides Date, Time, and TimeZone classes which can convert
"epoch seconds" from the AceTime Epoch (default 2050-01-01 UTC) to
human-readable local date and time fields. Those classes can also convert local
date and time between different time zones defined by the [IANA TZ
database](https://www.iana.org/time-zones) while accurately accounting for DST
transitions.

The default AceTime epoch is 2050-01-01, but it can be adjusted by the client
application at runtime. The epoch second has the type `acetime_t` which is a
32-bit signed integer, instead of a 64-bit signed integer. Using the smaller
32-bit integer allows the library to use less CPU and memory resources on 8-bit
and 32-bit microcontrollers without native 64-bit integer instructions. The
range of a 32-bit integer is about 136 years. To be safe, AceTime timezone
functions should be kept well within the bounds of this interval, for example,
straddling roughly +/- 60 years of the `Epoch::currentEpochYear()`.

The library provides 3 pre-generated ZoneInfo Databases which are
programmatically extracted from the IANA TZ database:

- [zonedb](src/zonedb) ("basic", not usually recommended)
    - accurate over the years `[2000,10000)`
    - contains a subset of zones (~450) compatible with `BasicZoneProcessor`
      and `BasicZoneManager`
- [zonedbx](src/zonedbx) ("extended", recommended for most situations)
    - accurate over the years `[2000,10000)`
    - contains all zones and links (~600) in the IANA TZ database
    - compatible with `ExtendedZoneProcessor` and `ExtendedZoneManager`
- [zonedbc](src/zonedbc) ("complete", mostly intended for validation testing,
  new in v2.3)
    - accurate over the years `[0001,10000)`
    - contains all zones and links (~600) the IANA TZ database
    - compatible with `CompleteZoneProcessor` and `CompleteZoneManager`

Client applications can choose to incorporate a subset of the zones provided by
the above databases to save flash memory size of the application.

Support for [Unix time](https://en.wikipedia.org/wiki/Unix_time) using the Unix
epoch of 1970-01-01 is provided through conversion functions of the `time_t`
type. Only the 64-bit version of the `time_t` type is supported to avoid the
[Year 2038 Problem](https://en.wikipedia.org/wiki/Year_2038_problem).

The companion library [AceTimeClock](https://github.com/bxparks/AceTimeClock)
provides Clock classes to retrieve the time from more accurate sources, such as
an [NTP](https://en.wikipedia.org/wiki/Network_Time_Protocol) server, or a
[DS3231
RTC](https://www.maximintegrated.com/en/products/analog/real-time-clocks/DS3231.html)
chip. A special version of the `Clock` class called the `SystemClock` provides a
fast and accurate "epoch seconds" across all Arduino compatible systems. This
"epoch seconds" can be given to the classes in this library to convert it into
human readable components in different timezones. On the ESP8266 and ESP32, the
AceTime library can be used with the SNTP client and the C-library `time()`
function through the 64-bit `time_t` value. See [ESP8266 and ESP32
TimeZones](#Esp8266AndEspTimeZones) below.

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

**Major Changes in v2.3**: Add `CompleteZoneProcessor`, `CompleteZoneManager`,
and the `zonedbc` database to support all timezones, for all transitions defined
in the IANA TZ database (`[1844,2087]`), and extending the validity of timezone
calculations from `[2000,10000)` to `[0001,10000)`.

**Version**: 3.0.0 (2025-01-28, TZDB 2025a)

**Changelog**: [CHANGELOG.md](CHANGELOG.md)

**Migration**: [MIGRATING.md](MIGRATING.md)

**User Guide**: [USER_GUIDE.md](USER_GUIDE.md)

**See Also**:

* AceTimeClock (https://github.com/bxparks/AceTimeClock)
* AceTimeValidation (https://github.com/bxparks/AceTimeValidation)
* AceTimeTools (https://github.com/bxparks/AceTimeTools)
* acetimepy (https://github.com/bxparks/acetimepy)
* acetimec (https://github.com/bxparks/acetimec)
* acetimego (https://github.com/bxparks/acetimego)

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
    * [ESP8266 and ESP32 TimeZones](#Esp8266AndEspTimeZones)
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
automatically install AceTime and its dependent libraries:

* AceTime (https://github.com/bxparks/AceTime)
* AceCommon (https://github.com/bxparks/AceCommon)
* AceSorting (https://github.com/bxparks/AceSorting)

The development version can be installed by cloning the above repos manually.
You can copy over the contents to the `./libraries` directory used by the
Arduino IDE. (The result is a set of directories named `./libraries/AceTime`,
`./libraries/AceCommon`, `./libraries/AceSorting`). Or you can create symlinks
from `./libraries` to these directories. Or you can `git clone` directly into
the `./libraries` directory.

The `develop` branch contains the latest development.
The `master` branch contains the stable releases.

<a name="SourceCode"></a>
### Source Code

The source files are organized as follows:

* `src/AceTime.h` - main header file
* main library code
    * `src/ace_time/` - date and time classes (`ace_time::` namespace)
    * `src/ace_time/common/` - shared classes and utilities
    * `src/ace_time/testing/` - files used in unit tests (`ace_time::testing`
        namespace)
    * `src/zoneinfo` - reading the zone databases, and normalizing the
      interface for accessing the data records
* zone databases
    * `src/zonedb/` - files generated from TZ Database for
        `BasicZoneProcessor` (`ace_time::zonedb` namespace)
    * `src/zonedbtesting/` - limited subset of `zonedb` for unit tests
    * `src/zonedbx/` - files generated from TZ Database for
        `ExtendedZoneProcessor` (`ace_time::zonedbx` namespace)
    * `src/zonedbxtesting/` - limited subset of `zonedbx` for unit tests
    * `src/zonedbc/` - files generated from TZ Database for
        `CompleteZoneProcessor` (`ace_time::zonedbc` namespace)
    * `src/zonedbctesting/` - limited subset of `zonedbc` for unit tests
* `tests/
    * unit tests using [AUnit](https://github.com/bxparks/AUnit)
* `examples/` - example programs and benchmarks
    * Simple
        * [HelloDateTime](examples/HelloDateTime)
            * Simple demo of `ZonedDateTime` class
        * [HelloZoneManager](examples/HelloZoneManager)
            * Simple demo of `ExtendedZoneManager` class
    * Intermediate
        * [CustomZoneRegistry](examples/CustomZoneRegistry)
            * Same as `HelloZoneManager`, but using a custom zone registry
              with only 7-8 timezones instead of the ~600 timezones in the
              full `zonedbx` registry.
    * Advanced
        * [EspTime](examples/EspTime)
            * Use AceTime with the built-in SNTP client of ESP8266 and ESP32.
    * Benchmarks
        * These are internal applications to benchmark various parts of this
          library. They are not meant to be examples for how to use the library.
        * [MemoryBenchmark](examples/MemoryBenchmark)
            * determine flash and static memory consumption of various classes
        * [AutoBenchmark](examples/AutoBenchmark)
            * determine CPU usage of various features
        * [ComparisonBenchmark](examples/ComparisonBenchmark)
            * compare AceTime with
            [Arduino Time Lib](https://github.com/PaulStoffregen/Time)
        * [CompareAceTimeToHinnantDate](examples/CompareAceTimeToHinnantDate)
            * compare the performance of AceTime to Hinnant date library
            * AceTime seems to be about 90X faster
    * Debugging
        * [DebugZoneProcessor](examples/DebugZoneProcessor)
            * Command-line debugging tool for ExtenedZoneProcessor using the
              EpoxyDuino environment
        * [ListZones](examples/ListZones)
            * List the zones managed by the `ExtendedZoneManager`, sorted
              by name, or by UTC offset and name.
            * Used to debug the `ZoneSorter` classes.

<a name="Dependencies"></a>
### Dependencies

The AceTime library depends on the following libraries:

* AceCommon (https://github.com/bxparks/AceCommon)
* AceSorting (https://github.com/bxparks/AceSorting)

Various programs in the `examples/` directory have one or more of the following
external dependencies. The comment section near the top of the `*.ino` file will
usually have more precise dependency information:

* AceTimeClock (https://github.com/bxparks/AceTimeClock)
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

<a name="HelloDateTime"></a>
### HelloDateTime

Here is a simple program (see [examples/HelloDateTime](examples/HelloDateTime))
which demonstrates how to create and manipulate date and times in different time
zones:

```C++
#include <Arduino.h>
#include <AceTime.h>

using namespace ace_time;

// ZoneProcessor instances should be created statically at initialization time.
static ExtendedZoneProcessor losAngelesProcessor;
static ExtendedZoneProcessor londonProcessor;

void setup() {
  delay(1000);
  Serial.begin(115200);
  while (!Serial); // Wait until Serial is ready - Leonardo/Micro

  // TimeZone objects are light-weight and can be created on the fly.
  TimeZone losAngelesTz = TimeZone::forZoneInfo(
      &zonedbx::kZoneAmerica_Los_Angeles,
      &losAngelesProcessor);
  TimeZone londonTz = TimeZone::forZoneInfo(
      &zonedbx::kZoneEurope_London,
      &londonProcessor);

  // Create from components. 2019-03-10T03:00:00 is just after DST change in
  // Los Angeles (2am goes to 3am).
  ZonedDateTime startTime = ZonedDateTime::forComponents(
      2019, 3, 10, 3, 0, 0, losAngelesTz);

  Serial.print(F("Epoch Seconds: "));
  acetime_t epochSeconds = startTime.toEpochSeconds();
  Serial.println(epochSeconds);

  Serial.print(F("Unix Seconds: "));
  int64_t unixSeconds = startTime.toUnixSeconds64();
  Serial.println((int32_t) unixSeconds);

  Serial.println(F("=== Los_Angeles"));
  ZonedDateTime losAngelesTime = ZonedDateTime::forEpochSeconds(
      epochSeconds, losAngelesTz);
  Serial.print(F("Time: "));
  losAngelesTime.printTo(Serial);
  Serial.println();

  Serial.print(F("Day of Week: "));
  Serial.println(
      DateStrings().dayOfWeekLongString(losAngelesTime.dayOfWeek()));

  // Print info about UTC offset
  TimeOffset offset = losAngelesTime.timeOffset();
  Serial.print(F("Total UTC Offset: "));
  offset.printTo(Serial);
  Serial.println();

  // Print info about the current time zone
  Serial.print(F("Zone: "));
  losAngelesTz.printTo(Serial);
  Serial.println();

  // Print the current time zone abbreviation, e.g. "PST" or "PDT"
  ZonedExtra ze = losAngelesTz.getZonedExtra(epochSeconds);
  Serial.print(F("Abbreviation: "));
  SERIAL_PORT_MONITOR.print(ze.abbrev());
  Serial.println();

  // Create from epoch seconds. London is still on standard time.
  ZonedDateTime londonTime = ZonedDateTime::forEpochSeconds(
      epochSeconds, londonTz);

  Serial.println(F("=== London"));
  Serial.print(F("Time: "));
  londonTime.printTo(Serial);
  Serial.println();

  // Print info about the current time zone
  Serial.print(F("Zone: "));
  londonTz.printTo(Serial);
  Serial.println();

  // Print the current time zone abbreviation, e.g. "GMT" or "BST"
  ze = londonTz.getZonedExtra(epochSeconds);
  Serial.print(F("Abbreviation: "));
  SERIAL_PORT_MONITOR.print(ze.abbrev());
  Serial.println();

  Serial.println(F("=== Compare ZonedDateTime"));
  Serial.print(F("losAngelesTime.compareTo(londonTime): "));
  Serial.println(losAngelesTime.compareTo(londonTime));
  Serial.print(F("losAngelesTime == londonTime: "));
  Serial.println((losAngelesTime == londonTime) ? "true" : "false");
}

void loop() {
}
```

Running this should produce the following on the Serial port:
```
Epoch Seconds: -972396000
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
losAngelesTime.compareTo(londonTime): 0
losAngelesTime == londonTime: false
```

(The default epoch for AceTime is 2050-01-01, so a date in 2019 will return a
negative epoch seconds.)

<a name="HelloZoneManager"></a>
### HelloZoneManager

The [examples/HelloZoneManager](examples/HelloZoneManager) example shows how to
load the entire `zonedbx` ZoneInfo Database into an `ExtendedZoneManager`, then
create 3 time zones using 3 different ways: `createForZoneInfo()`,
`createForZoneName()`, and `createForZoneId()`. This program requires a 32-bit
microcontroller environment because the `ExtendedZoneManager` with the
`zonedbx::kZoneAndLinkRegistry` consumes ~44 kB of flash which does not fit in
the 32 kB flash memory capacity of an Arduino Nano. If the `ExtendedZoneManager`
is replaced with the `BasicZoneManager` and the smaller `kZoneRegistry` is used
instead, the flash size goes down to about ~24 kB. Depending on the code size of
the rest of the application, this may fit inside an Arduino Nano.

```C++
#include <Arduino.h>
#include <AceTime.h>

using namespace ace_time;

// Create an ExtendedZoneManager with the entire TZ Database of Zone and Link
// entries. Cache size of 3 means that it can support 3 concurrent timezones
// without performance penalties.
static const int CACHE_SIZE = 3;
static ExtendedZoneProcessorCache<CACHE_SIZE> zoneProcessorCache;
static ExtendedZoneManager manager(
    zonedbx::kZoneAndLinkRegistrySize,
    zonedbx::kZoneAndLinkRegistry,
    zoneProcessorCache);

void setup() {
  Serial.begin(115200);
  while (!Serial); // Wait until ready - Leonardo/Micro

  // Create America/Los_Angeles timezone by ZoneInfo.
  TimeZone losAngelesTz = manager.createForZoneInfo(
        &zonedbx::kZoneAmerica_Los_Angeles);
  ZonedDateTime losAngelesTime = ZonedDateTime::forComponents(
      2019, 3, 10, 3, 0, 0, losAngelesTz);
  losAngelesTime.printTo(Serial);
  Serial.println();

  // Create Europe/London timezone by ZoneName.
  TimeZone londonTz = manager.createForZoneName("Europe/London");
  ZonedDateTime londonTime = losAngelesTime.convertToTimeZone(londonTz);
  londonTime.printTo(Serial);
  Serial.println();

  // Create Australia/Sydney timezone by ZoneId.
  TimeZone sydneyTz = manager.createForZoneId(zonedbx::kZoneIdAustralia_Sydney);
  ZonedDateTime sydneyTime = losAngelesTime.convertToTimeZone(sydneyTz);
  sydneyTime.printTo(Serial);
  Serial.println();
}

void loop() {
}
```

It produces the following output:
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
        * `ace_time::offset_date_time_mutation::`
        * `ace_time::zoned_date_time_mutation::`
* timezone classes
    * `ace_time::ZoneProcessor`
        * `ace_time::BasicZoneProcessor`
        * `ace_time::ExtendedZoneProcessor`
        * `ace_time::CompleteZoneProcessor`
    * `ace_time::TimeZone`
    * `ace_time::ZonedDateTime`
    * `ace_time::ZoneManager`
        * `ace_time::BasicZoneManager`
        * `ace_time::ExtendedZoneManager`
        * `ace_time::CompleteZoneManager`
        * `ace_time::ManualZoneManager`
* ZoneInfo Database
    * 3 sets of timezone data are provided (Basic, Extended, Complete) which
      support slightly different sets of zones and years
    * programmatically generated from the IANA TZ Database files
    * each timezone is identified in multiple ways
        * ZoneInfo: (opaque) pointer to a `ZoneInfo` data structure
        * ZoneId: unique and stable `uint32_t` identifier (e.g. `0xb7f7e8f2`)
        * ZoneName: unique human-readable string (e.g. "America/Los_Angeles")
    * Basic (not usually recommended)
        * 448 zones and links as of 2023c
        * ZoneInfo (`const ace_time::basic::ZoneInfo*`)
            * `ace_time::zonedb::kZoneAfrica_Abidjan`
            * ...
            * `ace_time::zonedb::kZonePacific_Wallis`
        * ZoneId (`uint32_t`)
            * `ace_time::zonedb::kZoneIdAfrica_Abidjan`
            * ...
            * `ace_time::zonedb::kZoneIdPacific_Wallis`
        * ZoneName (`const char*`)
            * `"Africa/Abidjan"`
            * ...
            * `"Pacific/Wallis"`
    * Extended (recommended for most cases)
        * 596 zones and links as of 2023c
        * ZoneInfo (`const ace_time::extended::ZoneInfo*`)
            * `ace_time::zonedbx::kZoneAfrica_Abidjan`
            * ...
            * `ace_time::zonedbx::kZonePacific_Wallis`
        * ZoneIds (`uint32_t`)
            * `ace_time::zonedbx::kZoneIdAfrica_Abidjan`
            * ...
            * `ace_time::zonedbx::kZoneIdPacific_Wallis`
        * ZoneName (`const char*`)
            * `"Africa/Abidjan"`
            * ...
            * `"Pacific/Wallis"`
    * Complete (useful when full range of years is necessary)
        * 596 zones and links as of 2023c
        * ZoneInfo (`const ace_time::complete::ZoneInfo*`)
            * `ace_time::zonedbc::kZoneAfrica_Abidjan`
            * ...
            * `ace_time::zonedbc::kZonePacific_Wallis`
        * ZoneId (`uint32_t`)
            * `ace_time::zonedbc::kZoneIdAfrica_Abidjan`
            * ...
            * `ace_time::zonedbc::kZoneIdPacific_Wallis`
        * ZoneName (`const char*`)
            * `"Africa/Abidjan"`
            * ...
            * `"Pacific/Wallis"`

<a name="Validation"></a>
## Validation

The details of how the Date, Time and TimeZone classes are validated are given
in [AceTimeValidation](https://github.com/bxparks/AceTimeValidation).

The ZoneInfo Database and the algorithms in this library were tested against
other date/time libraries written in different programming languages. The
`CompleteZoneProcessor` configured with the `zonedbc` database was used for most
of the validation testing, because this combination covers all zones and links
valid over a 400-year interval `[1800,2200)`. Some of these third party
libraries match AceTime exactly over the 400 year interval. Other libraries
contained bugs which prevented exact conformance with AceTime.

**Conformant**

These libraries match the AceTime library (using the
`CompleteZoneProcessor`/`CompleteZoneManager` with the `zonedbc` database)
exactly: every DST transition, DST offset, STD offset, the epochsecond of the
transition, the converted data-time components, and the abbreviation. The match
was verified for all time zones and links, from 1800 until 2200.

- [C++11/14/17 Hinnant date](https://github.com/HowardHinnant/date) library
- [GNU libc time](https://www.gnu.org/software/libc/libc.html) library
- [C# Noda Time](https://nodatime.org) library
- [acetimec](https://github.com/bxparks/acetimec) - C version of AceTime
- [acetimego](https://github.com/bxparks/acetimego) - Go or TinyGo version of
  AceTime
- [acetimepy](https://github.com/bxparks/acetimepy) - Python version of AceTime

**Non Conformant**

These third party libraries match AceTime for the most part, but seem to contain
bugs in some parts of their code. I have not had the time to look into the cause
of these bugs in these third party libraries.

- [Python pytz](https://pypi.org/project/pytz/) library from the year 2000
  until 2038
    * pytz cannot handle years after 2038
- [Python dateutil](https://pypi.org/project/python-dateutil/) library from
  the year 2000 until 2037
    * dateutil cannot handle years after 2038
- [Python 3.9 zoneinfo](https://docs.python.org/3/library/zoneinfo.html)
  library from the year 1800 until 2200
    * 31 zones produce incorrect DST offsets
- Java JDK 11
  [java.time](https://docs.oracle.com/en/java/javase/11/docs/api/java.base/java/time/package-summary.html)
  library from year 1800 until 2200
    * 3 IANA timezones are missing from `java.time`
    * ~100 zones seem to produce incorrect DST offsets
    * ~7 zones seem to produce incorrect epochSeconds
- [Go lang `time` package](https://pkg.go.dev/time) from 1800 to 2200
    * 23 zones produce incorrect results

<a name="ResourceConsumption"></a>
## Resource Consumption

<a name="SizeOfClasses"></a>
### SizeOf Classes

**8-bit processors**

```
Sizes of Objects:
sizeof(LocalDate): 4
sizeof(LocalTime): 4
sizeof(LocalDateTime): 8
sizeof(TimeOffset): 4
sizeof(OffsetDateTime): 12
sizeof(TimeZone): 5
sizeof(TimeZoneData): 5
sizeof(ZonedDateTime): 17
sizeof(ZonedExtra): 24
sizeof(TimePeriod): 4
Basic:
  sizeof(basic::ZoneContext): 20
  sizeof(basic::ZoneEra): 11
  sizeof(basic::ZoneInfo): 13
  sizeof(basic::ZoneRule): 9
  sizeof(basic::ZonePolicy): 3
  sizeof(basic::ZoneRegistrar): 5
  sizeof(BasicZoneProcessor): 143
  sizeof(BasicZoneProcessorCache<1>): 147
  sizeof(BasicZoneManager): 7
  sizeof(BasicZoneProcessor::Transition): 26
Extended:
  sizeof(extended::ZoneContext): 20
  sizeof(extended::ZoneEra): 11
  sizeof(extended::ZoneInfo): 13
  sizeof(extended::ZoneRule): 9
  sizeof(extended::ZonePolicy): 3
  sizeof(extended::ZoneRegistrar): 5
  sizeof(ExtendedZoneProcessor): 553
  sizeof(ExtendedZoneProcessorCache<1>): 557
  sizeof(ExtendedZoneManager): 7
  sizeof(ExtendedZoneProcessor::Transition): 49
  sizeof(ExtendedZoneProcessor::TransitionStorage): 412
  sizeof(ExtendedZoneProcessor::MatchingEra): 32
Complete:
  sizeof(complete::ZoneContext): 20
  sizeof(complete::ZoneEra): 15
  sizeof(complete::ZoneInfo): 13
  sizeof(complete::ZoneRule): 12
  sizeof(complete::ZonePolicy): 3
  sizeof(complete::ZoneRegistrar): 5
  sizeof(CompleteZoneProcessor): 553
  sizeof(CompleteZoneProcessorCache<1>): 557
  sizeof(CompleteZoneManager): 7
  sizeof(CompleteZoneProcessor::Transition): 49
  sizeof(CompleteZoneProcessor::TransitionStorage): 412
  sizeof(CompleteZoneProcessor::MatchingEra): 32
```

**32-bit processors**

```
Sizes of Objects:
sizeof(LocalDate): 4
sizeof(LocalTime): 4
sizeof(LocalDateTime): 8
sizeof(TimeOffset): 4
sizeof(OffsetDateTime): 12
sizeof(TimeZone): 12
sizeof(TimeZoneData): 8
sizeof(ZonedDateTime): 24
sizeof(ZonedExtra): 24
sizeof(TimePeriod): 4
Basic:
  sizeof(basic::ZoneContext): 28
  sizeof(basic::ZoneEra): 16
  sizeof(basic::ZoneInfo): 24
  sizeof(basic::ZoneRule): 9
  sizeof(basic::ZonePolicy): 8
  sizeof(basic::ZoneRegistrar): 8
  sizeof(BasicZoneProcessor): 208
  sizeof(BasicZoneProcessorCache<1>): 216
  sizeof(BasicZoneManager): 12
  sizeof(BasicZoneProcessor::Transition): 36
Extended:
  sizeof(extended::ZoneContext): 28
  sizeof(extended::ZoneEra): 16
  sizeof(extended::ZoneInfo): 24
  sizeof(extended::ZoneRule): 9
  sizeof(extended::ZonePolicy): 8
  sizeof(extended::ZoneRegistrar): 8
  sizeof(ExtendedZoneProcessor): 720
  sizeof(ExtendedZoneProcessorCache<1>): 728
  sizeof(ExtendedZoneManager): 12
  sizeof(ExtendedZoneProcessor::Transition): 60
  sizeof(ExtendedZoneProcessor::TransitionStorage): 516
  sizeof(ExtendedZoneProcessor::MatchingEra): 44
Complete:
  sizeof(complete::ZoneContext): 28
  sizeof(complete::ZoneEra): 20
  sizeof(complete::ZoneInfo): 24
  sizeof(complete::ZoneRule): 12
  sizeof(complete::ZonePolicy): 8
  sizeof(complete::ZoneRegistrar): 8
  sizeof(CompleteZoneProcessor): 720
  sizeof(CompleteZoneProcessorCache<1>): 728
  sizeof(CompleteZoneManager): 12
  sizeof(CompleteZoneProcessor::Transition): 60
  sizeof(CompleteZoneProcessor::TransitionStorage): 516
  sizeof(CompleteZoneProcessor::MatchingEra): 44
```

<a name="ZoneDbSize"></a>
### Zone DB Size

The ZoneInfo Database entries are stored in flash memory (using the `PROGMEM`
compiler directive) if the microcontroller allows it (e.g. AVR, ESP8266) so that
they do not consume static RAM. The
[examples/MemoryBenchmark](examples/MemoryBenchmark/) program shows the flash
memory consumption for the ZoneInfo data files are:

* `BasicZoneProcessor` (all zones and links)
    * 25 kB (8-bit processor)
    * 32 kB (32-bit processor)
* `ExtendedZoneProcessor` (all zones and links)
    * 40 kB (8-bit processor)
    * 51 kB (32-bit processor)
* `CompleteZoneProcessor` (all zones and links)
    * too large (8-bit processor)
    * 100 kB (32-bit processor)

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

Arduino Nano:

```
+----------------------------------------------------------------------+
| Functionality                          |  flash/  ram |        delta |
|----------------------------------------+--------------+--------------|
| baseline                               |    474/   11 |      0/    0 |
|----------------------------------------+--------------+--------------|
| LocalDateTime                          |   1108/   21 |    634/   10 |
| ZonedDateTime                          |   1444/   30 |    970/   19 |
| Manual ZoneManager                     |   1406/   13 |    932/    2 |
|----------------------------------------+--------------+--------------|
| Basic TimeZone (1 zone)                |   7624/  208 |   7150/  197 |
| Basic TimeZone (2 zones)               |   8170/  357 |   7696/  346 |
| BasicZoneManager (1 zone)              |   7834/  219 |   7360/  208 |
| BasicZoneManager (all zones)           |  19686/  599 |  19212/  588 |
| BasicZoneManager (all zones+links)     |  25066/  599 |  24592/  588 |
|----------------------------------------+--------------+--------------|
| Basic ZoneSorterByName [1]             |   8608/  221 |    774/    2 |
| Basic ZoneSorterByOffsetAndName [1]    |   8740/  221 |    906/    2 |
|----------------------------------------+--------------+--------------|
| Extended TimeZone (1 zone)             |  11326/  623 |  10852/  612 |
| Extended TimeZone (2 zones)            |  11924/ 1187 |  11450/ 1176 |
| ExtendedZoneManager (1 zone)           |  11506/  629 |  11032/  618 |
| ExtendedZoneManager (all zones)        |  34508/ 1111 |  34034/ 1100 |
| ExtendedZoneManager (all zones+links)  |  40540/ 1111 |  40066/ 1100 |
|----------------------------------------+--------------+--------------|
| Extended ZoneSorterByName [2]          |  12276/  631 |    770/    2 |
| Extended ZoneSorterByOffsetAndName [2] |  12360/  631 |    854/    2 |
|----------------------------------------+--------------+--------------|
| Complete TimeZone (1 zone)             |     -1/   -1 |     -1/   -1 |
| Complete TimeZone (2 zones)            |     -1/   -1 |     -1/   -1 |
| CompleteZoneManager (1 zone)           |     -1/   -1 |     -1/   -1 |
| CompleteZoneManager (all zones)        |     -1/   -1 |     -1/   -1 |
| CompleteZoneManager (all zones+links)  |     -1/   -1 |     -1/   -1 |
|----------------------------------------+--------------+--------------|
| Complete ZoneSorterByName [3]          |     -1/   -1 |      0/    0 |
| Complete ZoneSorterByOffsetAndName [3] |     -1/   -1 |      0/    0 |
+---------------------------------------------------------------------+
```

ESP8266:

```
+----------------------------------------------------------------------+
| Functionality                          |  flash/  ram |        delta |
|----------------------------------------+--------------+--------------|
| baseline                               | 260089/27892 |      0/    0 |
|----------------------------------------+--------------+--------------|
| LocalDateTime                          | 260613/27912 |    524/   20 |
| ZonedDateTime                          | 261573/27928 |   1484/   36 |
| Manual ZoneManager                     | 261553/27900 |   1464/    8 |
|----------------------------------------+--------------+--------------|
| Basic TimeZone (1 zone)                | 267117/28528 |   7028/  636 |
| Basic TimeZone (2 zones)               | 267533/28736 |   7444/  844 |
| BasicZoneManager (1 zone)              | 267277/28552 |   7188/  660 |
| BasicZoneManager (all zones)           | 283613/28552 |  23524/  660 |
| BasicZoneManager (all zones+links)     | 292141/28552 |  32052/  660 |
|----------------------------------------+--------------+--------------|
| Basic ZoneSorterByName [1]             | 268029/28552 |    752/    0 |
| Basic ZoneSorterByOffsetAndName [1]    | 268157/28552 |    880/    0 |
|----------------------------------------+--------------+--------------|
| Extended TimeZone (1 zone)             | 269653/29152 |   9564/ 1260 |
| Extended TimeZone (2 zones)            | 270069/29880 |   9980/ 1988 |
| ExtendedZoneManager (1 zone)           | 269813/29160 |   9724/ 1268 |
| ExtendedZoneManager (all zones)        | 301077/29160 |  40988/ 1268 |
| ExtendedZoneManager (all zones+links)  | 310645/29160 |  50556/ 1268 |
|----------------------------------------+--------------+--------------|
| Extended ZoneSorterByName [2]          | 270549/29160 |    736/    0 |
| Extended ZoneSorterByOffsetAndName [2] | 270613/29160 |    800/    0 |
|----------------------------------------+--------------+--------------|
| Complete TimeZone (1 zone)             | 270145/29508 |  10056/ 1616 |
| Complete TimeZone (2 zones)            | 271425/30236 |  11336/ 2344 |
| CompleteZoneManager (1 zone)           | 270289/29516 |  10200/ 1624 |
| CompleteZoneManager (all zones)        | 350449/29516 |  90360/ 1624 |
| CompleteZoneManager (all zones+links)  | 360017/29516 |  99928/ 1624 |
|----------------------------------------+--------------+--------------|
| Complete ZoneSorterByName [3]          | 271041/29516 |    752/    0 |
| Complete ZoneSorterByOffsetAndName [3] | 271089/29516 |    800/    0 |
+---------------------------------------------------------------------+
```

<a name="CPUUsage"></a>
### CPU Usage

[AutoBenchmark](examples/AutoBenchmark/) was used to determine the
CPU time consume by various features of the classes in this library. Two samples
are shown below:

Arduino Nano:

```
+--------------------------------------------------+----------+
| Method                                           |   micros |
|--------------------------------------------------+----------|
| EmptyLoop                                        |    5.000 |
|--------------------------------------------------+----------|
| LocalDate::forEpochDays()                        |  241.000 |
| LocalDate::toEpochDays()                         |   52.000 |
| LocalDate::dayOfWeek()                           |   49.000 |
|--------------------------------------------------+----------|
| OffsetDateTime::forEpochSeconds()                |  361.000 |
| OffsetDateTime::toEpochSeconds()                 |   78.000 |
|--------------------------------------------------+----------|
| ZonedDateTime::toEpochSeconds()                  |   75.000 |
| ZonedDateTime::toEpochDays()                     |   63.000 |
| ZonedDateTime::forEpochSeconds(UTC)              |  391.000 |
|--------------------------------------------------+----------|
| ZonedDateTime::forEpochSeconds(Basic_nocache)    | 1710.000 |
| ZonedDateTime::forEpochSeconds(Basic_cached)     |  707.000 |
| ZonedDateTime::forEpochSeconds(Extended_nocache) |   -1.000 |
| ZonedDateTime::forEpochSeconds(Extended_cached)  |   -1.000 |
| ZonedDateTime::forEpochSeconds(Complete_nocache) |   -1.000 |
| ZonedDateTime::forEpochSeconds(Complete_cached)  |   -1.000 |
|--------------------------------------------------+----------|
| ZonedDateTime::forComponents(Basic_nocache)      | 2252.000 |
| ZonedDateTime::forComponents(Basic_cached)       | 1254.000 |
| ZonedDateTime::forComponents(Extended_nocache)   |   -1.000 |
| ZonedDateTime::forComponents(Extended_cached)    |   -1.000 |
| ZonedDateTime::forComponents(Complete_nocache)   |   -1.000 |
| ZonedDateTime::forComponents(Complete_cached)    |   -1.000 |
|--------------------------------------------------+----------|
| ZonedExtra::forEpochSeconds(Basic_nocache)       | 1386.000 |
| ZonedExtra::forEpochSeconds(Basic_cached)        |  378.000 |
| ZonedExtra::forEpochSeconds(Extended_nocache)    |   -1.000 |
| ZonedExtra::forEpochSeconds(Extended_cached)     |   -1.000 |
| ZonedExtra::forEpochSeconds(Complete_nocache)    |   -1.000 |
| ZonedExtra::forEpochSeconds(Complete_cached)     |   -1.000 |
|--------------------------------------------------+----------|
| ZonedExtra::forComponents(Basic_nocache)         | 2276.000 |
| ZonedExtra::forComponents(Basic_cached)          | 1277.000 |
| ZonedExtra::forComponents(Extended_nocache)      |   -1.000 |
| ZonedExtra::forComponents(Extended_cached)       |   -1.000 |
| ZonedExtra::forComponents(Complete_nocache)      |   -1.000 |
| ZonedExtra::forComponents(Complete_cached)       |   -1.000 |
|--------------------------------------------------+----------|
| BasicZoneRegistrar::findIndexForName(binary)     |  118.000 |
| BasicZoneRegistrar::findIndexForIdBinary()       |   47.000 |
| BasicZoneRegistrar::findIndexForIdLinear()       |  299.000 |
|--------------------------------------------------+----------|
| ExtendedZoneRegistrar::findIndexForName(binary)  |   -1.000 |
| ExtendedZoneRegistrar::findIndexForIdBinary()    |   -1.000 |
| ExtendedZoneRegistrar::findIndexForIdLinear()    |   -1.000 |
|--------------------------------------------------+----------|
| CompleteZoneRegistrar::findIndexForName(binary)  |   -1.000 |
| CompleteZoneRegistrar::findIndexForIdBinary()    |   -1.000 |
| CompleteZoneRegistrar::findIndexForIdLinear()    |   -1.000 |
+--------------------------------------------------+----------+
Iterations_per_run: 1000
```

ESP8266:

```
+--------------------------------------------------+----------+
| Method                                           |   micros |
|--------------------------------------------------+----------|
| EmptyLoop                                        |    4.500 |
|--------------------------------------------------+----------|
| LocalDate::forEpochDays()                        |    7.500 |
| LocalDate::toEpochDays()                         |    4.000 |
| LocalDate::dayOfWeek()                           |    3.500 |
|--------------------------------------------------+----------|
| OffsetDateTime::forEpochSeconds()                |   12.000 |
| OffsetDateTime::toEpochSeconds()                 |    7.000 |
|--------------------------------------------------+----------|
| ZonedDateTime::toEpochSeconds()                  |    6.500 |
| ZonedDateTime::toEpochDays()                     |    5.500 |
| ZonedDateTime::forEpochSeconds(UTC)              |   13.500 |
|--------------------------------------------------+----------|
| ZonedDateTime::forEpochSeconds(Basic_nocache)    |  141.500 |
| ZonedDateTime::forEpochSeconds(Basic_cached)     |   21.500 |
| ZonedDateTime::forEpochSeconds(Extended_nocache) |  354.500 |
| ZonedDateTime::forEpochSeconds(Extended_cached)  |   28.000 |
| ZonedDateTime::forEpochSeconds(Complete_nocache) |  407.000 |
| ZonedDateTime::forEpochSeconds(Complete_cached)  |   28.000 |
|--------------------------------------------------+----------|
| ZonedDateTime::forComponents(Basic_nocache)      |  159.000 |
| ZonedDateTime::forComponents(Basic_cached)       |   46.000 |
| ZonedDateTime::forComponents(Extended_nocache)   |  241.500 |
| ZonedDateTime::forComponents(Extended_cached)    |    2.500 |
| ZonedDateTime::forComponents(Complete_nocache)   |  354.000 |
| ZonedDateTime::forComponents(Complete_cached)    |    2.500 |
|--------------------------------------------------+----------|
| ZonedExtra::forEpochSeconds(Basic_nocache)       |  134.500 |
| ZonedExtra::forEpochSeconds(Basic_cached)        |   11.000 |
| ZonedExtra::forEpochSeconds(Extended_nocache)    |  308.000 |
| ZonedExtra::forEpochSeconds(Extended_cached)     |   18.000 |
| ZonedExtra::forEpochSeconds(Complete_nocache)    |  396.000 |
| ZonedExtra::forEpochSeconds(Complete_cached)     |   17.500 |
|--------------------------------------------------+----------|
| ZonedExtra::forComponents(Basic_nocache)         |  184.500 |
| ZonedExtra::forComponents(Basic_cached)          |   48.500 |
| ZonedExtra::forComponents(Extended_nocache)      |  268.000 |
| ZonedExtra::forComponents(Extended_cached)       |   29.000 |
| ZonedExtra::forComponents(Complete_nocache)      |  332.500 |
| ZonedExtra::forComponents(Complete_cached)       |    5.000 |
|--------------------------------------------------+----------|
| BasicZoneRegistrar::findIndexForName(binary)     |   18.000 |
| BasicZoneRegistrar::findIndexForIdBinary()       |    6.500 |
| BasicZoneRegistrar::findIndexForIdLinear()       |   43.000 |
|--------------------------------------------------+----------|
| ExtendedZoneRegistrar::findIndexForName(binary)  |   24.500 |
| ExtendedZoneRegistrar::findIndexForIdBinary()    |    6.000 |
| ExtendedZoneRegistrar::findIndexForIdLinear()    |   50.500 |
|--------------------------------------------------+----------|
| CompleteZoneRegistrar::findIndexForName(binary)  |   18.500 |
| CompleteZoneRegistrar::findIndexForIdBinary()    |    6.500 |
| CompleteZoneRegistrar::findIndexForIdLinear()    |   43.000 |
+--------------------------------------------------+----------+
Iterations_per_run: 2000
```

<a name="SystemRequirements"></a>
## System Requirements

<a name="Hardware"></a>
### Hardware

**Tier 1: Fully supported**

These boards are tested on each release:

* Arduino Nano (16 MHz ATmega328P)
* SparkFun Pro Micro (16 MHz ATmega32U4)
* Seeed Studio XIAO M0 (SAMD21, 48 MHz ARM Cortex-M0+)
* STM32 Blue Pill (STM32F103C8, 72 MHz ARM Cortex-M3)
* Adafruit ItsyBitsy M4 (SAMD51, 120 MHz ARM Cortex-M4)
* NodeMCU 1.0 (ESP-12E module, 80 MHz ESP8266)
* WeMos D1 Mini (ESP-12E module, 80 MHz ESP8266)
* ESP32 dev board (ESP-WROOM-32 module, 240 MHz dual core Tensilica LX6)

**Tier 2: Should work**

These boards should work but I don't test them as often:

* ATtiny85 (8 MHz ATtiny85)
* Arduino Pro Mini (16 MHz ATmega328P)
* Mini Mega 2560 (Arduino Mega 2560 compatible, 16 MHz ATmega2560)
* Teensy LC (48 MHz ARM Cortex-M0+)
* Teensy 3.2 (96 MHz ARM Cortex-M4)

**Tier 3: May work, but not supported**

* Other SAMD21 based boards, e.g Arduino Zero
    * SAMD21 based boards are now split into 2 groups:
        * Those using the new ArduinoCore-API, usually Arduino-branded
        boards. These are explicitly blacklisted. See below.
        * Other 3rd party SAMD21 boards using the previous Arduino API.
    * The ones using the previous Arduino API *may* work but I have not
      explicitly tested any of them except for the Seeed Studio XIAO M0
      and Adafruit ItsyBitsy M4.

**Tier Blacklisted**

The following boards are *not* supported and are explicitly blacklisted to allow
the compiler to print useful error messages instead of hundreds of lines of
compiler errors:

* Any platform using the
  [ArduinoCore-API](https://github.com/arduino/ArduinoCore-api). For example:
    * Arduino Nano Every
    * Arduino Nano 33 IoT
    * Arduino MKRZero
    * Arduino UNO R4
    * Raspberry Pi Pico RP2040

<a name="ToolChain"></a>
### Tool Chain

This library was developed and tested using:

* [Arduino IDE 1.8.19](https://www.arduino.cc/en/Main/Software)
* [Arduino CLI 0.33.0](https://arduino.github.io/arduino-cli)
* [SpenceKonde ATTinyCore 1.5.2](https://github.com/SpenceKonde/ATTinyCore)
* [Arduino AVR Boards 1.8.6](https://github.com/arduino/ArduinoCore-avr)
* [Arduino SAMD Boards 1.8.9](https://github.com/arduino/ArduinoCore-samd)
* [SparkFun AVR Boards 1.1.13](https://github.com/sparkfun/Arduino_Boards)
* [SparkFun SAMD Boards 1.8.9](https://github.com/sparkfun/Arduino_Boards)
* [Seeeduino SAMD Boards 1.8.4](https://wiki.seeedstudio.com/Seeed_Arduino_Boards/)
* [STM32duino 2.5.0](https://github.com/stm32duino/Arduino_Core_STM32)
* [ESP8266 Arduino 3.0.2](https://github.com/esp8266/Arduino)
* [ESP32 Arduino 2.0.9](https://github.com/espressif/arduino-esp32)
* [Teensyduino 1.57](https://www.pjrc.com/teensy/td_download.html)

This library is *not* compatible with:

* Any platform using the
  [ArduinoCore-API](https://github.com/arduino/ArduinoCore-api), for example:
    * [Arduino megaAVR](https://github.com/arduino/ArduinoCore-megaavr/)
        * Nano Every
    * [Arduino SAMD Boards >=1.8.10](https://github.com/arduino/ArduinoCore-samd)
        * MKRZero
        * Nano 33 IoT
    * [ArduinoCore-renesas](https://github.com/arduino/ArduinoCore-renesas)
        * Arduino UNO R4
    * [Arduino-Pico](https://github.com/earlephilhower/arduino-pico)
        * Raspberry Pi Pico (RP2040)
    * [MegaCoreX](https://github.com/MCUdude/MegaCoreX)

It should work with [PlatformIO](https://platformio.org/) but I have
not tested it.

The library works on Linux or MacOS (using both g++ and clang++ compilers) using
the EpoxyDuino (https://github.com/bxparks/EpoxyDuino) emulation layer.

<a name="OperatingSystem"></a>
### Operating System

I use Ubuntu 22.04 for the vast majority of my development. I expect that the
library will work fine under MacOS and Windows, but I have not tested them.

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
accurately expressed. As far as I know, POSIX timezone strings can support only
2 DST transitions per year. However, there are a handful of timezones which have
(or used to have) 4 timezone transitions in a single year, which cannot be
represented by a POSIX string. Most Arduino timezone libraries use the POSIX
format (e.g. [ropg/ezTime](https://github.com/ropg/ezTime) or
[JChristensen/Timezone](https://github.com/JChristensen/Timezone)) for
simplicity and smaller memory footprint.

The libraries that incorporate the full IANA TZ Database are often far too large
to fit inside the resource constrained Arduino environments. The AceTime library
has been optimized to reduce the flash memory size of the library as much as
possible. The application can choose to load only of the smallest subset of the
TZ Database that is required to support the selected timezones (1 to 4 have been
extensively tested). Dynamic lookup of the time zone is possible using the
`ZoneManager`, and the app develop can customize it with the list of zones that
are compiled into the app. On microcontrollers with more than about 32kB of
flash memory (e.g. ESP8266, ESP32, Teensy 3.2) and depending on the size of the
rest of the application, it may be possible to load the entire IANA TZ database.
This will allow the end-user to select the timezone dynamically, just like
desktop-class machines.

When new versions of the database are released (several times a year), I can
regenerate the zone files, recompile the application, and it will instantly use
the new transition rules, without the developer needing to create a new POSIX
string.

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
[ComparisonBenchmark/README.md](examples/ComparisonBenchmark/README.md) file.
Two examples for the Arduino Nano and ESP8266 are shown below:

**Nano**

```
+----------------------------------------+----------+
| Method                                 |   micros |
|----------------------------------------+----------|
| EmptyLoop                              |    5.000 |
|----------------------------------------+----------|
| LocalDateTime::forEpochSeconds()       |  270.000 |
| breakTime()                            |  594.500 |
|----------------------------------------+----------|
| LocalDateTime::toEpochSeconds()        |   66.500 |
| makeTime()                             |  344.500 |
+----------------------------------------+----------+
```

**ESP8266**

```
+----------------------------------------+----------+
| Method                                 |   micros |
|----------------------------------------+----------|
| EmptyLoop                              |    0.800 |
|----------------------------------------+----------|
| LocalDateTime::forEpochSeconds()       |   13.100 |
| breakTime()                            |   42.600 |
|----------------------------------------+----------|
| LocalDateTime::toEpochSeconds()        |    4.500 |
| makeTime()                             |   24.800 |
+----------------------------------------+----------+
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
    * The `time()` function automatically increments through the
      `system_get_time()` system call.
    * Provides an SNTP client that can synchronize with an NTP service
      and resynchronize the `time()` function.
    * Adds `configTime()` functions to configure the behavior of the
      SNTP service, including POSIX timezones.
    * ESP8266 `TZ.h` containing pre-calculated POSIX timezone strings.

These libraries are all based upon the [traditional C/Unix library
methods](http://www.catb.org/esr/time-programming/) which can be difficult to
understand.

<a name="Esp8266AndEspTimeZones"></a>
### ESP8266 and ESP32 TimeZones

The ESP8266 platform provides a
[cores/esp8266/TZ.h](https://github.com/esp8266/Arduino/blob/master/cores/esp8266/TZ.h)
file which contains a list of pre-generated POSIX timezone strings. These can be
passed into the `configTime()` function that initializes the SNTP service.

The ESP32 platform does not provide a `TZ.h` file as far as I can tell. I
believe the same POSIX strings can be passed into its `configTime()` function.
But POSIX timezone strings have limitations that I described in
[Motivation](#Motivation), and the C-style library functions are often
confusing and hard to use.

Application developers can choose to use the built-in SNTP service and the
`time()` function on the ESP8266 and ESP32 as the source of accurate clock, but
take advantage of the versatility and power of the AceTime library for timezone
conversions. AceTime v1.10 adds the `forUnixSeconds64()` and
`toUnixSeconds64()` methods in various classes to make it far easier to interact
with the 64-bit `time_t` integers returned by the `time()` function on these
platforms.

See [EspTime](examples/EspTime) for an example of how to integrate AceTime with
the built-in SNTP service and `time()` function on the ESP8266 and ESP32. See
also the
[EspSntpClock](https://github.com/bxparks/AceTimeClock/blob/develop/src/ace_time/clock/EspSntpClock.h)
class in the AceTimeClock project which provides a thin-wrapper around this
service on the ESP platforms.

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
[HinnantBasicTest](https://github.com/bxparks/AceTimeValidation/tree/master/tests/HinnantBasicTest)
and
[HinnantExtendedTest](https://github.com/bxparks/AceTimeValidation/tree/master/tests/HinnantExtendedTest)
validation tests which compare the AceTime algorithms to the Hinnant Date
algorithms. For all times zones between the years 2000 until 2100, the AceTime
date-time components (`ZonedDateTime`) and abbreviations (`ZonedExtra`)
calculated from the given epochSeconds match the results from the Hinnant Date
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

If you have any questions, comments, or feature requests for this library,
please use the [GitHub
Discussions](https://github.com/bxparks/AceTime/discussions) for this project.
If you have bug reports, please file a ticket in [GitHub
Issues](https://github.com/bxparks/AceTime/issues). Feature requests should go
into Discussions first because they often have alternative solutions which are
useful to remain visible, instead of disappearing from the default view of the
Issue tracker after the ticket is closed.

Please refrain from emailing me directly unless the content is sensitive. The
problem with email is that I cannot reference the email conversation when other
people ask similar questions later.

<a name="Authors"></a>
## Authors

* Created by Brian T. Park (brian@xparks.net).
