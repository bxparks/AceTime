# AceTime User Guide

The primary purpose of AceTime classes is to convert between an integer
representing the number of seconds since the AceTime Epoch (2050-01-01T00:00:00
UTC) and the equivalent human-readable components in different timezones.
The epoch year is adjustable using the `Epoch::currentEpochYear(year)`. This
sets the epoch to be `{year}-01-01T00:00:00 UTC`.

The epoch seconds is represented by an `int32_t` integer (instead of an
`int64_t` used in most modern timezone libraries) to save resources on 8-bit
processors without native 64-bit integer instructions. The range of a 32-bit
integer is about 136 years which allows most features of the AceTime library to
work across at least about a 120-year interval straddling the current epoch
year.

The IANA TZ database is programmatically generated into 3 predefined databases:
`src/zonedb`, `src/zonedbx`, and `src/zonedbc` subdirectories. Different
databases have different accuracy ranges, and are designed to work with
different `ZoneProcessor` and `ZoneManager` classes.

**Version**: 2.3.1 (2024-01-12, TZDB 2023d)

**Related Documents**:

* [README.md](README.md): introductory background
* [Doxygen docs](https://bxparks.github.io/AceTime/html) hosted on GitHub

## Table of Contents

* [Overview](#Overview)
    * [Date And Time Overview](#DateAndTimeOverview)
    * [TimeZone Overview](#TimeZoneOverview)
    * [ZoneInfo Database Overview](#ZoneInfoDatabaseOverview)
* [Headers and Namespaces](#Headers)
* [Date and Time Classes](#DateTimeClasses)
    * [Epoch Seconds Typedef](#EpochSeconds)
    * [Adjustable Epoch](#AdjustableEpoch)
    * [LocalDate and LocalTime](#LocalDateAndLocalTime)
    * [Date Strings](#DateStrings)
    * [LocalDateTime](#LocalDateTime)
    * [TimePeriod](#TimePeriod)
    * [TimeOffset](#TimeOffset)
    * [OffsetDateTime](#OffsetDateTime)
* [TimeZone Classes](#TimeZoneClasses)
    * [TimeZone](#TimeZone)
        * [Manual TimeZone](#ManualTimeZone)
        * [Basic TimeZone](#BasicTimeZone)
        * [Extended TimeZone](#ExtendedTimeZone)
        * [TimeZone Type Recommendations](#TimeZoneTypeRecommendations)
    * [ZonedDateTime](#ZonedDateTime)
        * [Class Declaration](#ZonedDateTimeDeclaration)
        * [Creation](#ZonedDateTimeCreation)
        * [Conversion to Other Time Zones](#TimeZoneConversion)
        * [DST Transition Caching](#DstTransitionCaching)
    * [ZonedExtra](#ZonedExtra)
    * [ZoneManager](#ZoneManager)
        * [Class Hierarchy](#ClassHierarchy)
        * [Default Registries](#DefaultRegistries)
        * [ZoneProcessorCache](#ZoneProcessorCache)
        * [ZoneManager Creation](#ZoneManagerCreation)
        * [createForZoneName()](#CreateForZoneName)
        * [createForZoneId()](#CreateForZoneId)
        * [createForZoneIndex()](#CreateForZoneIndex)
        * [createForTimeZoneData()](#CreateForTimeZoneData)
        * [ManualZoneManager](#ManualZoneManager)
    * [Handling Gaps and Overlaps](#HandlingGapsAndOverlaps)
        * [Problems with Gaps and Overlaps](#ProblemsWithGapsAndOverlaps)
        * [Classes with Fold](#ClassesWithFold)
        * [Factory Methods with Fold](#FactoryMethodsWithFold)
        * [Resource Consumption with Fold](#ResourceConsumptionWithFold)
        * [Semantic Changes with Fold](#SemanticChangesWithFold)
        * [Examples with Fold](#ExamplesWithFold)
* [ZoneInfo Database](#ZoneInfoDatabase)
    * [ZoneInfo Records](#ZoneInfoRecords)
    * [ZoneDB](#ZoneDB)
        * [Basic zonedb](#BasicZonedb)
        * [Extended zonedbx](#ExtendedZonedbx)
        * [Complete zonedbc](#CompleteZonedbc)
        * [External Zone Classes](#ExternalZone)
        * [TZ Database Version](#TzDatabaseVersion)
        * [Zone Info Year Range](#ZoneInfoYearRange)
    * [Zones and Links](#ZonesAndLinks)
    * [Custom Zone Registry](#CustomZoneRegistry)
* [Zone Sorting](#ZoneSorting)
* [Print To String](#PrintToString)
* [Mutations](#Mutations)
    * [TimeOffset Mutations](#TimeOffsetMutations)
    * [LocalDate Mutations](#LocalDateMutations)
    * [OffsetDateTime Mutations](#OffsetDateTimeMutations)
    * [ZonedDateTime Mutations](#ZonedDateTimeMutations)
    * [ZonedDateTime Normalization](#ZonedDateTimeNormalization)
    * [TimePeriod Mutations](#TimePeriodMutations)
* [Error Handling](#ErrorHandling)
    * [Invalid Sentinels](#InvalidSentinels)
    * [isError()](#IsError)
* [Bugs and Limitations](#Bugs)

<a name="Overview"></a>
## Overview

The Date, Time, and TimeZone classes provide an abstraction layer to make it
easier to use and manipulate date and time fields, in different time zones. It
is difficult to organize the various parts of this library in an easily
digestible way, but perhaps they can be categorized into three parts:

* Simple Date and Time classes for converting date and time fields to and
  from the "epoch seconds", for example:
    - `acetime_t`
    - `LocalDate`, `LocalTime`, `LocalDateTime`, `OffsetDateTime`
    - `TimeOffset`, `TimePeriod`
* TimeZone related classes, for example:
    - `TimeZone`, `ZoneInfo`
    - `ZonedDateTime`, `ZonedExtra`
    - `BasicZoneProcessor`, `ExtendedZoneProcessor`, `CompleteZoneProcessor`
    - `BasicZoneManager`, `ExtendedZoneManager`, `CompleteZoneManager`
* ZoneInfo Databases generated from the IANA TZ Database
    * contains UTC offsets and the DST transition rules
    * `zonedb/`, `zonedbx/`, `zonedbc/` databases
    * registries: `kZoneRegistry`, `kZoneAndLinkRegistry`

<a name="DateAndTimeOverview"></a>
### Date and Time Overview

First we start with `LocalDate` and `LocalTime` classes which capture the simple
date and time fields respectively. They combine together to form the
`LocalDateTime` class which contains all date and time fields.

The `TimeOffset` class represents a simple shift in time, for example, +1h or
-4:30 hours. It can be used to represent a UTC offset, or a DST offset. The
`TimeOffset` class combines with the `LocalDateTime` class to form the
`OffsetDateTime` classes which represents a date and time that has been shifted
from UTC some offset.

Both the `LocalDateTime` and `OffsetDateTime` (and later `ZonedDateTime`)
classes provide the `toEpochSeconds()` method which returns the number of
seconds from an epoch date, the `forEpochSeconds()` method which constructs the
,ate and time fields from the epoch seconds. They also provide the
`forComponents()` method which constructs the object from the individual (year,
month, day, hour, minute, second) components.

The Epoch in AceTime defaults to 2050-01-01T00:00:00 UTC, in contrast to the
Epoch in Unix which is 1970-01-01T00:00:00 UTC. Internally, the current time is
represented as "seconds from Epoch" stored as a 32-bit signed integer
(`acetime_t` aliased to `int32_t`). The smallest 32-bit signed integer (`-2^31`)
is used to indicate an internal Error condition, so the range of valid
`acetime_t` value is `-2^31+1` to `2^31-1`. Therefore, the range of dates that
the `acetime_t` type can handle is about 132 years, and the largest date is
2118-01-20T03:14:07 UTC. (In contrast, the 32-bit Unix `time_t` range is
1901-12-13T20:45:52 UTC to 2038-01-19T03:14:07 UTC which is the cause of the
[Year 2038 Problem](https://en.wikipedia.org/wiki/Year_2038_problem)).

The various date classes (`LocalDate`, `LocalDateTime`, `OffsetDateTime`) store
the year component internally as a signed 16-bit integer valid from year 1 to
year 9999. Notice that these classes can represent all dates that can be
expressed by the `acetime_t` type, but the reverse is not true. There are date
objects that cannot be converted into a valid `acetime_t` value.

Most timezone related functions of the library use the `int32_t` epochseconds
for its internal calculations, so the date range should be constrained to +/- 68
years of the current epoch. The timezone calculations require some additional
buffers at the edges of the range (1-3 years), so the actual range of validity
is about +/- 65 years. To be conservative, client applications are advised to
limit the date range to about 100-120 years, in other words, about +/- 50 to 60
years from the current epoch year. Using the default epoch year of 2050, the
recommended range is `[2000,2100)` because a 100-year interval is easy to
remember.

<a name="TimeZoneOverview"></a>
### TimeZone Overview

The `TimeZone` class a real or abstract place or region whose local time is
shifted from UTC by some amount. It is combined with the `OffsetDateTime` class
to form the `ZonedDateTime` class. The `ZonedDateTime` allows conversions to
other timezones using the `ZonedDateTime::convertToTimeZone()` method.

The `TimeZone` object can be defined using the data and rules defined by the
[IANA TZ Database](https://www.iana.org/time-zones). AceTime provides 2
different algorithms to process this database:

* `BasicZoneProcessor`
    * simpler and smaller, but supports only about 70% of the timezones defined
      by the IANA TZ Database
* `ExtendedZoneProcessor`
    * bigger and more complex and handles the entire TZ database
* `CompleteZoneProcessor`
    * same as `ExtendedZoneProcessor` but handles the timezones defined by the
      `acetime::zonedbc` database, over a much larger year interval
      `[0001,10000)`.

Access to the 3 sets data in the ZoneInfo Database is provided by:

* `BasicZoneManager`:
    * holds a registry of the basic ZoneInfo data structures
    * holds a cache of `BasicZoneProcessor`
* `ExtendedZoneManager`:
    * holds a registry of the extended ZoneInfo data structures
    * holds a cache of `ExtendedZoneProcessor`
* `CompleteZoneManager`:
    * holds a registry of the extended ZoneInfo data structures
    * holds a cache of `CompleteZoneProcessor`

<a name="ZoneInfoDatabaseOverview"></a>
### ZoneInfo Database Overview

The official IANA TZ Database is processed and converted into an internal
AceTime database that we will call the ZoneInfo Database (to distinguish it from
the IANA TZ Database). The ZoneInfo Database contains statically defined C++
data structures, which each timezone in the TZ Database being represented by a
`ZoneInfo` data structure.

Three slightly different sets of ZoneInfo entries are generated:

* [zonedb/zone_infos.h](src/zonedb/zone_infos.h)
    * intended for `BasicZoneProcessor` or `BasicZoneManager`
    * 448 zones and links (as of version 2023c) over the year `[2000,10000)`
    * contains `kZone*` declarations (e.g. `kZoneAmerica_Los_Angeles`)
    * contains `kZoneId*` identifiers (e.g. `kZoneIdAmerica_Los_Angeles`)
    * slightly smaller and slightly faster, but does not supported detection of
      overlaps and gaps perfectly
* [zonedbx/zone_infos.h](src/zonedbx/zone_infos.h)
    * intended for `ExtendedZoneProcessor` or `ExtendedZoneManager`
    * all 596 (as of version 2023c) in the IANA TZ Database over the years
      `[2000,10000)`
    * contains `kZone*` declarations (e.g. `kZoneAfrica_Casablanca`)
    * contains `kZoneId*` identifiers (e.g. `kZoneIdAfrica_Casablanca`)
* [zonedbc/zone_infos.h](src/zonedbc/zone_infos.h)
    * intended for `CompleteZoneProcessor` or `CompleteZoneManager`
    * all 596 (as of version 2023c) in the IANA TZ Database over the years of
      `[0001,10000)`
    * contains `kZone*` declarations (e.g. `kZoneAfrica_Casablanca`)
    * contains `kZoneId*` identifiers (e.g. `kZoneIdAfrica_Casablanca`)

The internal helper classes which are used to encode the ZoneInfo Database
information are defined in the following namespaces. They are not expected to be
used by application developers under normal circumstances, so these are listed
here for reference:

* `ace_time::basic::ZoneXxx`
* `ace_time::extended::ZoneXxx`
* `ace_time::complete::ZoneXxx`

The `basic::ZoneInfo` and `extended::ZoneInfo` classes (and their associated
`ZoneProcessor` classes) have a resolution of 1 minute, which is sufficient to
represent all UTC offsets and DST shifts of all timezones after 1972
(Africa/Monrovia seems like the last timezone to conform to a one-minute
resolution on Jan 7, 1972). The `complete::ZoneInfo` classes have a resolution
of 1 second, which allows them to represent all timezones, for all years in the
TZ database, over the years `[0001,10000)`.

It is expected that most applications using AceTime will use only a small number
of timezones at the same time (1 to 4 zones have been extensively tested) and
that this set is known at compile-time. The C++ compiler will include only the
subset of ZoneInfo entries needed to support those timezones, instead of
compiling in the entire ZoneInfo Database. But on microcontrollers with enough
memory, the `ZoneManager` can be used to load the entire ZoneInfo Database into
the app and the `TimeZone` objects can be dynamically created as needed.

Each timezone in the ZoneInfo Database is identified by its fully qualified zone
name (e.g. `"America/Los_Angeles"`). On small microcontroller environments,
these strings can consume precious memory (e.g. 30 bytes for
`"America/Argentina/Buenos_Aires"`) and are not convenient to serialize over the
network or to save to EEPROM.

The AceTime library provides each timezone with an alternative `zoneId`
identifier of type `uint32_t` which is guaranteed to be unique and stable. For
example, the zoneId for `"America/Los_Angeles"` is provided by
`zonedb::kZoneIdAmerica_Los_Angeles` or `zonedbx::kZoneIdAmerica_Los_Angeles`
which both have the value `0xb7f7e8f2`. A `TimeZone` object can be saved as a
`zoneId` and then recreated using the `BasicZoneManager::createForZoneId()`
or `ExtendedZoneManager::createForZoneId()` method.

<a name="Headers"></a>
### Headers and Namespaces

Only a single header file `AceTime.h` is required to use this library.
To use the AceTime classes without prepending the namespace prefixes, use
the following `using` directive:

```C++
#include <AceTime.h>
using namespace ace_time;
```

To use the appropriate ZoneInfo data structures needed by the ZoneProcessor and
ZoneManager you will need *one* of the following:

```C++
using namespace ace_time::zonedb;
using namespace ace_time::zonedbx;
using namespace ace_time::zonedbc;
```

The following C++ namespaces are usually internal implementation details
which are not normally needed by the end users:

* `ace_time::basic`: for creating custom zone registries for `BasicZoneManager`
* `ace_time::extended`: for creating custom zone registries for
  `ExtendedZoneManager`
* `ace_time::complete`: for creating custom zone registries for
  `CompleteZoneManager`

<a name="DateTimeClasses"></a>
## Date and Time Classes

<a name="EpochSeconds"></a>
### Epoch Seconds Typedef

One of the fundamental types in AceTime is the `acetime_t` defined as:

```C++
namespace ace_time {

typedef int32_t acetime_t;

}
```

This represents the number of seconds since the Epoch. In AceTime, the Epoch is
defined by default to be 2050-01-01 00:00:00 UTC time (and can be changed using
the `Epoch::currentEpochYear()` function). In contrast, the Unix Epoch is
defined to be 1970-01-01 00:00:00 UTC. Since `acetime_t` is a 32-bit signed
integer, the largest value is 2,147,483,647. With the default epoch year of
2050, the largest date that can be represented as an epoch seconds is
2118-01-20T03:14:07 UTC.

The `acetime_t` is analogous to the `time_t` type in the standard C library,
with several major differences:

* The `time_t` does not exist on all Arduino platforms.
* Some Arduino platforms and older Unix platforms use a 32-bit `int32_t` to
  represent `time_t`.
* Modern implementations (e.g. ESP8266 and ESP32) use a 64-bit `int64_t` to
  represent `time_t` to prevent the "Year 2038" overflow problem. Unfortunately,
  AceTime does use 64-bit integers internally to avoid consuming flash memory
  on 8-bit processors.
* Most `time_t` implementations uses the Unix Epoch of 1970-01-01 00:00:00 UTC.
  AceTime uses an epoch of 2050-01-01 00:00:00 UTC (by default).

It is possible to convert between a `time_t` and an `acetime_t` by adding or
subtracting the number of seconds between the 2 Epoch dates. This value is given
by `Epoch::secondsToCurrentEpochFromUnixEpoch64()` which returns an `int64_t`
value to allow epoch years greater than 2038. If the date is within +/- 60 years
of the current epoch year, then the resulting epoch seconds will fit inside a
`int32_t` integer. Helper methods are available on various classes to avoid
manual conversion between these 2 epochs: `forUnixSeconds64()` and
`toUnixSeconds64()`.

<a name="AdjustableEpoch"></a>
### Adjustable Epoch

Starting with v2, the AceTime epoch is an **adjustable** parameter which is no
longer hard coded to 2000-01-01 (v1 default) or 2050-01-01 (v2 default). There
are a number of static functions on the `Epoch` class that support this feature:

```C++
namespace ace_time {

class Epoch {
  public:
    // Get the current epoch year.
    static int16_t currentEpochYear();

    // Set the current epoch year.
    static int16_t currentEpochYear(int16_t epochYear);

    // The number of days from the Unix epoch (1970-01-01T00:00:00)
    // to the current epoch ({yyyy}-01-01T00:00:00).
    static int32_t daysToCurrentEpochFromUnixEpoch();

    // The number of seconds from the Unix epoch (1970-01-01T00:00:00)
    // to the current epoch ({yyyy}-01-01T00:00:00).
    static int64_t secondsToCurrentEpochFromUnixEpoch64();

    // Return the lower limit year which generates valid epoch seconds for the
    // current epoch.
    static int16_t epochValidYearLower();

    // Return the upper limit year which generates valid epoch seconds for the
    // current epoch.
    static int16_t epochValidYearUpper();
};

}
```

Normally, the current epoch year is expected to be unchanged using the default
2050, or changed just once at the initialization phase of the application.
However in some situations, the client app may call `Epoch::currentEpochYear()`
during its runtime to extend the range of the years of interest. The
`BasicZoneProcessor`, `ExtendedZoneProcessor` and `CompleteZoneProcessor`
objects will automatically invalidate and regenerate its internal transition
cache when the epoch year is modified.

<a name="LocalDateAndLocalTime"></a>
### LocalDate and LocalTime

The `LocalDate` and `LocalTime` represent date and time components, without
reference to a particular time zone. They are not expected to be commonly used
by the end-users, but they are available if needed. The significant parts of the
class definitions are:

```C++
namespace ace_time {

class LocalTime {
  public:
    static const acetime_t kInvalidSeconds = INT32_MIN;

    static LocalTime forComponents(uint8_t hour, uint8_t minute,
        uint8_t second);

    static LocalTime forSeconds(acetime_t seconds);

    bool isError() const;

    uint8_t hour() const;
    void hour(uint8_t hour);

    uint8_t minute() const;
    void minute(uint8_t month);

    uint8_t second() const;
    void second(uint8_t second);

    acetime_t toSeconds() const;

    int8_t compareTo(const LocalTime& that) const;
    void printTo(Print& printer) const;
    ...
};

class LocalDate {
  public:
    static const int16_t kInvalidYear = INT16_MIN;
    static const int16_t kMinYear = 0;
    static const int16_t kMaxYear = 10000;

    static const int32_t kInvalidEpochDays = INT32_MIN;

    static const int32_t kInvalidEpochSeconds = INT32_MIN;
    static const int64_t kInvalidUnixSeconds64 = INT64_MIN;
    static const int32_t kMinEpochSeconds = INT32_MIN + 1;
    static const int32_t kMaxEpochSeconds = INT32_MAX;

    static const uint8_t kMonday = 1;
    static const uint8_t kTuesday = 2;
    static const uint8_t kWednesday = 3;
    static const uint8_t kThursday = 4;
    static const uint8_t kFriday = 5;
    static const uint8_t kSaturday = 6;
    static const uint8_t kSunday = 7;

    static LocalDate forComponents(int16_t year, uint8_t month, uint8_t day);
    static LocalDate forEpochDays(int32_t epochDays);
    static LocalDate forEpochSeconds(acetime_t epochSeconds);
    static LocalDate forUnixDays(int32_t unixDays);
    static LocalDate forUnixSeconds64(int64_t unixSeconds);

    int16_t year() const;
    void year(int16_t year);

    uint8_t month() const;
    void month(uint8_t month);

    uint8_t day() const;
    void day(uint8_t day);

    uint8_t dayOfWeek() const;
    bool isError() const;

    int32_t toEpochDays() const {
    acetime_t toEpochSeconds() const {

    int32_t toUnixDays() const {
    int64_t toUnixSeconds64() const {

    int8_t compareTo(const LocalDate& that) const {
    void printTo(Print& printer) const;
    ...
};

}
```

You can use them like this:

```C++
#include <AceTime.h>
using namespace ace_time;
...

// LocalDate that represents 2019-05-20
auto localDate = LocalDate::forComponents(2019, 5, 20);

// LocalTime that represents 13:00:00
auto localTime = LocalTime::forComponents(13, 0, 0);
```

You can ask the `LocalDate` to determine its day of the week, which returns
an integer where `1=Monday` and `7=Sunday` per
[ISO 8601](https://en.wikipedia.org/wiki/ISO_8601):

```C++
uint8_t dayOfWeek = localDate.dayOfWeek();
```

<a name="DateStrings"></a>
### Date Strings

To convert the `dayOfweek()` numerical code to a human-readable string for
debugging or display, we can use the `DateStrings` class:


```C++
namespace ace_time {

class DateStrings {
  public:
    static const uint8_t kBufferSize = 10;
    static const uint8_t kShortNameLength = 3;

    const char* monthLongString(uint8_t month);
    const char* monthShortString(uint8_t month);

    const char* dayOfWeekLongString(uint8_t dayOfWeek);
    const char* dayOfWeekShortString(uint8_t dayOfWeek);
};

}
```

The `DateStrings` object uses an internal buffer to hold the generated
human-readable strings. That makes this class stateful, which means that we need
to handle its lifecycle carefully. The recommended usage of this object is to
create an instance the stack, call one of the `dayOfWeek*String()` or
`month*String()` methods, copy the resulting string somewhere else (e.g. print
it to Serial), then allow the `DateStrings` object to go out of scope and
reclaimed from the stack. The class is not meant to be created and persisted for
a long period of time, unless you are sure that nothing else will reuse the
internal buffer between calls.

```C++
#include <AceTime.h>
using namespace ace_time;
...

auto localDate = LocalDate::forComponents(2019, 5, 20);
uint8_t dayOfWeek = localDate.dayOfWeek();
Serial.println(DateStrings().dayOfWeekLongString(dayOfWeek));
Serial.println(DateStrings().dayOfWeekShortString(dayOfWeek));
```

The `dayOfWeekShortString()` method returns the
first 3 characters of the week day (i.e. "Mon", "Tue", "Wed", "Thu",
"Fri", "Sat", "Sun").

Similarly the `LocalDate::month()` method returns an integer code where
`1=January` and `12=December`. This integer code can be translated into English
strings using `DateStrings().monthLongString()`:

```C++
uint8_t month = localDate.month();
Serial.println(DateStrings().monthLongString(month));
Serial.println(DateStrings().monthShortString(month));
```
The `monthShortString()` method returns the first 3 characters of the month
(i.e. "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct",
"Nov", "Dec").

**Caveat**: The `DateStrings` class supports only the English language. If you
need to convert to another language, you need to write the conversion class
yourself, possibly by copying the implementation details of the `DateStrings`
class.

<a name="LocalDateTime"></a>
### LocalDateTime

A `LocalDateTime` object holds both the date and time components
(year, month, day, hour, minute, second). Internally, it is implemented as a
combination of `LocalDate` and `LocalTime` and supports essentially all
operations on those classes. It does *not* support the notion of timezone.

```C++
namespace ace_time {

class LocalDateTime {
  public:
    static LocalDateTime forComponents(int16_t year, uint8_t month,
        uint8_t day, uint8_t hour, uint8_t minute, uint8_t second);
    static LocalDateTime forEpochSeconds(acetime_t epochSeconds);
    static LocalDateTime forUnixSeconds64(int64_t unixSeconds);
    static LocalDateTime forDateString(const char* dateString);

    bool isError() const;

    int16_t year() const; // 1 - 9999
    void year(int16_t year);

    uint8_t month() const; // 1 - 12
    void month(uint8_t month);

    uint8_t day() const; // 1 - 31
    void day(uint8_t day);

    uint8_t hour() const; // 0 - 23
    void hour(uint8_t hour);

    uint8_t minute() const; // 0 - 59
    void minute(uint8_t minute);

    uint8_t second() const; // 0 - 59 (no leap seconds)
    void second(uint8_t second);

    uint8_t dayOfWeek() const; // 1=Monday, 7=Sunday

    const LocalDate& localDate() const;
    const LocalTime& localTime() const;

    int32_t toEpochDays() const;
    acetime_t toEpochSeconds() const;

    int32_t toUnixDays() const;
    int64_t toUnixSeconds64() const;

    int8_t compareTo(const LocalDateTime& that) const;
    void printTo(Print& printer) const;
    ...
};

}
```

Here is a sample code that extracts the number of seconds since AceTime Epoch
(2050-01-01T00:00:00 UTC) using the `toEpochSeconds()` method:

```C++
// 2018-08-30T06:45:01-08:00
auto localDateTime = LocalDateTime::forComponents(2018, 8, 30, 6, 45, 1);
acetime_t epoch_seconds = localDateTime.toEpochSeconds();
```

We can go the other way and create a `LocalDateTime` from the Epoch Seconds:

```C++
auto localDateTime = LocalDateTime::forEpochSeconds(1514764800L);
localDateTime.printTo(Serial); // prints "2018-01-01T00:00:00"
```

Both `printTo()` and `forDateString()` are expected to be used only for
debugging. The `printTo()` prints a human-readable representation of the date in
[ISO 8601](https://en.wikipedia.org/wiki/ISO_8601) format (yyyy-mm-ddThh:mm:ss)
to the given `Print` object. The most common `Print` object is the `Serial`
object which prints on the serial port. The `forDateString()` parses the
ISO 8601 formatted string and returns the `LocalDateTime` object.

<a name="TimePeriod"></a>
### TimePeriod

The `TimePeriod` class can be used to represents a difference between two
`XxxDateTime` objects, if the difference is not too large. Internally, it is
implemented as 3 unsigned `uint8_t` integers representing the hour, minute and
second components. There is a 4th signed `int8_t` integer that holds the sign
(-1 or +1) of the time period. The largest (or smallest) time period that can be
represented by this class is +/- 255h59m59s, corresponding to +/- 921599
seconds.

```C++
namespace ace_time {

class TimePeriod {
  public:
    static const int32_t kInvalidPeriodSeconds = INT32_MIN;
    static const int32_t kMaxPeriodSeconds = 921599;

    static TimePeriod forError(int8_t sign = 0);

    explicit TimePeriod(uint8_t hour, uint8_t minute, uint8_t second,
        int8_t sign = 1);
    explicit TimePeriod(int32_t seconds = 0);

    uint8_t hour() const;
    void hour(uint8_t hour);

    uint8_t minute() const;
    void minute(uint8_t minute);

    uint8_t second() const;
    void second(uint8_t second);

    int8_t sign() const;
    void sign(int8_t sign);

    int32_t toSeconds() const;

    bool isError() const;

    int8_t compareTo(const TimePeriod& that) const;
    void printTo(Print& printer) const;
};

}
```

This class was created to show the difference between 2 dates in a
human-readable format, broken down by hours, minutes and seconds. For example,
we can print out a countdown to a target `LocalDateTime` from the current
`LocalDateTime` like this:

```C++
LocalDateTime current = ...;
LocalDateTime target = ...;
acetime_t diffSeconds = target.toEpochSeconds() - current.toEpochSeconds();
TimePeriod timePeriod(diffSeconds);
timePeriod.printTo(Serial)
```

The largest absolutely value of `diffSeconds` supported by this class is
`TimePeriod::kMaxPeriodSeconds` which is 921599, which corresponds to
`255h59m59s`. Calling the `TimePeriod(int32_t)` constructor outside of the `+/-
921599` range will return an object whose `isError()` returns `true`.

You can check the `TimePeriod::sign()` method to determine which one of the 3
cases apply. The `printTo()` method prints the following:

* generic error: `sign() == 0`, `printTo()` prints `<Error>`
* overflow: `sign() == 1`, `printTo()` prints `<+Inf>`
* underflow: `sign() == -1`, `printTo()` prints `<-Inf>`

It is sometimes useful to directly create a `TimePeriod` object that represents
an error condition. The `TimePeriod::forError(int8_t sign = 0)` factory method
uses the `sign` parameter to distinguish 3 different error types described
above. By default with no arguments, it create a generic error object with `sign
== 0`.

Calling `TimePeriod::toSeconds()` on an error object returns
`TimePeriod::kInvalidPeriodSeconds` regardless of the `sign` value. However,
you can call the `TimePeriod::sign()` method to distinguish among the 3
different error conditions.

<a name="TimeOffset"></a>
### TimeOffset

A `TimeOffset` class represents an amount of time shift from a reference point.
This is usually used to represent a timezone's standard UTC offset or its DST
offset in the summer. The time resolution of this class has progressively become
finer as this library handles more obscure timezones, causing the object to
become larger. Early versions used a single byte for a 15-minute resolution,
then 2 bytes for a 1-minute resolution, and now 4 bytes to capture 1-second
resolution. It now has a range of +/- 49710 days, which is more than enough
to handle the STD or DST offset of any timezone.

```C++
namespace ace_time {

class TimeOffset {
  public:
    static TimeOffset forHours(int8_t hours);
    static TimeOffset forMinutes(int16_t minutes);
    static TimeOffset forSeconds(int32_t seconds);

    static TimeOffset forHourMinute(int8_t hour, int8_t minute);
    static TimeOffset forHourMinuteSecond(
        int8_t hour, int8_t minute, int8_t second);

    int16_t toMinutes() const;
    int32_t toSeconds() const;
    void toHourMinute(int8_t& hour, int8_t& minute) const;
    void toHourMinuteSecond(int8_t& hour, int8_t& minute, int8_t second) const;

    bool isZero() const;
    bool isError() const;
    void printTo(Print& printer) const;
};

}
```

A `TimeOffset` can be created using the factory methods:

```C++
auto offset = TimeOffset::forHours(-8); // -08:00
auto offset = TimeOffset::forMinutes(135); // +02:15
auto offset = TimeOffset::forHourMinute(-2, -30); // -02:30
```

If the time offset is negative, then both the hour and minute components of
`forHourMinute()` must be negative. (The duplication of the negative sign allows
the creation of UTC-00:15, UTC-00:30 and UTC-00:45.)

A `TimeOffset` instance can be converted into different formats:

```C++
int32_t seconds = offset.toSeconds();
int16_t minutes = offset.toMinutes();

int8_t hour;
int8_t minute;
offset.toHourMinute(&hour, &minute);
```

When a method in some class (e.g. `OffsetDateTime` or `ZonedDateTime` below)
returns a `TimeOffset`, it is useful to indicate an error condition by returning
the special value created by the factory method `TimeOffset::forError()`. This
special error marker has the property that `TimeOffset::isError()` returns
`true`.

The convenience method `TimeOffset::isZero()` returns `true` if the offset has a
zero offset. This is often used to determine if a timezone is currently
observing Daylight Saving Time (DST).

<a name="OffsetDateTime"></a>
### OffsetDateTime

An `OffsetDateTime` is an object that can represent a `LocalDateTime` which is
offset from the UTC time zone by a fixed amount. Internally the `OffsetDateTime`
is an aggregation of `LocalDateTime` and `TimeOffset`. Use this class for
creating and writing timestamps for events which are destined for logging for
example. This class does not know about Daylight Saving Time transitions.

```C++
namespace ace_time {

class OffsetDateTime {
  public:
    static OffsetDateTime forComponents(int16_t year, uint8_t month,
        uint8_t day, uint8_t hour, uint8_t minute, uint8_t second,
        TimeOffset timeOffset);
    static OffsetDateTime forEpochSeconds(acetime_t epochSeconds,
        TimeOffset timeOffset);
    static OffsetDateTime forUnixSeconds64(int64_t unixSeconds,
        TimeOffset timeOffset);
    static OffsetDateTime forDateString(const char* dateString);

    bool isError() const;

    int16_t year() const;
    void year(int16_t year);

    uint8_t month() const;
    void month(uint8_t month);

    uint8_t day() const;
    void day(uint8_t day);

    uint8_t hour() const;
    void hour(uint8_t hour);

    uint8_t minute() const;
    void minute(uint8_t minute);

    uint8_t second() const;
    void second(uint8_t second);

    uint8_t dayOfWeek() const;
    const LocalDate& localDate() const;

    const LocalTime& localTime() const;
    TimeOffset timeOffset() const;

    void timeOffset(TimeOffset timeOffset);
    OffsetDateTime convertToTimeOffset(TimeOffset timeOffset) const;

    int32_t toEpochDays() const;
    acetime_t toEpochSeconds() const;

    int32_t toUnixDays() const;
    int64_t toUnixSeconds64() const;

    int8_t compareTo(const OffsetDateTime& that) const;
    void printTo(Print& printer) const;
};

}
```

We can create the object using the `forComponents()` method:

```C++
// 2018-01-01 00:00:00+00:15
auto offsetDateTime = OffsetDateTime::forComponents(
    2018, 1, 1, 0, 0, 0, TimeOffset::forHourMinute(0, 15));
int32_t epochDays = offsetDateTime.toEpochDays();
acetime_t epochSeconds = offsetDateTime.toEpochSeconds();

offsetDateTime.printTo(Serial); // prints "2018-01-01 00:00:00+00:15"
Serial.println(epochDays); // prints 6574
Serial.println(epochSeconds); // prints 568079100
```

We can create an `OffsetDateTime` object from the seconds from Epoch using
the `forEpochSeconds()` method:

```C++
auto offsetDateTime = OffsetDateTime::forEpochSeconds(
    568079100, TimeOffset::forHourMinute(0, 15));
```

Both `printTo()` and `forDateString()` are expected to be used only for
debugging. The `printTo()` prints a human-readable representation of the date in
[ISO 8601](https://en.wikipedia.org/wiki/ISO_8601) format
(yyyy-mm-ddThh:mm:ss+/-hh:mm) to the given `Print` object. The most common
`Print` object is the `Serial` object which prints on the serial port. The
`forDateString()` parses the ISO 8601 formatted string and returns the
`OffsetDateTime` object.

<a name="TimeZoneClasses"></a>
### TimeZone Related Classes

These classes build upon the simpler classes described above to provide
functionality related to time zones. These classes bridge the gap between the
information encoded in the ZoneInfo Database for the various time zones and the
expected date and time fields appropriate for those time zones.

<a name="TimeZone"></a>
### TimeZone

A "time zone" is often used colloquially to mean 2 different things:

* An offset from the UTC time by a fixed amount, or
* A geographical or political region whose local time is offset from the UTC
  time using various transition rules.

Both meanings of "time zone" are supported by the `TimeZone` class using
3 different types as defined by the value of `getType()`:

* `TimeZone::kTypeManual` (1): a fixed base offset and optional DST offset from
  UTC
* `BasicZoneProcessor::kTypeBasic` (3): utilizes a `BasicZoneProcessor` which
  can be encoded with (relatively) simple rules from the ZoneInfo Database
* `ExtendedZoneProcessor::kTypeExtended` (4): utilizes a `ExtendedZoneProcessor`
  which can handle all zones and links in the ZoneInfo Database
* `CompleteZoneProcessor::kTypeComplete` (5): utilizes a `CompleteZoneProcessor`
  which can handle all zones and links in the ZoneInfo Database, over the
  entire year range `[0001,10000)`

The class hierarchy of `TimeZone` is shown below, where the arrow means
"is-subclass-of" and the diamond-line means "is-aggregation-of". This is an
internal implementation detail of the `TimeZone` class that the application
developer will not normally need to be aware of all the time, but maybe this
helps make better sense of the usage of the `TimeZone` class. A `TimeZone` can
hold a reference to:

* nothing (`kTypeManual`),
* one `BasicZoneProcessor` object, (`kTypeBasic`), or
* one `ExtendedZoneProcessor` object (`kTypeExtended`)
* one `CompleteZoneProcessor` object (`kTypeComplete`)

```
               0..1
TimeZone <>-------- ZoneProcessor
                         ^
                         |
                   .-----+-----.
                   |     |     |
    BasicZoneProcessor   |    ExtendedZoneProcessor
                         |
                 CompleteZoneProcessor
```

Here is the class declaration of `TimeZone`:

```C++
namespace ace_time {

class TimeZone {
  public:
    static const uint8_t kTypeError = 0;
    static const uint8_t kTypeManual = 1;
    static const uint8_t kTypeReserved = 2;

    static TimeZone forTimeOffset(
        TimeOffset stdOffset,
        TimeOffset dstOffset = TimeOffset());

    static TimeZone forHours(int8_t stdHours, int8_t dstHours = 0);
    static TimeZone forMinutes(int16_t stdMinutes, int16_t dstMinutes = 0);

    static TimeZone forHourMinute(
        int8_t stdHour,
        int8_t stdMinute,
        int8_t dstHour = 0,
        int8_t dstMinute = 0);

    static TimeZone forZoneInfo(
        const basic::ZoneInfo* zoneInfo,
        BasicZoneProcessor* zoneProcessor);

    static TimeZone forZoneInfo(
        const extended::ZoneInfo* zoneInfo,
        ExtendedZoneProcessor* zoneProcessor);

    static TimeZone forZoneInfo(
        const complete::ZoneInfo* zoneInfo,
        CompleteZoneProcessor* zoneProcessor);

    static TimeZone forUtc();

    TimeZone(); // same as forUtc()
    bool isError() const;

    uint8_t getType() const;
    uint32_t getZoneId() const;

    OffsetDateTime getOffsetDateTime(const LocalDateTime& ldt) const;
    OffsetDateTime getOffsetDateTime(acetime_t epochSeconds) const;

    ZonedExtra getZonedExtra(const LocalDateTime& ldt) const;
    ZonedExtra getZonedExtra(acetime_t epochSeconds) const;

    // for kTypeManual only
    TimeOffset getStdOffset() const;
    TimeOffset getDstOffset() const;
    bool isUtc() const;
    bool isDst() const;

    TimeZoneData toTimeZoneData() const;

    void printTo(Print& printer) const;
    void printShortTo(Print& printer) const;
};

}
```

The TimeZone class is an immutable value type. It can be passed around by
value, but since it is between 5 bytes (8-bit processors) and 12 bytes
(32-bit processors) big, it may be slightly more efficient to pass by const
reference, then save locally by-value when needed. The ZonedDateTime holds
the TimeZone object by-value.

The following methods apply only to instances of the type `kTypeManual`:

* `forUtc()`
    * create a `TimeZone` instance for UTC+00:00
* `forTimeOffset(stdOffset, dstOffset)`
    * create a `TimeZone` instance using `TimeOffset`
* `forHours(stdHours, dstHours)`
    * create a `TimeZone` instance using hours offset
* `forMinutes(stdMinutes, dstMinutes)`
    * create a `TimeZone` instance using minutes offset
* `isUtc()`:
    * returns true if the instance is a UTC time zone instance
    * returns false if not `kTypeManual`
* `isDst()`:
    * returns true if the dstOffset is not zero
    * returns false if not `kTypeManual`

The following methods apply to a `kTypeBasic`, `kTypeExtended`, `kTypeComplete`:

* `forZoneInfo(zoneInfo, zoneProcessor)`
    * Create an instance of from the given `ZoneInfo*` pointer (e.g.
      `complete::kZoneAmerica_Los_Angeles`, or
      `extended::kZoneAmerica_Los_Angeles`)
* `getZoneId()`
    * Returns a `uint32_t` integer which is a unique and stable identifier for
      the IANA timezone. The zoneId identifier can be used to save and restore
      the `TimeZone`. See the [ZoneManager](#ZoneManager) subsection below.

The following methods apply to any type of `TimeZone`:

* `getOffsetDateTime(localDateTime)`
    * Returns the best guess of the `OffsetDateTime` at the given local date
      time. This method is used by `ZonedDateTime::forComponents()` and is
      exposed mostly for debugging.
    * The `fold` parameter of the `localDateTime` will be used by the
      `ExtendedZoneProcessor` and `CompleteZoneProcessor` to disambiguate
      date-time in the gap or overlap selecting the first (0) or second (1)
      transition line.
    * The `BasicZoneProcessor` does not support the `fold` parameter so will
      ignore it.
* `getOffsetDateTime(epochSeconds)`
    * Returns the `OffsetDateTime` that matches the given `epochSeconds`.
    * The `OffsetDateTime::fold` parameter indicates whether the date-time
      occurred the first time (0), or the second time (1)
* `printTo()`
    * Prints the fully-qualified unique name for the time zone. For example,
      `"UTC"`, `"-08:00"`, `"-08:00(DST)"`, `"America/Los_Angeles"`.
* `printShortTo()`
    * Similar to `printTo()` except that it prints the last component of the
      IANA TZ Database zone names.
    * In other words, `"America/Los_Angeles"` is printed as `"Los_Angeles"`.
      This is helpful for printing on small width OLED displays.

<a name="ManualTimeZone"></a>
#### Manual TimeZone (kTypeManual)

The default constructor creates a `TimeZone` in UTC time zone with no
offset. This is also identical to the `forUtc()` method:

```C++
TimeZone tz; // UTC+00:00
auto tz = TimeZone::forUtc(); // identical to above
```

To create `TimeZone` instances with other offsets, use the `forTimeOffset()`
factory method, or starting with v1.4, use the `forHours()`, `forMinutes()` and
`forHourMinute()` convenience methods:

```C++
// UTC-08:00, no DST shift
auto tz = TimeZone::forTimeOffset(TimeOffset::forHours(-8));
auto tz = TimeZone::forHours(-8); // identical to above

// UTC-04:30, no DST shift
auto tz = TimeZone::forTimeOffset(TimeOffset::forHourMinute(-4, -30));
auto tz = TimeZone::forHourMinute(-4, -30); // identical to above

// UTC-03:30 with a 01:00 DST shift, so effectively UTC-02:30
auto tz = TimeZone::forTimeOffset(
    TimeOffset::forHourMinute(-3, -30),
    TimeOffset::forHourMinute(1, 0));
auto tz = TimeZone::forHourMinute(-3, -30, 1, 0); // identical to above
```

The `TimeZone::isUtc()`, `TimeZone::isDst()` and `TimeZone::setDst(bool)`
methods work only if the `TimeZone` is a `kTypeManual`.

<a name="BasicTimeZone"></a>
#### Basic TimeZone (kTypeBasic)

This TimeZone is created using two objects:
* the `basic::ZoneInfo` data objects contained in
  [zonedb/zone_infos.h](src/zonedb/zone_infos.h)
* an external instance of `BasicZoneProcessor` needed for calculating zone
  transitions

```C++
BasicZoneProcessor zoneProcessor;

void someFunction() {
  auto tz = TimeZone::forZoneInfo(&zonedb::kZoneAmerica_Los_Angeles,
      &zoneProcessor);
  ...
}
```

The ZoneInfo entries were generated by a script using the IANA TZ Database. This
header file is already included in `<AceTime.h>` so you don't have to explicitly
include it. As of version 2021a of the database, it contains 266 Zone and 183
Link entries and whose time change rules are simple enough to be supported by
`BasicZoneProcessor`. The bottom of the `zone_infos.h` header file lists 117
zones whose zone rules are too complicated for `BasicZoneProcessor`.

The zone names are normalized so that the ZoneInfo variable is a valid C++ name:

* a `+` (plus) character in the zone name is replaced with `_PLUS_` to avoid
  conflict with a `-` (minus) character (e.g. `GMT+0` becomes `GMT_PLUS_0`)
* any remaining non-alphanumeric character (`0-9a-zA-Z_`) are replaced with
  an underscore (`_`) (e.g. `GMT-0` becomes `GMT_0`)

Some examples of `ZoneInfo` entries supported by `zonedb` are:

* `zonedb::kZoneAmerica_Los_Angeles` (`America/Los_Angeles`)
* `zonedb::kZoneAmerica_New_York` (`America/New_York`)
* `zonedb::kZoneAustralia_Darwin` (`Australia/Darwin`)
* `zonedb::kZoneEurope_London` (`Europe/London`)
* `zonedb::kZoneGMT_PLUS_10` (`GMT+10`)
* `zonedb::kZoneGMT_10` (`GMT-10`)

The following example creates a `TimeZone` which describes
`America/Los_Angeles`. A `TimeZone` instance is normally expected to be just
passed into a `ZonedDateTime` object through a factory method, but there are
a few ways that a `TimeZone` object can be used directly. See the
[ZonedExtra](#ZonedExtra) section below for more information:

```C++
#include <AceTime.h>
using namespace ace_time;
...

BasicZoneProcessor zoneProcessor;

void someFunction() {
  ...
  auto tz = TimeZone::forZoneInfo(&zonedb::kZoneAmerica_Los_Angeles,
      &zoneProcessor);

  // 2018-03-11T01:59:59-08:00 was still in STD time
  {
    auto dt = OffsetDateTime::forComponents(2018, 3, 11, 1, 59, 59,
      TimeOffset::forHours(-8));
    acetime_t epochSeconds = dt.toEpochSeconds();
    ZonedExtra ze = tz.getZonedExtra(epochSeconds);
    TimeOffset offset = ze.timeOffset(); // returns -08:00
  }

  // one second later, 2018-03-11T02:00:00-08:00 was in DST time
  {
    auto dt = OffsetDateTime::forComponents(2018, 3, 11, 2, 0, 0,
      TimeOffset::forHours(-8));
    acetime_t epochSeconds = dt.toEpochSeconds();
    ZonedExtra ze = tz.getZonedExtra(epochSeconds);
    TimeOffset offset = ze.timeOffset(); // returns -07:00
  }
  ...
}
```

<a name="ExtendedTimeZone"></a>
#### Extended TimeZone (kTypeExtended)

This TimeZone is created using two objects:
* the `extended::ZoneInfo` data objects contained in
  [zonedbx/zone_infos.h](src/zonedbx/zone_infos.h)
* an external instance of `ExtendedZoneProcessor` needed for calculating zone
  transitions

```C++
ExtendedZoneProcessor zoneProcessor;

void someFunction() {
  auto tz = TimeZone::forZoneInfo(&zonedbx::kZoneAmerica_Los_Angeles,
      &zoneProcessor);
  ...
}
```
(Notice that we use the `zonedbx::` namespace instead of the `zonedb::`
namespace. Although the data structures in the 2 namespaces are identical
currently (v1.2) but the *values* inside the data structure fields are not
the same, and they are interpreted differently.)

As of version 2021a of the IANA TZ Database, *all* 386 Zone and 207 Link entries
from the following TZ files are supported: `africa`, `antarctica`, `asia`,
`australasia`, `backward`, `etcetera`, `europe`, `northamerica`, `southamerica`.
There are 3 files which are not processed (`backzone`, `systemv`, `factory`)
because they don't seem to contain anything useful.

The zone infos which can be used by `ExtendedZoneProcessor` are in the
`zonedbx::` namespace instead of the `zonedb::` namespace. Some examples of the
zone infos which exists in `zonedbx::` but not in `zonedb::` are:

* `zonedbx::kZoneAfrica_Casablanca`
* `zonedbx::kZoneAmerica_Argentina_San_Luis`
* `zonedbx::kZoneAmerica_Indiana_Petersburg`
* `zonedbx::kZoneAsia_Hebron`
* `zonedbx::kZoneEurope_Moscow`

The following example creates a `TimeZone` which describes
`America/Los_Angeles`. A `TimeZone` instance is normally expected to be just
passed into a `ZonedDateTime` object through a factory method, but there are
a few ways that a `TimeZone` object can be used directly. See the
[ZonedExtra](#ZonedExtra) section below for more information:

```C++
ExtendedZoneProcessor zoneProcessor;

void someFunction() {
  ...
  TimeZone tz = TimeZone::forZoneInfo(&zonedbx::kZoneAmerica_Los_Angeles,
      &zoneProcessor);

  // 2018-03-11T01:59:59-08:00 was still in STD time
  {
    auto dt = OffsetDateTime::forComponents(2018, 3, 11, 1, 59, 59,
      TimeOffset::forHours(-8));
    acetime_t epochSeconds = dt.toEpochSeconds();
    ZonedExtra ze = tz.getZonedExtra(epochSeconds);
    TimeOffset offset = ze.timeOffset(); // returns -08:00
  }

  // one second later, 2018-03-11T02:00:00-08:00 was in DST time
  {
    auto dt = OffsetDateTime::forComponents(2018, 3, 11, 2, 0, 0,
      TimeOffset::forHours(-8));
    acetime_t epochSeconds = dt.toEpochSeconds();
    ZonedExtra ze = tz.getZonedExtra(epochSeconds);
    TimeOffset offset = ze.timeOffset(); // returns -07:00
  }
  ...
}
```

<a name="CompleteTimeZone"></a>
#### Complete TimeZone (kTypeComplete)

This TimeZone is created using two objects:
* the `complete::ZoneInfo` data objects contained in
  [zonedbc/zone_infos.h](src/zonedbc/zone_infos.h)
* an external instance of `CompleteZoneProcessor` needed for calculating zone
  transitions

```C++
CompleteZoneProcessor zoneProcessor;

void someFunction() {
  auto tz = TimeZone::forZoneInfo(&zonedbc::kZoneAmerica_Los_Angeles,
      &zoneProcessor);
  ...
}
```
(Notice that we use the `zonedbc::` namespace instead of the `zonedb::`
namespace. Although the data structures in the 2 namespaces are identical
currently (v1.2) but the *values* inside the data structure fields are not
the same, and they are interpreted differently.)

As of version 2021a of the IANA TZ Database, *all* 386 Zone and 207 Link entries
from the following TZ files are supported: `africa`, `antarctica`, `asia`,
`australasia`, `backward`, `etcetera`, `europe`, `northamerica`, `southamerica`.
There are 3 files which are not processed (`backzone`, `systemv`, `factory`)
because they don't seem to contain anything useful.

The zone infos which can be used by `CompleteZoneProcessor` are in the
`zonedbc::` namespace instead of the `zonedb::` namespace. Some examples of the
zone infos which exists in `zonedbc::` but not in `zonedb::` are:

* `zonedbc::kZoneAfrica_Casablanca`
* `zonedbc::kZoneAmerica_Argentina_San_Luis`
* `zonedbc::kZoneAmerica_Indiana_Petersburg`
* `zonedbc::kZoneAsia_Hebron`
* `zonedbc::kZoneEurope_Moscow`

The following example creates a `TimeZone` which describes
`America/Los_Angeles`. A `TimeZone` instance is normally expected to be just
passed into a `ZonedDateTime` object through a factory method, but there are
a few ways that a `TimeZone` object can be used directly. See the
[ZonedExtra](#ZonedExtra) section below for more information:

```C++
CompleteZoneProcessor zoneProcessor;

void someFunction() {
  ...
  TimeZone tz = TimeZone::forZoneInfo(&zonedbc::kZoneAmerica_Los_Angeles,
      &zoneProcessor);

  // 2018-03-11T01:59:59-08:00 was still in STD time
  {
    auto dt = OffsetDateTime::forComponents(2018, 3, 11, 1, 59, 59,
      TimeOffset::forHours(-8));
    acetime_t epochSeconds = dt.toEpochSeconds();
    ZonedExtra ze = tz.getZonedExtra(epochSeconds);
    TimeOffset offset = ze.timeOffset(); // returns -08:00
  }

  // one second later, 2018-03-11T02:00:00-08:00 was in DST time
  {
    auto dt = OffsetDateTime::forComponents(2018, 3, 11, 2, 0, 0,
      TimeOffset::forHours(-8));
    acetime_t epochSeconds = dt.toEpochSeconds();
    ZonedExtra ze = tz.getZonedExtra(epochSeconds);
    TimeOffset offset = ze.timeOffset(); // returns -07:00
  }
  ...
}
```

<a name="TimeZoneTypeRecommendations"></a>
### TimeZone Type Recommendations

There are 4 major types of `TimeZone` objects:

* `kTypeManual`: STD and DST offsets are fixed
* `kTypeBasic`: uses `BasicZoneProcessor`
* `kTypeExtended`: uses `ExtendedZoneProcessor`
* `kTypeComplete`: uses `CompleteZoneProcessor`

**tl;dr**: Most client applications should probably use `kTypeExtended`,
`ExtendedZoneProcessor`, `ExtendedZoneManager`, and the `zonedbx` database.

The `kTypeManual` was added mostly for completeness and for testing purposes. I
do not expect most applications to use the `kTypeManual`, because the primary
purpose of the AceTime library to provide access to the timezones defined by the
IANA TZ database, and the `kTypeManual` timezone does not provide that
functionality.

The advantage of `ExtendedZoneProcessor` over `BasicZoneProcessor` is that
`ExtendedZoneProcessor` will always support all time zones and links in the IANA
TZ Database. The `BasicZoneProcessor` supports only a limited subset of zones
which have certain simplifying properties. It is not uncommon for zones that
used to be supported by `BasicZoneProcessor` to change its DST rules in a
subsequent TZ DB release and becomes incompatible with the `BasicZoneProcessor`.
The `ExtendedZoneProcessor` does not have this problem.

The `ExtendedZoneProcessor` is also more accurate than `BasicZoneProcessor`
during DST gaps when using the `forComponents()` factory methods,  because the
`zonedbx::` entries contain transition information which are missing in the
`zonedb::` entries due to space constraints. The `ExtendedZoneProcessor`
provides complete control which `LocalDateTime` is selected during a gap or
overlap using the `fold` parameter. The `BasicZoneProcessor` ignores the `fold`
parameter and makes educated guesses when the `LocalDateTime` falls in a gap or
an overlap.

The `CompleteZoneProcessor` is identical to the `ExtendedZoneProcessor`, but it
uses the high resolution `zonedbc` database. The high resolution format of the
database allows all timezones to be represented with an accuracy range of
`[0001,10000)`.

The biggest difference between the `BasicZoneProcessor`,
`ExtendedZoneProcessor`, and `CompleteZoneProcessor` is the amount of flash and
static memory consumed. The `ExtendedZoneProcessor` and `CompleteZoneProcessor`
consumes [4 times](examples/AutoBenchmark/README.md) more static memory than
`BasicZoneProcessor` and is a bit slower. For most 32-bit processors, this will
not be an issue, so the `ExtendedZoneProcessor` or `CompleteZoneProcessor` is
recommended. For 8-bit processors, the `BasicZoneProcessor` consumes a lot less
resources, so if your timezone is supported, then it may be the appropriate
choice. In most cases, I think the `ExtendedZoneProcessor` should be preferred
unless memory resources are so constrained that `BasicZoneProcessor` must be
used.

Instead of managing the zone processors manually, you can use the `ZoneManager`
to manage a database of `ZoneInfo` entries, and a cache of multiple
`ZoneProcessor`s, and bind the `TimeZone` to its `ZoneInfo` and its
`ZoneProcessor` more dynamically through the `ZoneManager`. See the section
[ZoneManager](#ZoneManager) below for more information.

<a name="ZonedDateTime"></a>
### ZonedDateTime

A `ZonedDateTime` is an `OffsetDateTime` associated with a given `TimeZone`.
All internal types of `TimeZone` are supported, the `ZonedDateTime` class itself
does not care which one is used. You should use the `ZonedDateTime` when
interacting with human beings, who are aware of timezones and DST transitions.
It can also be used to convert time from one timezone to anther timezone.

<a name="ZonedDateTimeDeclaration"></a>
#### ZonedDateTime Declaration

```C++
namespace ace_time {

class ZonedDateTime {
  public:
    static const acetime_t kInvalidEpochSeconds = LocalTime::kInvalidSeconds;

    static ZonedDateTime forComponents(int16_t year, uint8_t month, uint8_t day,
        uint8_t hour, uint8_t minute, uint8_t second, const TimeZone& timeZone);
    static ZonedDateTime forEpochSeconds(acetime_t epochSeconds,
        const TimeZone& timeZone);
    static ZonedDateTime forUnixSeconds64(int64_t unixSeconds,
        const TimeZone& timeZone);
    static ZonedDateTime forDateString(const char* dateString);

    explicit ZonedDateTime();

    bool isError() const;

    int16_t year() const;
    void year(int16_t year);

    uint8_t month() const;
    void month(uint8_t month);

    uint8_t day() const;
    void day(uint8_t day);

    uint8_t hour() const;
    void hour(uint8_t hour);

    uint8_t minute() const;
    void minute(uint8_t minute);

    uint8_t second() const;
    void second(uint8_t second);

    uint8_t dayOfWeek() const;

    TimeOffset timeOffset() const;
    const TimeZone& timeZone() const;

    ZonedDateTime convertToTimeZone(const TimeZone& timeZone) const;

    int32_t toEpochDays() const;
    acetime_t toEpochSeconds() const;

    int32_t toUnixDays() const;
    int64_t toUnixSeconds64() const;

    int8_t compareTo(const ZonedDateTime& that) const;
    void printTo(Print& printer) const;
    ...
};

}
```

<a name="ZonedDateTimeCreation"></a>
#### ZonedDateTime Creation

There are 2 main factory methods for constructing this object:

* `ZonedDateTime::forComponents()`
* `ZonedDateTime::forEpochSeconds()`

Here is an example of how these can be used:

```C++
ExtendedZoneProcessor zoneProcessor;

void someFunction() {
  ...
  auto tz = TimeZone::forZoneInfo(
      &zonedbx::kZoneAmerica_Los_Angeles,
      &zoneProcessor);

  // Create instance for 2018-01-01T00:00:00-08:00[America/Los_Angeles]
  // using forComponents().
  auto zdt = ZonedDateTime::forComponents(2018, 1, 1, 0, 0, 0, tz);
  acetime_t epochSeconds = zdt.toEpochSeconds();
  Serial.println(epochSeconds); // prints 568108800

  // Create an instance one day later using forEpochSeconds()
  acetime_t oneDayAfterSeconds = epochSeconds + 86400;
  auto zdtPlus1d = ZonedDateTime::forEpochSeconds(oneDayAfterSeconds, tz);

  // Should print "2018-01-01T00:00:00-08:00[America/Los_Angeles]"
  zdt.printTo(Serial);
  Serial.println();

  // Should print "2018-01-02T00:00:00-08:00[America/Los_Angeles]"
  zdtPlus1d.printTo(Serial);
  Serial.println();
  ...
}
```

The `printTo()` prints a human-readable representation of the date in
[ISO 8601](https://en.wikipedia.org/wiki/ISO_8601) format
(yyyy-mm-ddThh:mm:ss+/-hh:mm) to the given `Print` object. The most common
`Print` object is the `Serial` object which prints on the serial port. The
`forDateString()` parses the ISO 8601 formatted string and returns the
`ZonedDateTime` object.

The third factory method, `ZonedDateTime::forDateString()`, converts a
human-readable ISO 8601 date format into an instance of a `ZonedDateTime`.
However, this is intended to be used only for debugging so its functionality is
limited as follows.

**Caveat**: The parser for `forDateString()` looks only at the UTC offset. It
does *not* recognize the IANA TZ Database identifier (e.g.
`[America/Los_Angeles]`). To handle the time zone identifier correctly, the
library needs to load the entire TZ Database into memory and use the
`ZoneManager` to manage the `BasicZoneProcessor`, `ExtendedZoneProcessor`, or
`CompleteZoneProcessor` objects dynamically. But the dataset is too large to fit
on most AVR microcontrollers with only 32kB of flash memory, so we currently do
not support this dynamic lookup. The `ZonedDateTime::timeZone()` will return
Manual `TimeZone` whose `TimeZone::getType()` returns `TimeZone::kTypeManual`.

<a name="TimeZoneConversion"></a>
#### Conversion to Other Time Zones

You can convert a given `ZonedDateTime` object into a representation in a
different time zone using the `DateTime::convertToTimeZone()` method:

```C++
static ExtendedZoneProcessor processorLosAngeles;
static ExtendedZoneProcessor processorZurich;

void someFunction() {
  ...
  auto tzLosAngeles = TimeZone::forZoneInfo(
      &zonedbx::kZoneAmerica_Los_Angeles, &processorLosAngeles);
  auto tzZurich = TimeZone::forZoneInfo(
      &zonedbx::kZoneEurope_Zurich, &processorZurich);

  // Europe/Zurich, 2018-01-01T09:20:00+01:00
  auto zurichTime = ZonedDateTime::forComponents(
      2018, 1, 1, 9, 20, 0, tzZurich);

  // Convert to America/Los_Angeles, 2018-01-01T01:20:00-08:00
  auto losAngelesTime = zurichTime.convertToTimeZone(tzLosAngeles);
  ...
}
```

The two `ZonedDateTime` objects will return the same value for `epochSeconds()`
because that is not affected by the time zone. However, the various date time
components (year, month, day, hour, minute, seconds) will be different.

<a name="DstTransitionCaching"></a>
#### DST Transition Caching

The conversion from an epochSeconds to date-time components using
`ZonedDateTime::forEpochSeconds()` is an expensive operation that requires the
computation of the relevant DST transitions for the given epochSeconds or
date-time components. To improve performance, the ZoneProcessors implement
internal transition caching based on the `year` component. This optimizes the
most commonly expected use case where the epochSeconds is incremented by a clock
(e.g. `SystemClock`) every second, and is converted to human-readable date-time
components once a second. According to [AutoBenchmark](examples/AutoBenchmark/),
the cache improves performance by a factor of 2-3X (8-bit AVR) to 10-20X (32-bit
processors) on consecutive calls to `forEpochSeconds()` with the same `year`.

<a name="ZonedExtra"></a>
### ZonedExtra

The most important feature of the AceTime library is the conversion from
`epochSeconds` to `ZonedDateTime` and vise versa. The `ZonedDateTime` object
contains the most common parameters that is expected to be needed by the user,
the Gregorian date components (provided by `LocalDateTime`) and the total UTC
offset at the specific instance (provided by `ZonedDateTime::timeOffset()`).

To keep memory size of the `ZonedDateTime` class reasonable, it does not contain
some of the less common information that the end-user may wish to know. The
extra time zone parameters are contained in the `ZonedExtra` class, which look
like this:

```C++
namespace ace_time {

class ZonedExtra {
  public:
    static const uint8_t kAbbrevSize = 6 + 1;

    static const uint8_t kTypeNotFound = 0;
    static const uint8_t kTypeExact = 1;
    static const uint8_t kTypeGap = 2;
    static const uint8_t kTypeOverlap = 3;

    static ZonedExtra forError();
    static ZonedExtra forComponents(
        int16_t year, uint8_t month, uint8_t day,
        uint8_t hour, uint8_t minute, uint8_t second,
        const TimeZone& tz, uint8_t fold = 0);
    static ZonedExtra forEpochSeconds(
        acetime_t epochSeconds, const TimeZone& tz);
    static ZonedExtra forLocalDateTime(
        const LocalDateTime& ldt, const TimeZone& tz);

    explicit ZonedExtra() {}
    explicit ZonedExtra(
        uint8_t type,
        int32_t stdOffsetSeconds,
        int32_t dstOffsetSeconds,
        int32_t reqStdOffsetSeconds,
        int32_t reqDstOffsetSeconds,
        const char* abbrev);

    bool isError() const;
    uint8_t type() const;

    TimeOffset timeOffset() const; // stdOffset + dstOffset
    TimeOffset stdOffset() const;
    TimeOffset dstOffset() const;

    TimeOffset reqTimeOffset() const; // reqStdOffst + reqDstOffset
    TimeOffset reqStdOffset() const;
    TimeOffset reqDstOffset() const;

    const char* abbrev() const;
};

}
```

The `ZonedExtra` instance is usually created through the 2 static factory
methods on the `ZonedExtra` class:

* `ZonedExtra::forEpochSeconds(epochSeconds, tz)`
* `ZonedExtra::forComponents(int16_t year, uint8_t month, uint8_t day,
   uint8_t hour, uint8_t minute, uint8_t second, const TimeZone& tz,
   uint8_t fold = 0)`

Often the `ZonedDateTime` will be created first from the epochSeconds, then the
`ZonedExtra` will be created to access additional information about the time zone at that particular epochSeconds (e.g. abbreviation):

```C++
ExtendedZoneProcessor zoneProcessor;
TimeZone tz = TimeZone::forZoneInfo(
    &zonedbx::kZoneAmerica_Los_Angeles,
    &zoneProcessor);
acetime_t epochSeconds = ...;
ZonedDateTime zdt = ZonedDateTime::forEpochSeconds(epochSeconds, tz);
ZonedExtra ze = ZonedExtra::forEpochSeconds(epochSeconds, tz);
```

The `ZonedExtra::type()` parameter identifies whether the given time instant
corresponded to a DST gap, or a DST overlap, or an exact match with no
duplicates.

The `ZonedExtra::stdOffset()` is the standard offset of the timezone at the
given time instant. For example, for `America/Los_Angeles` this will return
`-08:00` (unless the standard offset is changed through something like a
"permanent DST").

The `ZonedExtra::dstOffset()` is the DST offset that pertains to the given
time instant. For example, for `America/Los_Angeles` this will return `01:00`
during summer DST, and `00:00` during normal times.

The `ZonedExtra::timeOffset()` is a convenience method that returns the sum of
`stdOffset() + dstOffset()`. This value is identical to the total UTC offset
returned by `ZonedDateTime::timeOffset()`.

The `ZonedExtra::abbrev()` is the short abbreviation that is in effect at the
given time instant. For example, for `America/Los_Angeles`, this returns "PST"
or "PDT'. The abbreviation is copied into a small `char` buffer inside the
`ZonedExtra` object, so the pointer returned by `abbrev()` is safe to use during
the life of the `ZonedExtra` object.

The `ZonedExtra::kAbbrevSize` defines the `char` buffer size needed to hold any
abbreviation. This is currently defined as 7, which means the longest
abbreviation is 6 characters long.

The `ZonedExtra::reqStdOffset()` and `ZonedExtra::reqDstOffset()` are relevant
and different from the corresponding `stdOffset()` and `dstOffset()` only if the
`type()` is `kTypeGap`. This occurs only if the `ZonedExtra::forComponents()`
factory method is used. Following the algorithm described in [Python PEP
495](https://www.python.org/dev/peps/pep-0495/), the provided localDateTime is
imaginary during a gap so must be mapped to a real local time using the
`LocalDateTime::fold` parameter. When `fold=0`, the transition line before the
gap is extended forward until it hits the given `LocalDateTime`. When `fold=1`,
the transition line after the gap is extended backwards until it hits the given
`LocalDateTime`. The `reqStdOffset()` and `reqDstOffset()` are then derived from
the transition line that is used to convert the provided `LocalDateTime`
instance to `epochSeconds`. The `epochSeconds` is then normalized by converting
it back to `LocalDateTime` using the `stdOffset()` and `dstOffset()` which
matches the `epochSeconds`.

The `isError()` method returns true if the given `LocalDateTime` or
`epochSeconds` represents an error condition.

<a name="ZoneManager"></a>
### ZoneManager

The `TimeZone::forZoneInfo()` methods are simple to use but have the
disadvantage that the ZoneProcessors need to be created manually for each
`TimeZone` instance. This works well for a single time zone, but if you have an
application that needs 3 or more time zones, this can become cumbersome. Also,
it is difficult to reconstruct a `TimeZone` dynamically, say, from its fully
qualified name (e.g. `"America/Los_Angeles"`).

The `ZoneManager` solves these problems by implementing 2 features:

1) It supports a registry of `ZoneInfo` objects, so that a `TimeZone` can be
   created using its `zoneName` string, `zoneInfo` pointer, or `zoneId` integer.
2) It supports the use of cache of `ZoneProcessors` that can be mapped to
   a particular zone as needed.

<a name="ClassHierarchy"></a>
#### Class Hierarchy

Four implementations of the `ZoneManager` are provided. Prior to v1.9, they
were organized into a hierarchy with a top-level `ZoneManager` interface with
pure virtual functions. However, in v1.9, the top-level `ZoneManager` interface
was removed and all functions became nonvirtual. This saves about 1100-1200
bytes of flash on AVR processors.

```C++
namespace ace_time{

class BasicZoneManager {
  public:
    BasicZoneManager(
        uint16_t registrySize,
        const basic::ZoneInfo* const* zoneRegistry,
        BasicZoneProcessorCacheBase& zoneProcessorCache
    );

    uint16_t zoneRegistrySize() const;

    TimeZone createForZoneName(const char* name);
    TimeZone createForZoneId(uint32_t id);
    TimeZone createForZoneIndex(uint16_t index);
    TimeZone createForTimeZoneData(const TimeZoneData& d);
    TimeZone createForZoneInfo(const basic::ZoneInfo* zoneInfo);

    uint16_t indexForZoneName(const char* name) const;
    uint16_t indexForZoneId(uint32_t id) const;

    BasicZoneProcessor* getZoneProcessor(const char* name);
    BasicZone getZoneForIndex(uint16_t index) const;
};

class ExtendedZoneManager {
  public:
    ExtendedZoneManager(
        uint16_t registrySize,
        const extended::ZoneInfo* const* zoneRegistry,
        ExtendedZoneProcessorCacheBase& zoneProcessorCache
    );

    uint16_t zoneRegistrySize() const;

    TimeZone createForZoneInfo(const extended::ZoneInfo* zoneInfo);
    TimeZone createForZoneId(uint32_t id);
    TimeZone createForZoneIndex(uint16_t index);
    TimeZone createForTimeZoneData(const TimeZoneData& d);
    TimeZone createForZoneInfo(const extended::ZoneInfo* zoneInfo);

    uint16_t indexForZoneName(const char* name) const;
    uint16_t indexForZoneId(uint32_t id) const;

    ExtendedZoneProcessor* getZoneProcessor(const char* name);
    ExtendedZone getZoneForIndex(uint16_t index) const;
};

class CompleteZoneManager {
  public:
    CompleteZoneManager(
        uint16_t registrySize,
        const extended::ZoneInfo* const* zoneRegistry,
        CompleteZoneProcessorCacheBase& zoneProcessorCache
    );

    uint16_t zoneRegistrySize() const;

    TimeZone createForZoneInfo(const extended::ZoneInfo* zoneInfo);
    TimeZone createForZoneId(uint32_t id);
    TimeZone createForZoneIndex(uint16_t index);
    TimeZone createForTimeZoneData(const TimeZoneData& d);
    TimeZone createForZoneInfo(const extended::ZoneInfo* zoneInfo);

    uint16_t indexForZoneName(const char* name) const;
    uint16_t indexForZoneId(uint32_t id) const;

    CompleteZoneProcessor* getZoneProcessor(const char* name);
    CompleteZone getZoneForIndex(uint16_t index) const;
};

class ManualZoneManager {
  public:
    uint16_t zoneRegistrySize() const;

    TimeZone createForTimeZoneData(const TimeZoneData& d);
};

}
```

<a name="DefaultRegistries"></a>
#### Default Registries

The constructors for `BasicZoneManager`, `ExtendedZoneManager`, and
`CompleteZoneManager` take a `zoneRegistry` and its `zoneRegistrySize`, and
optionally the `linkRegistry` and `linkRegistrySize` parameters. The AceTime
library comes with a set of pre-defined default Zone and Link registries which
are defined by the following header files. These header files are automatically
included in the `<AceTime.h>` header:

* [zonedb/zone_registry.h](src/zonedb/zone_registry.h)
    * Zones and Links supported by `BasicZoneManager`
    * `ace_time::zonedb::kZoneAndLinkRegistry`
    * `ace_time::zonedb::kZoneAndLinkRegistrySize`
* [zonedbx/zone_registry.h](src/zonedbx/zone_registry.h)
    * Zones and Links supported by `ExtendedZoneManager`
    * `ace_time::zonedbx::kZoneAndLinkRegistry`
    * `ace_time::zonedbx::kZoneAndLinkRegistrySize`
* [zonedbc/zone_registry.h](src/zonedbx/zone_registry.h)
    * Zones and Links supported by `CompleteZoneManager`
    * `ace_time::zonedbc::kZoneAndLinkRegistry`
    * `ace_time::zonedbc::kZoneAndLinkRegistrySize`

Each database also defines a smaller registry named `kZoneRegistry` and
`kZoneRegistrySize`. These contain only the Zone entries from the IANA TZ
database, not the Link entries. However, for the perspective of the end-user,
there is no difference between a Zone entry and a Link entry. Therefore, I
recommend that client applications always use the `kZoneAndLinkRegistry` to
support all timezone identifiers defined by the IANA TZ database.

<a name="ZoneProcessorCache"></a>
#### ZoneProcessorCache

The `BasicZoneManager`, `ExtendedZoneManager`, and `CompleteZoneManager` classes
need to be given an instance of its ZoneProcessorCache, a
`BasicZoneProcessorCache<CACHE_SIZE>`, `ExtendedZoneProcessorCache<CACHE_SIZE>`,
`CompleteZoneProcessorCache<CACHE_SIZE>`:

```C++
BasicZoneProcessorCache<CACHE_SIZE> basicZoneProcessorCache;
ExtendedZoneProcessorCache<CACHE_SIZE> extendedZoneProcessorCache;
CompleteZoneProcessorCache<CACHE_SIZE> completeZoneProcessorCache;
```

These used to be defined internally inside the `BasicZoneManager`,
`ExtendedZoneManager`, and `CompleteZoneManager` classes. But when they were
refactored to be non-polymorphic to save flash memory, it was easier to extract
the ZoneProcessorCache objects into separate classes to be passed into the
ZoneManager classes.

The `CACHE_SIZE` template parameter is an integer that specifies the size of the
internal cache. This should be set to the number of time zones that your
application is expected to use *at the same time*. If your app never changes its
time zone after initialization, then this can be `<1>` (although in this case,
you may not even want to use the `ZoneManager`). If your app allows the user to
dynamically change the time zone (e.g. from a menu of time zones), then this
should be at least `<2>`. This allows the system to maintain 2 active
`ZoneProcessors` to quickly compare the old time zone to the new time zone
selected by the user. In general, the `CACHE_SIZE` should be set to the number
of timezones displayed to the user concurrently, plus an additional 1 if the
user is able to change the timezone dynamically.

<a name="ZoneManagerCreation"></a>
#### ZoneManager Creation

The ZoneManager object (except for `ManualZoneManager`) is initialized with
a zone registry and its zone cache, like this:

```C++
static const uint8_t CACHE_SIZE = 2; // tuned for application

BasicZoneProcessorCache<CACHE_SIZE> zoneProcessorCache;
BasicZoneManager zoneManager(
    zonedb::kZoneAndLinkRegistrySize,
    zonedb::kZoneAndLinkRegistry,
    zoneProcessorCache);

ExtendedZoneProcessorCache<CACHE_SIZE> zoneProcessorCache;
ExtendedZoneManager zoneManager(
    zonedbx::kZoneAndLinkRegistrySize,
    zonedbx::kZoneAndLinkRegistry,
    zoneProcessorCache);

CompleteZoneProcessorCache<CACHE_SIZE> zoneProcessorCache;
CompleteZoneManager zoneManager(
    zonedbc::kZoneAndLinkRegistrySize,
    zonedbc::kZoneAndLinkRegistry,
    zoneProcessorCache);
```

Once the `ZoneManager` is configured with the appropriate registries, you can
use one of the `createForXxx()` methods to create a `TimeZone` as shown in the
subsections below.

It is possible to create your own custom Zone and Link registries. See the
[Custom Zone Registry](#CustomZoneRegistry) subsection below.

<a name="CreateForZoneName"></a>
#### createForZoneName()

The `ZoneManager` allows creation of a `TimeZone` using the fully qualified
zone name:

```C++
ExtendedZoneProcessorCache<CACHE_SIZE> zoneProcessorCache;
ExtendedZoneManager zoneManager(
    zonedbx::kZoneAndLinkRegistrySize,
    zonedbx::kZoneAndLinkRegistry,
    zoneProcessorCache);

void someFunction() {
  TimeZone tz = zoneManager.createForZoneName("America/Los_Angeles");
  if (tz.isError()) {
    // handle error
  }
  ...
}
```

Of course, you probably wouldn't actually do this because the same functionality
could be done more efficiently using the `createForZoneInfo()` like this:

```C++
ExtendedZoneProcessorCache<CACHE_SIZE> zoneProcessorCache;
ExtendedZoneManager zoneManager(
    zonedbx::kZoneAndLinkRegistrySize,
    zonedbx::kZoneAndLinkRegistry,
    zoneProcessorCache);

void someFunction() {
  TimeZone tz = zoneManager.createForZoneInfo(zonedb::kZoneAmerica_Los_Angeles);
  ...
}
```

I think the only time the `createForZoneName()` would be necessary is when the
user is allowed to type in the zone name, or the timezone name is provided by an
outside source (e.g. text of date-time components) and the `TimeZone` needs to
be created from the user-provided string.

<a name="CreateForZoneId"></a>
#### createForZoneId()

Each zone in the `zonedb::` and `zonedbx::` database is given a unique
and stable zoneId. There are at least 3 ways to extract this zoneId:

* the `kZoneId{zone name}` constants in `src/zonedb/zone_infos.h`,
  `src/zonedbx/zone_infos.h`, and `src/zonedbc/zone_infos.h`:
    * `const uint32_t kZoneIdAmerica_New_York = 0x1e2a7654; // America/New_York`
    * `const uint32_t kZoneIdAmerica_Los_Angeles = 0xb7f7e8f2; // America/Los_Angeles`
    * ...
* the `TimeZone::getZoneId()` method:
    * `uint32_t zoneId = tz.getZoneId();`
* the `ZoneInfo` pointer using the `BasicZone()`, `ExtendedZone`, and
  `CompleteZone` helper object:
    * `uint32_t zoneId = BasicZone(&zonedb::kZoneAmerica_Los_Angeles).zoneId();`
    * `uint32_t zoneId = ExtendedZone(&zonedbx::kZoneAmerica_Los_Angeles).zoneId();`
    * `uint32_t zoneId = CompleteZone(&zonedbc::kZoneAmerica_Los_Angeles).zoneId();`

The `ZoneManager::createForZoneId()` method returns the `TimeZone` object
corresponding to the given `zoneId`:

```C++
ExtendedZoneProcessorCache<CACHE_SIZE> zoneProcessorCache;
ExtendedZoneManager zoneManager(
    zonedbx::kZoneAndLinkRegistrySize,
    zonedbx::kZoneAndLinkRegistry,
    zoneProcessorCache);

void someFunction() {
  TimeZone tz = zoneManager.createForZoneId(kZoneIdAmerica_New_York);
  if (tz.isError() {
    // handle error
  }
  ...
}
```

If the `ZoneManager` cannot find the `zoneId` in its internal zone registry,
then the `TimeZone::forError()` is returned. The application developer should
check for this, and substitute a reasonable default TimeZone when this happens.
This situation is not unique to the zoneId. The same problem would occur if the
fully qualified zone name was used.

The ZoneId is created using a hash of the fully qualified zone name. It is
guaranteed to be unique and stable by the `tzcompiler.py` tool that generated
the `zonedb::` and `zonedbx::` data sets.  By "unique", I mean that
no 2 time zones will have the same zoneId. By "stable", it means that once
a zoneId has been assigned to a fully qualified zone name, it will remain
unchanged forever in the database.

The `zoneId` has an obvious advantage over the fully qualified `zoneName` for
storage purposes. It is far easier to save a 4-byte zoneId (e.g. `0xb7f7e8f2`)
rather than a variable length string (e.g. `"America/Los_Angeles"`).
Since the `zoneId` is derived from just the zoneName, a `TimeZone` created by
the `BasicZoneManager` has the same zoneId as one created using the
`ExtendedZoneManager` if it has the same name. This means that a TimeZone can be
saved using a `BasicZoneManager` but recreated using an `ExtendedZoneManager`. I
am not able to see how this could be an issue, but let me know if you find this
to be a problem.

Another useful feature of `ZoneManager::createForZoneId()` over
`createForZoneInfo()` is that `createForZoneId()` lives at the root
`ZoneManager` interface. In contrast, there are 2 different versions of
`createForZoneInfo()` which live in the corresponding ZoneManager implementation
classes because each version needs a different `ZoneInfo` type
(`basic::ZoneInfo` and `extended::ZoneInfo`). If your code has a reference or
pointer to the top-level `ZoneManager` interface, then it will be far easier to
create a `TimeZone` using `createForZoneId()`. You do pay a penalty in
efficiency because `createForZoneId()` must scan the database, where as
`createForZoneInfo()` does not perform a search since it has direct access to
the `ZoneInfo` data structure.

<a name="CreateForZoneIndex"></a>
#### createForZoneIndex()

The `ZoneManager::createForZoneIndex()` creates a `TimeZone` from its integer
index into the Zone registry, from 0 to `registrySize - 1`. This is useful when
you want to show the user with a menu of zones from the `ZoneManager` and allow
the user to select one of the options.

The `ZoneManager::indexForZoneName()` and `ZoneManager::indexForZoneId()` are
two useful methods to convert an arbitrary time zone reference (either
by zoneName or zoneId) into an index into the registry.

<a name="CreateForTimeZoneData"></a>
#### createForTimeZoneData()

The `ZoneManager::createForTimeZoneDAta()` creates a `TimeZone` from an instance
of `TimeZoneData`. The `TimeZoneData` can be retrieved from
`TimeZone::toTimeZoneData()` method. It contains the minimum set of identifiers
of a `TimeZone` object in a format that can be serialized easily, for example,
to EEPROM.

<a name="ManualZoneManager"></a>
#### ManualZoneManager

The `ManualZoneManager` is a type of `ZoneManager` that implements only the
`createForTimeZoneData()` method, and handles only `TimeZoneData::kTypeManual`.
In other words, it can only create `TimeZone` objects with fixed standard and
DST offsets.

This class reduces the amount of conditional code (using `#if` statements)
needed in applications which are normally targeted to use `BasicZoneManager`,
`ExtendedZoneManager`, or `CompleteZoneManager`, but are sometimes targeted to
small-memory microcontrollers (typically AVR chips), for testing purposes for
example. This class allows many of the function and constructor signatures to
remain the same, reducing the amount of conditional code.

If an application is specifically targeted to a low-memory chip, and it is known
at compile-time that only `TimeZone::kTypeManual` are supported, then you should
not need to use the `ManualZoneManager`. You can use `TimeZone::forTimeOffset()`
factory method directory.

<a name="HandlingGapsAndOverlaps"></a>
### Handling Gaps and Overlaps

Better control over DST gaps and overlaps was added using the techniques
described by the [PEP 495](https://www.python.org/dev/peps/pep-0495/) document
in Python 3.6.

1) An additional parameter called `fold` was added to the `LocalTime`,
   `LocalDateTime`, `OffsetDateTime`, and `ZonedDateTime` classes.
2) Support for the `fold` parameter was added to `ExtendedZoneProcessor` and
   `CompleteZoneProcessor`.
3) The `BasicZoneProcessor` does *not* support the `fold` parameter and will
   ignore it.

<a name="ProblemsWithGapsAndOverlaps"></a>
#### Problems with Gaps and Overlaps

As a quick background, when a timezone changes its DST offset in the spring or
fall, it creates either a gap (the UTC offset increases by one hour), or an
overlap (the UTC offset decreases by one hour) in the local representation of
the time. For example, in the "America/Los_Angeles" timezone (aka "US/Pacific"),
the wall clock goes from 2am to 3am in the spring (spring forward) and goes back
from 2am to 1am in the fall (fall back). In the spring, there are local time
instances which are illegal because they never existed, and in the fall,
there are local time instances which occur twice.

Different date-time libraries in different languages handle these situations
slightly differently. For example,

* [Java 11 java.time package](https://docs.oracle.com/en/java/javase/11/docs/api/java.base/java/time/chrono/ChronoLocalDate.html)
    * returns the `ZonedDateTime` shifted forward by one hour during a gap
    * returns the earlier `ZonedDateTime` during an overlap
    * choices offered with additional methods:
        * `ZonedDateTime.withEarlierOffsetAtOverlap()`
        * `ZonedDateTime.withLaterOffsetAtOverlap()`
* [C++ Hinnant date library](https://howardhinnant.github.io/date/tz.html)
    * throws an exception in a gap or overlap if a specifier `choose::earliest`
      or `choose::latest` is not specified
* [Noda Time](https://nodatime.org/3.0.x/api/NodaTime.ZonedDateTime.html)
    * throws `AmbiguousTimeException` or `SkippedTimeException`
    * can specify an `Offset` to `ZonedDateTime` class to resolve ambiguity
* [Python datetime](https://docs.python.org/3/library/datetime.html)
    * uses a `fold` parameter to resolve ambiguous or non-existent time
    * see [PEP 495](https://www.python.org/dev/peps/pep-0495/)

The AceTime library cannot use exceptions because the Arduino C++ environment
does not support exceptions. I chose to follow the techniques described by
Python PEP 495 because it is well-documented, easy to understand, and relatively
simple to implement.

<a name="ClassesWithFold"></a>
#### Classes with Fold

An optional `fold` parameter was added to various constructors and factory
methods. The default is `fold=0` if not specified. The `fold()` accessor and
mutator methods were added to the various classes as well.

```C++
namespace ace_time {

class LocalTime {
  public:
    static LocalTime forComponents(uint8_t hour, uint8_t minute,
        uint8_t second, uint8_t fold = 0);

    uint8_t fold() const;
    void fold(uint8_t fold);

    [...]
};

class LocalDateTime {
  public:
    static LocalDateTime forComponents(int16_t year, uint8_t month,
        uint8_t day, uint8_t hour, uint8_t minute, uint8_t second,
        uint8_t fold = 0);

    uint8_t fold() const;
    void fold(uint8_t fold);

    [...]
};

class OffsetDateTime {
  public:
    static OffsetDateTime forComponents(int16_t year, uint8_t month,
        uint8_t day, uint8_t hour, uint8_t minute, uint8_t second,
        TimeOffset timeOffset, uint8_t fold = 0) {

    uint8_t fold() const;
    void fold(uint8_t fold);

    [...]
};

class ZonedDateTime {
  public:
    static ZonedDateTime forComponents(int16_t year, uint8_t month, uint8_t day,
        uint8_t hour, uint8_t minute, uint8_t second,
        const TimeZone& timeZone, uint8_t fold = 0) {

    uint8_t fold() const;
    void fold(uint8_t fold);

    [...]
};

}
```

<a name="FactoryMethodsWithFold"></a>
#### Factory Methods with Fold

There are 2 main factory methods on `ZonedDateTime`: `forEpochSeconds()` and
`forComponents()`. The `fold` parameter is an *output* parameter for
`forEpochSeconds()`, and an *input* parameter for `forComponents()`. The
mapping functionality of these methods are described in detail in the PEP 495
document, but here is an ASCII diagram for reference:

```
              ^
LocalDateTime |
              |                         (overlap)   /
          2am |                             /|    /
              |                           /  |  /
          1am |                         /    |/
              |          /
              |        /
          3am |       |
              |  (gap)|
          2am |       |
              |      /
              |    /
              |  /
              +----------------------------------------->
              spring-forward            fall-backward

                          UTC/epochSeconds
```

The `forEpochSeconds()` takes the UTC/epochSeconds value and maps it to the
LocalDateTime axis. It is a single-valued function which is defined for all
values of epochSeconds, even with a DST shift forward or backward. The `fold`
parameter is an *output* of the `forEpochSeconds()` function. During an overlap,
a `ZonedDataTime` can occur twice. The earlier occurrence is returned with
`fold==0`, and the later occurrence is returned with `fold==1`. For all other
cases where there is only a unique occurrence, the `fold` parameter is set to 0.

The `forComponents()` takes the LocalDateTime value and maps it to the
UTC/epochSeconds axis. During a gap, there are certain LocalDateTime
components which do not exist and are illegal. During an overlap, there are 2
epochSeconds which can correspond to the given LocalDateTime. The `fold`
parameter is an *input* parameter to the `forComponents()` in both cases.
The impact of the `fold` parameter is as follows:

**Normal**: Not a gap or overlap. The `forComponents()` method ignores the
`fold` parameter if there is no ambiguity about the local date-time components.
The returned `ZonedDateTime` object will contain a `fold()` value that preserves
the input `fold` parameter.

**Overlap**: If `ZonedDateTime::forComponents()` is called with during an
overlap of `LocalDateTime` (e.g. 2:30am during a fall back from 2am to 3am), the
factory method uses the user-provided `fold` parameter to select the following:

* `fold==0`
    * Selects the *earlier* Transition element and returns the earlier
      LocalDateTime.
    * So 01:30 is interpreted as 01:30-07:00.
* `fold==1`
    * Selects the *later* Transition element and returns the later
      LocalDateTime.
    * So 01:30 is interpreted as 01:30-08:00.

**Gap**: If `ZonedDateTime::forComponents()` is called with a `LocalDateTime`
that does not exist (e.g. 2:30am during a spring forward night from 2am to 3am),
the factory method *normalizes* the resulting `ZonedDateTime` object so that the
components of the object are legal. The algorithm for normalization depends on
the `fold` parameter. The `2:30am` value becomes either `3:30am` or `1:30am` in
the following, and perhaps counter-intuitive, manner:

* `fold==0`
    * Selects the *earlier* Transition element, extended forward to apply to the
      given LocalDateTime,
    * Which maps to the *later* UTC/epochSeconds,
    * Which becomes normalized to the *later* ZonedDateTime which has the
      *later* UTC offset.
    * So 02:30 is interpreted as 02:30-08:00, which is normalized to
      03:30-07:00, and the `fold` after normalization is set to 1 to indicate
      that the later transition was selected.
* `fold==1`
    * Selects the *later* Transition element, extended backward to apply to the
      given LocalDateTime,
    * Which maps to the *earlier* UTC/epochSeconds,
    * Which becomes normalized to the *earlier* ZonedDateTime which has the
      *earlier* UTC offset.
    * So 02:30 is interpreted as 02:30-07:00, which is normalized to
      01:30-08:00, and the `fold` after normalization is set to 0 to indicate
      that the earlier transition was selected.

The time shift during a gap seems to be the *opposite* of the shift during an
overlap, but internally this is self-consistent. Just as importantly, this
follows the same logic as PEP 495.

Note that the `fold` parameter flips its value (from 0 to 1, or vise versea) if
`forComponents()` is called in the gap. Currently, this is the only publicly
exposed mechanism for detecting that a given date-time is in the gap.

<a name="SemanticChangesWithFold"></a>
#### Semantic Changes with Fold

The `fold` parameter has no effect on most existing methods. It is ignored in
all comparison operators:

* `operator==()`, `operator!=()` ignore the `fold`
* `operator<()`, `operator>()`, etc. ignore the `fold`
* `compareTo()` ignores the `fold`

It impacts the behavior the factory methods of `LocalTime`, `LocalDateTime`,
`OffsetDateTime` only trivially, causing the `fold` value to be passed into the
internal holding variable:

* `LocalTime::forSeconds()`
* `LocalTime::forComponents()`
* `LocalDateTime::forEpochSeconds()`
* `LocalDateTime::forComponents()`
* `OffsetDateTime::forEpochSeconds()`
* `OffsetDateTime::forComponents()`

The `fold` parameter has significant impact only on the `ZonedDateTime` factory
methods, and only if the `ExtendeZoneProcessor` is used:

* `ZonedDateTime::forEpochSeconds()`
* `ZonedDateTime::forComponents()`

The `fold` parameter is not exposed through any of the existing `printTo()` and
`printShortTo()` methods. It can only be accessed and changes by the `fold()`
accessor and mutator methods.

A more subtle, but important semantic change, is that the `fold` parameter
preserves information during gaps and overlaps. This means that we can do
round-trip conversions of `ZonedDateTime` properly. We can start with
epochSeconds, convert to components, then back to epochSeconds, and get back the
same epochSeconds. Without the `fold` parameter, this round-trip was not
guaranteed during an overlap.

<a name="ResourceConsumptionWithFold"></a>
#### Resource Consumption with Fold

According to [MemoryBenchmark](examples/MemoryBenchmark), adding support for
`fold` increased flash usage of `ExtendedZoneProcessor` by about 600 bytes on
AVR processors and 400-600 bytes on 32-bit processors. (The `BasicZoneProcessor`
which ignores the `fold` parameter increased by ~150 bytes on AVR processors,
because of the overhead of copying the internal `fold` parameter in various
objects.) The static memory footprint of various classes increased by one byte
on AVR processors, and 2-4 bytes on 32-bit processors due to 32-bit alignment.

According to [AutoBenchmark](examples/AutoBenchmark), the performance of various
functions did not change at all, except for `ZonedDataTime::forComponents()`,
which became 5X *faster* on AVR processors and 1.5-3X faster on 32-bit
processors. This is because the `fold` parameter tells us exactly when the
internal normalization process is required, which allows us to skip the
normalization step in the common case outside the gap. Within the gap, the
`forComponents()` method performs about the same as before.

<a name="ExamplesWithFold"></a>
#### Examples with Fold

Here are some examples taken from
[ZonedDateTimeExtendedTest](tests/ZonedDateTimeExtendedTest):

```C++
ExtendedZoneProcessorCache<1> zoneProcessorCache;
ExtendedZoneManager extendedZoneManager(
    zonedbx::kZoneAndLinkRegistrySize,
    zonedbx::kZoneAndLinkRegistry,
    zoneProcessorCache);
TimeZone tz = extendedZoneManager.createForZoneInfo(
    &zonedbx::kZoneAmerica_Los_Angeles);

// During fall back. This is the second occurrence of this local time, so should
// print:
// 2022-11-06T01:29:00-08:00[America/Los_Angeles]
// fold=1
acetime_t epochSeconds = 721042140;
auto dt = ZonedDateTime::forEpochSeconds(epochSeconds, tz);
Serial.printTo(dt); Serial.println();
Serial.print("fold="); Serial.println(dt.fold());

// During spring forward. In the gap, fold=0 selects earlier transition,
// so selects -08:00 offset, which gets normalized to -07:00, so should print:
// 2022-03-13T03:29:00-07:00[America/Los_Angeles]
dt = ZonedDateTime::forComponents(2022, 3, 13, 2, 29, 0, tz, 0 /*fold*/);
Serial.printTo(dt); Serial.println();

// During spring forward. In the gap, fold=1 selects later transition,
// so selects -07:00 offset, which gets normalized to -08:00, so should print:
// 2022-03-13T01:29:00-08:00[America/Los_Angeles]
dt = ZonedDateTime::forComponents(2022, 3, 13, 2, 29, 0, tz, 1 /*fold*/);
Serial.printTo(dt); Serial.println();
```

<a name="ZoneInfoDatabase"></a>
## ZoneInfo Database

<a name="ZoneInfoRecords"></a>
### ZoneInfo Records

The data structures that describe the zoneinfo database are in
[src/zoneinfo](src/zoneinfo) directory. The exact nature of how the zoneinfo
files are stored and retrieved is an implementation detail that is subject to
periodic improvements. The earliest versions stored the zoneinfo database in
memory, which quickly overwhelmed the limited static memory size (e.g. 2kB) of
8-bit processors. Currently, the zone database entries are stored in in flash
memory instead of static RAM using the
[PROGMEM](https://www.arduino.cc/reference/en/language/variables/utilities/progmem/)
keyword on microcontrollers which support this feature (AVR, ESP8266).

The following classes represent the various objects stored in `PROGMEM`, and are
defined in the `zoneinfo/ZoneInfoXxx.h` header files:

* `ZoneContext`
* `ZoneRule`
* `ZonePolicy`: referencing a collection of `ZoneRule`
* `ZoneEra`
* `ZoneInfo`: referencing a collection of `ZoneEra`

In v2.3, three versions of these ZoneInfo records were created, to support the 3
different zonedb types:

* `ace_time::basic::ZoneXxx` - used with `BasicZoneProcessor`
* `ace_time::extended::ZoneXxx` - used with `ExtendedZoneProcessor`
* `ace_time::complete::ZoneXxx` - used with `CompleteZoneProcessor`

Information stored in `PROGMEM` must be retrieved using special functions (e.g.
`pgm_read_byte()`, `pgm_read_word()`, etc). A thin layer of indirection is
provided to hide the implementation details of these access functions. The
abstraction layer is provided by `zoneinfo/Brokers.h`:

* `ZoneContextBroker`
* `ZoneRuleBroker`
* `ZonePolicyBroker`
* `ZoneEraBroker`
* `ZoneInfoBroker`
* `ZoneRegistryBroker`
* `ZoneInfoStore` - a factory of `ZoneInfoBroker`

There are 3 sets of these broker classes, duplicated into 2 different C++
namespaces:

* `ace_time::basic::ZoneXxxBroker`
* `ace_time::extended::ZoneXxxBroker`
* `ace_time::complete::ZoneXxxBroker`

The separate namespaces allows compile-time verification that the correct
`zonedb*` database is used with the correct `BasicZoneProcessor`,
`ExtendedZoneProcessor`, or `CompleteZoneProcessor`.

<a name="ZoneDB"></a>
### ZoneDB

There are 6 zonedb databases provided in this library. Three are meant for
general consumption:

* `zonedb` for `BasicZoneProcessor`
* `zonedbx` for `ExtendedZoneProcessor`
* `zonedbc` for `CompleteZoneProcessor`

These 3 are meant for unit tests:

* `zonedbtesting` for `BasicZoneProcessor`
* `zonedbxtesting` for `ExtendedZoneProcessor`
* `zonedbctesting` for `CompleteZoneProcessor`

<a name="BasicZonedb"></a>
#### Basic zonedb

The `zonedb/` database is intended to contain timezones which are compatible
with the `BasicZoneProcessor` and `BasicZoneManager` classes. The database
format is optimized for small size, at the expense of excluding some timezones
with complex DST saving rules. If a zone is excluded, the reason for the
exclusion can be found at the bottom of the
[zonedb/zone_infos.h](src/zonedb/zone_infos.h) file. The criteria for selecting
the Basic `zonedb` entries are embedded in the `transformer.py` script and
summarized in [BasicZoneProcessor.h](src/ace_time/BasicZoneProcessor.h):

* the year fields are stores as 8-bit integer offsets (from a base year of 2100)
  instead of using the full 16-bit integer
    * this limits the year range to `[1974,2225]`
* the DST offset is a multiple of 15-minutes ranging from -1:00 to 2:45
* the STDOFF offset is a multiple of 1-minute (all timezones after year 2000
  satisfy this)
* the AT or UNTIL fields must occur at one-year boundaries (this is the biggest
  filter)
* the LETTER field can be an arbitrary string
* the UNTIL time suffix can only be 'w' (not 's' or 'u')
* there can be only one DST transition in a single month

As of version v2.3 (with TZDB 2022c), the `zonedb` database contains 446 Zones
and Links (out of a total of 596), supporting the years roughly `[2000,2200]`.

<a name="ExtendedZonedbx"></a>
#### Extended zonedbx

The goal of the `zonedbx/` database is to support all zones listed in the TZ
Database for modern years using the `ExtendedZoneProcessor` and
`ExtendedZoneManager` classes. The year range is restricted to `[2000,32765]`.

* the year fields are stores as 8-bit integer offsets (from a base year of 2100)
  instead of using the full 16-bit integer
    * this limits the year range to `[1974,2225]`
* the DST offset is a multiple of 15-minutes ranging from -1:00 to 2:45
  (all timezones from about 1972 support this)
* the STDOFF offset is a multiple of 1-minute (all timezones after year 2000
  satisfy this)
* the AT and UNTIL fields are multiples of 1-minute
* the UNTIL time suffix can be 'w', 's, or 'u'
* the LETTER field can be an arbitrary string

As of version v2.3 (with TZDB 2022c), the `zonedbx` database contains all 596
Zones and Links, over all years in the range of `[2000,32765]`.

<a name="CompleteZonedbc"></a>
#### Complete zonedbc

The goal of the `zonedbc/` database is to support all zones listed in the TZ
Database, for all years in that database, from 1844 onwards without limit,
using the `CompleteZoneProcessor` and the `CompleteZoneManager` classes. This is
the largest of the 3 zonedb databases. Its features are:

* the year fields are stored as 16-bit signed integers, which allows any year
  from `[-32767,32765]` (-32768, +32766, +32767 are used for internal purposes)
* the DST offset can be a multiple of 1-minute, which is satisfied by all
  timezones across all years
* the STDOFF ofset can be an arbitrary multiple of 1-second
* the AT and UNTIL fields can be an arbitrary multiple of 1-second
* the UNTIL time suffix can be 'w', 's, or 'u'
* the LETTER field can be an arbitrary string

As of version v2.3 (with TZDB 2023c), the `zonedbc` database contains all 596
Zone and Link entries, supporting all years in the range of `[0001,10000)`.

<a name="TzDatabaseVersion"></a>
#### TZ Database Version

The IANA TZ Database is updated continually. As of this writing, the latest
stable version is 2023c. When a new version of the database is released, I
regenerate the ZoneInfo entries under the `src/zonedXxx/` directories.

The current TZ Database version can be programmatically accessed using the
`kTzDatabaseVersion` constant:

```C++
#include <AceTime.h>
using namespace ace_time;

void printVersionTzVersions() {
    Serial.print("zonedb TZ version: ");
    Serial.println(zonedb::kTzDatabaseVersion); // e.g. "2023c"

    Serial.print("zonedbx TZ version: ");
    Serial.println(zonedbx::kTzDatabaseVersion); // e.g. "2023c"

    Serial.print("zonedbc TZ version: ");
    Serial.println(zonedbc::kTzDatabaseVersion); // e.g. "2023c"
}
```

It is theoretically possible for the 3 versions to be different, but since they
are generated by the same set of scripts, I expect they will always be the same.

<a name="ZoneInfoYearRange"></a>
#### ZoneInfo Year Range

The zonedb databases are generated with a specific requested `startYear` and
`untilYear` range, which filters out DST transitions rules before `startYear`
and after `untilYear`. Currently (as of 2023c) the first transition in the IANA
TZ database is in 1844 and the last transition in 2087. If the `startYear` is
before 1844, then the AceTime library will be accurate for all years down to the
year 0001. Similarly, if the requested `untilYear` is after 2087, then the
library will be accurate for all years up to year 10000.

To convey the actual range of years for which the library is accurate, 2 new
parameters were added in v2.3: `startYearAccurate` and `untilYearAccurate`. The
`[startYearAccurate,untilYearAccurate)` range determines the years which the
database is known to be accurate. The value of `kMinYear (-32767)` means
`-Infinity`, and the value of `kMaxUntilYear (+32767)`means `+Infinity`.

The limit of `[0001,10000)` is imposed by the `LocalDate` class for practical
reasons to limit the number of digits in a year to 4-digits, and because certain
internal algorithms do not work for negative years.

It is possible to access the various `startYear` and `untilYear` of the zonedb
databases through the `ZoneContextBroker` class, but this is not something that
was intended to be used by the client application:

```C++
#include <AceTime.h>
using namespace ace_time;

void printStartAndUntilYears() {
    ace_time::extended::ZoneInfoBroker info(&zonedbx::kZoneAmerica_Los_Angeles);
    extended::ZoneContextBroker context = info.zoneContext();

    Serial.print("startYear: ");
    Serial.print(context.startYear()); // e.g. 2000

    Serial.print("; untilYear: ");
    Serial.println(context.untilYear()); // e.g. 2100

    Serial.print("startYearAccurate: ");
    Serial.print(context.startYearAccurate()); // e.g. 2000

    Serial.print("; untilYearAccurate: ");
    Serial.println(context.untilYearAccurate()); // e.g. +32767
}
```

The library supports some amount of "graceful degradation". When the
`ZonedDateTime` class is created outside the
`[startYearAccurate,untilYearAccurate)` range, the datetime object is still
usable but its accuracy may be diminished if the DST transitions of the specific
timezone were filtered out of the zone database.

<a name="ExternalZone"></a>
#### External Zone Classes

The `basic::ZoneInfo`, `extended::ZoneInfo`, and `complete::ZoneInfo` objects
are meant to be used as *opaque* pointers and simply passed into the `TimeZone`
class (which in turn, passes the pointer into the corresponding `ZoneProcessor`
objects.) The internal formats of the `ZoneInfo` structures may change without
warning, and users of this library should not access its internal data members
directly.

Instead, client applications should use the `BasicZone`, `ExtendedZone`, and
`CompleteZone` classes which aims to provide stable external access to some of
the internal fields of the `ZoneInfo` class:

```C++
namespace ace_time {

class BasicZone {
  public:
    BasicZone(const basic::ZoneInfo* zoneInfo);

    bool isNull() const;
    uint32_t zoneId() const;
    TimeOffset stdOffset() const;
    ace_common::KString kname() const;

    void printNameTo(Print& printer) const;
    void printShortNameTo(Print& printer) const;
};

class ExtendedZone {
  public:
    ExtendedZone(const extended::ZoneInfo* zoneInfo);

    bool isNull() const;
    uint32_t zoneId() const;
    TimeOffset stdOffset() const;
    ace_common::KString kname() const;

    void printNameTo(Print& printer) const;
    void printShortNameTo(Print& printer) const;
}

class CompleteZone {
  public:
    CompleteZone(const complete::ZoneInfo* zoneInfo);

    bool isNull() const;
    uint32_t zoneId() const;
    TimeOffset stdOffset() const;
    ace_common::KString kname() const;

    void printNameTo(Print& printer) const;
    void printShortNameTo(Print& printer) const;
}
```

The `isNull()` method returns true if the object is a wrapper around a
`nullptr`. This is often used to indicate an error condition, or a "Not Found"
condition.

The `stdOffset()` method returns the standard (i.e. normal time, not DST
summer time) timezone offset of the zone for the last occurring `ZoneEra` record
in the database. In almost all cases, this should correspond to the current
standard timezone, unless the timezone is scheduled to changes its standard
timezone offset in the near future. The value of this method is used by the
`ZoneSorterByOffsetAndName` class when sorting timezones by its offset.

The `kname()` method returns the `KString` object representing the name of the
zone. The `KString` object represents a string that has been compressed using a
fragment dictionary. This object knows how to decompress the encoded form. This
method is used by the `ZoneSorterByName` and `ZoneSorterByOffsetAndName` classes
when sorting timezones.

The `printNameTo()` method prints the full zone name (e.g.
`America/Los_Angeles`), and `printShortNameTo()` prints only the last component
(e.g. `Los_Angeles`).

These objects are meant to be used transiently, created on the stack then thrown
away. For example:

```C++
const extended::ZoneInfo* zoneInfo = ...;
ExtendedZone(zoneInfo).printNameTo(Serial);
Serial.println();
```

These classes are light-weight wrapper objects around a `const ZoneInfo*`
pointer. In fact, they are so light-weight that the C++ compiler should be able
to optimize away both wrapper classes entirely, so that they are equivalent to
using the `const ZoneInfo*` pointer directly.

If you need to copy the zone names into memory, use the `PrintStr<N>` class from
the AceCommon library (https://github.com/bxparks/AceCommon) to print the
zone name into the memory buffer, then extract the string from the buffer:

```C++
#include <AceCommon.h>
using ace_common::PrintStr;
...

const extended::ZoneInfo* zoneInfo = ...;
PrintStr<32> printStr; // buffer of 32 bytes on the stack
ExtendedZone(zoneInfo).printNameTo(printStr);

const char* name = printStr.cstr();
// do stuff with 'name', but only while 'printStr' is alive
...
```

See also the [Print To String](#PrintToString) section below.

<a name="ZonesAndLinks"></a>
### Zones and Links

The IANA TZ database contains 2 types of timezones:

- Zones, implemented by the `ZONE` keyword
- Links, implemented by the `LINK` keyword

A Zone entry is the canonical name of a given time zone in the IANA database
(e.g. `America/Los_Angeles`). A Link entry is an alias, an alternate name, for a
canonical entry (e.g. `US/Pacific` which points to `America/Los_Angeles`).

The semantics of a Link entry in the TZDB have changed recently, so that Links
should be considered to be first class citizens compared to Zone entries. Prior
to v2.3, Link entries could be implemented as "symbolic links" to Zones. After
on or after v2.3, Link entries are now implemented as "hard links" to Zone
entries, with most of the code unaware of the difference between the two. This
simplifies the implementation code.

The `zonedbXxx/zone_registry.h` file provides 2 registries:

- `kZoneRegistry`: contains only Zones
- `kZoneAndLinkRegistry`: contains both Zones and Links

For most applications, the `kZoneAndLinkRegistry` should be used. The
`kZoneRegistry` is primarily retained for for backwards compatibility, and to
reduce the parameter space of the validation tests so that they can complete
faster.

All methods on the `TimeZone` class apply to both Zone and Link time zones. The
client application will not be able to distinguish between the two. There are 2
methods on the `TimeZone` class which apply only to Links:

```C++
class TimeZone {
  public:
    bool isLink() const;
    printTargetNameTo(Print& printer) const;
};
```

The `TimeZone::isLink()` method returns `true` if the current time zone is a
Link entry instead of a Zone entry. For example `US/Pacific` is a link to
`America/Los_Angeles`.

The `TimeZone::printTargetNameTo(Print&)` prints the name of the target zone if
the current time zone is a Link. Otherwise it prints nothing. For example, for
the time zone `US/Pacific` (which is a Link to `America/Los_Angeles`):

* `printTo(Print&)` prints "US/Pacific"
* `printTargetNameTo(Print&)` prints "America/Los_Angeles"

<a name="CustomZoneRegistry"></a>
### Custom Zone Registry

On small microcontrollers, the default zone registries (`kZoneRegistry` and
`kZoneAndLinkRegistry`) may be too large. The `ZoneManager` can be configured
with a custom zone registry. It needs to be given an array of `ZoneInfo`
pointers when constructed. For example, here is a `ExtendedZoneManager` with 4
zones from the `zonedbx::` data set:

```C++
#include <AceTime.h>
using namespace ace_time;
...
static const extended::ZoneInfo* const kCustomRegistry[] ACE_TIME_PROGMEM = {
  &zonedbx::kZoneAmerica_Los_Angeles,
  &zonedbx::kZoneAmerica_Denver,
  &zonedbx::kZoneAmerica_Chicago,
  &zonedbx::kZoneAmerica_New_York,
};

static const uint16_t kCustomRegistrySize =
    sizeof(kCustomRegistry) / sizeof(extended::ZoneInfo*);

static const uint16_t CACHE_SIZE = 2;
static ExtendedZoneProcessorCache<CACHE_SIZE> zoneProcessorCache;
static ExtendedZoneManager zoneManager(
    kCustomRegistrySize, kCustomRegistry, zoneProcessorCache);
```

The `ACE_TIME_PROGMEM` macro is defined in
[compat.h](src/ace_time/common/compat.h) and indicates whether the `ZoneInfo`
entries are stored in normal RAM or flash memory (i.e. `PROGMEM`). It **must**
be used for custom zoneRegistries because the ZoneManagers expect to find them
in static RAM or flash memory according to this macro.

An example is shown in:

* [examples/CustomZoneRegistry](examples/CustomZoneRegistry)

Various unit tests also use custom registries:

* [tests/ZoneRegistrarTest](tests/ZoneRegistrarTest)
* [tests/TimeZoneTest](tests/TimeZoneTest)
* [tests/ZonedDateTimeBasicTest](tests/ZonedDateTimeBasicTest)
* [tests/ZonedDateTimeExtendedTest](tests/ZonedDateTimeExtendedTest)
* [tests/ZonedDateTimeCompleteTest](tests/ZonedDateTimeCompleteTest)

(**TBD**: I think it would be useful to create a script that can generate the
C++ code representing these custom zone registries from a list of zones.)

(**TBD**: It might also be useful for app developers to create custom datasets
with different range of years. The tools are all here, but not explicitly
documented currently. Examples of how to this do exist inside the various
`Makefile` files in the AceTimeValidation project.)

<a name="ZoneSorting"></a>
## Zone Sorting

When a client application supports only a handful of zones in the `ZoneManager`,
the order in which the zones are presented to the user may not be too important
since the user can scan the entire list quickly. But when the `ZoneManager`
contains the entire zoneInfo database (up to 594 zones and links), it becomes
useful to sort the zones in a predictable way before showing them to the user.

The `ZoneSorterByName` and `ZoneSorterByOffsetAndName` are 2 classes which can
sort the list of zones. They look like this:

```C++
namespace ace_time {

template <typename ZM>
class ZoneSorterByName {
  public:
    ZoneSorterByName(const ZM& zoneManager);

    void fillIndexes(uint16_t indexes[], uint16_t size) const;

    void sortIndexes(uint16_t indexes[], uint16_t size) const;
    void sortIds(uint32_t ids[], uint16_t size) const;
    void sortNames(const char* names[], uint16_t size) const;
};

template <typename ZM>
class ZoneSorterByOffsetAndName {
  public:
    ZoneSorterByOffsetAndName(const ZM& zoneManager);

    void fillIndexes(uint16_t indexes[], uint16_t size) const;

    void sortIndexes(uint16_t indexes[], uint16_t size) const;
    void sortIds(uint32_t ids[], uint16_t size) const;
    void sortNames(const char* names[], uint16_t size) const;
};

}
```

The `ZoneSorterByName` class sorts the given zones in ascending order by the
zone's name. The `ZoneSorterByOffsetAndName` class sorts the zones by its UTC
offset during standard time, then by the zone's name within the same UTC offset.
Both of these are templatized on the `BasicZoneManager` or the
`ExtendedZoneManager` classes because they require the methods implemented by
those classes. The ZoneSorter classes will not compile if the
`ManualZoneManager` class is given because it does not make sense.

To use these classes, the calling client should follow these steps:

1) Wrap an instance of a `ZoneSorterByName` class or `ZoneSorterByOffsetAndName`
   class around the `ZoneManager` class.
2) Create an array filled in the following manner:
    * an `indexes[]` array filled with the index into the `zoneRegistry`; or
    * an `ids[]` array filled with the 32-bit zone id; or
    * a `names[]` array filled with the `const char*` string of the zone name.
3) Call one of the `sortIndexes()`, `sortIds()`, or `sortNames()` methods of the
  `ZoneSorter` class to sort the array.

The code will look like this:

```C++
#include <AceTime.h>

using namespace ace_time;
using namespace ace_time::zonedbx;

ExtendedZoneProcessorCache<1> zoneProcessorCache;
ExtendedZoneManager zoneManager(
  zonedbx::kZoneAndLinkRegistrySize,
  zonedbx::kZoneAndLinkRegistry,
  zoneProcessorCache
);

// Print each zone in the form of:
// "UTC-08:00 America/Los_Angeles"
// "UTC-07:00 America/Denver"
// [...]
void printZones(uint16_t indexes[], uint16_t size) {
  for (uint16_t i = 0; i < size; i++) {
    ExtendedZone zone = zoneManager.getZoneForIndex(indexes[i]);
    TimeOffset stdOffset = TimeOffset::forMinutes(zone.stdOffsetMinutes());

    // Print "UTC-08:00 America/Los_Angeles".
    SERIAL_PORT_MONITOR.print(F("UTC"));
    stdOffset.printTo(SERIAL_PORT_MONITOR);
    SERIAL_PORT_MONITOR.print(' ');
    zone.printNameTo(SERIAL_PORT_MONITOR);
    SERIAL_PORT_MONITOR.println();
  }
}

void sortAndPrintZones() {
  // Create the indexes[kZoneAndLinkRegistrySize] on the stack. This has 594
  // elements as of TZDB 2022a, so this requires a microcontroller which can
  // support at least 1188 bytes on the stack.
  uint16_t indexes[zonedbx::kZoneAndLinkRegistrySize];

  // Create the sorter.
  ZoneSorterByOffsetAndName<ExtendedZoneManager> zoneSorter(zoneManager);

  // Fill the array with indexes from 0 to 593.
  zoneSorter.fillIndexes(indexes, zonedbx::kZoneAndLinkRegistrySize);

  // Sort the indexes.
  zoneSorter.sortIndexes(indexes, zonedbx::kZoneAndLinkRegistrySize);

  // Print in human readable form.
  printZones(indexes, zonedbx::kZoneAndLinkRegistrySize);
}
```

The `fillIndexes(uint16_t indexes[], uint16_t size)` method is a convenience
method that fills the given `indexes[]` array from `0` to `size-1`, so that it
can be sorted according to the specified sorting order. In other words, it is a
short hand for:

```C++
for (uint16_t i = 0; i < size; i++ ) {
  indexes[i] = i;
}
```

See [examples/ListZones](examples/ListZones) for more examples.

The calling code can choose to sort only a subset of the zones registered into
the `ZoneManager`. In the following example, 4 zone ids are placed into an array
of 4 slots, then sorted by offset and name:

```C++
#include <AceTime.h>

using namespace ace_time;

ExtendedZoneProcessorCache<1> zoneProcessorCache;
ExtendedZoneManager zoneManager(
  zonedbx::kZoneAndLinkRegistrySize,
  zonedbx::kZoneAndLinkRegistry,
  zoneProcessorCache
);

uint32_t zoneIds[4] = {
  zonedbx::kZoneIdAmerica_Los_Angeles,
  zonedbx::kZoneIdAmerica_New_York,
  zonedbx::kZoneIdAmerica_Denver,
  zonedbx::kZoneIdAmerica_Chicago,
};

void sortIds() {
  ZoneSorterByOffsetAndName<ExtendedZoneManager> zoneSorter(zoneManager);
  zoneSorter.sortIds(zoneIds, 4);
  ...
}
```

<a name="PrintToString"></a>
## Print To String

Many classes provide a `printTo(Print&)` method which prints a human-readable
string to the given `Print` object. Any subclass of the `Print` class can be
passed into these methods. The most familiar is the global the `Serial` object
which prints to the serial port.

The AceCommon library (https://github.com:bxparks/AceCommon) provides a
subclass of `Print` called `PrintStr` which allows printing to an in-memory
buffer. The contents of the in-memory buffer can be retrieved as a normal
c-string using the `PrintStr::cstr()` method.

Instances of the `PrintStr` object is expected to be created on the stack. The
object will be destroyed automatically when the stack is unwound after returning
from the function where this is used. The size of the buffer on the stack is
provided as a compile-time constant. For example, `PrintStr<32>` creates an
object with a 32-byte buffer on the stack.

An example usage looks like this:

```C++
#include <AceCommon.h>
#include <AceTime.h>

using ace_common::PrintStr;
using namespace ace_time;
...
{
  TimeZone tz = TimeZone::forTimeOffset(TimeOffset::forHours(-8));
  ZonedDateTime dt = ZonedDateTime::forComponents(
      2018, 3, 11, 1, 59, 59, tz);

  PrintStr<32> printStr; // 32-byte buffer
  dt.printTo(printStr);
  const char* cstr = printStr.cstr();

  // do stuff with cstr...

  printStr.flush(); // needed only if this will be used again
}
```

<a name="Mutations"></a>
## Mutations

Mutating the date and time classes can be tricky. In fact, many other
time libraries (such as [Java 11
Time](https://docs.oracle.com/en/java/javase/11/docs/api/java.base/java/time/package-summary.html),
[Joda-Time](https://www.joda.org/joda-time/), and [Noda
Time](https://nodatime.org/)) avoid the problem altogether by making all objects
immutable. In those libraries, mutations occur by creating a new copy of the
target object with a new value for the mutated parameter. Making the objects
immutable is definitely cleaner, but it causes the code size to increase
significantly. For the case of the
[WorldClock](https://github.com/bxparks/clocks/tree/master/WorldClock) program,
the code size increased by 500-700 bytes, which I could not afford because the
program takes up almost the entire flash memory of an Arduino Pro Micro with
only 28672 bytes of flash memory.

Most date and time classes in the AceTime library are mutable. Except for
primitive mutations of setting specific fields (e.g.
`ZonedDateTime::year(uint16_t)`), most higher-level mutation operations are not
implemented within the class itself to avoid bloating the class API surface. The
mutation functions live as functions in separate namespaces outside of the class
definitions:

* `time_period_mutation.h`
* `time_offset_mutation.h`
* `local_date_time_mutation.h`
* `offset_date_time_mutation.h`
* `zoned_date_time_mutation.h`

Additional mutation operations can be written by the application developer and
added into the *same* namespace, since C++ allows things to be added to a
namespace multiple times.

Most of these mutation functions were created to solve a particular UI problem
in my various clock applications. In those clocks, the user is provided an OLED
display and 2 buttons. The user can change the time by long-pressing
the Select button. One of the components of the date or time will blink. The
user can press the other Change button to increment the component. Pressing the
Select button will move the blinking cursor to the next field. After all the
fields have been set, the user can long-press the Select button again to save
the new date and time into the `SystemClock`.

The mutation functions directly manipulate the underlying date and time
components of `ZonedDateTime` and other target classes. No validation rules are
applied. For example, the `zoned_date_time_mutation::incrementDay()` method will
increment the `ZonedDateTime::day()` field from Feb 29 to Feb 30, then to Feb
31, then wrap around to Feb 1. The object will become normalized when it is
converted into an Epoch seconds (using `toEpochSeconds()`), then converted back
to a `ZonedDateTime` object (using `forEpochSeconds()`). By deferring this
normalization step until the user has finished setting all the clock fields, we
can reduce the size of the code in flash. (The limiting factor for many 8-bit
Arduino environments is the code size, not the CPU time.)

Mutating the `ZonedDateTime` requires calling the `ZonedDateTime::normalize()`
method after making the changes. See the subsection on [ZonedDateTime
Normalization](#ZonedDateTimeNormalization) below.

It is not clear that making the AceTime objects mutable was the best design
decision. But it seems to produce far smaller code sizes (hundreds of bytes of
flash memory saved for something like
[WorldClock](https://github.com/bxparks/clocks/tree/master/WorldClock)), while
providing the features that I need to implement the various Clock applications.

<a name="TimeOffsetMutations"></a>
### TimeOffset Mutations

The `TimeOffset` object can be mutated with:

```C++
namespace ace_time {

void setMinutes(int16_t minutes) {

namespace time_offset_mutation {

void increment15Minutes(TimeOffset& offset);

}
}
```

<a name="LocalDateMutations"></a>
### LocalDate Mutations

The `LocalDate` object can be mutated with the following methods and functions:

```C++
namespace ace_time {

void LocalDate::year(int16_t year);
void LocalDate::month(uint8_t month);
void LocalDate::day(uint8_t month);

namespace local_date_mutation {

void incrementOneDay(LocalDate& ld);
void decrementOneDay(LocalDate& ld);

}
}
```

<a name="OffsetDateTimeMutations"></a>
### OffsetDateTime Mutations

The `OffsetDateTime` object can be mutated using the following methods and
functions:

```C++
namespace ace_time {

void OffsetDateTime::year(int16_t year);
void OffsetDateTime::month(uint8_t month);
void OffsetDateTime::day(uint8_t month);
void OffsetDateTime::hour(uint8_t hour);
void OffsetDateTime::minute(uint8_t minute);
void OffsetDateTime::second(uint8_t second);
void OffsetDateTime::timeZone(const TimeZone& timeZone);

namespace zoned_date_time_mutation {

void incrementYear(OffsetDateTime& dateTime);
void incrementMonth(OffsetDateTime& dateTime);
void incrementDay(OffsetDateTime& dateTime);
void incrementHour(OffsetDateTime& dateTime);
void incrementMinute(OffsetDateTime& dateTime);

}

}
```

<a name="ZonedDateTimeMutations"></a>
### ZonedDateTime Mutations

The `ZonedDateTime` object can be mutated using the following methods and
functions:

```C++
namespace ace_time {

void ZonedDateTime::year(int16_t year);
void ZonedDateTime::month(uint8_t month);
void ZonedDateTime::day(uint8_t month);
void ZonedDateTime::hour(uint8_t hour);
void ZonedDateTime::minute(uint8_t minute);
void ZonedDateTime::second(uint8_t second);
void ZonedDateTime::timeZone(const TimeZone& timeZone);

namespace zoned_date_time_mutation {

void incrementYear(ZonedDateTime& dateTime);
void incrementMonth(ZonedDateTime& dateTime);
void incrementDay(ZonedDateTime& dateTime);
void incrementHour(ZonedDateTime& dateTime);
void incrementMinute(ZonedDateTime& dateTime);

}

}
```

<a name="ZonedDateTimeNormalization"></a>
### ZonedDateTime Normalization

When the `ZonedDateTime` object is mutated using the methods and functions
listed above, the client code must call `ZonedDateTime::normalize()` before
calling a method that calculates derivative information, in particular, the
`ZonedDateTime::toEpochSeconds()` method. Otherwise, the resulting epochSeconds
may be incorrect if the old `ZonedDateTime` and the new `ZonedDatetime` crosses
a DST boundary. Multiple mutations can be batched before calling
`normalize()`.

For example:

```C++

TimeZone tz = ...;
ZonedDateTime zdt = ZonedDateTime::forComponent(2000, 1, 1, 0, 0, 0, tz);

zdt.year(2021);
zdt.month(4);
zdt.day(20);
zdt.normalize();
acetime_t newEpochSeconds = zdt.toEpochSeconds();
```

Adding this single call to `normalize()` seems to increase flash consumption by
220 bytes on an 8-bit AVR processor. Unfortunately, it must be called to ensure
accuracy across DST boundaries.

<a name="TimePeriodMutations"></a>
### TimePeriod Mutations

The `TimePeriod` can be mutated using the following methods:

```C++
namespace ace_time {

void TimePeriod::hour(uint8_t hour);
void TimePeriod::minute(uint8_t minute);
void TimePeriod::second(uint8_t second);
void TimePeriod::sign(int8_t sign);

namespace time_period_mutation {

void negate(TimePeriod& period);
void incrementHour(TimePeriod& period, uint8_t limit);
void incrementHour(TimePeriod& period);
void incrementMinute(TimePeriod& period);

}
}
```

<a name="ErrorHandling"></a>
## Error Handling

Many features of the date and time classes have explicit or implicit range of
validity in their inputs and outputs. The Arduino programming environment does
not use C++ exceptions, so we handle invalid values by returning special version
of various date/time objects to the caller.

<a name="InvalidSentinels"></a>
### Invalid Sentinels

Many methods return an return integer value. Error conditions are indicated by
special constants, many of whom are defined in the `LocalDate` class:

* `int32_t LocalDate::kInvalidEpochDays`
    * Error value returned by `toEpochDays()` methods
* `int32_t LocalDate::kInvalidEpochSeconds`
    * Error value returned by `toEpochSeconds()` methods
* `int64_t LocalDate::kInvalidUnixSeconds64`
    * Error value returned by `toUnixSeconds64()` methods

Similarly, many factory methods accept an `acetime_t`, `int32_t`, or `int64_t`
arguments and return objects of various classes (e.g. `LocalDateTime`,
`OffsetDateTime` or `ZonedDateTime`). When these methods are given the error
constants, they return an object whose `isError()` method returns `true`.

It is understandable that error checking is often neglected, since it adds to
the maintenance burden. And sometimes, it is not always clear what should be
done when an error occurs, especially in a microcontroller environment. However,
I encourage the application to check for these errors conditions as much as
practical, and try to degrade to some reasonable default behavior when an error
is detected.

<a name="IsError"></a>
### isError()

The `isError()` method on these
classes will return `true` upon a data range error:

```C++
bool LocalDate::isError() const;
bool LocalTime::isError() const;
bool LocalDateTime::isError() const;
bool OffsetDatetime::isError() const;
bool ZonedDateTime::isError() const;
bool TimeOffset::isError() const;
bool TimeZone::isError() const;
```

A well-crafted application should check for these error conditions before
writing or displaying the objects to the user.

For example, the `LocalDate` and `LocalDateTime` classes support only 4-digit
`year` component, from `[1, 9999]`. The year 0 is used internally to indicate
`-Infinity` and the year `10000` is used internally as `+Infinity`.

The following are examples of invalid instances, where `dt.isError()` will
return true:

```C++
auto dt = LocalDateTime::forComponents(-1, 1, 1, 0, 0, 0); // invalid year

auto dt = LocalDateTime::forComponents(2000, 0, 1, 0, 0, 0); // invalid month

auto dt = LocalDateTime::forComponents(2000, 1, 32, 0, 0, 0); // invalid day

auto dt = LocalDateTime::forComponents(2000, 1, 1, 24, 0, 0); // invalid hour

auto dt = LocalDateTime::forComponents(2000, 1, 1, 0, 61, 0); // invalid minute

auto dt = LocalDateTime::forComponents(2000, 1, 1, 0, 0, 61); // invalid second
```

Another example, the `ZonedDateTime` class uses the generated ZoneInfo Database
in the `zonedb::` and `zonedbx::` namespaces. These data files are valid from
2000 until 10000. If you try to create a date outside of this range, an error
`ZonedDateTime` object will returned. The following snippet will print "true":

```C++
BasicZoneProcessor zoneProcessor;
auto tz = TimeZone::forZoneInfo(&zonedb::kZoneAmerica_Los_Angeles,
    &zoneProcessor);
auto dt = ZonedDateTime::forComponents(1998, 3, 11, 1, 59, 59, tz);
Serial.println(dt.isError() ? "true" : "false");
```

<a name="Bugs"></a>
## Bugs and Limitations

* Leap seconds
    * This library does not support
      [leap seconds](https://en.wikipedia.org/wiki/Leap_second) and will
      probably never do so.
    * The library does not implement
      [TAI (International Atomic Time)](https://en.wikipedia.org/wiki/International_Atomic_Time).
    * The `epochSeconds` is like `unixSeconds` in that it is unaware of
      leap seconds. When a leap seconds occurs, the `epochSeconds` is held
      constant over 2 seconds, just like `unixSeconds`.
    * The `SystemClock` is unaware of leap seconds so it will continue
      to increment `epochSeconds` through the leap second. In other words,
      the SystemClock will be 1 second ahead of UTC after the leap second
      occurs.
        * If the referenceClock is the `NtpClock`, that clock happens to
          be leap second aware, and the `epochSeconds` will bounce back one
          second upon the next synchronization, becoming synchronized to UTC.
        * If the referenceClock is the `DS3231Clock`, that clock is *not*
          leap second aware, so the `epochSeconds` will continue to be ahead of
          UTC by one second even after synchronization.
* `acetime_t`
    * AceTime uses a default epoch of 2050-01-01T00:00:00 UTC by default. The
      epoch can be changed using the `Epoch::currentEpochYear(year)` function.
    * The `acetime_t` type is a 32-bit signed integer whose smallest value
      is `-2^31` and largest value is `2^31-1`. However, the smallest value is
      used to indicate an internal "Error" condition, therefore the actual
      smallest `acetime_t` is `-2^31+1`. Therefore, the smallest and largest
      dates that can be represented by `acetime_t` is theoretically
      1981-12-13T20:45:53 UTC to 2118-01-20T03:14:07 UTC (inclusive).
    * To be conservative, users of this library should limit the range of the
      epoch seconds to +/- 60 years of the current epoch, in other words,
      `[1990,2110)`, or even easier to remember, `[2000,2100)`.
* `LocalDate`, `LocalDateTime`
    * The class checks that the `year` component in the range of `[0,
      10000]`, which is a smaller range than the `[-32767,32765]` range
      supported by the various `zonedb`, `zonedbx`, `zonedbc` databases.
    * The `isError()` returns `true` outside of the `[0,10000]` year range.
    * Due to the interaction of complex boundary effects, the actual range of
      accuracy of various algorithms in this library is probably `[1, 9999]`.
      Client applications should stay within this range.
    * This limit allows the library to assume that the year can always be
      formatted into 4-digits.
* `forDateString()`
    * Various classes provide a `forDateString()` method to construct
      the object from a human-readable string. These methods are mostly meant to
      be used for debugging. The parsers are not robust and do not perform very
      much error checking, but they may be sufficient for your needs.
    * `ZonedDateTime::forDateString()` cannot support TZ Database zone
      identifiers (e.g. "America/Los_Angeles") because the AceTime library does
      not load the entire TZ Database due to memory constraints of most Arduino
      boards.
* `TimeZone`
    * It might be possible to use different `TimeZone` instances created
      different zonedb database (i.e. `zonedb`, `zonedbx`, `zonedbc`).
      However, this is not a configuration that is expected to be used often, so
      it has not been tested well, if at all.
    * One potential problem is that the equality of two `TimeZone` depends only
      on the `zoneId`, so a Basic `TimeZone` created with a
      `zonedb::kZoneAmerica_Los_Angeles` will be considered equal to an Extended
      `TimeZone` created with a `zonedbx::kZoneAmerica_Los_Angeles`.
* `ZonedDateTime::forComponents()`
    * The `ZonedDateTime::forComponents()` method takes the local wall time and
      `TimeZone` instance as parameters which can be ambiguous or invalid for
      some values.
        * During the Standard time to DST transitions, a one-hour gap of
          illegal values may exist. For example, 2am (Standard) shifts to 3am
          (DST), therefore wall times between 02:00 and 03:00 (exclusive) are
          not valid.
        * During DST to Standard time transitions, a one-hour interval occurs
          twice. For example, 2am (DST) shifts to 1am, so all times between
          01:00 and 02:00 (exclusive) occurs twice in one day.
   * The `ZonedDateTime::forCommponent()` methods makes an educated guess
     at what the user meant, but the algorithm may not be robust, is not tested
     as well as it could be, and the algorithm may change in the future. To keep
     the code size within reasonble limits of a small Arduino controller, the
     algorithm may be permanently sub-optimal.
* `ZonedDateTime` objects should remain within roughly +/- 60 years of the
  current AceTime Epoch.
    * Otherwise, internal integer variables may overflow without warning
      and incorrect results may be calculated.
    * The internal time zone calculations use the same `int32_t` type as the
      `acetime_t` epoch seconds. This has a range of about 136 years.
* `BasicZoneProcessor`
    * Supports 1-minute resolution for the AT, UNTIL, STDOFF fields.
    * Supports only a 15-minute resolution for the DST offset field.
    * Sufficient to support ~450 zones out of ~600 total,
      from the year 2000 onwards.
* `ExtendedZoneProcessor`
    * Supports 1-second resolution for the AT, UNTIL, STDOFF, but the
      `zonedbx` database supports only 1-minute resolution of these fields.
    * Supports 1-second resolution for the DST offset field, but the `zonedbx`
      database supports only a 15-minute resolution of this field.
    * These restricts do not impact any timezones on or after the year 1974, and
      the `zonedbx` database starts at the year 2000.
* `CompleteZoneProcessor`
    * Is identical to `ExtendedZonProcessor`, but is able to use the
      high-resolution `zonedbc` database.
    * Supports all timezones for all years over `[0001,10000)`.
* `zonedb/`, `zonedbx/`, `zonedbc` databases
    * These data structures are loaded into flash memory using the `PROGMEM`
      keyword.
    * The ZoneInfo entries have *not* been compressed using bit-fields.
        * It may be possible to decrease the size of the full database using
          these compression techniques. However, compression will increase the
          size of the program file, so for applications that use only a small
          number of zones, it is not clear if the ZoneInfo entry compression
          will provide a reduction in the size of the overall program.
    * The TZ database files `backzone`, `systemv` and `factory` are
      not processed by the `tzcompiler.py` tool.
        * They don't seem to contain anything worthwhile.
    * TZ Database version 2019b contains the first use of the
      `{onDayOfWeek<=onDayOfMonth}` syntax that I have seen (specifically `Rule
      Zion, FROM 2005, TO 2012, IN Apr, ON Fri<=1`).
        * The actual transition date can shift into the previous month (or to
          the next month in the case of `>=`). However, shifting into the
          previous year or the next year is not supported.
        * The `tzcompiler.py` will exclude and flag the Rules which could
          potentially shift to a different year.
        * No such Rule has been observed as of 2023c.
* SAMD21 Boards
    * SAMD21 boards using the traditional Arduino API are supported.
        * For example, Adafruit ItsyBitsy M0, Seeeduino XIAO M0.
    * SAMD boards using the Arduino samd Core >= 1.8.10 are explicitly
      blacklisted, because they use the
      [ArduinoCore-API](https://github.com/arduino/ArduinoCore-api) which
      is not compatible with this library.
    * Arduino Zero is moved to Tier 3 (may work but not supported).
        * I don't own an Arduino Zero, so I cannot validate anything on that
          board.
        * It has [2 USB ports](https://www.arduino.cc/en/Guide/ArduinoZero)
          which is too confusing.
        * You may be able to fix some of the serial port problem by setting
          `ACE_TIME_CLOBBER_SERIAL_PORT_MONITOR` to `1` in
          `src/ace_time/common/compat.h`. But I do not test this option often,
          so it may be broken.
