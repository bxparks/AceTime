# AceTime User Guide

The primary purpose of AceTime classes is to convert between an integer
representing the number of seconds since the AceTime Epoch (2000-01-01T00:00:00
UTC) and the equivalent human-readable components in different timezones.

**Version**: 1.10.0 (2021-01-18, TZDB 2021e)

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
    * [ZonedDateTime](#ZonedDateTime)
        * [Class Declaration](#ZonedDateTimeDeclaration)
        * [Creation](#ZonedDateTimeCreation)
        * [Conversion to Other Time Zones](#TimeZoneConversion)
        * [DST Transition Caching](#DstTransitionCaching)
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
    * [ZoneInfo Entries](#ZoneInfoEntries)
        * [Basic zonedb](#BasicZonedb)
        * [Extended zonedbx](#ExtendedZonedbx)
        * [BasicZone and ExtendedZone](#BasicZoneAndExtendedZone)
        * [TZ Database Version](#TzDatabaseVersion)
        * [Zone Info Year Range](#ZoneInfoYearRange)
    * [Zones and Links](#ZonesAndLinks)
        * [Ghost Links (Prior to v1.5)](#GhostLinks)
        * [Fat Links (From v1.5)](#FatLinks)
        * [Thin Links (From v1.6)](#ThinLinks)
        * [Custom Zone Registry](#CustomZoneRegistry)
* [Zone Sorting](#ZoneSorting)
* [Print To String](#PrintToString)
* [Mutations](#Mutations)
    * [TimeOffset Mutations](#TimeOffsetMutations)
    * [LocalDate Mutations](#LocalDateMutations)
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
is difficult to organize the various parts of this library in the most easily
digestible way, but perhaps they can be categorized into three parts:

* Simple Date and Time classes for converting date and time fields to and
  from the "epoch seconds",
* TimeZone related classes which come in 2 forms:
    * classes that extend the simple Date and Time classes that account for time
      zones which can be described in a time zone database (e.g. `TimeZone`,
     `ZonedDateTime`, `ZoneProcessor`)
    * classes and types that manage the TZ Database and provide access to its
      data (e.g. `ZoneManager`, `ZoneInfo`, `ZoneId`)
* The ZoneInfo Database generated from the IANA TZ Database that contains UTC
  offsets and the rules for determining when DST transitions occur for a
  particular time zone

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

The Epoch in AceTime is defined to be 2000-01-01T00:00:00 UTC, in contrast to
the Epoch in Unix which is 1970-01-01T00:00:00 UTC. Internally, the current time
is represented as "seconds from Epoch" stored as a 32-bit signed integer
(`acetime_t` aliased to `int32_t`). The smallest 32-bit signed integer (`-2^31`)
is used to indicate an internal Error condition, so the range of valid
`acetime_t` value is `-2^31+1` to `2^31-1`. Therefore, the range of dates that
the `acetime_t` type can handle is 1931-12-13T20:45:53Z to 2068-01-19T03:14:07Z
(inclusive). (In contrast, the 32-bit Unix `time_t` range is
1901-12-13T20:45:52Z to 2038-01-19T03:14:07Z which is the cause of the [Year
2038 Problem](https://en.wikipedia.org/wiki/Year_2038_problem)).

The various date classes (`LocalDate`, `LocalDateTime`, `OffsetDateTime`) store
the year component internally as a signed 8-bit integer offset from the year
2000. The range of this integer is -128 to +127, but -128 is used to indicate an
internal Error condition, so the actual range is -127 to +127. Therefore, these
classes can represent dates from 1873-01-01T00:00:00 to 2127-12-31T23:59:59
(inclusive). Notice that these classes can represent all dates that can be
expressed by the `acetime_t` type, but the reverse is not true. There are date
objects that cannot be converted into a valid `acetime_t` value. To be safe,
users of this library should stay at least 1 day away from the lower and upper
limits of `acetime_t` (i.e. stay within the year 1932 to 2067 inclusive).

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

Access to the two sets data in the ZoneInfo Database is provided by:

* `BasicZoneManager`:
    * contains a registry of the basic ZoneInfo data structures
    * holds a cache of `BasicZoneProcessor`
* `ExtendedZoneManager`:
    * contains a registry of the extended ZoneInfo data structures
    * the holds a cache of `ExtendedZoneProcessor`

<a name="ZoneInfoDatabaseOverview"></a>
### ZoneInfo Database Overview

The official IANA TZ Database is processed and converted into an internal
AceTime database that we will call the ZoneInfo Database (to distinguish it from
the IANA TZ Database). The ZoneInfo Database contains statically defined C++
data structures, which each timezone in the TZ Database being represented by a
`ZoneInfo` data structure.

Two slightly different sets of ZoneInfo entries are generated, under 2 different
directories, using 2 different C++ namespaces to avoid cross-contamination:

* [zonedb/zone_infos.h](src/ace_time/zonedb/zone_infos.h)
    * intended for `BasicZoneProcessor` or `BasicZoneManager`
    * 266 zones and 183 links (as of version 2021a) from the year 2000 until
      2050, about 70% of the full IANA TZ Database
    * contains `kZone*` declarations (e.g. `kZoneAmerica_Los_Angeles`)
    * contains `kZoneId*` identifiers (e.g. `kZoneIdAmerica_Los_Angeles`)
    * slightly smaller and slightly faster
* [zonedbx/zone_infos.h](src/ace_time/zonedbx/zone_infos.h)
    * intended for `ExtendedZoneProcessor` or `ExtendedZoneManager`
    * all 386 zones and 207 links (as of version 2021a) in the IANA TZ Database
      from the year 2000 until 2050.
    * contains `kZone*` declarations (e.g. `kZoneAfrica_Casablanca`)
    * contains `kZoneId*` identifiers (e.g. `kZoneIdAfrica_Casablanca`)

The internal helper classes which are used to encode the ZoneInfo Database
information are defined in the following namespaces. They are not expected to be
used by application developers under normal circumstances, so these are listed
here for reference:

* `ace_time::basic::ZoneContext`
* `ace_time::basic::ZoneEra`
* `ace_time::basic::ZoneInfo`
* `ace_time::basic::ZonePolicy`
* `ace_time::basic::ZoneRule`
* `ace_time::extended::ZoneContext`
* `ace_time::extended::ZoneInfo`
* `ace_time::extended::ZoneEra`
* `ace_time::extended::ZonePolicy`
* `ace_time::extended::ZoneRule`

The ZoneInfo entries (and their associated `ZoneProcessor` classes) have a
resolution of 1 minute, which is sufficient to represent all UTC offsets and DST
shifts of all timezones after 1972 (Africa/Monrovia seems like the last timezone
to conform to a one-minute resolution on Jan 7, 1972).

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
`zonedb::kZoneIdAmerica_Los_Angeles` or `zonedbx::kZoneIdAmerica_Los_Angele`
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

To use the Basic ZoneInfo data structures needed by `BasicZoneProcessor` and
`BasicZoneManager`, you will need:

```C++
using namespace ace_time::zonedb;
```

To use the Extended ZoneInfo data structures needed by `ExtendedZoneProcessor`
and `ExtendedZoneManager`, you will need:

```C++
using namespace ace_time::zonedbx;
```

The following C++ namespaces are usually internal implementation details
which are not normally needed by the end users:

* `ace_time::basic`: for creating custom zone registries for `BasicZoneManager`
* `ace_time::extended`: for creating custom zone registries for
  `ExtendedZoneManager`
* `ace_time::internal`

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
defined to be 2000-01-01 00:00:00 UTC time. In contrast, the Unix Epoch is
defined to be 1970-01-01 00:00:00 UTC. Since `acetime_t` is a 32-bit signed
integer, the largest value is 2,147,483,647. Therefore, the largest date
that can be represented in this library is 2068-01-19T03:14:07 UTC.

The `acetime_t` is analogous to the `time_t` type in the standard C library,
with several major differences:

* The `time_t` does not exist on all Arduino platforms.
* Some Arduino platforms and older Unix platforms use a 32-bit `int32_t` to
  represent `time_t`.
* Modern implementations (e.g. ESP8266 and ESP32) use a 64-bit `int64_t` to
  represent `time_t` to prevent the "Year 2038" overflow problem. Unfortunately,
  AceTime cannot use 64-bit integers internally because they are too resource
  intensive on 8-bit processors.
* Most `time_t` implementations uses the Unix Epoch of 1970-01-01 00:00:00 UTC.
  AceTime uses an epoch of 2000-01-01 00:00:00 UTC.

It is possible to convert between a `time_t` and an `acetime_t` by adding or
subtracting the number of seconds between the 2 Epoch dates. This constant is
946684800 and defined by `LocalDate::kSecondsSinceUnixEpoch`. Helper methods
are available on various classes to avoid manual conversion between these 2
epochs: `forUnixSeconds()` and `toUnixSeconds()`, and the corresponding
`forUnixSeconds64()` and `toUnixSeconds64()` 64-bit versions added in v1.10.

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
    static const int16_t kEpochYear = 2000;
    static const acetime_t kSecondsSinceUnixEpoch = 946684800;

    static const int32_t kInvalidEpochDays = INT32_MIN;
    static const acetime_t kInvalidEpochSeconds = INT32_MIN;

    static const int32_t kInvalidUnixDays = INT32_MIN;
    static const int32_t kInvalidUnixSeconds = INT32_MIN;
    static const int64_t kInvalidUnixSeconds64 = INT64_MIN;

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
    static LocalDate forUnixSeconds(int32_t unixSeconds);
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
    int32_t toUnixSeconds() const {
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
    static LocalDateTime forUnixSeconds(int32_t unixSeconds);
    static LocalDateTime forUnixSeconds64(int64_t unixSeconds);
    static LocalDateTime forDateString(const char* dateString);

    bool isError() const;

    int16_t year() const; // 1872 - 2127
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
    int32_t toUnixSeconds() const;
    int64_t toUnixSeconds64() const;

    int8_t compareTo(const LocalDateTime& that) const;
    void printTo(Print& printer) const;
    ...
};

}
```

Here is a sample code that extracts the number of seconds since AceTime Epoch
(2000-01-01T00:00:00Z) using the `toEpochSeconds()` method:

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

    static TimePeriod forError();

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

Sometimes it is useful to create a `TimePeriod` object that represents an error
condition, for example, if the time interval is too large. The
`TimePeriod::forError()` factory method returns a special instance that
represents an error. The `TimePeriod::isError()` method on that object will
return `true`. Calling `TimePeriod::toSeconds()` on an error object returns
`kInvalidPeriodSeconds`.

<a name="TimeOffset"></a>
### TimeOffset

A `TimeOffset` class represents an amount of time shift from a reference point.
This is usually used to represent a timezone's standard UTC offset or its DST
offset in the summer. The time resolution of this class changed from 15 minutes
(using a single byte `int8_t` implementation prior to v0.7) to 1 minute (using a
2-byte `int16_t` implementation since v0.7). The range of an `int16_t` is
[-32768, +32767], but -32768 is used to indicate an error condition, so the
actual range is [-32767, +32767] minutes. In practice, the range of values
actually used is probably within [-48, +48] hours, or [-2880, +2800] minutes

```C++
namespace ace_time {

class TimeOffset {
  public:
    static TimeOffset forHours(int8_t hours);
    static TimeOffset forMinutes(int16_t minutes);
    static TimeOffset forHourMinute(int8_t hour, int8_t minute);

    int16_t toMinutes() const;
    int32_t toSeconds() const;
    void toHourMinute(int8_t& hour, int8_t& minute) const;

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
`true`. Internally, this is an instance whose internal integer is -32768.

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
    static OffsetDateTime forUnixSeconds(int32_t unixSeconds,
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
    int32_t toUnixSeconds() const;
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
* A geographical or political region whose local time is offset
from the UTC time using various transition rules.

Both meanings of "time zone" are supported by the `TimeZone` class using
3 different types as follows:

* `TimeZone::kTypeManual`: a fixed base offset and optional DST offset from UTC
* `TimeZone::kTypeBasic`: utilizes a `BasicZoneProcessor` which can
be encoded with (relatively) simple rules from the ZoneInfo Database
* `TimeZone::kTypeExtended`: utilizes a `ExtendedZoneProcessor` which can
handle all zones in the ZoneInfo Database

The class hierarchy of `TimeZone` is shown below, where the arrow means
"is-subclass-of" and the diamond-line means "is-aggregation-of". This is an
internal implementation detail of the `TimeZone` class that the application
developer will not normally need to be aware of all the time, but maybe this
helps make better sense of the usage of the `TimeZone` class. A `TimeZone` can
hold a reference to:

* nothing (`kTypeManual`),
* one `BasicZoneProcessor` object, (`kTypeBasic`), or
* one `ExtendedZoneProcessor` object (`kTypeExtended`)

```
               0..1
TimeZone <>-------- ZoneProcessor
                         ^
                         |
                   .-----+-----.
                   |           |
    BasicZoneProcessor       ExtendedZoneProcessor
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
    static TimeZone forMinutes(int8_t stdMinutes, int8_t dstMinutes = 0);

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

    static TimeZone forUtc();

    TimeZone(); // same as forUtc()
    bool isError() const;

    uint8_t getType() const;
    TimeOffset getStdOffset() const;
    TimeOffset getDstOffset() const;
    uint32_t getZoneId() const;

    TimeOffset getUtcOffset(acetime_t epochSeconds) const;
    TimeOffset getDeltaOffset(acetime_t epochSeconds) const;
    const char* getAbbrev(acetime_t epochSeconds) const;

    OffsetDateTime getOffsetDateTime(const LocalDateTime& ldt) const;

    bool isUtc() const;
    bool isDst() const;

    void setStdOffset(TimeOffset stdOffset);
    void setDstOffset(TimeOffset offset);

    TimeZoneData toTimeZoneData() const;

    void printTo(Print& printer) const;
    void printShortTo(Print& printer) const;
};

}
```

The `getUtcOffset(epochSeconds)` returns the total `TimeOffset` (including any
DST offset) at the given `epochSeconds`. The `getDeltaOffset()` returns only the
additional DST offset; if DST is not in effect at the given `epochSeconds`, this
returns a `TimeOffset` whose `isZero()` returns true.

The `getAbbrev(epochSeconds)` method returns the human-readable timezone
abbreviation used at the given `epochSeconds`. For example, this be "PST" for
Pacific Standard Time, or "BST" for British Summer Time. The returned c-string
should be used as soon as possible (e.g. printed to Serial) because the pointer
points to a temporary buffer whose contents may change upon subsequent calls to
`getUtcOffset()`, `getDeltaOffset()` and `getAbbrev()`. If the abbreviation
needs to be saved for a longer period of time, it should be saved to another
char buffer.

The `getOffsetDateTime(localDateTime)` method returns the best guess of
the `OffsetDateTime` at the given local date time. This method is used by
`ZonedDateTime::forComponents()` and is exposed mostly for debugging. The reaon
that this is a best guess is because the local date time is sometime ambiguious
during a DST transition. For example, if the local clock shifts from 01:00 to
02:00 at the start of summer, then the time of 01:30 does not exist. If the
`getOffsetDateTime()` method is given a non-existing time, it makes an educated
guess at what the user meant. Additionally, when the local time transitions from
02:00 to 01:00 in the autumn, a given local time such as 01:30 occurs twice. If
the `getOffsetDateTime()` method is given a time of 01:30, it will arbitrarily
decide which offset to return.

The `isUtc()`, `isDst()` and `setDstOffset(TimeOffset)` methods are valid *only*
if the `TimeZone` is a `kTypeManual`. Otherwise, `isUtc()` and `isDst()` return
`false` and `setDstOffset()` does nothing.

The `getZoneId()` returns a `uint32_t` integer which is a unique and stable
identifier for the IANA timezone. This can be used to save and restore
the `TimeZone`. See the [ZoneManager](#ZoneManager) subsection below.

The `printTo()` prints the fully-qualified unique name for the time zone.
For example, `"UTC"`, `"-08:00"`, `"-08:00(DST)"`, `"America/Los_Angeles"`.

The `printShortTo()` is similar to `printTo()` except that it prints the
last component of the IANA TZ Database zone names. In other words,
`"America/Los_Angeles"` is printed as `"Los_Angeles"`. This is helpful for
printing on small width OLED displays.

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

The `setDstOffset()` takes a `TimeOffset` as the argument instead of a simple
`bool` because there are a handful of rare zones (e.g. Europe/Dublin, I think
there is one other) which use a negative offset in the winter, instead of adding
a positive offset in the summer.

The `setStdOffset()` allows the base time offset to be changed, but this
method is not expected to be used often.

<a name="BasicTimeZone"></a>
#### Basic TimeZone (kTypeBasic)

This TimeZone is created using two objects:
* the `basic::ZoneInfo` data objects contained in
  [zonedb/zone_infos.h](src/ace_time/zonedb/zone_infos.h)
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
`America/Los_Angeles`:

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
    auto offset = tz.getUtcOffset(epochSeconds); // returns -08:00
  }

  // one second later, 2018-03-11T02:00:00-08:00 was in DST time
  {
    auto dt = OffsetDateTime::forComponents(2018, 3, 11, 2, 0, 0,
      TimeOffset::forHours(-8));
    acetime_t epochSeconds = dt.toEpochSeconds();
    auto offset = tz.getUtcOffset(epochSeconds); // returns -07:00
  }
  ...
}
```

<a name="ExtendedTimeZone"></a>
#### Extended TimeZone (kTypeExtended)

This TimeZone is created using two objects:
* the `extended::ZoneInfo` data objects contained in
  [zonedbx/zone_infos.h](src/ace_time/zonedbx/zone_infos.h)
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
    auto offset = tz.getUtcOffset(epochSeconds); // returns -08:00
  }

  // one second later, 2018-03-11T02:00:00-08:00 was in DST time
  {
    auto dt = OffsetDateTime::forComponents(2018, 3, 11, 2, 0, 0,
      TimeOffset::forHours(-8));
    acetime_t epochSeconds = dt.toEpochSeconds();
    auto offset = tz.getUtcOffset(epochSeconds); // returns -07:00
  }
  ...
}
```

The advantage of `ExtendedZoneProcessor` over `BasicZoneProcessor` is that
`ExtendedZoneProcessor` supports all time zones in the IANA TZ Database. The
cost is that it consumes *5 times* more static memory and is a bit slower. If
`BasicZoneProcessor` supports the zone that you want using the ZoneInfo entries
in the `zonedb::` namespace, you should normally use that instead of
`ExtendedZoneProcessor`.

The one other advantage of `ExtendedZoneProcessor` over `BasicZoneProcessor` is
that `ExtendedZoneProcessor::forComponents()` is more accurate than
`BasicZoneProcessor::forComponents()` because the `zonedbx::` entries contain
transition information which are missing in the `zonedb::` entries due to
space constraints.

Instead of managing the `BasicZoneProcessor` or `ExtendedZoneProcessor`
manually, you can use the `ZoneManager` to manage a database of `ZoneInfo`
entries, and a cache of multiple `ZoneProcessor`s, and bind the `TimeZone` to
its `ZoneInfo` and its `ZoneProcessor` more dynamically through the
`ZoneManager`. See the section [ZoneManager](#ZoneManager) below for more
information.

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
    static ZonedDateTime forUnixSeconds(int32_t unixSeconds,
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
    int32_t toUnixSeconds() const;
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
`ZoneManager` to manage the `BasicZoneProcessor` or `ExtendedZoneProcessor`
objects dynamically. But the dataset is too large to fit on most AVR
microcontrollers with only 32kB of flash memory, so we currently do not support
this dynamic lookup. The `ZonedDateTime::timeZone()` will return Manual
`TimeZone` whose `TimeZone::getType()` returns `TimeZone::kTypeManual`.

<a name="TimeZoneConversion"></a>
#### Conversion to Other Time Zones

You can convert a given `ZonedDateTime` object into a representation in a
different time zone using the `DateTime::convertToTimeZone()` method:

```C++
static BasicZoneProcessor processorLosAngeles;
static BasicZoneProcessor processorZurich;

void someFunction() {
  ...
  auto tzLosAngeles = TimeZone::forZoneInfo(
      &zonedb::kZoneAmerica_Los_Angeles, &processorLosAngeles);
  auto tzZurich = TimeZone::forZoneInfo(
      &zonedb::kZoneEurope_Zurich, &processorZurich);

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
`ZonedDateTime::forEpochSeconds()` is an expensive operation
that requires the computation of the relevant DST transitions for the given
epochSeconds or date-time components. To improve performance, the
`BasicZoneProcessor` and `ExtendedZoneProcessor` implement internal transition
caching based on the `year` component. This optimizes the most commonly expected
use case where the epochSeconds is incremented by a clock (e.g. `SystemClock`)
every second, and is converted to human-readable date-time components once a
second. According to [AutoBenchmark](examples/AutoBenchmark/), the cache
improves performance by a factor of 2-3X (8-bit AVR) to 10-20X (32-bit
processors) on consecutive calls to `forEpochSeconds()` with the same `year`.

<a name="ZoneManager"></a>
### ZoneManager

The `TimeZone::forZoneInfo()` methods are simple to use but have the
disadvantage that the `BasicZoneProcessor` or `ExtendedZoneProcessor` needs to
be created manually for each `TimeZone` instance. This works well for a single
time zone, but if you have an application that needs 3 or more time zones, this
can become cumbersome. Also, it is difficult to reconstruct a `TimeZone`
dynamically, say, from its fully qualified name (e.g. `"America/Los_Angeles"`).

The `ZoneManager` solves these problems by implementing 2 features:

1) It supports a registry of `ZoneInfo` objects, so that a `TimeZone` can be
   created using its `zoneName` string, `zoneInfo` pointer, or `zoneId` integer.
2) It supports the use of cache of `ZoneProcessors` that can be mapped to
   a particular zone as needed.

<a name="ClassHierarchy"></a>
#### Class Hierarchy

Three implementations of the `ZoneManager` are provided. Prior to v1.9, they
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
        BasicZoneProcessorCacheBase& zoneProcessorCache
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

class ManualZoneManager {
  public:
    uint16_t zoneRegistrySize() const;

    TimeZone createForTimeZoneData(const TimeZoneData& d);
};

}
```

<a name="DefaultRegistries"></a>
#### Default Registries

The constructors for `BasicZoneManager` and `ExtendedZoneManager` take a
`zoneRegistry` and its `zoneRegistrySize`, and optionally the `linkRegistry` and
`linkRegistrySize` parameters. The AceTime library comes with a set of
pre-defined default Zone and Link registries which are defined by the following
header files. These header files are automatically included in the `<AceTime.h>`
header:

* [zonedb/zone_registry.h](src/ace_time/zonedb/zone_registry.h)
    * Zones and Links supported by `BasicZoneManager`
    * `ace_time::zonedb` namespace
* [zonedbx/zone_registry.h](src/ace_time/zonedbx/zone_registry.h)
    * Zones and Links supported by `ExtendedZoneManager`
    * `ace_time::zonedbx` namespace


<a name="ZoneProcessorCache"></a>
#### ZoneProcessorCache

The `BasicZoneManager` and the `ExtendedZoneManager` classes need to be given an
instance of a `BasicZoneProcessorCache<CACHE_SIZE>` or
`ExtendedZoneProcessorCache<CACHE_SIZE>` object.

```C++
BasicZoneProcessorCache<CACHE_SIZE> basicZoneProcessorCache;
ExtendedZoneProcessorCache<CACHE_SIZE> extendedZoneProcessorCache;
```

These used to be defined internally inside the `BasicZoneManager` and
`ExtendedZoneManager` classes. But when they were refactored to be
non-polymorphic to save flash memory, it was easier to extract the
ZoneProcessorCache objects into separate classes to be passed into the
ZoneManager classes.

The `CACHE_SIZE` template parameter is an integer that specifies the size of the
internal cache. This should be set to the number of time zones that your
application is expected to use *at the same time*. If your app never changes its
time zone after initialization, then this can be `<1>` (although in this case,
you may not even want to use the `ZoneManager`). If your app allows the user to
dynamically change the time zone (e.g. from a menu of time zones), then this
should be at least `<2>` (to allow the system to compare the old time zone to
the new time zone selected by the user). In general, the `CACHE_SIZE` should be
set to the number of timezones displayed to the user concurrently, plus an
additional 1 if the user is able to change the timezone dynamically.

<a name="ZoneManagerCreation"></a>
#### ZoneManager Creation

If you decide to use the default registries, there are 4 possible configurations
of the ZoneManager constructors as shown below. The following also shows the
number of zones and links supported by each configuration, as well as the flash
memory consumption of each configuration, as determined by
[MemoryBenchmark](examples/MemoryBenchmark). These numbers are correct as of
v1.6 with TZDB version 2021a:

```C++
static const uint8_t CACHE_SIZE = 2; // tuned for application

BasicZoneProcessorCache<CACHE_SIZE> basicZoneProcessorCache;
ExtendedZoneProcessorCache<CACHE_SIZE> extendedZoneProcessorCache;

// BasicZoneManager, Zones only
// 266 zones
// 21.6 kB (8-bits)
// 27.1 kB (32-bits)
BasicZoneManager zoneManager(
    zonedb::kZoneRegistrySize,
    zonedb::kZoneRegistry,
    basicZoneProcessorCache);

// BasicZoneManager, Zones and Fat Links
// 266 zones, 183 fat links
// 25.7 kB (8-bits)
// 33.2 kB (32-bits)
BasicZoneManager zoneManager(
    zonedb::kZoneAndLinkRegistrySize,
    zonedb::kZoneAndLinkRegistry,
    basicZoneProcessorCache);

// ExtendedZoneManager, Zones only
// 386 Zones
// 33.5 kB (8-bits)
// 41.7 kB (32-bits)
ExtendedZoneManager zoneManager(
    zonedbx::kZoneRegistrySize,
    zonedbx::kZoneRegistry,
    extendedZoneProcessorCache);

// ExtendedZoneManager, Zones and Fat Links
// 386 Zones, 207 fat Links
// 38.2 kB (8-bits)
// 48.7 kB (32-bits)
ExtendedZoneManager zoneManager(
    zonedbx::kZoneAndLinkRegistrySize,
    zonedbx::kZoneAndLinkRegistry,
    extendedZoneProcessorCache);
```

A more complicated option of using *thin link* through the `LinkManager` is
explained the [Zones and Links](#ZonesAndLinks) section below.

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
    zonedbx::kZoneRegistrySize,
    zonedbx::kZoneRegistry,
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
    zonedbx::kZoneRegistrySize,
    zonedbx::kZoneRegistry,
    zoneProcessorCache);

void someFunction() {
  TimeZone tz = zoneManager.createForZoneInfo(zonedb::kZoneAmerica_Los_Angeles);
  ...
}
```

I think the only time the `createForZoneName()` might be useful is if
the user was allowed to type in the zone name, and you wanted to create a
`TimeZone` from the string typed in by the user.

<a name="CreateForZoneId"></a>
#### createForZoneId()

Each zone in the `zonedb::` and `zonedbx::` database is given a unique
and stable zoneId. There are at least 3 ways to extract this zoneId:

* the `kZoneId{zone name}` constants in `src/ace_time/zonedb/zone_infos.h`
  and `src/ace_time/zonedbx/zone_infos.h`:
    * `const uint32_t kZoneIdAmerica_New_York = 0x1e2a7654; // America/New_York`
    * `const uint32_t kZoneIdAmerica_Los_Angeles = 0xb7f7e8f2; // America/Los_Angeles`
    * ...
* the `TimeZone::getZoneId()` method:
    * `uint32_t zoneId = tz.getZoneId();`
* the `ZoneInfo` pointer using the `BasicZone()` helper object:
    * `uint32_t zoneId = BasicZone(&zonedb::kZoneAmerica_Los_Angeles).zoneId();`
    * `uint32_t zoneId = ExtendedZone(&zonedbx::kZoneAmerica_Los_Angeles).zoneId();`

The `ZoneManager::createForZoneId()` method returns the `TimeZone` object
corresponding to the given `zoneId`:

```C++
ExtendedZoneProcessorCache<CACHE_SIZE> zoneProcessorCache;
ExtendedZoneManager zoneManager(
    zonedbx::kZoneRegistrySize,
    zonedbx::kZoneRegistry,
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
`createForZoneInfo()` which live in the corresponding implementation classes
(`BasicZoneManager` and `ExtendedZoneManager`) because each version needs a
different `ZoneInfo` type (`basic::ZoneInfo` and `extended::ZoneInfo`). If your
code has a reference or pointer to the top-level `ZoneManager` interface, then
it will be far easier to create a `TimeZone` using `createForZoneId()`. You do
pay a penalty in efficiency because `createForZoneId()` must scan the database,
where as `createForZoneInfo()` does not perform a search since it has direct
access to the `ZoneInfo` data structure.

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
needed in applications which are normally targeted to use `BasicZoneManager` and
`ExtendedZoneManager`, but are sometimes targeted to small-memory
microcontrollers (typically AVR chips), for testing purposes for example. This
class allows many of the function and constructor signatures to remain the same,
reducing the amount of conditional code.

If an application is specifically targeted to a low-memory chip, and it is known
at compile-time that only `TimeZone::kTypeManual` are supported, then you should
not need to use the `ManualZoneManager`. You can use `TimeZone::forTimeOffset()`
factory method directory.

<a name="HandlingGapsAndOverlaps"></a>
### Handling Gaps and Overlaps

(Added in v1.10)

Better control over DST gaps and overlaps was added in v1.10 using the
techniques described by the [PEP 495](https://www.python.org/dev/peps/pep-0495/)
document in Python 3.6.

1) An additional parameter called `fold` was added to the `LocalTime`,
   `LocalDateTime`, `OffsetDateTime`, and `ZonedDateTime` classes.
2) Support for the `fold` parameter was added to `ExtendedZoneProcessor`. The
  `BasicZoneProcessor` does *not* support the `fold` parameter and will ignore
  it.

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
              |                  (overlap)   /
          2am |                      /|    /
              |                    /  |  /
          1am |                  /    |/
              |          /
              |        /
          3am |       |
              |  (gap)|
          2am |       |
              |      /
              |    /
              |  /
              +----------------------------------------->
              spring-forward      fall-backward

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
      03:30-07:00, and the `fold` after normalization is set to 0.
* `fold==1`
    * Selects the *later* Transition element, extended backward to apply to the
      given LocalDateTime,
    * Which maps to the *earlier* UTC/epochSeconds,
    * Which becomes normalized to the *earlier* ZonedDateTime which has the
      *earlier* UTC offset.
    * So 02:30 is interpreted as 02:30-07:00, which is normalized to
      01:30-08:00, and the `fold` after normalization is set to 0.

The time shift during a gap seems to be the *opposite* of the shift during an
overlap, but internally this is self-consistent. Just as importantly, this
follows the same logic as PEP 495.

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
same epochSeconds. With the `fold` parameter, this round-trip was not guaranteed
during an overlap.

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
    zonedbx::kZoneRegistrySize,
    zonedbx::kZoneRegistry,
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

<a name="ZoneInfoEntries"></a>
### ZoneInfo Entries

Starting with v0.4, the ZoneInfo entries are stored in in flash memory
instead of static RAM using the
[PROGMEM](https://www.arduino.cc/reference/en/language/variables/utilities/progmem/)
keyword on microcontrollers which support this feature. On an 8-bit
microcontroller, the `zonedb/` database consumes about 15 kB of flash
memory, so it may be possible to create small programs that can dynamically
access all timezones supported by `BasicZoneProcessor`. The `zonedbx/` database
consumes about 24 kB of flash memory and the addition code size from various
classes will exceed the 30-32kB limit of a typical Arduino 8-bit
microcontroller.

The exact format of the ZoneInfo entries (under the `zonedb/` and `zonedbx/`
directories) are considered to be an implementation detail and may change in the
future to support future timezones. Applications should not depend on the
internal structure of `ZoneInfo` data structures.

<a name="BasicZonedb"></a>
#### Basic zonedb

The `zonedb/` entries do not support all the timezones in the IANA TZ Database.
If a zone is excluded, the reason for the exclusion can be found at the
bottom of the [zonedb/zone_infos.h](src/ace_time/zonedb/zone_infos.h) file.
The criteria for selecting the Basic `zonedb` entries are embedded
in the `transformer.py` script and summarized in
[BasicZoneProcessor.h](src/ace_time/BasicZoneProcessor.h):

* the DST offset is a multiple of 15-minutes (all current timezones satisfy
  this)
* the STDOFF offset is a multiple of 1-minute (all current timezones
  satisfy this)
* the AT or UNTIL fields must occur at one-year boundaries (this is the biggest
  filter)
* the LETTER field must contain only a single character
* the UNTIL time suffix can only be 'w' (not 's' or 'u')
* there can be only one DST transition in a single month

As of version v1.9 (with TZDB 2021e), this database contains 258 Zone entries
and 193 Link entries, supported from the year 2000 to 2049 (inclusive).

<a name="ExtendedZonedbx"></a>
#### Extended zonedbx

The goal of the `zonedbx/` entries is to support all zones listed in the TZ
Database. Currently, as of version 2021a of the IANA TZ Database, this goal is
met from the year 2000 to 2049 inclusive. Some restrictions of this database
are:

* the DST offset is a multiple of 15-minutes ranging from -1:00 to 2:45
  (all timezones from about 1972 support this)
* the STDOFF offset is a multiple of 1-minute
* the AT and UNTIL fields are multiples of 1-minute
* the LETTER field can be arbitrary strings

As of version v1.9 (with TZDB 2021e), this database contains all 377 Zone
entries and 217 Link entries, supported from the year 2000 to 2049 (inclusive).

<a name="TzDatabaseVersion"></a>
#### TZ Database Version

The IANA TZ Database is updated continually. As of this writing, the latest
stable version is 2021a. When a new version of the database is released, I
regenerate the ZoneInfo entries under the `src/ace_time/zonedb/` and
`src/ace_time/zonedbx/` directories.

The current TZ Database version can be programmatically accessed using the
`kTzDatabaseVersion` constant:

```C++
#include <AceTime.h>
using namespace ace_time;

void printVersionTzVersions() {
    Serial.print("zonedb TZ version: ");
    Serial.println(zonedb::kTzDatabaseVersion); // e.g. "2020d"

    Serial.print("zonedbx TZ version: ");
    Serial.println(zonedbx::kTzDatabaseVersion); // e.g. "2020d"
}
```

It is technically possible for the 2 versions to be different, but since they
are generated by the same set of scripts, I expect they will always be the same.

<a name="ZoneInfoYearRange"></a>
#### ZoneInfo Year Range

As mentioned above, both the `zonedb` and `zonedbx` databases are generated with
a specific `startYear` and `untilYear` range. If you try to create a
`ZonedDateTime` object outside of the year range, the constructed object will be
`ZonedDateTime::forError()` whose `isError()` method returns `true`.

Applications can access the valid `startYear` and `untilYear` of the `zonedb` or
`zonedbx` databases through the `kZoneContext` data structure:

```C++
#include <AceTime.h>
using namespace ace_time;

void printStartAndUntilYears() {
    Serial.print("zonedb: startYear: ");
    Serial.print(zonedb::kZoneContext.startYear); // e.g. 2000
    Serial.print("; untilYear: ");
    Serial.println(zonedb::kZoneContext.untilYear); // e.g. 2050

    Serial.print("zonedbx: startYear: ");
    Serial.print(zonedbx::kZoneContext.startYear); // e.g. 2000
    Serial.print("; untilYear: ");
    Serial.println(zonedbx::kZoneContext.untilYear); // e.g. 2050
}
```

I looked into supporting some sort of "graceful degradation" mode of the
`ZonedDateTime` class, where creating instances before `startYear` or after
`untilYear` would actually succeed, even though those instances would have some
undefined errors due to incorrect or missing DST offsets. However, so much of
the code in `BasicZoneProcessor` and `ExtendedZonedProcessor` depend on the
intricate details of the ZoneInfo entries being in a valid state, I could not
guarantee that a catastrophic situation (e.g. infinite loop) could be avoided
outside of the safe zone. Therefore, attempting to create a `ZonedTimeDate`
object outside of the supported `startYear` and `untilYear` range will always
return an error object. Applications should either check the year range first
before creating a `ZonedDateTime` object, or check the
`ZonedDateTime::isError()` method after creation.

<a name="BasicZoneAndExtendedZone"></a>
#### BasicZone and ExtendedZone

The `basic::ZoneInfo` and `extended::ZoneInfo` (and its related data structures)
objects are meant to be *opaque* and simply passed into the `TimeZone`
class (which in turn, passes the pointer into the `BasicZoneProcessor` and
`ExtendedZoneProcessor` objects.) The internal formats of the `ZoneInfo`
structures may change without warning, and users of this library should not
access its internal data members directly.

Two helper classes, `BasicZone` and `ExtendedZone`, provide stable access to
some of the internal fields:

```C++
namespace ace_time {

class BasicZone {
  public:
    BasicZone(const basic::ZoneInfo* zoneInfo);

    bool isNull() const;
    uint32_t zoneId() const;
    int16_t stdOffsetMinutes() const;
    ace_common::KString kname() const;

    void printNameTo(Print& printer) const;
    void printShortNameTo(Print& printer) const;
};


class ExtendedZone {
  public:
    ExtendedZone(const extended::ZoneInfo* zoneInfo);

    bool isNull() const;
    uint32_t zoneId() const;
    int16_t stdOffsetMinutes() const;
    ace_common::KString kname() const;

    void printNameTo(Print& printer) const;
    void printShortNameTo(Print& printer) const;
}
```

The `isNull()` method returns true if the object is a wrapper around a
`nullptr`. This is often used to indicate an error condition, or a "Not Found"
condition.

The `stdOffsetMinutes()` method returns the standard (i.e. normal time, not DST
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

The `BasicZone` and `ExtendedZone` objects are meant to be used transiently,
created on the stack then thrown away. For example:

```C++
const basic::ZoneInfo* zoneInfo = ...;
BasicZone(zoneInfo).printNameTo(Serial);
Serial.println();
```

Both `BasicZone` and `ExtendedZone` are light-weight wrapper objects around a
`const ZoneInfo*` pointer. In fact, they are so light-weight that the C++
compiler should be able to optimize away both wrapper classes entirely, so that
they are equivalent to using the `const ZoneInfo*` pointer directly.

If you need to copy the zone names into memory, use the `PrintStr<N>` class from
the AceCommon library (https://github.com/bxparks/AceCommon) to print the
zone name into the memory buffer, then extract the string from the buffer:

```C++
#include <AceCommon.h>
using ace_common::PrintStr;
...

const basic::ZoneInfo* zoneInfo = ...;
PrintStr<32> printStr; // buffer of 32 bytes on the stack
BasicZone(zoneInfo).printNameTo(printStr);

const char* name = printStr.cstr();
// do stuff with 'name', but only while 'printStr' is alive
...
```

See also the [Print To String](#PrintToString) section below.

<a name="ZonesAndLinks"></a>
### Zones and Links

The IANA TZ database contains 2 types of timezones:

* Zones, implemented by the `ZONE` keyword
* Links, implemented by the `LINK` keyword

A Zone entry is the canonical name of a given time zone in the IANA database
(e.g. `America/Los_Angeles`). A Link entry is an alias, an alternate name, for a
canonical entry (e.g. `US/Pacific` which points to `America/Los_Angeles`).

AceTime implements 3 types of Links, of which 2 are available in the most recent
version:

* Ghost Links (prior to v1.5, but no longer implemented)
* Fat Links (from v1.5 onwards)
* Thin Links (from v1.6 onwards)

<a name="GhostLinks"></a>
#### Ghost Links (Prior to v1.5)

Prior to AceTime v1.5, a Link entry was implemented using a C++ reference to the
corresponding Zone entry. In other words, the `kZoneUS_Pacific` symbol
was a C++ reference to the `kZoneAmerica_Los_Angeles` Zone data structure, and
the `kZoneUS_Pacific` object did not have an existence of its own. Ghost Links
existed only in the C++ source code, they did not exist within the AceTime
library on the Arduino side.

The positive aspects of this design were:

* the `kZoneUS_Pacific` Link entry consumed *no* additional flash memory,
  because the data structure for the Link did not exist
* the implementation was very simple, just a C++ reference

There were several negative consequences however:

* the `name` component of a `kZoneUS_Pacific` data structure was set to
  `"America/Los_Angeles"`, not `"US/Pacific"`, because the Link entry was just a
  C++ reference to the Zone entry
    * this means that `ZonedDateTime.printTo()` prints `America/Los_Angeles`,
      not `US/Pacific`
* similarly, the `zoneId` component of a `kZoneUS_Pacific` struct was set to
  the same `zoneId` of `kZoneAmerica_Los_Angeles`
* the `kZoneIdUS_Pacific` constant did not exist, in contrast to the target
  `KZoneIdAmerica_Los_Angeles` constant
* the `zonedb::kZoneRegistry` and `zonedbx::kZoneRegistry` contained only
  the Zone entries, not the Link entries
    * it was then not possible to create a `TimeZone` object through
      `ZoneManager::createForZoneName()` or `ZoneManager::createForZoneId()`
      using the Link identifiers instead of the Zone identifiers

But the biggest problem was that the IANA TZ database does not guarantee the
forward stability of the Zone and Link identifiers. Sometimes a Zone entry
becomes a Link entry. For example, in TZ DB version 2020f, the Zone
`Australia/Currie` became a Link that points to Zone `Australia/Hobart`. This
causes problems at both compile-time and run-time for applications using
AceTime:

* Compile-time
    * If the application was using the ZoneId symbol `kZoneIdAustralia_Currie`,
      the app would no longer compile after upgrading to TZDB 2020f.
* Run-time
    * If the application had stored the ZoneId of `Australia/Currie` or the
      string `"Australia/Currie"` into the EEPROM, the application would no
      longer be able to obtain the `TimeZone` object of `Australia/Currie` after
      upgrading to TZDB 2020f.

These disadvantages led me to abandon ghost links, and implement Fat Links and
Thin Links described below.

<a name="FatLinks"></a>
#### Fat Links (From v1.5)

AceTime v1.5 provides support for Fat Links which make Links essentially
identical to their corresponding Zones. In other words:

* the ZoneInfo `kZone{xxx}` constant exists for every Link entry,
* the ZoneId `kZoneId{xxx}` constant exists for every Link entry,
* the `kZone{xxx}.name` field points to the full name of the **Link** entry,
  not the target Zone entry,
* the `kZone{xxx}.zoneId` field is set to the ZoneId of the **Link** entry.

From the point of view on the Arduino side, there is no difference between Links
and Zones. In fact, there is no ability to distinguish between a Zone and a Fat
Link at runtime.

The underlying `ZonePolicy` and `ZoneInfo` data structures are shared between
the corresponding Zone and Link entries, but those Link entries themselves
do consume extra flash memory.

To avoid performing a lookup in 2 separate registries, Fat Links are merged into
the Zone registry. The ZoneManagers can constructed using the combined registry,
like this:

```C++
BasicZoneManager zoneManager(
    zonedb::kZoneAndLinkRegistrySize,
    zonedb::kZoneAndLinkRegistry);

ExtendedZoneManager zoneManager(
    zonedbx::kZoneAndLinkRegistrySize,
    zonedbx::kZoneAndLinkRegistry);
```

<a name="ThinLinks"></a>
#### Thin Links (From v1.6)

**Breaking Change in v1.8**: Thin link functionality has been extracted from
the `ZoneManager` into `LinkManager`. See [Migrating to
LinkManagers](MIGRATING.md#MigratingToLinkManagers) for migrating instructions.

*Thin links* a lighter weight alternatives to *fat links* that may be useful if
flash memory is tight. A thin link preserves the forward stability of **zoneId**
(e.g. `0xb7f7e8f2`) but not the **zoneName** (e.g. `America/Los_Angeles`). Since
only the 4-byte zoneIds are stored, they do not take up as much flash memory as
fat links, but they are more tricky to use so I recommend them only if there is
no other option.

If the application stores the `zoneId` in EEPROM (for example), it is possible
that a subsequent version of the TZDB changes the Zone into a Link. If the
smaller `kZoneRegistry` is used instead of the full `kZoneAndLinkRegistry`, then
previous `zoneId` would fail to resolve. The thin link registry provides a small
lookup table that maps the old `zoneId` (which has now become a `linkId`) to its
target `zoneId`.

A separate registry called `kLinkRegistry` is provided that contains only the
mapping of the `linkId` to the corresponding `zoneId`. These are available
through the `BasicLinkManager` and `ExtendedLinkManager` classes, like this:

```C++
namespace ace_time {

class LinkManager {
  public:
    static const uint16_t kInvalidZoneId = 0x0;
};

class BasicLinkManager: public LinkManager {
  public:
    BasicLinkManager(
        uint16_t linkRegistrySize,
        const basic::LinkEntry* linkRegistry
    );

    uint32_t zoneIdForLinkId(uint32_t linkId) const;
    uint16_t linkRegistrySize() const;
};

class ExtendedLinkManager: public LinkManager {
  public:
    ExtendedLinkManager(
        uint16_t linkRegistrySize,
        const extended::LinkEntry* linkRegistry
    );

    uint32_t zoneIdForLinkId(uint32_t linkId) const;
    uint16_t linkRegistrySize() const;
};

}
```

These link managers are initialized with the `kLinkRegistrySize` and
`kLinkRegistry` parameters from `zonedb/zone_registry.h` and
`zonedbx/zone_registry.h` header files like this:

```C++
#include <AceTime.h>
using namespace ace_time;

...

// Thin links for the zonedb database
BasicLinkManager linkManager(
    zonedb::kLinkRegistrySize, zonedb::kLinkRegistry);

// Thin links for the zonedbx database
ExtendedLinkManager linkManager(
    zonedbx::kLinkRegistrySize, zonedbx::kLinkRegistry);
```

When the search for a `zoneId` fails, then the client application can choose to
perform a second lookup in the link registry, like this:

```C++
ExtendedZoneManager zoneManager(
    zonedbx::kZoneRegistrySize,
    zonedbx::kZoneRegistry);

ExtendedLinkManager linkManager(
    zonedbx::kLinkRegistrySize,
    zonedbx::kLinkRegistry);

TimeZone findTimeZone(uint32_t zoneId) {
  TimeZone tz = zoneManager.createForZoneId(zoneId);
  if (tz.isError()) {
    // Search the link registry.
    zoneId = linkManager.zoneIdForLinkId(zoneId);
    if (zoneId != LinkManager::kInvalidZoneId) {
      tz = zoneManager.createForZoneId(zoneId);
    }
  }
  return tz;
}
```

<a name="CustomZoneRegistry"></a>
#### Custom Zone Registry

On small microcontrollers, the default zone registries may be too large. The
`ZoneManager` can be configured with a custom zone registry. It needs
to be given an array of `ZoneInfo` pointers when constructed. For example, here
is a `BasicZoneManager` with only 4 zones from the `zonedb::` data set:

```C++
#include <AceTime.h>
using namespace ace_time;
...
static const basic::ZoneInfo* const kZoneRegistry[] ACE_TIME_PROGMEM = {
  &zonedb::kZoneAmerica_Los_Angeles,
  &zonedb::kZoneAmerica_Denver,
  &zonedb::kZoneAmerica_Chicago,
  &zonedb::kZoneAmerica_New_York,
};

static const uint16_t kZoneRegistrySize =
    sizeof(kZoneRegistry) / sizeof(basic::ZoneInfo*);

static const uint16_t CACHE_SIZE = 2;
static BasicZoneProcessorCache<CACHE_SIZE> zoneProcessorCache;
static BasicZoneManager zoneManager(
    kZoneRegistrySize, kZoneRegistry, zoneProcessorCache);
```

Here is the equivalent `ExtendedZoneManager` with 4 zones from the `zonedbx::`
data set:

```C++
#include <AceTime.h>
using namespace ace_time;
...
static const extended::ZoneInfo* const kZoneRegistry[] ACE_TIME_PROGMEM = {
  &zonedbx::kZoneAmerica_Los_Angeles,
  &zonedbx::kZoneAmerica_Denver,
  &zonedbx::kZoneAmerica_Chicago,
  &zonedbx::kZoneAmerica_New_York,
};

static const uint16_t kZoneRegistrySize =
    sizeof(kZoneRegistry) / sizeof(extended::ZoneInfo*);

static const uint16_t CACHE_SIZE = 2;
static ExtendedZoneProcessorCache<CACHE_SIZE> zoneProcessorCache;
static ExtendedZoneManager zoneManager(
    kZoneRegistrySize, kZoneRegistry, zoneProcessorCache);
```

The `ACE_TIME_PROGMEM` macro is defined in
[compat.h](src/ace_time/common/compat.h) and indicates whether the `ZoneInfo`
entries are stored in normal RAM or flash memory (i.e. `PROGMEM`). It **must**
be used for custom zoneRegistries because the `BasicZoneManager` and
`ExtendedZoneManager` expect to find them in static RAM or flash memory
according to this macro.

See examples in various unit tests:

* [tests/ZoneRegistrarTest](tests/ZoneRegistrarTest)
* [tests/TimeZoneTest](tests/TimeZoneTest)
* [tests/ZonedDateTimeBasicTest](tests/ZonedDateTimeBasicTest)
* [tests/ZonedDateTimeExtendedTest](tests/ZonedDateTimeExtendedTest)

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
  // elements as of TZDB 2021e, so this requires a microcontroller which can
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
* `acetime_t LocalDate::kInvalidEpochSeconds`
    * Error value returned by `toEpochSeconds()` methods
* `int32_t LocalDate::kInvalidUnixDays`
    * Error value returned by `toUnixDays()` methods
* `int32_t LocalDate::kInvalidUnixSeconds`
    * Error value returned by `toUnixSeconds()` methods
* `int64_t LocalDate::kInvalidUnixSeconds64`
    * Error value returned by `toUnixSeconds64()` methods

Similarly, many factory methods accept an `acetime_t`, `int32_t`, or `int64_t`
arguments and return objects of various classes (e.g. `LocalDateTime`,
`OffsetDateTime` or `ZonedDateTime`). When these methods are given the error
constants, they return an object whoses `isError()` method returns `true`.

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
```

A well-crafted application should check for these error conditions before
writing or displaying the objects to the user.

For example, the `LocalDate` class uses a single byte `int8_t` instead of 2 byte
`int16_t` to store the year. (This saves memory). The range of a `int8_t` type
is -128 to 127, which is interpreted to be the offset from the year 2000. The
value of -128 is a reserved value, so the actual valid range of a valid year is
1873 to 2127. Other data and time classes in the library use the `LocalDate`
class internally so will have the same range of validity. If you try to create
an instance with a year component outside of this range, an error object is
returned whose `isError()` method returns `true`. The following code snippet
prints "true":

```C+++
auto dt = LocalDateTime::forComponents(1800, 1, 1, 0, 0, 0); // invalid year
Serial.println(dt.isError() ? "true" : "false");
```

Another example, the `ZonedDateTime` class uses the generated ZoneInfo Database
in the `zonedb::` and `zonedbx::` namespaces. These data files are valid from
2000 until 2050. If you try to create a date outside of this range, an error
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
    * AceTime uses an epoch of 2000-01-01T00:00:00Z.
      The `acetime_t` type is a 32-bit signed integer whose smallest value
      is `-2^31` and largest value is `2^31-1`. However, the smallest value is
      used to indicate an internal "Error" condition, therefore the actual
      smallest `acetime_t` is `-2^31+1`. Therefore, the smallest and largest
      dates that can be represented by `acetime_t` is 1931-12-13T20:45:53Z to
      2068-01-19T03:14:07Z (inclusive).
    * To be safe, users of this library should stay at least 1 year away from
      the lower and upper limits of `acetime_t` (i.e. stay within the year 1932
      to 2067, inclusive).
* `toUnixSeconds()` and `forUnixSeconds()`
    * [Unix time](https://en.wikipedia.org/wiki/Unix_time) uses an epoch of
      1970-01-01T00:00:00Z. This method returns an `int32_t` like most 32-bit
      Unix systems. The range of dates is 1901-12-13T20:45:52Z to
      2038-01-19T03:14:07Z.
* `toUnixSeconds64()` and `forUnixSeconds64()`
    * 64-bit versions of `toUnixSeconds()` and `forUnixSeconds()` were added in
      v1.10. However, the largest value returned by this method is 3094168447,
      far smaller than `INT64_MAX`. This value corresponds to
      2068-01-19T03:14:07Z which is the largest date that can be stored
      internally using the 32-bit `acetime_t` type.
* `TimeOffset`
    * Implemented using `int16_t` in 1 minute increments.
* `LocalDate`, `LocalDateTime`
    * These classes (and all other Date classes which are based on these) use
      a single 8-bit signed byte to represent the 'year' internally. This saves
      memory, at the cost of restricting the range.
    * The value of -128 (`INT8_MIN`) is used to indicate an "invalid" value, so
      the actual range is [-127, 127]. This restricts the year range to [1873,
      2127].
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
    * It might be possible to use both a basic `TimeZone` created using a
      `zonedb::ZoneInfo` entry, and an extended `TimeZone` using a
      `zonedbx::ZoneInfo` entry, together in a single program. However, this is
      not a configuration that is expected to be used often, so it has not been
      tested well, if at all.
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
* `ZonedDateTime` Must Be Within StartYear and UntilYear
    * The `src/ace_time/zonedb` and `src/ace_time/zonedbx` zone files are
      valid only within the specified `startYear` and `untilYear` range, defined
      in the `kZoneContext` struct:
        * `ace_time::zonedb::kZoneContext`
        * `ace_time::zonedbx::kZoneContext`
    * A `ZonedDateTime` object cannot be created outside of that valid year
      range. This is explained in [ZoneInfo Year Range](#ZoneInfoYearRange).
* `BasicZoneProcessor`
    * Supports 1-minute resolution for AT and UNTIL fields.
    * Supports only a 15-minute resolution for the STDOFF and DST offset fields.
    * Sufficient to support the vast majority of timezones since 2000.
* `ExtendedZoneProcessor`
    * Has a 1-minute resolution for AT, UNTIL and STDOFF fields.
    * Supports only a 15-minute resolution for DST field.
    * All timezones after 1974 has DST offsets in 15-minutes increments.
* `zonedb/` and `zonedbx/` ZoneInfo entries
    * These statically defined data structures are loaded into flash memory
      using the `PROGMEM` keyword. The vast majority of the data structure
      fields will stay in flash memory and not copied into RAM.
    * The ZoneInfo entries have *not* been compressed using bit-fields or any
      other compression techniques. It may be possible to decrease the size of
      the full database using these compression techniques. However, compression
      will increase the size of the program file, so for applications that use
      only a small number of zones, it is not clear if the ZoneInfo entry
      compression will provide a reduction in the size of the overall program.
    * The TZ database files `backzone`, `systemv` and `factory` are
      not processed by the `tzcompiler.py` tool. They don't seem to contain
      anything worthwhile.
    * TZ Database version 2019b contains the first use of the
      `{onDayOfWeek<=onDayOfMonth}` syntax that I have seen (specifically `Rule
      Zion, FROM 2005, TO 2012, IN Apr, ON Fri<=1`). The actual transition date
      can shift into the previous month (or to the next month in the case of
      `>=`). However, shifting into the previous year or the next year is not
      supported. The `tzcompiler.py` will exclude and flag the Rules which could
      potentially shift to a different year. As of version 2019b, no such Rule
      seems to exist.
* Links
    * Even with the implementation of "fat links" (see *Zones and Links* section
      above), it is not possible to determine whether a given `ZoneInfo`
      instance is a Zone or a Link at run time.
    * Adding a byte-flag would be straight forward, but would increase flash
      memory consumption of `kZoneAndLinkRegistry` by 593 bytes. It's not clear
      if this feature is worth the cost of extra memory usage.
* Arduino Zero and SAMD21 Boards
    * SAMD21 boards (which all identify themselves as `ARDUINO_SAMD_ZERO`) are
      supported, but there are some tricky points.
    * If you are using an original Arduino Zero and using the "Native USB Port",
      you may encounter problems with nothing showing up on the Serial Monitor.
        * The original Arduino Zero has [2 USB
          ports](https://www.arduino.cc/en/Guide/ArduinoZero). The Programming
          port is connected to `Serial` object and the Native port is connected
          to `SerialUSB` object. You can select either the "Arduino/Genuino Zero
          (Programming Port)" or the "Arduino/Genuino Zero (Native USB Port)" on
          the Board Manager selector in the Arduino IDEA. Unfortunately, if you
          select "(Native USB Port)", the `SERIAL_MONITOR_PORT` macro *should*
          be defined to be `SerialUSB`, but it continues to point to `Serial`,
          which means that nothing will show up on the Serial Monitor.
        * You may be able to fix this by setting
          `ACE_TIME_CLOBBER_SERIAL_PORT_MONITOR` to `1` in
          `src/ace_time/common/compat.h`. (I do not test this option often, so
          it may be broken.)
    * If you are using a SAMD21 development or breakout board, or one of the
      many clones called something like "Ardunio SAMD21 M0 Mini" (this is what I
      have), I have been unable to find a board configuration that is an exact
      match. You have a few choices:
        * If you are running the [AceTime unit tests](tests/), you need to
          have a working `SERIAL_PORT_MONITOR`, so the "Arduino MKR ZERO" board
          might work better, instead of the "Arduino Zero (Native USB Port)"
          board.
        * If you are running an app that requires proper pin configuration,
          it seems that the `Arduino MKR ZERO" configuration is not correct for
          this clone board. You need to go back to the "Arduino/Genuino Zero
          (Native USB Port)" board configuration.
        * You may also try installing the [SparkFun
          Boards](https://github.com/sparkfun/Arduino_Boards) and select
          the "SparkFun SAMD21 Mini Breakout" board. The advantage of using
          this configuration is that the `SERIAL_PORT_MONITOR` is configured
          properly as well as the port pin numbers. However, I have found that
          the USB connection can be a bit flaky.
    * The `MKR Zero` board generates far faster code (30%?) than the `SparkFun
      SAMD21 Mini Breakout` board. The `MKR Zero` could be using a different
      (more recent?) version of the GCC tool chain. I have not investigated
      this.
