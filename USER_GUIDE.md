# AceTime Library User Guide

See the [README.md](README.md) for introductory background.

Version: 0.4 (2019-07-09, TZ DB version 2019a, beta)

## Installation

The latest stable release is available in the Arduino IDE Library
Manager since version 0.3.1. Search for "AceTime". Click Install.

The development version can be installed by cloning the
[GitHub repository](https://github.com/bxparks/AceTime), checking out the
`develop` branch, then manually copying over the contents to the `./libraries`
directory used by the Arduino IDE. (The result is a directory named
`./libraries/AceTime`.) The `master` branch contains the stable release.

### Source Code

The source files are organized as follows:
* `src/AceTime.h` - main header file
* `src/ace_time/` - date and time classes (`ace_time::`)
* `src/ace_time/common/` - shared classes and utilities (`ace_time::common`,
  `ace_time::logging`)
* `src/ace_time/internal/` - internal classes (`ace_time::basic`,
  `ace_time::extended`)
* `src/ace_time/hw/` - thin hardware abstraction layer (`ace_time::hw`)
* `src/ace_time/clock/` - system clock from RTC or NTP sources
  (`ace_time::clock`)
* `src/ace_time/testing/` - files used in unit tests (`ace_time::testing`)
* `src/ace_time/zonedb/` - files generated from TZ Database for
  `BasicZoneSpecifier` (`ace_time::zonedb`)
* `src/ace_time/zonedbx/` - files generated from TZ Database for
  `ExtendedZoneSpecifier` (`ace_time::zonedbx`)
* `tests/` - unit tests using [AUnit](https://github.com/bxparks/AUnit)
* `tests/validation` - integration tests using AUnit which must be run
   on desktop Linux or MacOS machines
* `examples/` - example programs
* `tools/` - parser for the TZ Database files, code generators for `zonedb::`
  and `zonedbx::` zone files, and code generators for various unit tests

### Dependencies

The vast majority of the AceTime library has no dependency to any other external
libraries. There is an optional dependency to
[AceRoutine](https://github.com/bxparks/AceRoutine) if you want to use the
`SystemClockSyncCoroutine` class for automatic syncing. (This is recommended but
not strictly necessary). The `ace_time/hw/CrcEeprom.h` class has a dependency to
the FastCRC library but the `CrcEeprom.h` file is not included in the
`AceTime.h` main header file, so you should not need FastCRC to compile AceTime.
(The `CrcEeprom.h` header file does not strictly belong in the AceTime library
but many of my "clock" projects that use the AceTime library also use the
`CrcEeprom` class, so this is a convenient place to keep it.)

Various programs in the `examples/` directory have one or more of the following
external dependencies. The comment section near the top of the `*.ino` file will
usually have more precise dependency information:

* [AceRoutine](https://github.com/bxparks/AceRoutine)
* [AceButton](https://github.com/bxparks/AceButton)
* [FastCRC](https://github.com/FrankBoesing/FastCRC)
* [SSD1306Ascii](https://github.com/greiman/SSD1306Ascii)
* [Arduino Time Lib](https://github.com/PaulStoffregen/Time)
* [Arduino Timezone](https://github.com/JChristensen/Timezone)

Various scripts in the `tools/` directory depend on:

* [TZ Database on GitHub](https://github.com/eggert/tz)
* [pytz library](https://pypi.org/project/pytz/)
* Python 3.5 or greater
* Java OpenJDK 11

### Doxygen Docs

The [docs/](docs/) directory contains the
[Doxygen docs on GitHub Pages](https://bxparks.github.io/AceTime/html).

### Examples

The following programs are provided in the `examples/` directory:

* [HelloDateTime](examples/HelloDateTime/)
    * demo program of various date and time classes
* [HelloSystemClock](examples/HelloSystemClock/)
    * demo program of `SystemClock`
* [HelloSystemClockCoroutine](examples/HelloSystemClockCoroutine/)
    * same as `SystemClock` but using AceRoutine coroutines
* [CommandLineClock](examples/CommandLineClock/)
    * a clock with a DS3231 RTC chip, an NTP client, and using the serial port
      for receiving commands and printing results, useful for debugging
* [OledClock](examples/OledClock/)
    * a digital clock using a DS3231 RTC chip, an NTP client, 2 buttons, and an
      SSD1306 OLED display
* [WorldClock](examples/WorldClock/)
    * a clock with 3 OLED screens showing the time at 3 different time zones
* [AutoBenchmark](examples/AutoBenchmark/)
    * perform CPU and memory benchmarking of various methods and print a report
* [ComparisonBenchmark](examples/ComparisonBenchmark/)
    * compare AceTime with
    [Arduino Time Lib](https://github.com/PaulStoffregen/Time)
* [CrcEepromDemo](examples/CrcEepromDemo/)
    * a program that verifies the `CrcEeprom` class

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
the TZ Database that is required to support the selected timezones (1 to 3 have
been tested). (A future version on the ESP8266 and ESP32 microcontrollers (which
have far more memory than AVR chips) may provide an option to load the entire TZ
database at run time. This will allow the end-user to select the timezone
dynamically, just like on the big-iron machines.)

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
to move most of the complex memory handling logic into the `ZoneSpecifier` class
hierarhcy. These are relatively large objects which are meant to be opaque
objects to the application developer, created statically at start-up time of
the application, and never deleted during the lifetime of the application.

The [Arduino Time](https://github.com/PaulStoffregen/Time) library uses a set of
C functions similar to the [traditional C/Unix library
methods](http://www.catb.org/esr/time-programming/) (e.g `makeTime()` and
`breaktime()`). Arduino Time Library also uses the Unix epoch of
1970-01-01T00:00:00Z and a `int32_t` type as its `time_t` to track the number of
seconds since the epoch. That means that the largest date it can handle is
2038-01-19T03:14:07Z. AceTime uses an epoch that starts on 2000-01-01T00:00:00Z
using the same `int32_t` as its `ace_time::acetime_t`, which means that maximum
date increases to 2068-01-19T03:14:07Z. AceTime is also quite a bit faster than
the Arduino Time Library (although in most cases, performance of the Time
Library is not an issue): AceTime is **2-5X** faster on an ATmega328P, **3-5X**
faster on the ESP8266, **7-8X** faster on the ESP32, and **7-8X** faster on the
Teensy ARM processor.

AceTime aims to be the smallest library that can run on the basic Arduino
platform (e.g. Nano with 32kB flash and 2kB of RAM) that fully supports all
timezones in the TZ Database at compile-time. Memory constraints of the smallest
Arduino boards may limit the number of timezones supported by a single program
at runtime to 1-3 timezones. The library also aims to be as portable as
possible, and supports AVR microcontrollers, as well as ESP8266, ESP32 and
Teensy microcontrollers.

## Headers and Namespaces

Only a single header file `AceTime.h` is required to use this library.
To prevent name clashes with other libraries that the calling code may use, all
classes are separated into a number of namespaces. They are related in the
following way, where the arrow means "depends on":

```
ace_time::clock     ace_time::testing
      |       \        /
      |        v      v
      |        ace_time
      |         |\     \
      |         | \     v
      |         |  \    ace_time::zonedb
      |         |   \   ace_time::zonedbx
      |         |    \     |
      v         |     v    v
ace_time::hw    |     ace_time::basic
          \     |     ace_time::extended
           \    |     /
            \   |    /
             v  v   v
           ace_time::common
           ace_time::logging
```

To use the classes without prepending the namespace prefixes, use one or more of
the following `using` directives:

```C++
#include <AceTime.h>
using namespace ace_time;
using namespace ace_time::clock;
using namespace ace_time::common;
using namespace ace_time::zonedb;
using namespace ace_time::zonedbx;
...
```

## Date and Time Classes

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
    static const acetime_t kInvalidEpochDays = INT32_MIN;
    static const acetime_t kInvalidEpochSeconds = LocalTime::kInvalidSeconds;
    static const acetime_t kSecondsSinceUnixEpoch = 946684800;

    static const uint8_t kMonday = 1;
    static const uint8_t kTuesday = 2;
    static const uint8_t kWednesday = 3;
    static const uint8_t kThursday = 4;
    static const uint8_t kFriday = 5;
    static const uint8_t kSaturday = 6;
    static const uint8_t kSunday = 7;

    static LocalDate forComponents(int16_t year, uint8_t month, uint8_t day);
    static LocalDate forEpochDays(acetime_t epochDays);
    static LocalDate forUnixDays(acetime_t unixDays);
    static LocalDate forEpochSeconds(acetime_t epochSeconds);
    static LocalDate forUnixSeconds(acetime_t unixSeconds);

    int16_t year() const;
    void year(int16_t year);

    uint8_t month() const;
    void month(uint8_t month);

    uint8_t day() const;
    void day(uint8_t day);

    uint8_t dayOfWeek() const;
    bool isError() const;

    acetime_t toEpochDays() const {
    acetime_t toUnixDays() const {
    acetime_t toEpochSeconds() const {
    acetime_t toUnixSeconds() const {

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

### Date Strings

To convert the `dayOfweek()` numerical code to a human-readable string for
debugging or display, we can use the `common::DateStrings` class:


```C++
namespace ace_time {
namespace common {

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

The `DateStrings` class is inside the `ace_time::common` namespace, so you need
to prefix it with `common::`, unless a `using` statement is used.

```C++
#include <AceTime.h>
using namespace ace_time;
using common::DateStrings;
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
    static LocalDateTime forUnixSeconds(acetime_t unixSeconds);
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

    acetime_t toEpochDays() const;
    acetime_t toUnixDays() const;
    acetime_t toEpochSeconds() const;
    acetime_t toUnixSeconds() const;

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

### TimePeriod

The `TimePeriod` class can be used to represents a difference between two
`XxxDateTime` objects, if the difference is not too large. Internally, it is
implemented as 3 unsigned `uint8_t` integers representing the hour, minute and
seconds. There is a 4th signed `int8_t` integer that holds the sign (-1 or +1)
of the time period. The largest (or smallest) time period that can be
represented by this class is +/- 255h59m59s, corresponding to +/- 921599
seconds.

```C++
namespace ace_time {

class TimePeriod {
  public:
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

### TimeOffset

A `TimeOffset` class represents an amount of time shift from a reference point.
Often the reference is the UTC time and this class represents the amount of time
shift from UTC. Currently (year 2019) every time zone in the world is shifted
from UTC by a multiple of 15 minutes (e.g. -03:30 or +01:00). `TimeOffset` is a
thin wrapper around a single `int8_t` type which can encode integers from
[-128, 127]. Internally -128 is used to indicate an error condition, so we can
represent a UTC shift of from -31:45 to +31:45 hours, which is more than enough
to encode all UTC offsets currently in use in the world.

```C++
namespace ace_time {

class TimeOffset {
  public:
    static TimeOffset forHour(int8_t hour);
    static TimeOffset forHourMinute(int8_t hour, int8_t minute);
    static TimeOffset forMinutes(int16_t minutes);

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
auto offset = TimeOffset::forHour(-8); // -08:00
auto offset = TimeOffset::forHourMinute(-2, -30); // -02:30
auto offset = TimeOffset::forMinutes(135); // +02:15
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
`true`. Internally, this is an instance whose internal integer code is -128.

The convenience method `TimeOffset::isZero()` returns `true` if the offset has a
zero offset. This is often used to determine if a timezone is currently
observing Daylight Saving Time (DST).

### OffsetDateTime

An `OffsetDateTime` is an object that can represent a `LocalDateTime` which is
offset from the UTC time zone by a fixed amount. Internally the `OffsetDateTime`
is a aggregation of `LocalDateTime` and `TimeOffset`. Use this class for
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
    static OffsetDateTime forUnixSeconds(acetime_t unixSeconds,
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

    acetime_t toEpochDays() const;
    acetime_t toUnixDays() const;
    acetime_t toEpochSeconds() const;
    acetime_t toUnixSeconds() const;

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
acetime_t epochDays = offsetDateTime.toEpochDays();
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

### TimeZone

A "time zone" is often used colloquially to mean 2 different things:
* A time which is offset from the UTC time by a fixed amount, or
* A physical (or conceptual) region whose local time is offset
from the UTC time using various transition rules.

Both meanings of "time zone" are supported by the `TimeZone` class using
3 different types as follows:

* `TimeZone::kTypeManual`: a fixed base offset and optional DST offset from UTC
* `TimeZone::kTypeBasic` with `BasicZoneSpecifier`: zones which can
be encoded with (relatively) simple rules from the TZ Database
* `TimeZone::kTypeExtended` with `ExtendedZoneSpecifier`:  all zones in
the TZ Database

The class hierarchy of `TimeZone` is shown below, where the arrow means
"is-subclass-of" and the diamond-line means "is-aggregation-of". A `TimeZone`
can hold a reference to 0 or 1 `ZoneSpecifier` class, as shown below. When it is
a `kTypeFixed` it holds no `ZoneSpecifier` object. When it is a `kTypeManual`,
`kTypeBasic` or `kTypeExtended`, it holds a reference one of the implementation
classes of `ZoneSpecifier`:

```
              0..1
TimeZone <>------- ZoneSpecifier
                         ^
                         |
                   .---- +----.
                   |          |
                Basic        Extended
         ZoneSpecifier       ZoneSpecifier
```

Here is the class declaration of `TimeZone`:

```C++
namespace ace_time {

class TimeZone {
  public:
    static const uint8_t kTypeManual = 0;
    static const uint8_t kTypeBasic = ZoneSpecifier::kTypeBasic;
    static const uint8_t kTypeExtended = ZoneSpecifier::kTypeExtended;

    static TimeZone forTimeOffset(TimeOffset stdOffset,
        TimeOffset dstOffset = TimeOffset());
    static TimeZone forZoneSpecifier(const ZoneSpecifier* zoneSpecifier);
    TimeZone();

    uint8_t getType() const;
    TimeOffset getUtcOffset(acetime_t epochSeconds) const;
    TimeOffset getDeltaOffset(acetime_t epochSeconds) const;
    TimeOffset getUtcOffsetForDateTime(const LocalDateTime& ldt) const;

    bool isUtc() const;
    bool isDst() const;
    void setDstOffset(TimeOffset offset);

    void printTo(Print& printer) const;
    void printShortTo(Print& printer) const;
    void printAbbrevTo(Print& printer, acetime_t epochSeconds) const;
};

}
```

The `getUtcOffset(epochSeconds)` returns the total `TimeOffset` (including any
DST offset) at the given `epochSeconds`. The `getDeltaOffset()` returns only the
additional DST offset; if DST is not in effect at the given `epochSeconds`, this
returns a `TimeOffset` whose `isZero()` returns true.

The `getUtcOffsetForDateTime(localDateTime)` method returns the best guess of
the total UTC offset at the given local date time. The reaon that this is a best
guess is because the local date time is sometime ambiguious during a DST
transition. For example, if the local clock shifts from 01:00 to 02:00 at the
start of summer, then the time of 01:30 does not exist. If the
`getUtcOffsetForDateTime()` method is given a non-existing time, it makes an
educated guess at what the user meant. Additionally, when the local time
transitions from 02:00 to 01:00 in the autumn, a given local time such as 01:30
occurs twice. If the `getUtcOffsetForDateTime()` method is given a time of
01:30, it will arbitrarily decide which offset to return.

The `isUtc()`, `isDst()` and `setDstOffset(TimeOffset)` methods are valid *only*
if the `TimeZone` is a `kTypeManual`. Otherwise, `isUtc()` and `isDst()` return
`false` and `setDstOffset()` does nothing.

The `printTo()` prints the fully-qualified unique name for the time zone.
For example, `"UTC"`, `"-08:00", `"-08:00(DST)"`, `"America/Los_Angeles"`.

The `printShortTo()` is similar to `printTo()` except that it prints the
last component of the IANA TZ Database zone names. In other words,
`"America/Los_Angeles"` is printed as `"Los_Angeles"`. This is helpful for
printing on small width OLED displays.

The `printAbbrevTo(printer, epochSeconds)` method prints the human-readable
timezone abbreviation used at the given `epochSeconds` to the `printer`. For
example, this be "PST" for Pacific Standard Time, or "BST" for British Summer
Time.

#### Manual TimeZone (kTypeManual)

The default constructor creates a `TimeZone` in UTC time zone with no
offset:

```C++
TimeZone tz; // UTC+00:00
```

To create `TimeZone` instances with other offsets, use the `forTimeOffset()`
factory method:

```C++
auto tz = TimeZone::forTimeOffset(TimeOffset::forHour(-8)); // UTC-08:00
auto tz = TimeZone::forTimeOffset(TimeOffset::forHourMinute(-4, -30)); // UTC-04:30
auto tz = TimeZone::forTimeOffset(TimeOffset::forHour(-8),
    TimeOffset::forHour(1)); // UTC-08:00+01:00 (effectively -07:00)
```

The `TimeZone::isUtc()`, `TimeZone::isDst()` and `TimeZone::setDst(bool)`
methods work only if the `TimeZone` is a `kTypeManual`.

The `setDstOffset()` takes a `TimeOffset` as the argument instead of a simple
`bool` because there are some zones (e.g. Europe/Dublin) which uses a negative
offset in the winter, instead of adding a postive offset in the summer.

#### BasicZoneSpecifier (kTypeBasic)

The `BasicZoneSpecifier` represents a time zone defined by the TZ Database. The
constructor accepts a pointer to a `basic::ZoneInfo`:

```C++
BasicZoneSpecifier(const basic::ZoneInfo* zoneInfo);
```

The supported `basic::ZoneInfo` data objects are contained in this header file:

* [zonedb/zone_infos.h](src/ace_time/zonedb/zone_infos.h)

The zoneinfo files were generated by a script using the TZ Database. This header
file is already included in `<AceTime.h>` so you don't have to explicitly
include it. As of version 2019a of the database, it contains 270 Zone and 182
Link entries and whose time change rules are simple enough to be supported by
`BasicZoneSpecifier`. The bottom of the `zone_infos.h` header file lists 117
zones whose zone rules are too complicated for `BasicZoneSpecifier`.

The zone names are normalized so that the ZoneInfo variable is a valid C++ name:

* a `+` (plus) character in the zone name is replaced with `_PLUS_` to avoid
  conflict with a `-` (minus) character (e.g. `GMT+0` becomes `GMT_PLUS_0`)
* any remaining non-alphanumeric character (`0-9a-zA-Z_`) are replaced with
  an underscore (`_`) (e.g. `GMT-0` becomes `GMT_0`)

Some examples of `ZoneInfo` entries supported by `zonedb` are:

* `zonedb::kZoneAmerica_Los_Angeles`
* `zonedb::kZoneAmerica_New_York`
* `zonedb::kZoneAustralia_Darwin`
* `zonedb::kZoneEurope_London`
* `zonedb::kZoneGMT_PLUS_10`

The following example creates a `TimeZone` using a `BasicZoneSpecifier` which
describes `America/Los_Angeles`:

```C++
#include <AceTime.h>
using namespace ace_time;
...

BasicZoneSpecifier zoneSpecifier(&zonedb::kZoneAmerica_Los_Angeles);

void someFunction() {
  ...
  auto tz = TimeZone::forZoneSpecifier(&zoneSpecifier);

  // 2018-03-11T01:59:59-08:00 was still in STD time
  {
    auto dt = OffsetDateTime::forComponents(2018, 3, 11, 1, 59, 59,
      TimeOffset::forHour(-8));
    acetime_t epochSeconds = dt.toEpochSeconds();
    auto offset = tz.getUtcOffset(epochSeconds); // returns -08:00
  }

  // 2018-03-11T02:00:00-08:00 was in DST time
  {
    auto dt = OffsetDateTime::forComponents(2018, 3, 11, 2, 0, 0,
      TimeOffset::forHour(-8));
    acetime_t epochSeconds = dt.toEpochSeconds();
    auto offset = tz.getUtcOffset(epochSeconds); // returns -07:00
  }
  ...
}
```

#### ExtendedZoneSpecifier (kTypeExtended)

The `ExtendedZoneSpecifier` is very similar to `BasicZoneSpecifier` except that
it supports (almost) all zones in the TZ Database instead of a subset. The
constructor accepts a pointer to an `extended::ZoneInfo`:

```C++
ExtendedZoneSpecifier(const extended::ZoneInfo* zoneInfo);
```

The supported zones are given in this header file:

* [zonedbx/zone_infos.h](src/ace_time/zonedbx/zone_infos.h)

As of version 2019a of TZ Database, *all* 387 Zone and 205 Link entries from the
following TZ files are supported: `africa`, `antarctica`, `asia`, `australasia`,
`backward`, `etcetera`, `europe`, `northamerica`, `southamerica`. There are 3
files which are not processed (`backzone`, `systemv`, `factory`) because they
don't seem to contain anything useful.

The zone infos which can be used by `ExtendedZoneSpecifier` are in the
`zonedbx::` namespace instead of the `zonedb::` namespace. Some examples of the
zone infos which exists in `zonedbx::` but not in `zonedb::` are:
* `zonedbx::kZoneAfrica_Casablanca`
* `zonedbx::kZoneAmerica_Argentina_San_Luis`
* `zonedbx::kZoneAmerica_Indiana_Petersburg`
* `zonedbx::kZoneAsia_Hebron`
* `zonedbx::kZoneEurope_Moscow`

The usage is the same as `BasicZoneSpecifier`:

```C++
ExtendedZoneSpecifier zoneSpecifier(&zonedbx::kZoneAmerica_Los_Angeles);

void someFunction() {
  ...
  TimeZone tz = TimeZone::forZoneSpecifier(&zoneSpecifier);

  // 2018-03-11T01:59:59-08:00 was still in STD time
  {
    auto dt = OffsetDateTime::forComponents(2018, 3, 11, 1, 59, 59,
      TimeOffset::forHour(-8));
    acetime_t epochSeconds = dt.toEpochSeconds();
    auto offset = tz.getUtcOffset(epochSeconds); // returns -08:00
  }

  // 2018-03-11T02:00:00-08:00 was in DST time
  {
    auto dt = OffsetDateTime::forComponents(2018, 3, 11, 2, 0, 0,
      TimeOffset::forHour(-8));
    acetime_t epochSeconds = dt.toEpochSeconds();
    auto offset = tz.getUtcOffset(epochSeconds); // returns -07:00
  }
  ...
}
```

The advantage of `ExtendedZoneSpecifier` over `BasicZoneSpecifier` is that
`ExtendedZoneSpecifier` supports all time zones in the TZ Database. The cost is
that it consumes 5 times more memory and is a bit slower. If
`BasicZoneSpecifier` supports the zone that you want using the zone files in the
`zonedb::` namespace, you should normally use that instead of
`ExtendedZoneSpecifier`. The one other advatnage of `ExtendedZoneSpecifier` over
`BasicZoneSpecifier` is that `ExtendedZoneSpecifier::forComponents()` is more
accurate than `BasicZoneSpecifier::forComponents()` because the `zonedbx::` data
files contain transition information which are missing in the `zonedb::` data
files due to space constraints.

### ZonedDateTime

A `ZonedDateTime` is a `LocalDateTime` associated with a given `TimeZone`. This
is analogous to an`OffsetDateTime` being a `LocalDateTime` associated with a
`TimeOffset`. All 4 types of `TimeZone` are supported, the `ZonedDateTime`
class itself does not care which one is used. You should use the `ZonedDateTime`
when interacting with human beings, who are aware of timezones and DST
transitions. It can also be used to convert time from one timezone to anther
timezone.

```C++
namespace ace_time {

class ZonedDateTime {
  public:
    static const acetime_t kInvalidEpochSeconds = LocalTime::kInvalidSeconds;

    static ZonedDateTime forComponents(int16_t year, uint8_t month, uint8_t day,
            uint8_t hour, uint8_t minute, uint8_t second,
            const TimeZone& timeZone);

    static ZonedDateTime forEpochSeconds(acetime_t epochSeconds,
        const TimeZone& timeZone);

    static ZonedDateTime forUnixSeconds(acetime_t unixSeconds,
        const TimeZone& timeZone);

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

    acetime_t toEpochDays() const;
    acetime_t toUnixDays() const;
    acetime_t toEpochSeconds() const;
    acetime_t toUnixSeconds() const;

    int8_t compareTo(const ZonedDateTime& that) const;
    void printTo(Print& printer) const;

    ...
};

}
```

Here is an example of how to create one and extract the epoch seconds:

```C++
BasicZoneSpecifier zoneSpecifier(&zonedb::kZoneAmerica_Los_Angeles);

void someFunction() {
  ...
  auto tz = TimeZone::forZoneSpecifier(&zoneSpecifier);

  // 2018-01-01 00:00:00+00:15
  auto zonedDateTime = ZonedDateTime::forComponents(
      2018, 1, 1, 0, 0, 0, tz);
  acetime_t epochDays = zonedDateTime.toEpochDays();
  acetime_t epochSeconds = zonedDateTime.toEpochSeconds();

  zonedDateTime.printTo(Serial); // prints "2018-01-01 00:00:00-08:00"
  Serial.println(epochDays); // prints 6574 [TODO: Check]
  Serial.println(epochSeconds); // prints 568079100 [TODO: Check]
  ...
}
```

Both `printTo()` and `forDateString()` are expected to be used only for
debugging. The `printTo()` prints a human-readable representation of the date in
[ISO 8601](https://en.wikipedia.org/wiki/ISO_8601) format
(yyyy-mm-ddThh:mm:ss+/-hh:mm) to the given `Print` object. The most common
`Print` object is the `Serial` object which prints on the serial port. The
`forDateString()` parses the ISO 8601 formatted string and returns the
`ZonedDateTime` object.

**Caveat**: The parser for `forDateString()` looks only at the UTC offset. It
does *not* recognize the TZ Database identifier (e.g. `[America/Los_Angeles]`).
To handle the time zone identifier correctly, the library needs to load
the entire TZ Database into memory and be able to create the
`BasicZoneSpecifier` or `ExtendedZoneSpecifier` dynamically. But the dataset
is too large to fit on most AVR microcontrollers with only 32kB of flash memory.
(It may be possible to add this functionality for the ESP8266 or ESP32
platforms which have far more memory.) The `ZonedDateTime::timeZone()` will
return a `TimeZone` instance whose `TimeZone::getType()` returns
`TimeZone::kTypeFixed`.

#### Conversion to Other Time Zones

You can convert a given `ZonedDateTime` object into a representation in a
different time zone using the `DateTime::convertToTimeZone()` method:

```C++
static BasicZoneSpecifier zspecLosAngeles(&zonedb::kZoneAmerica_Los_Angeles);
static BasicZoneSpecifier zspecZurich(&zonedb::kZoneEurope_Zurich);

void someFunction() {
  ...
  auto tzLosAngeles = TimeZone::forZoneSpecifier(&zspecLosAngeles);
  auto tzZurich = TimeZone::forZoneSpecifier(&zspecZurich);

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

#### Caching

The conversion from an epochSeconds to date-time components using
`ZonedDateTime::forEpochSeconds()` is an expensive operation (see
[AutoBenchmark](examples/AutoBenchmark/)). To improve performance, the
`BasicZoneSpecifier` and `ExtendedZoneSpecifier` implement internal caching
based on the `year` component. This optimizes the most commonly expected
use case where the epochSeconds is incremented by a clock (e.g. `SystemClock`)
every second, and is converted to human-readable date-time components once a
second. According to [AutoBenchmark](examples/AutoBenchmark/), the cache
improves performance by a factor of 2-3X (8-bit AVR) to 10-20X (32-bit
processors) on consecutive calls to `forEpochSeconds()` with the same `year`.

### ZoneInfo Files

As of version 0.4, the zoneinfo files are stored in in flash memory instead of
static RAM using the
[PROGMEM](https://www.arduino.cc/reference/en/language/variables/utilities/progmem/)
keyword on microcontrollers which support this feature. On an 8-bit
microcontroller, the `zonedb/` database consumes about 14kB of flash
memory, so it may be possible to create small programs that can dynamically
access all timezones supported by `BasicZoneSpecifier`. The `zonedbx/` database
consumes about 23kB of flash memory and the addition code size from various
classes will exceed the 30-32kB limit of a typical Arduino 8-bit
microcontroller.

The `zonedb/` files do not support all the timezones in the TZ Database.
The list of these zones and The reasons for excluding them are given at the
bottom of the [zonedb/zone_infos.h](src/ace_time/zonedb/zone_infos.h) file.

Although the `zonedbx/` files support all zones from its TZ input files, there
are number of timezones whose DST transitions in the past happened at 00:01
(instead of exactly at midnight 00:00). To save memory, the internal
representation used by AceTime supports transitions only at
15-minute boundaries. For these timezones, the DST transition time is shifted to
00:00 instead, and the transition happens one-minute earlier than it should. As
of TZ DB version 2019a, there are 5 zones affected by this rounding, as listed
at the bottom of [zonedbx/zone_infos.h](src/ace_time/zonedbx/zone_infos.h), and
these all occur before the year 2012.

#### BasicZone and ExtendedZone

The `basic::ZoneInfo` and `extended::ZoneInfo` (and its related data structures)
objects are meant to be *opaque* and simply passed into `BasicZoneSpecifier`,
`ExtendedZoneSpecifier`, `BasicZoneRegistrar` and `ExtendedZoneRegistrar`. The
internal formats of the `ZoneInfo` structures may change without warning, and
users of this library should not access its internal data members directly.

Two helper classes, `BasicZone` and `ExtendedZone`, provide stable access to
some of the internal fields:

```C++
namespace ace_time {

class BasicZone {
  public:
    BasicZone(const basic::ZoneInfo* zoneInfo);

#if ACE_TIME_USE_BASIC_PROGMEM
    const __FlashStringHelper* name() const;
    const __FlashStringHelper* shortName() const;
#else
    const char* name() const;
    const char* shortName() const;
#endif
};


class ExtendedZone {
  public:
    ExtendedZone(const extended::ZoneInfo* zoneInfo);

#if ACE_TIME_USE_EXTENDED_PROGMEM
    const __FlashStringHelper* name() const;
    const __FlashStringHelper* shortName() const;
#else
    const char* name() const;
    const char* shortName() const;
#endif

}
```

They are meant to be used transiently, for example:
```C++
...
const basic::ZoneInfo* zoneInfo = ...;
Serial.println(BasicZone(zoneInfo).shortName());
...
```

The return type of `name()` and `shortName()` change whether or not the zone
name is stored in flash memory or in static memory. The `name()` method returns
the full zone name from the TZ Database (e.g. `"America/Los_Angeles"`). The
`shortName()` method returns only the last component (e.g. `"Los_Angeles"`).

### ZoneRegistrar

**Caveat**: This is an experimental feature. Previously, `BasicZoneSpecifier`
and `ExtendedZoneSpecifier` were considered immutable objects. The ZoneRegistrar
enables dynamic lookup of `ZoneInfo`, which resulted in the `BasicZoneSpecifier`
and `ExtendedZoneSpecifier` becoming mutable with the `setZoneInfo()` method.
The consequence of this change has not been fully explored.

Two classes provide a lookup service from the zone identifier string
(e.g. `"America/Los_Angeles"`) to the zoneinfo files: `BasicZoneRegistrar` and
`ExtendedZoneRegistrar`. The class definitions look like this:

```C++
class BasicZoneRegistrar {
  public:
    BasicZoneRegistrar(const basic::ZoneInfo* zoneRegistry,
        uint16_t registrySize);

    uint16_t registrySize() const;
    bool isSorted() const;
    const basic::ZoneInfo* getZoneInfo(uint16_t i) const;
    const basic::ZoneInfo* getZoneInfo(const char* name) const;
};

class ExtendedZoneRegistrar {
  public:
    ExtendedZoneRegistrar(const extended::ZoneInfo* zoneRegistry,
        uint16_t registrySize);

    uint16_t registrySize() const;
    bool isSorted() const;
    const extended::ZoneInfo* getZoneInfo(uint16_t i) const;
    const extended::ZoneInfo* getZoneInfo(const char* name) const;
};
```

#### Default Zone Registry

On microcontrollers with more than ~30kB of flash (e.g. ESP8266, ESP32,
and Teensy), the entire `zonedb` or `zonedbx` database can be loaded and
timezones can be dynamically selected using the zone identifier strings.
The default zoneinfo registry is available at:

* [zonedb/zone_registry.h](src/ace_time/zonedb/zone_registry.h)
* [zonedbx/zone_registry.h](src/ace_time/zonedbx/zone_registry.h)

The `BasicZoneRegistrar` can loaded with the `zonedb::kZoneRegistry` default zone
registry and used to configure the `BasicZoneSpecifier` like this:

```C++
#include <AceTime.h>
...
static const basic::ZoneInfo* const defaultZoneInfo =
    zonedb::kZoneAmerica_Los_Angeles;
static BasicZoneSpecifier zoneSpecifier(defaultZoneInfo);
static const BasicZoneRegistrar zoneRegistrar(
    zonedb::kZoneRegistrySize, zonedb::kZoneRegistry);

void setZone(const char* zoneName) {
  const auto* zoneInfo = manager.getZoneInfo(zoneName);
  if (zoneInfo) {
    zoneSpecifier.setZoneInfo(zoneInfo);
    auto tz = TimeZone::forZoneSpecifier(&zoneSpecifier);
    ...
  }
}
```

Similarly, `ExtendedZoneRegistrar` can be configured with the
`zonedbx::kZoneRegistry` and used to configure the `ExtendedZoneSpecifier` like
this:

```C++
#include <AceTime.h>
...
static const extended::ZoneInfo* const defaultZoneInfo =
    zonedbx::kZoneAmerica_Los_Angeles;
static ExtendedZoneSpecifier zoneSpecifier(defaultZoneInfo);
static const ExtendedZoneRegistrar zoneRegistrar(
    zonedbx::kZoneRegistrySize, zonedbx::kZoneRegistry);

void setZone(const char* zoneName) {
  const auto* zoneInfo = manager.getZoneInfo(zoneName);
  if (zoneInfo) {
    zoneSpecifier.setZoneInfo(zoneInfo);
    auto tz = TimeZone::forZoneSpecifier(&zoneSpecifier);
    ...
  }
}
```

#### Custom Zone Registry

The default zone registries (`zonedb::kZoneRegistry` and
`zonedbx::kZoneRegistry`) pull in every zone in the respective data sets, which
consumes significant amounts of flash memory (see
[MemoryBenchmark](examples/MemoryBenchmark/)). If flash memory is tight, the
ZoneRegistrar can be initialized with a custom list of zones, to pull in only the
zones of interest. For example, here is a `kZoneRegistry` with 4 zones from the
`zonedb::` data set:

```C++
#include <AceTime.h>
...
static const basic::ZoneInfo* const kBasicZoneRegistry[]
    ACE_TIME_BASIC_PROGMEM = {
  &zonedb::kZoneAmerica_Los_Angeles,
  &zonedb::kZoneAmerica_Denver,
  &zonedb::kZoneAmerica_Chicago,
  &zonedb::kZoneAmerica_New_York,
};

static const uint16_t kZoneRegistrySize =
    sizeof(Controller::kZoneRegistry) / sizeof(basic::ZoneInfo*);

static const BasicZoneRegistrar zoneRegistrar(kZoneRegistrySize, kZoneRegistry);
```

and here is the equivalent `kExtendedZoneRegistry`:

```C++
#include <AceTime.h>
...
static const extended::ZoneInfo* const kZoneRegistry[]
    ACE_TIME_EXTENDED_PROGMEM = {
  &zonedbx::kZoneAmerica_Los_Angeles,
  &zonedbx::kZoneAmerica_Denver,
  &zonedbx::kZoneAmerica_Chicago,
  &zonedbx::kZoneAmerica_New_York,
};

static const uint16_t kZoneRegistrySize =
    sizeof(Controller::kZoneRegistry) / sizeof(extended::ZoneInfo*);

static const BasicZoneRegistrar zoneRegistrar(kZoneRegistrySize, kZoneRegistry);
```

(The `ACE_TIME_BASIC_PROGMEM` and `ACE_TIME_EXTENDED_PROGMEM` macros are defined
in [config.h](src/ace_time/config.h) and determines whether the ZoneInfo files
are stored in normal RAM or flash memory. They are needed because the
ZoneRegistrar need to know where the ZoneInfo files are stored.)

See [CommandLineClock](examples/CommandLineClock/) for an example of how these
custom registries can be created and used.

### TZ Database Version

The IANA TZ Database is updated continually. As of this writing, the latest
stable version is 2019a. When a new version of the database is released, it is
relatively easy to regenerate the `zonedb/` and `zonedbx/` zoneinfo files.
However, it is likely that I would delay the release of a new version until the
corresponding `pytz` package is updated to the latest TZ database version, so
that the validation test suites pass (See Testing section below). Otherwise, I
don't have a way to verify that the AceTime library with the new TZ Database
version is correctly functioning.

## Mutations

Mutating the date and time classes can be tricky. In fact, many other
time libraries (such as [Java 11
Time](https://docs.oracle.com/en/java/javase/11/docs/api/java.base/java/time/package-summary.html),
[Joda-Time](https://www.joda.org/joda-time/), and [Noda
Time](https://nodatime.org/)) avoid the problem altogether by making all objects
immutable. In those libraries, mutations occur by creating a new copy of the
target object with a new value for the mutated parameter. Making the objects
immutable is definitely cleaner, but it causes the code size to increase
significantly. For the case of the [WorldClock](example/WorldClock) program,
the code size increased by 500-700 bytes, which I could not afford because the
program takes up almost the entire flash memory of an Ardunio Pro Micro with
only 28672 bytes of flash memory.

Most date and time classes in the AceTime library are mutable. The mutation
operations are not implemented within the class itself to avoid bloating
the class API surface. The mutation functions live as functions in separate
namespaces outside of the class definitions:

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
can reduce the size of the code in flash. (The limiting factor for many Arduino
environments is the code size, not the CPU time.)

It is not clear that making the AceTime objects mutable was the best design
decision. But it seems to produce far smaller code sizes (hundreds of bytes of
flash memory saved for something like
[examples/WorldClock](examples/WorldClock)), while providing the features that I
need to implement the various Clock applications.

### TimeOffset Mutation

The `TimeOffset` object can be mutated with:

```C++
namespace ace_time {
namespace time_offset_mutation {

void increment15Minutes(TimeOffset& offset);

}
}
```

### LocalDate Mutation

The `LocalDate` object can be mutated with the following methods:

```C++
namespace ace_time {
namespace local_date_mutation {

void incrementOneDay(LocalDate& ld);
void decrementOneDay(LocalDate& ld);

}
}
```

### ZonedDateTime Mutation

The `ZonedDateTime` object can be mutated using the following methods:

```C++
namespace ace_time {
namespace zoned_date_time_mutation {

void incrementYear(ZonedDateTime& dateTime);
void incrementMonth(ZonedDateTime& dateTime);
void incrementDay(ZonedDateTime& dateTime);
void incrementHour(ZonedDateTime& dateTime);
void incrementMinute(ZonedDateTime& dateTime);

]
}
```

### TimePeriod Mutation

The `TimePeriod` can be mutated using the following methods:

```C++
namespace ace_time {
namespace time_period_mutation {

void negate(TimePeriod& period);
void incrementHour(TimePeriod& period, uint8_t limit);
void incrementHour(TimePeriod& period);
void incrementMinute(TimePeriod& period);

}
}
```

## Error Handling

Many features of the date and time classes have explicit or implicit range of
validity in their inputs and outputs. The Arduino programming environment does
not use C++ exceptions, so we handle invalid values by returning special version
of various date/time objects to the caller.

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

Another example, the `ZonedDateTime` class uses the generated TZ Database in
the `zonedb::` and `zonedbx::` namespaces. These data files are valid from 2000
until 2050. If you try to create a date outside of this range, an error
`ZonedDateTime` object will returned. The following snippet will print "true":

```C++
BasicZoneSpecifier zoneSpecifier(&zonedb::kZoneAmerica_Los_Angeles);
auto tz = TimeZone::forZoneSpecifier(&zoneSpecifier);
auto dt = ZonedDateTime::forComponents(1998, 3, 11, 1, 59, 59, tz);
Serial.println(dt.isError() ? "true" : "false");
```

### LocalDate::kInvalidEpochSeconds

Many methods return an `acetime_t` type. For example, `toEpochSeconds()` or
`toUnixSeconds()` on `LocalDateTime`, `OffsetDateTime` or `ZonedDateTime`.
These methods will return `LocalDate::kInvalidEpochSeconds` if an invalid
value is calculated.

Similarly, there are many methods which accept an `acetime_t` as an argument and
returns a object of time `LocalDateTime`, `OffsetDateTime` or `ZonedDateTime`.
When these methods are passed a value of `LocalDate::kInvalidEpochSeconds`, the
resulting object will return a true value for `isError()`.

## Clock

The `acetime::clock` namespace contains classes needed to implement the
`SystemClock`. The `SystemClock` is a source of time (normally represented as a
`acetime_t` type, in other words, seconds from AceTime Epoch). The `SystemClock`
is normally powered by the internal `millis()` function, but that function is
usually not accurate enough. So the `SystemClock` has the ability to synchronize
against more accurate external clocks (e.g. NTP server). The `SystemClock` also
has the ability to backup the current time to a non-volatile time source (e.g.
DS3231 chip) so that the current time can be restored when the power is
restored.

The class hierarchy diagram looks like this, where the arrow means
"is-subclass-of") and the diamond-line means ("is-aggregation-of"):
```
                        0..1
           TimeProvider ------------.
          ^     ^                   |
         /      |      0..1         |
        /   TimeKeeper --------.    |
       NTP      ^  ^           |    |
TimeProvider   /   |           |    |
              /    |           |    |
         DS3231    |           |    |
      TimeKeeper   |           |    |
                   |           |    |
              SystemClock <>---+----'
```

The library currently provides only a single implementation of `TimeProvider`
and a single implementation of `TimeKeeper`. More could be added later.

### TimeProvider and TimeKeeper

These 2 interfaces distinguish between clocks that provide a source of time but
cannot be set to a particular time (e.g. NTP servers or GPS modules), and clocks
whose time can be set by the user (e.g. DS3231 RTC chip). The `TimeProvider`
interface implements the `TimeProvider::getNow()` method which returns an
`acetime_t` type. The `TimeKeeper` is a subinterface of `TimeProvider` and
implements the `TimeKeeper::setNow(acetime_t)` method which sets the current
time.

```C++
namespace ace_time {
namespace clock {

class TimeProvider {
  public:
    static const acetime_t kInvalidSeconds = LocalTime::kInvalidSeconds;

    virtual acetime_t getNow() const = 0;
    ...
};

class TimeKeeper: public TimeProvider {
  public:
    virtual void setNow(acetime_t epochSeconds) = 0;
};

}
}
```

The `acetime_t` value can be converted into the desired time zone using the
`ZonedDateTime` and `TimeZone` classes desribed in the previous section.

```C++
TimeZone tz = ...;
TimeProvider timeProvider = ...;
acetime_t nowSeconds = timeProvider.getNow();
auto nowDateTime = ZonedDateTime::forEpochSeconds(nowSeconds, tz);
nowDateTime.printTo(Serial);
```

Various implementations of `TimeProvider` and `TimeKeeper` are described in
more detail the following subsections.

### NTP Time Provider

The `NtpTimeProvider` is available on the ESP8266 and ESP32 which have builtin
WiFi capability. (I have not tested the code on the Arduino WiFi shield
because I don't have that hardware.) This class uses an NTP client to fetch the
current time from the specified NTP server. The constructor takes 3 parameters
which have default values so they are optional.

The class declaration looks like this:

```C++
namespace ace_time {
namespace clock {

class NtpTimeProvider: public TimeProvider {
  public:
    explicit NtpTimeProvider(
            const char* server = kNtpServerName,
            uint16_t localPort = kLocalPort,
            uint16_t requestTimeout = kRequestTimeout);
    void setup(const char* ssid, const char* password);

    bool isSetup() const;
    acetime_t getNow() const override;
    ...
};

}
}
```

You need to call the `setup()` with the `ssid` and `password` of the WiFi
connection. The method will time out after 5 seconds if the connection cannot
be established. Here is a sample of how it can be used:

```C++
#include <AceTime.h>
using namespace ace_time;
using namespace ace_time::clock;

const char SSID[] = ...; // Warning: don't store SSID in GitHub
const char PASSWORD[] = ...; // Warning: don't store passwd in GitHub

NtpTimeProvider ntpTimeProvider;

void setup() {
  Serial.begin(115200);
  while(!Serial); // needed for Leonardo/Micro
  ...
  ntpTimeProvider.setup(SSID, PASSWORD);
  if (ntpTimeProvider.isSetup()) {
    Serial.println("WiFi connection failed... try again.");
    ...
  }
}

// Print the NTP time every 10 seconds, in UTC-08:00 time zone.
void loop() {
  acetime_t nowSeconds = ntpTimeProvider.getNow();
  OffsetDateTime odt = OffsetDateTime::forEpochSeconds(
      nowSeconds, TimeOffset::forHour(-8)); // convert epochSeconds to UTC-08:00
  odt.printTo(Serial);
  delay(10000); // wait 10 seconds
}
```

**Security Warning**: You should avoid committing your SSID and PASSWORD into a
public repository like GitHub because they will become public to anyone. Even if
you delete the commit, they can be retrieved from the git history.

### DS3231 Time Keeper

The `DS3231TimeKeeper` is the class describing the DS3231 RTC chip. It contains
an internal temperature-compensated osciallator that counts time in 1
second steps. It is often connected to a battery or a supercapacitor to survive
power failures. The DS3231 chip stores the time broken down by various date and
time components (i.e. year, month, day, hour, minute, seconds). It contains
internal logic that knows about the number of days in an month, and leap years.
It supports dates from 2000 to 2099. It does *not* contain the concept of a time
zone. Therefore, The `DS3231TimeKeeper` assumes that the date/time components
stored on the chip is in **UTC** time.

The class declaration looks like this:

```C++
namespace ace_time {
namespace clock {

class DS3231TimeKeeper: public TimeKeeper {
  public:
    explicit DS3231TimeKeeper();
    void setup();

    acetime_t getNow() const override;
    void setNow(acetime_t epochSeconds) override;
};

}
}
```

The `DS3231TimeKeeper::getNow()` returns the number of seconds since
AceTime Epoch by converting the UTC date and time components to `acetime_t`
(using `LocalDatetime` internally). Users can convert the epoch seconds
into either an `OffsetDateTime` or a `ZonedDateTime` as needed.

The `DS3231TimeKeeper::setup()` should be called from the global `setup()`
function to initialize the object. Here is a sample that:

```C++
#include <AceTime.h>
using namespace ace_time;
using namespace ace_time::clock;

DS3231TimeKeeper dsTimeKeeper;
...
void setup() {
  Serial.begin(115200);
  while(!Serial); // needed for Leonardo/Micro
  ...
  dsTimeKeeper.setup();
  dsTimeKeeper.setNow(0); // 2000-01-01T00:00:00Z
}

void loop() {
  acetime_t nowSeconds = dsTimeKeeper.getNow();
  OffsetDateTime odt = OffsetDateTime::forEpochSeconds(
      nowSeconds, TimeOffset::forHour(-8)); // convert epochSeconds to UTC-08:00
  odt.printTo(Serial);
  delay(10000); // wait 10 seconds
}
```

### SystemClock

The `SystemClock` is a special `TimeKeeper` that uses the Arduino built-in
`millis()` method as the source of its time. The biggest advantage of
`SystemClock` is that its `getNow()` has very little overhead so it can be
called as frequently as needed. The `getNow()` method of other `TimeProviders`
can consume a significant amount of time. For example, the `DS3231TimeKeeper`
must talk to the DS3231 RTC chip over an I2C bus. Even worse, the
`NtpTimeProvider` must the talk to the NTP server over the network which can be
unpredictably slow.

The `SystemClock` class looks like this:

```C++
namespace ace_time {
namespace clock {

class SystemClock: public TimeKeeper {
  public:

    explicit SystemClock(
            TimeProvider* syncTimeProvider /* nullable */,
            TimeKeeper* backupTimeKeeper /* nullable */):
        mSyncTimeProvider(syncTimeProvider),
        mBackupTimeKeeper(backupTimeKeeper);
    void setup();

    acetime_t getNow() const override;
    void setNow(acetime_t epochSeconds) override;

    void sync(acetime_t epochSeconds);
    acetime_t getLastSyncTime() const;
    bool isInit() const;

  protected:
    virtual unsigned long millis() const;
};

}
}
```

Unfortunately, the `millis()` internal clock of most (all?) Arduino boards is
not very accurate and unsuitable for implementing an accurate clock. Therefore,
the `SystemClock` provides a mechanism to synchronize its clock to an
external (and presumably more accurate clock) `TimeProvider`.

The `SystemClock` also provides a way to save the current time to a
`backupTimeKeeper` (e.g. the `DS3231TimeKeeper` using the DS3231 chip with
battery backup). When the `SystemClock` starts up, it will read the backup
`TimeKeeper` and set the current time. Then it can synchronize with an external
clock source (e.g. the `NtpTimeProvider`). The time is saved to the backup time
keeper whenever the `SystemClock` is synced with the external time
clock.

Here is how to set up the `SystemClock` with the `NtpTimeProvider` and
`DS3231TimeKeeper` as sync and backup time sources:

```C++
#include <AceTime.h>
using namespace ace_time;
using namespace ace_time::clock;

DS3231TimeKeeper dsTimeKeeper;
NtpTimeProvider ntpTimeProvider(SSID, PASSWORD);
SystemClock systemClock(
  &ntpTimeProvider /*sync*/, &dsTimeKeeper /*backup*/);
...

void setup() {
  Serial.begin(115200);
  while(!Serial); // needed for Leonardo/Micro
  ...
  dsTimeKeeper.setup();
  ntpTimeProvider.setup();
  systemClock.setup();
}

void loop() {
  acetime_t nowSeconds = systemClock.getNow();
  auto odt = OffsetDateTime::forEpochSeconds(
      nowSeconds, TimeOffset::forHour(-8)); // convert epochSeconds to UTC-08:00
  odt.printTo(Serial);
  delay(10000); // wait 10 seconds
}
```

If you wanted to use the `DS3231TimeKeeper` as *both* the backup and sync
time sources, then the setup would something like this:

```C++
#include <AceTime.h>
using namespace ace_time;
using namespace ace_time::clock;

DS3231TimeKeeper dsTimeKeeper;
SystemClock systemClock(
    &dsTimeKeeper /*sync*/, &dsTimeKeeper /*backup*/);
...

void setup() {
  dsTimeKeeper.setup();
  systemClock.setup();
  ...
}
```

You could also choose not to have either the backup or sync time sources, in
which case you can give `nullptr` as the correspond argument. For example,
to use no backup time keeper:

```C++
#include <AceTime.h>
using namespace ace_time;
using namespace ace_time::clock;

DS3231TimeKeeper dsTimeKeeper;
SystemClock systemClock(&dsTimeKeeper /*sync*/, nullptr /*backup*/);
...

void setup() {
  dsTimeKeeper.setup();
  systemClock.setup();
  ...
}
```

### System Clock KeepAlive and Syncing

The `SystemClock` requires 2 maintenance tasks to run periodically
to help it keep proper time.

First, the `SystemClock::getNow()` or `keepAlive()` method must be called
peridically before an internal integer overflow occurs, even if the `getNow()`
is not needed. The internal integer overflow happens every 65.536 seconds.
If your application is *guaranteed* to call `SystemClock::getNow()` more
frequently than every 65 seconds, then you don't need to worry about this.
However, if you want to be prudent, it does not cost very much to call the
`SystemClock::keepAlive()` function in the global `loop()` method.

Secondly, since the internal `millis()` clock is not very accurate, we must
synchronize the `SystemClock` periodically with a more accurate time
source. The frequency of this syncing depends on the accuracy of the `millis()`
(which depends on the hardware oscillator of the chip) and the cost of the call
to the `getNow()` method of the syncing time clock. If the syncing time
source is the DS3231 chip, syncing once every 1-10 minutes might be sufficient
since talking to the RTC chip is relatively cheap. If the syncing time source is
the `NtpTimeProvider`, the network connection is fairly expensive so maybe once
every 1-12 hours might be advisable. The `SystemClock` provides 2 ways to
perform this syncing.

**Method 1: Using SystemClockSyncLoop**

You can use the `SystemClockSyncLoop` class and insert it somewhere into the
global `loop()` method, like this:

```C++
#include <AceTime.h>
using namespace ace_time;
using namespace ace_time::clock;
...

SystemClock systemClock(...);
SystemClockSyncLoop systemClockSyncLoop(systemClock);

void loop() {
  ...
  systemClock.keepAlive();
  systemClockSyncLoop.loop();
  ...
}
```

**Method 2: Using SystemClockSyncCoroutine**

You can use two [AceRoutine](https://github.com/bxparks/AceRoutine) coroutines
to perform sync. First, `#include <AceRoutine.h>` *before* the
`#include <AceTime.h>` (which activates the `SystemClockSyncCoroutine`
class), then configure it to run:

```C++
#include <AceRoutine.h> // include this before <AceTime.h>
#include <AceTime.h>
using namespace ace_time;
using namespace ace_time::clock;
using namespace ace_routine;
...

SystemClock systemClock(...);
SystemClockSyncCoroutine systemClockSync(systemClock);

void setup() {
  ...
  systemClockSyncCoroutine.setupCoroutine(F("systemClockSync"));
  CoroutineScheduler::setup();
  ...
}

void loop() {
  ...
  systemClock.keepAlive();
  CoroutineScheduler::loop();
  ...
}
```

The biggest advantage of using AceRoutine coroutines is that the syncing process
becomes non-blocking. In other words, if you are using the `NtpTimeProvider` to
provide syncing, the `SystemClockSyncLoop` object calls its `getNow()` method,
which blocks the execution of the program until the NTP server returns a
response (or the request times out after 1000 milliseconds). If you use the
`SystemClockSyncCoroutine`, the program continues to do other things (e.g.
update displays, scan for buttons) while the `NtpTimeProvider` is waiting for a
response from the NTP server.

## Testing

Writing tests for this library was very challenging, probably taking up 3-4X
more effort than the core of the library. I think the reason is that the number
input variables into the library and the number of output variables are
substantially large, making it difficult to write isolated unit tests. Secondly,
the TZ Database zone files are deceptively easy to read by humans, but
contain so many implicit rules that are incredibly difficult to translate into
computer algorithms, creating a large number of code paths to test.

It is simply impractical to manually create the inputs and expected outputs
using the TZ database. The calculation of one data point can take several
minutes manually. The solution would be to programmatically generate the data
points. To that end, I wrote the 2 different implementations of `ZoneSpecifier`
(`BasicZoneSpecifier` and `ExtendedZoneSpecifier`) partially as an attempt to
write different versions of the algorithms to validate them against each other.
(I think I wrote 4-5 different versions altogether, of which only 2 made it into
this library). However, it turned out that the number of timezones supported by
the `ExtendedZoneSpecifier` was much larger than the ones supported by
`BasicZoneSpecifier` so it became infeasible to test the non-overlapping
timezones.

My next idea was to validate AceTime against a known, independently created,
timezone library that also supports the TZ Database. The Python pytz library was
a natural choice since the `tzcompiler.py` was already written in Python. The
`BasicValidationUsingPythonTest` and `ExtendedValidationUsingPythonTest` tests
are the results, where I use `pytz` to determine the list of DST transitions for
all timezones, then determine the expected (year, month, day, hour, minute,
second) components that `ZonedDateTime` should produce. The `tzcompiler.py`
generates a `validation_data.cpp` file which contains the test data points for
all supported timezones. The resulting program no longer fits in any Arduino
microcontroller that I am aware of, but through the use of the
[unitduino](https://github.com/bxparks/AUnit/tree/develop/unitduino) emulation
framework in [AUnit](https://github.com/bxparks/AUnit), I can run these large
validation test suites on a Linux or Mac desktop. This worked great until I
discovered that `pytz` supports [dates only until
2038](https://answers.launchpad.net/pytz/+question/262216). That meant that I
could not validate the `ZonedDateTime` classes after 2038.

I then turned to Java 11 `java.time` library, which supports years through the
[year 1000000000
(billion)](https://docs.oracle.com/en/java/javase/11/docs/api/java.base/java/time/class-use/Instant.html).
I wrote the [TestDataGenerator.java](tools/java/TestDataGenerator) program to
generate a `validation_data.cpp` file in exactly the same format as the
`tzcompiler.py` program, and produced data points from year 2000 to year 2050,
which is the exact range of years supported by the `zonedb::` and `zonedbx::`
zoneinfo files.

The end result is the 4 validation programs under `tests/validation`:

* `BasicValidationUsingJavaTest`
* `BasicValidationUsingPythonTest`
* `ExtendedValidationUsingJavaTest`
* `ExtendedValidationUsingPythonTest`

When these tests pass, they show that the timezone algorithms in AceTime produce
the same results as the Python `pytz` library and the Java 11 `java.time`
library, showing that 3 independently written libraries and algorithms agree
with each other. These validation tests give me good confidence that AceTime
produces correct results for the most part, but it is entirely expected that
some obscure edge-case bugs will be found in the future.

## Benchmarks

### CPU

The [AutoBenchmark.ino](examples/AutoBenchmark/) program measures the
amount of CPU cycles taken by some of the more expensive methods. Here is a
summary of the elapsed time for `OffsetDateTime::forEpochSeconds()` for some
Arduino boards that I have access to:
```
----------------------------+---------+
Board or CPU                |  micros |
----------------------------+---------+
ATmega328P 16MHz (Nano)     | 321.600 |
ESP8266 80MHz               |  13.400 |
ESP32 240MHz                |   1.470 |
Teensy 3.2 96MHz            |   2.130 |
----------------------------+---------+
```

### Memory

Here is a quick summary of the amount of static RAM consumed by various
classes (more details at [examples/AutoBenchmark](examples/AutoBenchmark):

**8-bit processors**

```
sizeof(LocalDate): 3
sizeof(LocalTime): 3
sizeof(LocalDateTime): 6
sizeof(TimeOffset): 1
sizeof(OffsetDateTime): 7
sizeof(BasicZoneSpecifier): 99
sizeof(ExtendedZoneSpecifier): 397
sizeof(TimeZone): 3
sizeof(ZonedDateTime): 10
sizeof(TimePeriod): 4
sizeof(SystemClock): 17
sizeof(DS3231TimeKeeper): 3
sizeof(SystemClockSyncLoop): 14
sizeof(SystemClockSyncCoroutine): 31
```

**32-bit processors**
```
sizeof(LocalDate): 3
sizeof(LocalTime): 3
sizeof(LocalDateTime): 6
sizeof(TimeOffset): 1
sizeof(OffsetDateTime): 7
sizeof(BasicZoneSpecifier): 156
sizeof(ExtendedZoneSpecifier): 500
sizeof(TimeZone): 8
sizeof(ZonedDateTime): 16
sizeof(TimePeriod): 4
sizeof(SystemClock): 24
sizeof(NtpTimeProvider): 88 (ESP8266), 116 (ESP32)
sizeof(SystemClockSyncLoop): 20
sizeof(SystemClockSyncCoroutine): 52
```

The [MemoryBenchmark](examples/MemoryBenchmark) program gives a more
comprehensive answer to the amount of memory taken by this library.
Here is a short summary for an 8-bit microcontroller (e.g. Arduino Nano):

* Using the `TimeZone` class with a `BasicZoneSpecifier` for one timezone takes
  about 6140 bytes of flash memory and 193 bytes of static RAM.
* Using 2 timezones with `BasiCZoneSpecifierincreases the consumption to 6628
  bytes of flash and 231 bytes of RAM.
* Loading the entire `zonedb::` zoneinfo database consumes 20354 bytes of flash
  and 601 bytes of RAM.
* Adding the `SystemClock` to the `TimeZone` and `BasicZoneSpecifier` with one
  timezone consumes 8436 bytes of flash and 344 bytes of RAM.

These numbers indicate that the AceTime library is useful even on a limited
8-bit controller with only 30-32kB of flash and 2kB of RAM. As a concrete
example, the [WorldClock](examples/WorldClock) program contains 3 OLED displays
over SPI, 2 buttons, one DS3231 chip, and 3 timezones using AceTime, and these
all fit inside a Arduino Pro Micro limit of 30kB flash and 2.5kB of RAM.

## Comparisons to Other Time Libraries

### Arduino Time Library

The AceTime library can be substantially faster than the equivalent methods in
the [Arduino Time Library](https://github.com/PaulStoffregen/Time). The
[ComparisonBenchmark.ino](examples/ComparisonBenchmark/) program compares the
CPU run time of `LocalDateTime::forEpochSeconds()` and
`LocalDateTime::toEpochSeconds()` with the equivalent `breakTime()` and
`makeTime()` functions of the Arduino Time Library. Details are given in the
[ComparisonBenchmark/README.md](examples/ComparisonBenchmark/README.md) file in
that folder, but here is a summary of the roundtrip times for various boards (in
microseconds):

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

### AVR Libc

The [AVR libc time
library](https://www.nongnu.org/avr-libc/user-manual/group__avr__time.html), is
based on the UNIX/POSIX time library. I have not tried to use it on an Arduino
platform. There are 2 things going against it: First it works only on AVR
processors, and I wanted a time library that worked across multiple processors
(like the ESP8266 and ESP32). Second, the AVR time library is based on the
[traditional C/Unix library methods](http://www.catb.org/esr/time-programming/)
which can be difficult to understand.

### ezTime

The [ezTime](https://github.com/ropg/ezTime) is a library that seems to be
composed of 2 parts: A client library that runs on the microcontroller and a
server part that provides a translation from the timezone name to the POSIX DST
transition string. Unfortunately, this means that the controller must have
network access for this library to work. I wanted to create a library that was
self-contained and could run on an Arduino Nano with just an RTC chip without a
network shield.

### Micro Time Zone

The [Micro Time Zone](https://github.com/evq/utz) is a pure-C library
that can compile on the Arduino platform. It contains a limited subset of the TZ
Database encoded as C structs and determines the DST transitions using the
encoded structs. It supports roughly of 45 zones with just a 3kB tzinfo
database. The initial versions of AceTime, particularly the `BasicZoneSpecifier`
class was directly inspired by this library. It would be interesting to run this
library to the same set of "validation" unit tests that checks the AceTime logic
and see how accurate this library is. One problem with Micro Time Zone library
is that it loads the entire tzinfo database for all 45 time zones, even if only
one zone is used. Therefore, the AceTime library will consume less flash memory
for the database part if only a handful of zones are used. But the AceTime
library contains more algorithmic code so will consume more flash memory. It is
not entirely clear which library is smaller for 1-3 time zones. (This may be an
interesting investigation the future.)

### Java Time, Joda-Time, Noda Time

Many time libraries (such as [Java 11
Time](https://docs.oracle.com/en/java/javase/11/docs/api/java.base/java/time/package-summary.html),
[Joda-Time](https://www.joda.org/joda-time/), and [Noda
Time](https://nodatime.org/)) provide substantially more fine-grained class
hierarchies to provider better type-safety. For example, those libraries
just mentioned provided an `Instant` class, a `Duration` class, an `Interval`
class. The `java.time` package also provides other fine-grained classes such as
`OffsetTime`, `OffsetDate`, `Year`, `Month`, `MonthDay` classes. For the AceTime
library, I decided to avoid providing too many classes. The API of the library
is already too large, I did not want to make them larger than necessary.

### HowardHinnant Libraries

A number of C++ libraries from Howard Hinnant are based the `<chrono>` standard
library:

* [date](http://howardhinnant.github.io/date/date.html)
* [tz](http://howardhinnant.github.io/date/tz.html)
* [iso_week](http://howardhinnant.github.io/date/iso_week.html)
* [julian](http://howardhinnant.github.io/date/julian.html)
* [islamic](http://howardhinnant.github.io/date/islamic.html)

To be honest, I have not looked very closely at these libraries, mostly because
of my suspicion that they are too large to fit into an Arduino microcontroller.

### Google cctz

The [cctz](https://github.com/google/cctz) library from Google is also based on
the `<chrono>` library. Again, I did not look at this library closely because I
did not think it would fit inside an Arduino controller.

## Bugs and Limitations

* Leap seconds
    * This library does not support [leap
      seconds](https://en.wikipedia.org/wiki/Leap_second) and will probably
      never do so.
* `acetime_t`
    * AceTime uses an epoch of 2000-01-01T00:00:00Z.
      The `acetime_t` type is a 32-bit signed integer whose largest value is
      `INT32_MAX`, which corresponds to 2068-01-19T03:14:07Z.
    * The smallest date the `acetime_t` is `INT32_MIN` but that value is used to
      indicate an "invalid" value. Therefore, the smallest normal value is
      `INT32_MIN+1` which corresponds to 1931-12-13T20:45:53.
* `TimeOffset`
    * Implemented using `int8_t` to save memory.
    * Represents time offsets in increments of 15 minutes. All timezones after
      2012 are in multiples of 15 minutes.
    * Five zones before 2012 have transitions at 00:01 which cannot be
      represented by this class. Those transitions have been truncated to 00:00.
      See the bottom of the generated
      [zonedb/zone_infos.h](src/ace_time/zonedb/zone_infos.h) and
      [zonedbx/zone_infos.h](src/ace_time/zonedbx/zone_infos.h) files for the
      up-to-date list.
* `LocalDate`, `LocalDateTime`
    * These classes (and all other Date classes which are based on these) use
      a single 8-bit signed byte to represent the 'year' internally. This saves
      memory, at the cost of restricting the range.
    * The value of -128 (`INT8_MIN`) is used to indicate an "invalid" value, so
      the actual range is [-127, 127]. This restricts the year range to [1873,
      2127].
* `toUnixSeconds()`
    * [Unix time](https://en.wikipedia.org/wiki/Unix_time) uses an epoch of
      1970-01-01T00:00:00Z. This method returns an `acetime_t` which is
      a signed integer, just like the old 32-bit Unix systems. This method will
      fail just after 2038-01-19T03:14:07Z, even though the underlying date
      object is still valid.
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
    * There is not currently the ability to extract a unique identifier for a
      given TimeZone for serialization purposes (which would allow save and
      restore). The application developer must create an ad-hoc serialization
      convention using `TimeZone::getType()` and some other information outside
      of the AceTime library framework.
* `ZonedDateTime::forComponents()`
    * The `ZonedDateTime::forComponents()` method takes the local wall time and
      `TimeZone` instance as parameters which can be ambiguous or invalid for
      some values.
        * During the Standard time to DST transitions, a one-hour gap of
      illegal values may exist. For example, 2am (Standard) shifts to 3am (DST),
      therefore wall times between 02:00 and 03:00 (exclusive) are not valid.
        * During DST to Standard time transitions, a one-hour interval occurs
        twice. For example, 2am (DST) shifts to 1am, so all times between 01:00
        and 02:00 (exclusive) occurs twice in one day.
   * The `ZonedDateTime::forCommponent()` methods makes an educated guess
     at what the user meant, but the algorithm may not be robust, is not tested
     as well as it could be, and the algorithm may change in the future. To keep
     the code size within reasonble limits of a small Arduino controller, the
     algorithm may be permanently sub-optimal.
* `BasicZoneSpecifier`, `ExtendedZoneSpecifier`
    * Tested using both Python and Java libraries.
    * Python [pytz](https://pypi.org/project/pytz/) library supports dates only
      from 2000 until 2038.
    * Java `java.time` library has an upper limit far beyond the year 2068 limit
      of `ZonedDateTime`. Testing was performed from 2000 to until 2050.
* `ExtendedZoneSpecifier`
    * There are 5 time zones (as of version 2019a of the TZ Database, see
      the bottom of `zonedbx/zone_infos.h`) which have DST transitions that
      occur at 00:01 (one minute after midnight). This transition cannot be
      represented as a multiple of 15-minutes. The transition times of these
      zones have been shifted to the nearest 15-minute boundary, in other words,
      the transitions occur at 00:00 instead of 00:01. Clocks based on
      `ExtendedZoneSpecifier` will be off by one hour during the 1-minute
      interval from 00:00 and 00:01.
    * Fortunately all of these transitions happen before 2012. If you are
      interested in only dates after 2019, then this will not affect you.
* `NtpTimeProvider`
    * The `NtpTimeProvider` on an ESP8266 calls `WiFi.hostByName()` to resolve
      the IP address of the NTP server. Unfortunately, when I tested this
      library, it seems to be blocking call (later versions may have fixed
      this). When the DNS resolver is working properly, this call returns in
      ~10ms or less. But sometimes, the DNS resolver seems to get into a state
      where it takes 4-5 **seconds** to time out. Even if you use AceRoutine
      coroutines, the entire program will block for those 4-5 seconds.
    * [NTP](https://en.wikipedia.org/wiki/Network_Time_Protocol) uses an epoch
      of 1900-01-01T00:00:00Z, with 32-bit unsigned integer as the seconds
      counter. It will overflow just after 2036-02-07T06:28:15Z.
* `BasicValidationUsingPythonTest` and `ExtendedValidationUsingPythonTest`
    * These tests compare the transition times calculated by AceTime to Python's
      [pytz](https://pypi.org/project/pytz/) library. Unfortunately, pytz does
      not support dates after Unix signed 32-bit `time_t` rollover at
      (2038-01-19T03:14:07Z).
    * These are too big to run on any Arduino controller. They are designed to
      run on a Linux or MacOS machine through the Makefiles using the
      [unitduino](https://github.com/bxparks/AUnit/tree/develop/unitduino)
      emulator.
* `BasicValidationUsingJavaTest` and `ExtendedValidationUsingJavaTest`
    * These tests compare the transition times calculated by AceTime to Java 11
      `java.time` package which should support the entire range of dates that
      AceTime can represent. We have artificially limited the range of testing
      from 2000 to 2050.
    * These are too big to run on any Arduino controller. They are designed to
      run on a Linux or MacOS machine through the Makefiles using the
      [unitduino](https://github.com/bxparks/AUnit/tree/develop/unitduino)
      emulator.
* `zonedb/` and `zonedbx/` zoneinfo files
    * These statically defined data structures are loaded into flash memory
      then copied to RAM when the application starts. Fortunately, most
      `ZoneInfo` entries are only 40-60 bytes each and the corresponding
      `ZonePolicy` entries are 50-100 bytes each.
    * It may be possible to use the `PROGMEM` keyword to store them only on
      flash memory, but that will increase the flash memory size due to the code
      needed to read these data structures from flash. In some applications,
      flash memory may be more precious than RAM so it is not clear that using
      `PROGMEM` for these data structures is the appropriate solution.
    * The TZ database files `backzone`, `systemv` and `factory` are
      not processed by the `tzcompiler.py` tool. They don't seem to contain
      anything worthwhile.
    * The datasets and `*ZoneSpecifier` classes have been *not* been tested or
      validated for years prior to 2000.
    * TZ Database version 2019b contains the first use of the
      `{onDayOfWeek<=onDayOfMonth}` syntax that I have seen (specifically `Rule
      Zion, FROM 2005, TO 2012, IN Apr, ON Fri<=1`). The actual transition date
      can shift into the previous month (or to the next month in the case of
      `>=`). However, shifting into the previous year or the next year is not
      supported. The `tzcompiler.py` will exclude and flag the Rules which could
      potentially shift to a different year. As of version 2019b, no such Rule
      seems to exist.
* `Link` entries
    * The TZ Database `Link` entries are implemented as C++ references to
      the equivalent `Zone` entries. For example,
      `zonedb::kZoneUS_Pacific` is just a reference to
      `zonedb::kZoneAmerica_Los_Angeles`. This means that if a `ZonedDateTime`
      is created with a `TimeZone` associated with `kZoneUS_Pacific`, the
      `ZonedDateTime::printTo()` will print "[America/Los_Angeles]" not
      "[US/Pacific]".
