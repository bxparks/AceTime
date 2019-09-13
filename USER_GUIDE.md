# AceTime Library User Guide

See the [README.md](README.md) for introductory background.

Version: 0.8.1 (2019-08-26, TZ DB version 2019b, beta)

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
  `BasicZoneProcessor` (`ace_time::zonedb`)
* `src/ace_time/zonedbx/` - files generated from TZ Database for
  `ExtendedZoneProcessor` (`ace_time::zonedbx`)
* `tests/` - unit tests using [AUnit](https://github.com/bxparks/AUnit)
* `tests/validation` - integration tests using AUnit which must be run
   on desktop Linux or MacOS machines using
   [UnixHostDuino](https://github.com/bxparks/UnixHostDuino)
* `examples/` - example programs
* `tools/` - parser for the TZ Database files, code generators for `zonedb::`
  and `zonedbx::` zone files, and code generators for various unit tests

### Dependencies

The vast majority of the AceTime library has no dependency to any other external
libraries. There is an optional dependency to
[AceRoutine](https://github.com/bxparks/AceRoutine) if you want to use the
`SystemClockCoroutine` class for automatic syncing. (This is recommended but
not strictly necessary). The `ace_time/hw/CrcEeprom.h` class has a dependency to
the [FastCRC](https://github.com/FrankBoesing/FastCRC) library but the
`CrcEeprom.h` file is not included in the `AceTime.h` main header file, so you
should not need FastCRC to compile AceTime. (The `CrcEeprom.h` header file does
not strictly belong in the AceTime library but many of my "clock" projects that
use the AceTime library also use the `CrcEeprom` class, so this is a convenient
place to keep it.)

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

* [IANA TZ Database on GitHub](https://github.com/eggert/tz)
* [pytz library](https://pypi.org/project/pytz/)
* [Hinnant date library](https://github.com/HowardHinnant/date)
* Python 3.5 or greater
* Java OpenJDK 11

If you want to run the unit tests or some of the command line examples using a
Linux or MacOS machine, you need:

* [UnixHostDuino](https://github.com/bxparks/UnixHostDuino)

### Doxygen Docs

The [docs/](docs/) directory contains the
[Doxygen docs](https://bxparks.github.io/AceTime/html) on GitHub Pages.
This may be useful to navigate the various classes in this library
and to lookup the signatures of the methods in those classes.

### Examples

The following programs are provided in the `examples/` directory:

* [HelloDateTime](examples/HelloDateTime/)
    * demo program of various date and time classes
* [HelloSystemClock](examples/HelloSystemClock/)
    * demo program of `SystemClock`
* [HelloSystemClockCoroutine](examples/HelloSystemClockCoroutine/)
    * same as `HelloSystemClock` but using AceRoutine coroutines
* [HelloNtpClock](examples/HelloNtpClock/)
    * demo program of `NtpClock`
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
* [MemoryBenchmark](examples/MemoryBenchmark/)
    * compiles `MemoryBenchmark.ino` for 13 different features and collecs the
      flash and static RAM usage from the compiler into a `*.txt` file for
      a number of platforms (AVR, SAMD, ESP8266, etc)
    * the `README.md` transforms the `*.txt` file into a human-readable form
* [ComparisonBenchmark](examples/ComparisonBenchmark/)
    * compare AceTime with
    [Arduino Time Lib](https://github.com/PaulStoffregen/Time)

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
* A geographical (or conceptual) region whose local time is offset
from the UTC time using various transition rules.

Both meanings of "time zone" are supported by the `TimeZone` class using
5 different types as follows:

* `TimeZone::kTypeManual`: a fixed base offset and optional DST offset from UTC
* `TimeZone::kTypeBasic`: utilizes a `BasicZoneProcessor` which can
be encoded with (relatively) simple rules from the TZ Database
* `TimeZone::kTypeExtended`: utilizes a `ExtendedZoneProcessor` which can
handle all zones in the TZ Database
* `TimeZone::kTypeBasicManaged`: same as `kTypeBasic` but the
  `BasicZoneProcessor` is managed by the `ZoneManager`
* `TimeZone::kTypeExtendedManaged`: same as `kTypeExtended` but the
  `ExtendedZoneProcessor` is managed by the `ZoneManager`

The class hierarchy of `TimeZone` is shown below, where the arrow means
"is-subclass-of" and the diamond-line means "is-aggregation-of". This is an
internal implementation detail of the `TimeZone` class that the application
develper will not normally need to be aware of all the time, but maybe this
helps make better sense of the usage of the `TimeZone` class. A `TimeZone` can
hold a reference to:

* nothing (`kTypeManual`),
* one `ZoneProcessor` object, (`kTypeBasic` or `kTypeExtended`) class, or
* one `ZoneProcessorCache` object (`kTypeBasicManaged` or
  `kTypeExtendedManaged`).

```
           .-------------------------------.
         <>    0..1                         \   0..1
TimeZone <>-------- ZoneProcessor            ------- ZoneProcessorCache
                          ^                                ^
                          |                                |
                   .----- +----.                     .---- +-----.
                   |           |                     |           |
              BasicZone        ExtendedZone       BasicZone     ExtendedZone
              Processor        Processor     ProcessorCache     ZoneProcessor
```

Here is the class declaration of `TimeZone`:

```C++
namespace ace_time {

class TimeZone {
  public:
    static const uint8_t kTypeError = 0;
    static const uint8_t kTypeManual = 1;
    static const uint8_t kTypeBasic = ZoneProcessor::kTypeBasic;
    static const uint8_t kTypeExtended = ZoneProcessor::kTypeExtended;
    static const uint8_t kTypeBasicManaged = ZoneProcessorCache::kTypeExtended;
    static const uint8_t kTypeExtendedManaged =
        ZoneProcessorCache::kTypeExtended;

    static TimeZone forTimeOffset(TimeOffset stdOffset,
        TimeOffset dstOffset = TimeOffset());
    static TimeZone forZoneInfo(const basic::ZoneInfo* zoneInfo,
        BasicZoneProcessor* zoneProcessor);
    static TimeZone forZoneInfo(const extended::ZoneInfo* zoneInfo,
        ExtendedZoneProcessor* zoneProcessor);
    static TimeZone forUtc();

    TimeZone(); // same as forUtc()

    uint8_t getType() const;
    TimeOffset getStdOffset() const;
    TimeOffset getDstOffset() const;
    uint32_t getZoneId() const;

    TimeOffset getUtcOffset(acetime_t epochSeconds) const;
    TimeOffset getDeltaOffset(acetime_t epochSeconds) const;
    const char* getAbbrev(acetime_t epochSeconds) const;
    TimeOffset getUtcOffsetForDateTime(const LocalDateTime& ldt) const;

    bool isUtc() const;
    bool isDst() const;
    void setDstOffset(TimeOffset offset);

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

The `getUtcOffsetForDateTime(localDateTime)` method returns the best guess of
the total UTC offset at the given local date time. This method is not
normally expected to be used by the app developer directly. The reaon that this
is a best guess is because the local date time is sometime ambiguious during a
DST transition. For example, if the local clock shifts from 01:00 to 02:00 at
the start of summer, then the time of 01:30 does not exist. If the
`getUtcOffsetForDateTime()` method is given a non-existing time, it makes an
educated guess at what the user meant. Additionally, when the local time
transitions from 02:00 to 01:00 in the autumn, a given local time such as 01:30
occurs twice. If the `getUtcOffsetForDateTime()` method is given a time of
01:30, it will arbitrarily decide which offset to return.

The `isUtc()`, `isDst()` and `setDstOffset(TimeOffset)` methods are valid *only*
if the `TimeZone` is a `kTypeManual`. Otherwise, `isUtc()` and `isDst()` return
`false` and `setDstOffset()` does nothing.

The `getZoneId()` returns a `uint32_t` integer which is a unique and stable
identifier for the IANA timezone. This can be used to save and restore
the `TimeZone`. See the section on `ZoneManager` below.

The `printTo()` prints the fully-qualified unique name for the time zone.
For example, `"UTC"`, `"-08:00", `"-08:00(DST)"`, `"America/Los_Angeles"`.

The `printShortTo()` is similar to `printTo()` except that it prints the
last component of the IANA TZ Database zone names. In other words,
`"America/Los_Angeles"` is printed as `"Los_Angeles"`. This is helpful for
printing on small width OLED displays.

#### Manual TimeZone (kTypeManual)

The default constructor creates a `TimeZone` in UTC time zone with no
offset. This is also identical to the `forUtc()` method:

```C++
TimeZone tz; // UTC+00:00
auto tz = TimeZone::forUtc(); // UTC+00:00
```

To create `TimeZone` instances with other offsets, use the `forTimeOffset()`
factory method:

```C++
auto tz = TimeZone::forTimeOffset(TimeOffset::forHours(-8)); // UTC-08:00
auto tz = TimeZone::forTimeOffset(TimeOffset::forHourMinute(-4, -30)); // UTC-04:30
auto tz = TimeZone::forTimeOffset(
    TimeOffset::forHours(-8),
    TimeOffset::forHours(1)); // UTC-08:00+01:00 (effectively -07:00)
```

The `TimeZone::isUtc()`, `TimeZone::isDst()` and `TimeZone::setDst(bool)`
methods work only if the `TimeZone` is a `kTypeManual`.

The `setDstOffset()` takes a `TimeOffset` as the argument instead of a simple
`bool` because there are some zones (e.g. Europe/Dublin) which uses a negative
offset in the winter, instead of adding a postive offset in the summer.

The `setStdOffset()` allows the base time offset to be changed, but this
method is not expected to be used often.

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

The zoneinfo files were generated by a script using the TZ Database. This header
file is already included in `<AceTime.h>` so you don't have to explicitly
include it. As of version 2019b of the database, it contains 270 Zone and 182
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
currently (v0.8) but the *values* inside the data structure fields are not
the same, and they are interpreted differently.)

As of version 2019b of TZ Database, *all* 387 Zone and 205 Link entries from the
following TZ files are supported: `africa`, `antarctica`, `asia`, `australasia`,
`backward`, `etcetera`, `europe`, `northamerica`, `southamerica`. There are 3
files which are not processed (`backzone`, `systemv`, `factory`) because they
don't seem to contain anything useful.

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
`ExtendedZoneProcessor` supports all time zones in the TZ Database. The cost is
that it consumes *5 times* more memory and is a bit slower. If
`BasicZoneProcessor` supports the zone that you want using the zone files in the
`zonedb::` namespace, you should normally use that instead of
`ExtendedZoneProcessor`. The one other advatnage of `ExtendedZoneProcessor` over
`BasicZoneProcessor` is that `ExtendedZoneProcessor::forComponents()` is more
accurate than `BasicZoneProcessor::forComponents()` because the `zonedbx::` data
files contain transition information which are missing in the `zonedb::` data
files due to space constraints.

#### Basic Managed TimeZone (kTypeBasicManaged)

This TimeZone is similar to a `kTypeBasic` TimeZone, except that it is created
using the `BasicZoneManager`, like this:

```C++
// Create ZoneManager (see ZoneManager section below)
...
const int NUM_ZONES = 2;
BasicZoneManager<NUM_ZONES> basicZoneManager(
    kBasicZoneRegistrySize, kBasicZoneRegistry);
...

void someFunction() {
  auto tz = basicZoneManager.createForZoneInfo(
      &zonedb::kZoneAmerica_Los_Angeles);
  ...
}

```

See the *ZoneManager* section below for information on how to create a
`BasicZoneManager`.

#### Extended Managed TimeZone (kTypeExtendedManaged)

This TimeZone is similar to the `kTypeExtended` TimeZone, except that it is
created using the `ExtendedZoneManager`, like this:

```C++
// Create ZoneManager (see ZoneManager section below)
...
const int NUM_ZONES = 2;
ExtendedZoneManager<NUM_ZONES> basicZoneManager(
    kExtendedZoneRegistrySize, kExtendedZoneRegistry);
...

void someFunction() {
  auto tz = extendedZoneManager.createForZoneInfo(
      &zonedbx::kZoneAmerica_Los_Angeles);
  ...
}
```

See the *ZoneManager* section below for information on how to create an
`ExtendedZoneManager`.

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
BasicZoneProcessor zoneProcessor;

void someFunction() {
  ...
  auto tz = TimeZone::forZoneInfo(&zonedb::kZoneAmerica_Los_Angeles,
      &zoneProcessor);

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
the entire TZ Database into memory and use the `ZoneManager` to manage
the `BasicZoneProcessor` or `ExtendedZoneProcessor` objects dynamically. But the
dataset is too large to fit on most AVR microcontrollers with only 32kB of flash
memory, so we currently do not support this dynamic lookup. The
`ZonedDateTime::timeZone()` will return Manual `TimeZone` whose
`TimeZone::getType()` returns `TimeZone::kTypeManual`.

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

#### Caching

The conversion from an epochSeconds to date-time components using
`ZonedDateTime::forEpochSeconds()` is an expensive operation (see
[AutoBenchmark](examples/AutoBenchmark/)). To improve performance, the
`BasicZoneProcessor` and `ExtendedZoneProcessor` implement internal caching
based on the `year` component. This optimizes the most commonly expected
use case where the epochSeconds is incremented by a clock (e.g. `SystemClock`)
every second, and is converted to human-readable date-time components once a
second. According to [AutoBenchmark](examples/AutoBenchmark/), the cache
improves performance by a factor of 2-3X (8-bit AVR) to 10-20X (32-bit
processors) on consecutive calls to `forEpochSeconds()` with the same `year`.

### ZoneInfo Files

Starting with v0.4, the zoneinfo files are stored in in flash memory
instead of static RAM using the
[PROGMEM](https://www.arduino.cc/reference/en/language/variables/utilities/progmem/)
keyword on microcontrollers which support this feature. On an 8-bit
microcontroller, the `zonedb/` database consumes about 15 kB of flash
memory, so it may be possible to create small programs that can dynamically
access all timezones supported by `BasicZoneProcessor`. The `zonedbx/` database
consumes about 24 kB of flash memory and the addition code size from various
classes will exceed the 30-32kB limit of a typical Arduino 8-bit
microcontroller.

The exact format of the zoneinfo files (`zonedb/` and `zonedbx/`) are considered
to be an implementation detail and may change in the future to support future
timezones. Applications should not depend on the internal structure of zoneinfo
data structures.

#### Basic zonedb

The `zonedb/` files do not support all the timezones in the TZ Database.
If a zone is excluded, the reason for the exclusion can be found at the
bottom of the [zonedb/zone_infos.h](src/ace_time/zonedb/zone_infos.h) file.
The criteria for selecting the Basic `zonedb` files are embedded
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

In the current version (v0.8), this database contains 270 zones from the year
2000 to 2049 (inclusive).

#### Extended zonedbx

The goal of the `zonedbx/` files is to support all zones listed in the TZ
Database. Currently, as of TZ Database version 2019b, this goal is met
from the year 2000 to 2049 inclusive. Some restrictions of this database
are:

* the DST offset is a multipole of 15-minutes ranging from -1:00 to 2:45
  (all timezones from about 1972 support this)
* the STDOFF offset is a multiple of 1-minute
* the AT and UNTIL fields are multiples of 1-minute
* the LETTER field can be arbitrary strings

In the current version (v0.8), this database contains all 387 timezones from
the year 2000 to 2049 (inclusive).

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

    uint32_t zoneId() const;

#if ACE_TIME_USE_PROGMEM
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

    uint32_t zoneId() const;

#if ACE_TIME_USE_PROGMEM
    const __FlashStringHelper* name() const;
    const __FlashStringHelper* shortName() const;
#else
    const char* name() const;
    const char* shortName() const;
#endif

}
```

The `BasicZone` and `ExtendedZone` objects are meant to be used transiently,
for example:
```C++
...
const basic::ZoneInfo* zoneInfo = ...;
Serial.println(BasicZone(zoneInfo).shortName());
...
```

The return type of `name()` and `shortName()` change whether or not the zone
name is stored in flash memory or in static memory. As of v0.4,
`ACE_TIME_USE_PROGMEM=1` for all platforms. On platforms which do not directly
support `PROGMEM`, they provide macros which retain compatibilty with `PROGMEM`
so everything should work transparently.

The `name()` method returns the full zone name from the TZ Database (e.g.
`"America/Los_Angeles"`). The `shortName()` method returns only the last
component (e.g. `"Los_Angeles"`).

### ZoneManager

The `TimeZone::forZoneInfo()` methods are simple to use but have the
disadvantage that the `BasicZoneProcessor` or `ExtendedZoneProcessor` need to be
created manually for each `TimeZone` instance. This works well for a single time
zone, but if you have an application that needs 3 or more time zones, this may
become cumbersome. Also, it is difficult to reconstruct a `TimeZone`
dynamically, say, from its fullly qualified name (e.g. `"America/Los_Angeles"`).
The `ZoneManager` solves these problems. It keeps an internal cache or
`ZoneProcessors`, reusing them as needed. And it holds a registry of `ZoneInfo`
objects, so that a `TimeZone` can be created using its `zoneName`, `zoneInfo`,
or `zoneId`.

```C++
namespace ace_time{

template<uint16_t SIZE>
class BasicZoneManager {
  public:
    BasicZoneManager(uint16_t registrySize);
        const basic::ZoneInfo* const* zoneRegistry,

    TimeZone createForZoneInfo(const basic::ZoneInfo* zoneInfo);
    TimeZone createForZoneName(const char* name);
    TimeZone createForZoneId(uint32_t id);
    TimeZone createForZoneIndex(uint16_t index);

    TimeZone createForTimeZoneData(const TimeZoneData& d);

    uint16_t indexForZoneName(const char* name);
    uint16_t indexForZoneId(uint32_t id) const;
};

template<uint16_t SIZE>
class ExtendedZoneManager {
  public:
    ExtendedZoneManager(uint16_t registrySize,
        const extended::ZoneInfo* const* zoneRegistry);

    [...same as above...]
};

}
```

The `SIZE` template parameter is the size of the internal cache of
`ZoneProcessor` objects. This should be set to the number of time zones that
your application is expected to use *at the same time*. If your app never
changes its time zone after initialization, then this can be `<1>`. If your app
allows the user to dynamically change the time zone (e.g. from a menu of time
zones), then this should be at least `<2>` (to allow the system to compare the
old time zone to the new time zone selected by the user). In general, the `SIZE`
should be set to the number of timezones displayed to the user concurrently,
plus an additional 1 if the user is able to change the timezone dynamically.

The constructor take a `zoneRegistry` and its `zoneRegistrySize`. It is a
pointer to an array of pointers to the `zonedb::kZone*` or `zonedbx::kZone*`
objects. You can use the default zone registry (which contains ALL zones in the
`zonedb::` or `zonedbx::` database, or you can create your own custom zone
registry, as described below.

#### Default Zone Registry

The default zoneinfo registry is available at:

* [zonedb/zone_registry.h](src/ace_time/zonedb/zone_registry.h)
* [zonedbx/zone_registry.h](src/ace_time/zonedbx/zone_registry.h)

It contains the entire `zonedb` or `zonedbx` database. On an 8-bit processor,
the basic `zonedb::` data set is about 14kB and the extended `zonedbx::`
database is about 23kB. On 32-bit processors, the `zonedb::` data set is about
23kB and the extended `zonedbx::` data set is about 36kB.

```C++
#include <AceTime.h>
using namespace ace_time;
...
static const uint16_t SIZE = 2;
static BasicZoneManager<SIZE> zoneManager(
    zonedb::kZoneRegistrySize, zonedb::kZoneRegistry);

void someFunction(const char* zoneName) {
  TimeZone tz = zoneManager.createForZoneName("America/Los_Angeles");
  if (tz.isError()) {
    tz = TimeZone::forUtc();
    ...
  }
}
```

#### Custom Zone Registry

On small microcontrollers, the default zone registries are too large. The
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
static BasicZoneManager<CACHE_SIZE> zoneManager(
    kZoneRegistrySize, kZoneRegistry);
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
static ExtendedZoneManager<CACHE_SIZE> zoneManager(
    kZoneRegistrySize, kZoneRegistry);
```

The `ACE_TIME_PROGMEM` macro is defined in
[compat.h](src/ace_time/common/compat.h) and indicates whether the ZoneInfo
files are stored in normal RAM or flash memory (i.e. `PROGMEM`). It must be used
for custom zoneRegistries because the `BasicZoneManager` and
`ExtendedZoneManager` expect to find them in static RAM or flash memory
according to this macro.

See [CommandLineClock](examples/CommandLineClock/) for an example of how these
custom registries can be created and used.

#### createForZoneName()

The `ZoneManager` allows creation of a `TimeZone` using the fully qualified
zone name:

```C++
BasicZoneManager<NUM_ZONES> manager(...);

void someFunction() {
  auto tz = manager.createForZoneName("America/Los_Angeles");
  ...
}
```

Of course, you wouldn't actually do this because the same functionality could
be done more efficiently (less memory, less CPU time) using:
```C++
BasicZoneManager<NUM_ZONES> manager(...);

void someFunction() {
  auto tz = manager.createForZoneInfo(zonedb::kZoneAmerica_Los_Angeles);
  ...
}
```

I think the only time the `createForZoneName()` might be useful is if
the user was allowed to type in the zone name, and you wanted to create a
`TimeZone` from the string typed in by the user.

#### createForZoneId()

Each zone in the `zonedb::` and `zonedbx::` database is given a unique
and stable zoneId. This can be retrieved from the `TimeZone` object using:
```C++
TimeZone tz = zoneManager.createFor...();
uint32_t zoneId = tz.getZoneId();
```
from the `ZoneInfo` pointer using the `BasicZone()` helper object:
```C++
uint32_t zoneId = BasicZone(&zonedb::kZoneAmerica_Los_Angeles).zoneId();
```

The ZoneId is created using a hash of the fully qualified zone name. It is
guaranteed to be unique and stable by the `tzcompiler.py` tool that generated
the `zonedb::` and `zonedbx::` data sets.  By "unique", I mean that
no 2 time zones will have the same zoneId. By "stable", it means that once
a zoneId has been assigned to a fully qualified zone name, it will remain
unchanged forever in the database. This means that we can save the zoneId of a
TimeZone to persistent memory (e.g. EEPROM), then retrieve the zoneId, and
recreate the `TimeZone` using the following for a `BasicZoneManager`:

```C++
BasicZoneManager<NUM_ZONES> manager(...);

void someFunction() {
  uint32_t zoneId = ...;
  ...
  auto tz = manager.createForZoneId(zoneId);
  ...
}
```

and similarly for the `ExtendedZoneManager`:

```C++
ExtendedZoneManager<NUM_ZONES> manager(...);

void someFunction() {
  uint32_t zoneId = ...;
  ...
  auto tz = manager.createForZoneId(zoneId);
  ...
```

The `zoneId` has an obvious advantage over the fully qualified `zoneName` for
storage purposes. It is far easier to save a 4-byte zoneId (e.g. `0xb7f7e8f2`)
rather than a variable length string (e.g. `"America/Los_Angeles"`).
Since the `zoneId` is derived from just the zoneName, a `TimeZone` created by
the `BasicZoneManager` has the same zoneId as one created using the
`ExtendedZoneManager` if it has the same name. This means that a TimeZone can be
saved using a `BasicZoneManager` but recreated using an `ExtendedZoneManager`. I
am not able to see how this could be an issue, but let me know if you find this
to be a problem.

If the `ZoneManager` cannot find the `zoneId` in its internal zone registry,
then the `TimeZone::forError()` is returned. The application developer should
check for this, and substitute a reasonable default TimeZone when this happens.
This situation is not unique to the zoneId. The same problem would occur if the
fully qualified zone name was used.

#### createForZoneIndex()

The `ZoneManager::createForZoneIndex()` creates a `TimeZone` from its integer
index into the Zone registry, from 0 to `registrySize - 1`. This is useful when
you want to show the user with a menu of zones from the `ZoneManager` and allow
the user to select one of the options.

The `ZoneManager::indexForZoneName()` and `ZoneManager::indexForZoneId()` are
two useful methods to convert an arbitrary time zone reference (either
by zoneName or zoneId) into an index into the registry.

### TZ Database Version

The IANA TZ Database is updated continually. As of this writing, the latest
stable version is 2019b. When a new version of the database is released, it is
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
BasicZoneProcessor zoneProcessor;
auto tz = TimeZone::forZoneInfo(&zonedb::kZoneAmerica_Los_Angeles,
    &zoneProcessor);
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

## Clocks

The `acetime::clock` namespace contains classes needed to implement various
types of clocks using different sources. For example, the `DS3231Clock` uses the
DS3231 RTC chip, and the `NtpClock` uses an NTP server. The `SystemClock` is
powered by the internal `millis()` function, but that function is usually not
accurate enough. So the `SystemClock` has the ability to synchronize against
more accurate external clocks such as the `DS3231Clock` and the `NtpClock`. The
`SystemClock` also has the ability to backup the current time to a non-volatile
time source (e.g. `DS3231Clock`) so that the current time can be restored when
the power is restored.

The class hierarchy diagram for these various classes looks like this, where the
arrow means "is-subclass-of") and the diamond-line means ("is-aggregation-of"):
```
                    0..2
             Clock  ------------.
            ^  ^  ^             |
           /   |   \            |
          /    |    \           |
DS3231Clock    |    NtpClock    |
             System             |
             Clock <> ----------'
               ^ ^
              /   \
             /     \
   SystemClock    SystemClock
          Loop    Coroutine
```

### Clock Class

This is an abstract class which provides 3 functionalities:

* `setNow(acetime_t now)`: set the current time
* `acetime_ getNow()`: get current time (blocking)
* `sendRequest()`, `isResponseReady()`, `readResponse()`: get current time (non-blocking)

```C++
namespace ace_time {
namespace clock {

class Clock {
  public:
    static const acetime_t kInvalidSeconds = LocalTime::kInvalidSeconds;

    virtual void setNow(acetime_t epochSeconds) {}
    virtual acetime_t getNow() const = 0;

    virtual void sendRequest() const {}
    virtual bool isResponseReady() const { return true; }
    virtual acetime_t readResponse() const { return getNow(); }
};

}
}
```

Examples of the `Clock` include an NTP client, a GPS client, or a DS3231 RTC
chip.

Not all clocks can implement the `setNow()` method (e.g. an NTP client)
so the default implementation `Clock::setNow()` is a no-op. However, all clocks
are expected to provide a `getNow()` method. On some clocks, the `getNow()`
function can consume a large amount (many seconds) of time (e.g. `NtpClock`) so
these classes are expected to provide a non-blocking implementation of the
`getNow()` functionality through the `sendRequest()`, `isResponseReady()` and
`readResponse()` methods. The `Clock` base class provides a default
implementation of the non-blocking API by simply calling the `getNow()` blocking
API, but subclasses are expected to provide the non-blocking interface when
needed.

The `acetime_t` value from `getNow()` can be converted into the desired time
zone using the `ZonedDateTime` and `TimeZone` classes desribed in the previous
sections.

### NTP Clock

The `NtpClock` class is available on the ESP8266 and ESP32 which have builtin
WiFi capability. (I have not tested the code on the Arduino WiFi shield because
I don't have that hardware.) This class uses an NTP client to fetch the current
time from the specified NTP server. The constructor takes 3 parameters which
have default values so they are optional.

The class declaration looks like this:

```C++
namespace ace_time {
namespace clock {

class NtpClock: public Clock {
  public:
    explicit NtpClock(
        const char* server = kNtpServerName,
        uint16_t localPort = kLocalPort,
        uint16_t requestTimeout = kRequestTimeout);

    void setup(const char* ssid, const char* password);
    bool isSetup() const;
    const char* getServer() const;

    acetime_t getNow() const override;

    void sendRequest() const override;
    bool isResponseReady() const override;
    acetime_t readResponse() const override;
};

}
}
```

The constructor takes the name of the NTP server. The default value is
`kNtpServerName` which is `us.pool.npt.org`. The default `kLocalPort` is set to
8888. And the default `kRequestTimeout` is 1000 milliseconds.

You need to call the `setup()` with the `ssid` and `password` of the WiFi
connection. The method will time out after 5 seconds if the connection cannot
be established. Here is a sample of how it can be used:

```C++
#include <AceTime.h>
using namespace ace_time;
using namespace ace_time::clock;

const char SSID[] = ...; // Warning: don't store SSID in GitHub
const char PASSWORD[] = ...; // Warning: don't store passwd in GitHub

NtpClock ntpClock;

void setup() {
  Serial.begin(115200);
  while(!Serial); // needed for Leonardo/Micro
  ...
  ntpClock.setup(SSID, PASSWORD);
  if (ntpClock.isSetup()) {
    Serial.println("WiFi connection failed... try again.");
    ...
  }
}

// Print the NTP time every 10 seconds, in UTC-08:00 time zone.
void loop() {
  acetime_t nowSeconds = ntpClock.getNow();
  // convert epochSeconds to UTC-08:00
  OffsetDateTime odt = OffsetDateTime::forEpochSeconds(
      nowSeconds, TimeOffset::forHours(-8));
  odt.printTo(Serial);
  delay(10000); // wait 10 seconds
}
```

**Security Warning**: You should avoid committing your SSID and PASSWORD into a
public repository like GitHub because they will become public to anyone. Even if
you delete the commit, they can be retrieved from the git history.

### DS3231 Clock

The `DS3231Clock` class uses the DS3231 RTC chip. It contains
an internal temperature-compensated osciallator that counts time in 1
second steps. It is often connected to a battery or a supercapacitor to survive
power failures. The DS3231 chip stores the time broken down by various date and
time components (i.e. year, month, day, hour, minute, seconds). It contains
internal logic that knows about the number of days in an month, and leap years.
It supports dates from 2000 to 2099. It does *not* contain the concept of a time
zone. Therefore, The `DS3231Clock` assumes that the date/time components
stored on the chip is in **UTC** time.

The class declaration looks like this:

```C++
namespace ace_time {
namespace clock {

class DS3231Clock: public Clock {
  public:
    explicit DS3231Clock();

    void setup();
    acetime_t getNow() const override;
    void setNow(acetime_t epochSeconds) override;
};

}
}
```

The `DS3231Clock::getNow()` returns the number of seconds since
AceTime Epoch by converting the UTC date and time components to `acetime_t`
(using `LocalDatetime` internally). Users can convert the epoch seconds
into either an `OffsetDateTime` or a `ZonedDateTime` as needed.

The `DS3231Clock::setup()` should be called from the global `setup()`
function to initialize the object. Here is a sample that:

```C++
#include <AceTime.h>
using namespace ace_time;
using namespace ace_time::clock;

DS3231Clock dsClock;
...
void setup() {
  Serial.begin(115200);
  while(!Serial); // needed for Leonardo/Micro
  ...
  dsClock.setup();
  dsClock.setNow(0); // 2000-01-01T00:00:00Z
}

void loop() {
  acetime_t nowSeconds = dsClock.getNow();
  // convert epochSeconds to UTC-08:00
  OffsetDateTime odt = OffsetDateTime::forEpochSeconds(
      nowSeconds, TimeOffset::forHours(-8));
  odt.printTo(Serial);
  delay(10000); // wait 10 seconds
}
```

### System Clock

The `SystemClock` is a special `Clock` that uses the Arduino built-in `millis()`
method as the source of its time. The biggest advantage of `SystemClock` is that
its `getNow()` has very little overhead so it can be called as frequently as
needed, compared to the `getNow()` method of other `Clock` classes which can
consume a significant amount of time. For example, the `DS3231Clock` must talk
to the DS3231 RTC chip over an I2C bus. Even worse, the `NtpClock` must the talk
to the NTP server over the network which can be unpredictably slow.

Unfortunately, the `millis()` internal clock of most (all?) Arduino boards is
not very accurate and unsuitable for implementing an accurate clock. Therefore,
the `SystemClock` provides the option to synchronize its clock to an external
`reference Clock`.

The other problem with the `millis()` internal clock is that it does not survive
a power failure. The `SystemClock` provides a way to save the current time
to a `backupClock` (e.g. the `DS3231Clock` using the DS3231 chip with battery
backup). When the `SystemClock` starts up, it will read the `backup Clock` and
set the current time. When it synchronizes with the `referenceClock`, (e.g. the
`NtpClock`), it saves a copy of it into the `backupClock`.

The `SystemClock` is an abstract class, and this library provides 2 concrete
implementations, `SystemClockLoop` and `SystemClockCoroutine`:

```C++
namespace ace_time {
namespace clock {

class SystemClock: public Clock {
  public:
    void setup();

    acetime_t getNow() const override;
    void setNow(acetime_t epochSeconds) override;

    bool isInit() const;
    void forceSync();
    acetime_t getLastSyncTime() const;

  protected:
    virtual unsigned long clockMillis() const { return ::millis(); }

    explicit SystemClock(
        Clock* referenceClock /* nullable */,
        Clock* backupClock /* nullable */);
};

class SystemClockLoop: public SystemClock {
  public:
    explicit SystemClockLoop(
        Clock* referenceClock /* nullable */,
        Clock* backupClock /* nullable */,
        uint16_t syncPeriodSeconds = 3600,
        uint16_t initialSyncPeriodSeconds = 5);

    void loop();
};

#if defined(ACE_ROUTINE_VERSION)

class SystemClockCoroutine: public SystemClock, public ace_routine::Coroutine {
  public:
    explicit SystemClockCoroutine(
        Clock* referenceClock /* nullable */,
        Clock* backupClock /* nullable */,
        uint16_t syncPeriodSeconds = 3600,
        uint16_t initialSyncPeriodSeconds = 5,
        uint16_t requestTimeoutMillis = 1000,
        common::TimingStats* timingStats = nullptr);

    int runCoroutine() override;
    uint8_t getRequestStatus() const;
};

#endif

}
}
```

The `SystemClockCoroutine` class is available only if you have installed the
[AceRoutine](https://github.com/bxparks/AceRoutine) library and include its
header before `<AceTime.h>`, like this:

```C++
#include <AceRoutine.h>
#include <AceTime.h>
...
```

The constructor of each of the 2 concrete implementations take optional
parameters, the `referenceClock` and the `backupClock`. Each of these parameters
are optional, so there are 4 combinations:

```C++
SystemClock*(nullptr, nullptr); // no referenceClock or backupClock

SystemClock*(referenceClock, nullptr); // referenceClock only

SystemClock*(nullptr, backupClock); // backupClock only

SystemClock*(referenceClock, backupClock); // both clocks
```

### SystemClock Maintenance Tasks

There are 2 internal maintenance tasks that must be performed periodically.

First, the `SystemClock` must be synchronized to the `millis()` function
before an internal integer overflow occurs. The internal integer overflow
happens every 65.536 seconds.

Second (optionally), the SystemClock can be synchronized to the `referenceClock`
since internal `millis()` clock is not very accurate. How often this should
happen depends on the accuracy of the `millis()`, which depends on the hardware
oscillator of the chip, and the cost of the call to the `getNow()` method of the
syncing time clock. If the `referenceClock` is the DS3231 chip, syncing once
every 1-10 minutes might be acceptable since talking to the RTC chip over
I2C is relatively cheap. If the `referenceClock` is the `NtpClock`, the network
connection is fairly expensive so maybe once every 1-12 hours might be
advisable.

The `SystemClock` provides 2 subclasses which differ in the way they perform
these maintenance tasks:

* the `SystemClockLoop` class uses the `::loop()` method which should be called
  from the global `loop()` function, and
* the `SystemClockCoroutine` class uses the `::runCoroutine()` method which
  uses the AceRoutine library

### SystemClockLoop

This class synchronizes to the `referenceClock` through the
`SystemClockLoop::loop()` method that is meant to be called from the global
`loop()` method, like this:

```C++
#include <AceTime.h>
using namespace ace_time;
using namespace ace_time::clock;
...

DS3231Clock dsClock;
SystemClockLoop systemClock(dsClock, nullptr /*backup*/);

void setup() {
  dsClock.setup();
  systemClock.setup();
}

void loop() {
  ...
  systemClock.loop();
  ...
}
```

`SystemClockLoop` keeps an internal counter to limit the syncing process to
every 1 hour. This is configurable through parameters in the `SystemClockLoop()`
constructor.

### SystemClockCoroutine

This class synchronizes to the `referenceClock` using an
[AceRoutine](https://github.com/bxparks/AceRoutine) coroutine.

```C++
#include <AceRoutine.h> // include this before <AceTime.h>
#include <AceTime.h>
using namespace ace_time;
using namespace ace_time::clock;
using namespace ace_routine;
...

DS3231Clock dsClock;
SystemClock systemClock(dsClock, nullptr /*backup*/);

void setup() {
  ...
  dsClock.setup();
  systemClock.setupCoroutine(F("systemClock"));
  CoroutineScheduler::setup();
  ...
}

void loop() {
  ...
  CoroutineScheduler::loop();
  ...
}
```

`SystemClockCoroutine` keeps internal counters to limit the syncing process to
every 1 hour. This is configurable through parameters in the
`SystemClockCoroutine()` constructor.

Previously, the `SystemClockLoop::loop()` used the blocking `Clock::getNow()`
method, and the `SystemClockCoroutine::runCoroutine()` used the non-blocking
methods of `Clock`. However, since version 0.6, both of them use the
*non-blocking* calls, so there should be little difference between the two
except in how the methods are called.

### SystemClock Examples

Here is an example of a `SystemClockLoop` that uses no `referenceClock` or a
`backupClock`. The accuracy of this clock is limited by the accuracy of the
internal `millis()` function, and the clock has no backup against power failure.
Upon reboot, the user must be asked to call `SystemClock::setNow()` to set the
current time. The `SystemClock::loop()` must still be called to perform a
maintenance task to synchronize to `millis()`.

```C++
#include <AceTime.h>
using namespace ace_time;
using namespace ace_time::clock;

SystemClock systemClock(&dsClock /*reference*/, nullptr /*backup*/);
...

void setup() {
  systemClock.setup();
  ...
}

void loop() {
  systemClock.loop();
  ...
}
```

Here is a more realistic example of a `SystemClockLoop` using the `NtpClock` as
the `referenceClock` and the `DS3231Clock` as the `backupClock`.

```C++
#include <AceTime.h>
using namespace ace_time;
using namespace ace_time::clock;

DS3231Clock dsClock;
NtpClock ntpClock(SSID, PASSWORD);
SystemClockLoop systemClock(&ntpClock /*reference*/, &dsClock /*backup*/);
...

void setup() {
  Serial.begin(115200);
  while(!Serial); // needed for Leonardo/Micro
  ...
  dsClock.setup();
  ntpClock.setup();
  systemClock.setup();
}

// do NOT use delay(), it breaks systemClock.loop()
void loop() {
  static acetime_t prevNow = systemClock.getNow();

  systemClock.loop();
  acetime_t now = systemClock.getNow();
  if (now - prevNow >= 10) {
    auto odt = OffsetDateTime::forEpochSeconds(
        now, TimeOffset::forHours(-8)); // convert epochSeconds to UTC-08:00
    odt.printTo(Serial);
  }
}
```

If you wanted to use the `DS3231Clock` as *both* the backup and sync
time sources, then the setup would something like this:

```C++
#include <AceTime.h>
using namespace ace_time;
using namespace ace_time::clock;

DS3231Clock dsClock;
SystemClockLoop systemClock(&dsClock /*reference*/, &dsClock /*backup*/);
...

void setup() {
  dsClock.setup();
  systemClock.setup();
  ...
}

void loop() {
  systemClock.loop();
  ...
}
```

## Testing

Writing tests for this library was very challenging, probably taking up 2-3X
more effort than the core of the library. I think the reason is that the number
input variables into the library and the number of output variables are
substantially large, making it difficult to write isolated unit tests. Secondly,
the TZ Database zone files are deceptively easy to read by humans, but
contain so many implicit rules that are incredibly difficult to translate into
computer algorithms, creating a large number of code paths to test.

It is simply impractical to manually create the inputs and expected outputs
using the TZ database. The calculation of one data point can take several
minutes manually. The solution would be to programmatically generate the data
points. To that end, I wrote the 2 different implementations of `ZoneProcessor`
(`BasicZoneProcessor` and `ExtendedZoneProcessor`) partially as an attempt to
write different versions of the algorithms to validate them against each other.
(I think I wrote 4-5 different versions altogether, of which only 2 made it into
this library). However, it turned out that the number of timezones supported by
the `ExtendedZoneProcessor` was much larger than the ones supported by
`BasicZoneProcessor` so it became infeasible to test the non-overlapping
timezones.

My next idea was to validate AceTime against a known, independently created,
timezone library that also supports the TZ Database. Currently, I validate
the AceTime library against 3 other timezone libraries:

* Python [pytz](https://pypi.org/project/pytz/)
* Java 11 [java.time](https://docs.oracle.com/en/java/javase/11/docs/api/java.base/java/time/package-summary.html)
* C++11/14/17 [Hinnant date](https://github.com/HowardHinnant/date)

When these tests pass, I become confident that AceTime is producing the correct
results, but it is entirely expected that some obscure edge-case bugs will be
found in the future.

### Python pytz

The Python pytz library was a natural choice since the `tzcompiler.py` was
already written in Python. I created:

* [BasicValidationUsingPythonTest](tests/validation/BasicValidationUsingPythonTest/)
* [ExtendedValidationUsingPythonTest](tests/validation/ExtendedValidationUsingPythonTest/)

The `pytz` library is used to generate various C++ source code
(`validation_data.cpp`, `validation_data.h`, `validation_tests.cpp`) which
contain a list of epochSeconds, the UTC offset, the DST offset, at DST
transition points, for all timezones. The integration test then compiles in the
`ZonedDateTime` and verifies that the expected DST transitions and date
components are identical.

The resulting data test set contains between 150k to 220k data points, and can
no longer fit in any Arduino microcontroller that I am aware of. They can be
executed only on desktop-class Linux or MacOS machines through the use of the
[UnixHostDuino](https://github.com/bxparks/UnixHostDuino) emulation framework.

The `pytz` library supports [dates only until
2038](https://answers.launchpad.net/pytz/+question/262216). It is also tricky to
match the `pytz` version to the TZ Database version used by AceTime. The
following combinations have been tested:

* TZ Datbase: 2019a; pytz: 2019.1
* TZ Datbase: 2019b; pytz: 2019.2

### Java java.time

The Java 11 `java.time` library is not limited to 2038 but supports years
through the [year 1000000000
(billion)](https://docs.oracle.com/en/java/javase/11/docs/api/java.base/java/time/class-use/Instant.html).
I wrote the [TestDataGenerator.java](tools/java/TestDataGenerator) program to
generate a `validation_data.cpp` file in exactly the same format as the
`tzcompiler.py` program, and produced data points from year 2000 to year 2050,
which is the exact range of years supported by the `zonedb::` and `zonedbx::`
zoneinfo files.

The result is 2 validation programs under `tests/validation`:

* [BasicValidationUsingJavaTest](tests/validation/BasicValidationUsingJavaTest/)
* [ExtendedValidationUsingJavaTest](tests/validation/ExtendedValidationUsingJavaTest/)

The most difficult part of using Java is figuring out how to install it
and figuring out which of the many variants of the JDK to use. On Ubuntu 18.04,
I used `openjdk 11.0.4 2019-07-16` which seems to use TZ Database 2018g. I have
no recollection how I installed, I think it was something like `$ sudo apt
install openjdk-11-jdk:amd64`.

The underlying timezone database used by the `java.time` package seems to be
locked to the release version of the JDK. I have not been able to figure out a
way to upgrade the timezone database independently (it's something to do with
the
[TZUpdater](https://www.oracle.com/technetwork/java/javase/documentation/tzupdater-readme-136440.html)
but I haven't figured it out.)

### C++ Hinnant Date

I looked for a timezone library that allowed me to control the specific
version of the TZ Database. This led me to the C++11/14/17 [Hinnant
date](https://github.com/HowardHinnant/date) library, which has apparently been
accepted into the C++20 standard. This date and timezone library is incredible
powerful, complex and difficult to use. I managed to incorporate it into 2 more
validation tests, and verified that the AceTime library matches the Hinnant date
library for all timezones from 2000 to 2049 (inclusive):

* [BasicValidationUsingHinnantDateTest](tests/validation/BasicValidationUsingHinnantDateTest/)
* [ExtendedValidationUsingHinnantDateTest](tests/validation/ExtendedValidationUsingHinnantDateTest/)

I have validated the AceTime library with the following TZ versions against
the Hinnant date library with the same TZ version:

* TZ Database: 2019a
* TZ Database: 2019b

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
sizeof(TimeOffset): 2
sizeof(OffsetDateTime): 8
sizeof(BasicZoneProcessor): 113
sizeof(ExtendedZoneProcessor): 453
sizeof(BasicZoneManager<1>): 121
sizeof(ExtendedZoneManager<1>): 461
sizeof(TimeZone): 5
sizeof(ZonedDateTime): 13
sizeof(TimePeriod): 4
sizeof(DS3231Clock): 3
sizeof(SystemClockLoop): 34
sizeof(SystemClockCoroutine): 44
```

**32-bit processors**
```
sizeof(LocalDate): 3
sizeof(LocalTime): 3
sizeof(LocalDateTime): 6
sizeof(TimeOffset): 2
sizeof(OffsetDateTime): 8
sizeof(BasicZoneProcessor): 156
sizeof(ExtendedZoneProcessor): 532
sizeof(BasicZoneManager<1>): 176
sizeof(ExtendedZoneManager<1>): 552
sizeof(TimeZone): 12
sizeof(ZonedDateTime): 20
sizeof(TimePeriod): 4
sizeof(NtpClock): 88 (ESP8266), 116 (ESP32)
sizeof(SystemClockLoop): 44
sizeof(SystemClockCoroutine): 68
```

The [MemoryBenchmark](examples/MemoryBenchmark) program gives a more
comprehensive answer to the amount of memory taken by this library.
Here is a short summary for an 8-bit microcontroller (e.g. Arduino Nano):

* Using the `TimeZone` class with a `BasicZoneProcessor` for one timezone takes
  about 6 kB of flash memory and 193 bytes of static RAM.
* Using 2 timezones with `BasicZoneProcessor increases the consumption to
  about 7 kB of flash and 207 bytes of RAM.
* Loading the entire `zonedb::` zoneinfo database consumes 21 kB bytes of flash
  and 597 bytes of RAM.
* Adding the `SystemClock` to the `TimeZone` and `BasicZoneProcessor` with one
  timezone consumes 9 kB bytes of flash and 352 bytes of RAM.

These numbers indicate that the AceTime library is useful even on a limited
8-bit controller with only 30-32 kB of flash and 2 kB of RAM. As a concrete
example, the [WorldClock](examples/WorldClock) program contains 3 OLED displays
over SPI, 2 buttons, one DS3231 chip, and 3 timezones using AceTime, and these
all fit inside a Arduino Pro Micro limit of 30 kB flash and 2.5 kB of RAM.

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

### C Time Library (time.h)

Some version of the standard Unix/C library `<time.h>` is available in *some*
Arduino platforms, but not others:

* The [AVR libc time
  library](https://www.nongnu.org/avr-libc/user-manual/group__avr__time.html),
  contains methods such as `gmtime()` to convert `time_t` integer into date time
  components `struct tm`, and `mk_gmtime()` to convert components into a
  `time_t` integer. The `time_t` integer is unsigned, and starts at
  2000-01-01T00:00:00Z. There is no support for timezones.
* The SAMD21 and Teensy platforms do not seem to have a `<time.h>` library.
* The ESP8266 and ESP32 have a `<time.h>` library. There seems to be some
  rudimentary support for POSIX formatted timezones. It does not have
  the equivalent of the (non-standard) `mk_gmtime()` AVR function.

These libraries are all based upon the [traditional C/Unix library
methods](http://www.catb.org/esr/time-programming/) which can be difficult to
understand.

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

### HowardHinnant Date Library

The [date](https://github.com/HowardHinnant/date) package by Howard Hinnant is
based upon the `<chrono>` standard library and consists of several libraries of
which `date.h` and `tz.h` are comparable to AceTime. Modified versions of these
libraries were voted into the C++20 standard.

Unfortunately these libaries are not suitable for an Arduino microcontroller
environment because:

* The libraries depend extensively on 64-bit integers which are
  impractical on 8-bit microcontrollers with only 32kB of flash memory.
* The `tz.h` library has the option of downloading the TZ Database files over
  the network using `libcurl` to the OS filesystem then parsing the files, or
  using the native zoneinfo files on the host OS. Neither options are practical
  on small microcontrollers. The raw TZ Database files consume about 1MB in
  gzip'ed format, which are not suitable for a 32kB Arduino microcontroller.
* The libraries has dependencies on other libraries such as `<iostream>` and
  `<chrono>` which don't exist on most Arduino platforms.
* The libraries are heavily templatized to provide maximum flexibility
  and type-safety. But this makes the libraries incredibly hard to understand
  and cumbersome to use for the simple use cases targeted by the AceTime
  library.

The Hinnant date libraries were invaluable for writing the
[BasicValidationUsingHinnantDateTest](tests/validation/BasicValidationUsingHinnantDateTest/)
and
[ExtendedValidationUsingHinnantDateTest](tests/validation/ExtendedValidationUsingHinnantDateTest/)
validation tests (in v0.7) which are the AceTime algorithms to the Hinnant Date
algorithms. For all times zones between the years 2000 until 2050, the AceTime
UTC offsets (`TimeZone::getUtcOffset()`), timezone abbreviations
(`TimeZone::getAbbrev()`), and epochSecond conversion to date components
(`ZonedDateTime::fromEpochSeconds()`) match the results from the Hinannt Date
libraries.

### Google cctz

The [cctz](https://github.com/google/cctz) library from Google is also based on
the `<chrono>` library. I have not looked at this library closely because I
assumed that it would fit inside an Arduino controller. Hopefully I will get
some time to take a closer look in the future.

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
* `toUnixSeconds()`
    * [Unix time](https://en.wikipedia.org/wiki/Unix_time) uses an epoch of
      1970-01-01T00:00:00Z. This method returns an `acetime_t` which is
      a signed integer, just like the old 32-bit Unix systems. The range of
      dates is 1901-12-13T20:45:52Z to 2038-01-19T03:14:07Z.
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
    * It might be possible to use a Basic `TimeZone` created using a `zonedb::`
      zoneinfo file, and an Extended `TimeZone` using a `zonedbx::` zoneinfo
      file. However, this is not a configuration that is expected to be used
      often, so it has not been tested well, if at all.
    * One potential problem is that the equality of two `TimeZone` depends only
      on the `zoneId`, so a Basic `TimeZone` created with a
      `zonedb::kZoneAmerica_Los_Angeles` will be considered equal to an Extended
      `TimeZone` created with a `zonedbx::kZoneAmerica_Los_Angeles`.
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
* `NtpClock`
    * The `NtpClock` on an ESP8266 calls `WiFi.hostByName()` to resolve
      the IP address of the NTP server. Unfortunately, when I tested this
      library, it seems to be blocking call (later versions may have fixed
      this). When the DNS resolver is working properly, this call returns in
      ~10ms or less. But sometimes, the DNS resolver seems to get into a state
      where it takes 4-5 **seconds** to time out. Even if you use AceRoutine
      coroutines, the entire program will block for those 4-5 seconds.
    * [NTP](https://en.wikipedia.org/wiki/Network_Time_Protocol) uses an epoch
      of 1900-01-01T00:00:00Z, with 32-bit unsigned integer as the seconds
      counter. It will overflow just after 2036-02-07T06:28:15Z.
* `BasicZoneProcessor`
    * Supports 1-minute resolution for AT and UNTIL fields.
    * Supports only a 15-minute resolution for STDOFF and DST offset fields,
      but this is sufficient to support the vast majority of timezones since
      2000.
    * The `zonedb/` files have been filtered to satisfy these constraints.
    * Tested again Python [pytz](https://pypi.org/project/pytz/) from
      2000 until 2038 (limited by pytz).
    * Tested against Java `java.time` from 2000 to until 2050.
    * Tested against C++11/14/17
      [Hinnant date](https://github.com/HowardHinnant/date) from 2000 until
      2050.
* `ExtendedZoneProcessor`
    * Has a 1-minute resolution for all time fields.
    * The `zonedbx/` files currently (version 2019b) do not have any timezones
      with 1-minute resolution.
    * Tested again Python [pytz](https://pypi.org/project/pytz/) from
      2000 until 2038 (limited by pytz).
    * Tested against Java `java.time` from 2000 to until 2050.
    * Tested against [Hinnant date](https://github.com/HowardHinnant/date)
      using 1-minute resolution from 1975 to 2050. See
      [ExtendedValidationUsingHinnantDateTest](tests/validation/ExtendedValidationUsingHinnantDateTest).
* `zonedb/` and `zonedbx/` zoneinfo files
    * These statically defined data structures are loaded into flash memory
      using the `PROGMEM` keyword. The vast majority of the data structure
      fields will stay in flash memory and not copied into RAM.
    * The zoneinfo files have *not* been compressed using bit-fields or any
      other compression techniques. It may be possible to decrease the size of
      the full database using these compression techniques. However, compression
      will increase the size of the program file, so for applications that use
      only a small number of zones, it is not clear if the zoneinfo file
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
* `Link` entries
    * The TZ Database `Link` entries are implemented as C++ references to
      the equivalent `Zone` entries. For example,
      `zonedb::kZoneUS_Pacific` is just a reference to
      `zonedb::kZoneAmerica_Los_Angeles`. This means that if a `ZonedDateTime`
      is created with a `TimeZone` associated with `kZoneUS_Pacific`, the
      `ZonedDateTime::printTo()` will print "[America/Los_Angeles]" not
      "[US/Pacific]".
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
        * If you are running the [AceTime unit tests](tests/), you need to have
          a working `SERIAL_PORT_MONITOR`, so the "Arduino MKR ZERO" board
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
    * The SAMD21 microcontroller does *not* provide any EEPROM. Therefore,
      this feature is disabled in the apps under `examples` (e.g.
      `CommandLineClock`, `OledClock`, and `WorldClock`) which use this feature.
    * The `MKR Zero` board generates far faster code (30%?) than the `SparkFun
      SAMD21 Mini Breakout` board. The `MKR Zero` could be using a different
      (more recent?) version of the GCC tool chain. I have not investigated
      this.


## Untested RTC hardware compatibility

As DS1307 and DS3232 RTC chip have exactly same interface as DS3231 for used
features except temperature on DS1307 (this one lacks this functionality),
this class could be be used to control all of them.

Any problem with these RTCs please, contact @Naguissa (https://github.com/Naguissa/AceTime)
