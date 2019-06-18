# AceTime Library User Guide

## Installation

The latest stable release will soon be available in the Arduino IDE Library
Manager. Search for "AceTime". Click Install.

The development version can be installed by cloning the
[GitHub repository](https://github.com/bxparks/AceTime), checking out the
`develop` branch, then manually copying over the contents to the `./libraries`
directory used by the Arduino IDE. (The result is a directory named
`./libraries/AceTime`.) The `master` branch contains the stable release.

### Source Code

The source files are organized as follows:
* `src/AceTime.h` - main header file
* `src/ace_time/` - date and time files
* `src/ace_time/common/` - internal shared files
* `src/ace_time/hw/` - thin hardware abstraction layer
* `src/ace_time/clock/` - system clock from RTC or NTP sources
* `src/ace_time/testing/` - files used in unit tests
* `src/ace_time/zonedb/` - files generated from TZ Database for
  `BasicZoneSpecifier`
* `src/ace_time/zonedbx/` - files generated from TZ Database for
  `ExtendedZoneSpecifier`
* `tests/` - unit tests using [AUnit](https://github.com/bxparks/AUnit)
* `examples/` - example programs
* `tools/` - parser for the TZ Database files, code generators for `zonedb::`
  and `zonedbx::` zone files, and code generators for various unit tests

### Dependencies

The vast majority of the AceTime library has no dependency to any other external
libraries. There is an optional dependency to
[AceRoutine](https://github.com/bxparks/AceRoutine) if you want to use the
`SystemClockSyncCoroutine` and `SystemClockHeartbeatCoroutine` classes for
automatic syncing and freshening. (This is recommended but not strictly
necessary). The `ace_time/hw/CrcEeprom.h` class has a dependency to the FastCRC
library but the `CrcEeprom.h` file is not included in the `AceTime.h` main
header file, so you should not need FastCRC to compile AceTime. (The
`CrcEeprom.h` header file does not strictly belong in the AceTime library but
many of my "clock" projects that use the AceTime library also use the
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

* [HelloTime](examples/HelloTime/)
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

The names and API of AceTime classes is heavily borrowed from the [Java JDK 11
java.time](https://docs.oracle.com/en/java/javase/11/docs/api/java.base/java/time/package-summary.html)
package. Some important differences come from the fact that in Java, most
objects are reference objects and created on the heap. To allow AceTime
to work on an Arduino chip with only 2kB of RAM and 32kB of flash, I made sure
that the AceTime C++ classes perform *no* heap allocations (i.e. no calls to
`operator new()` or `malloc()`). Many of the smaller classes in the library are
expected to be used as "value objects", in other words, created on the stack and
copied by value. Fortunately, the C++ compilers are extremely good at optimizing
away unnecessary copies of these small objects. It is not possible to remove all
complex memory allocations when dealing with the TZ Database. In the AceTime
library, I managed to move most (if not all) of the complex memory handling
logic into the `ZoneSpecifier` class hierarhcy. These are relatively large
objects which are meant to be opaque objects (to the application developer),
created statically at start-up time of the application, and never deleted during
the lifetime of the application.

The [Arduino Time](https://github.com/PaulStoffregen/Time) library uses
a set of C functions similar to the
[traditional C/Unix library methods](http://www.catb.org/esr/time-programming/)
(e.g `makeTime()` and `breaktime()`). Unfortunately, those C library functions
can be very confusing to understand. Arduino Time Library also uses the Unix
epoch of 1970-01-01T00:00:00Z and a `int32_t` type as its `time_t` to track the
number of seconds since the epoch. That means that the largest date it can
handle is 2038-01-19T03:14:07Z. AceTime uses an epoch that starts on
2000-01-01T00:00:00Z using the same `int32_t` as its `ace_time::acetime_t`,
which means that maximum date increases to 2068-01-19T03:14:07Z. AceTime is also
quite a bit faster than the Arduino Time Library (although in most cases,
performance of the Time Library is not an issue): AceTime is **2-5X** faster on
an ATmega328P, **3-4X** faster on the ESP8266, **3-5X** faster on the ESP32, and
**7-20X** faster on the Teensy ARM processor.

AceTime aims to be the smallest library that can run on the basic Arduino
platform (e.g. Nano with 32kB flash and 2kB of RAM) that fully supports all
timezones in the TZ Database. It also aims to be as portable as possible, and
will support AVR microcontrollers, as well as ESP8266, ESP32 and most Teensy
microcontrollers.

## Headers and Namespaces

Only a single header file `AceTime.h` is required to use this library.
To prevent name clashes with other libraries that the calling code may use, all
classes are separated into a number of namespaces. They are related in the
following way, where the arrow means "depends on":

```
ace_time::clock
      |      \
      |       \
      |        v
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
LocalDate localDate = LocalDate::forComponents(2019, 5, 20);

// LocalTime that represents 13:00:00
LocalTime localTime = LocalTime::forComponents(13, 0, 0);
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

    const char* monthLongString(uint8_t month) const;
    const char* monthShortString(uint8_t month) const;

    const char* weekDayLongString(uint8_t weekDay) const;
    const char* weekDayShortString(uint8_t weekDay) const;
};

}
}
```

The `DateStrings` object uses a temporary internal buffer to hold the generated
human-readable strings, which makes it stateful. The recommended usage of this
object is to create an instance the stack, call one of the `weekDay*String()` or
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

LocalDate localDate = LocalDate::forComponents(2019, 5, 20);
uint8_t dayOfWeek = localDate.dayOfWeek();
Serial.println(DateStrings().weekDayLongString(dayOfWeek));
Serial.println(DateStrings().weekDayShortString(dayOfWeek));
```

The `weekDayShortString()` method returns the
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
yourself. See the implementation details of the `DateStrings` class to see how
that can be done.

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
LocalDateTime localDateTime;

// 2018-08-30T06:45:01-08:00
localDateTime = LocalDateTime::forComponents(2018, 8, 30, 6, 45, 1);
acetime_t epoch_seconds = localDateTime.toEpochSeconds();
```

We can go the other way and create a `LocalDateTime` from the Epoch Seconds:

```C++
LocalDateTime localDateTime = LocalDateTime::forEpochSeconds(1514764800L);
localDateTime.printTo(Serial); // prints "2018-01-01T00:00:00"
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
TimeOffset offset = TimeOffset::forHour(-8); // -08:00
TimeOffset offset = TimeOffset::forHourMinute(-2, -30); // -02:30
TimeOffset offset = TimeOffset::forMinutes(135); // +02:15
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

An `OffsetDateTime` is an object that can represent a date&time which is
offset from the UTC time zone by a fixed amount. Internally the `OffsetDateTime`
is a aggregation of `LocalDateTime` and `TimeOffset`.

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
OffsetDateTime offsetDateTime = OffsetDateTime::forComponents(
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
OffsetDateTime offsetDateTime = OffsetDateTime::forEpochSeconds(
    568079100, TimeOffset::forHourMinute(0, 15));
```

### TimeZone

A "time zone" is often used colloquially to mean 2 different things:
* A time which is offset from the UTC time by a fixed amount, or
* A physical (or conceptual) region whose local time is offset
from the UTC time using various transition rules.

Both meanings of "time zone" are supported by the `TimeZone` class using
4 different types as follows:

* `TimeZone::kTypeFixed`: a fixed offset from UTC
* `TimeZone::kTypeManual` with `ManualZoneSpecifier`: A user-defined
fixed offset with a user-defined DST flag
* `TimeZone::kTypeBasic` with `BasicZoneSpecifier`: zones which can
be encoded with (relatively) simple rules from the TZ Database
* `TimeZone::kTypeExtended` with `ExtendedZoneSpecifier`:  all zones in
the TZ Database

```C++
namespace ace_time {

class TimeZone {
  public:
    static const uint8_t kTypeFixed = 0;
    static const uint8_t kTypeManual = ZoneSpecifier::kTypeManual;
    static const uint8_t kTypeBasic = ZoneSpecifier::kTypeBasic;
    static const uint8_t kTypeExtended = ZoneSpecifier::kTypeExtended;

    static TimeZone forTimeOffset(TimeOffset offset);
    static TimeZone forZoneSpecifier(const ZoneSpecifier* zoneSpecifier);
    TimeZone();

    uint8_t getType() const;
    TimeOffset getUtcOffset(acetime_t epochSeconds) const;
    TimeOffset getDeltaOffset(acetime_t epochSeconds) const;
    TimeOffset getUtcOffsetForDateTime(const LocalDateTime& ldt) const;

    void printTo(Print& printer) const;
    void printAbbrevTo(Print& printer, acetime_t epochSeconds) const;

    bool isDst() const;
    void isDst(bool dst);

    ...
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

The `printAbbrevTo(printer, epochSeconds)` method prints the human-readable timezone
abbreviation used at the given `epochSeconds` to the `printer`.

#### Fixed TimeZone (kTypeFixed)

The default constructor creates a fixed `TimeZone` in UTC time zone with no
offset:

```C++
TimeZone tz; // UTC+00:00
```

To create `TimeZone` instances with other offsets, use one of the factory
methods:

```C++
TimeZone tz = TimeZone::forTimeOffset(TimeOffset::forHour(-8)); // UTC-08:00
TimeZone tz = TimeZone::forTimeOffset(TimeOffset::forHourMinute(-4, -30)); // UTC-04:30
```

#### ManualZoneSpecifier (kTypeManual)

A `ManualZoneSpecifier` describes a time zone which allows the user to set
the UTC offset, and to select whether or not the DST offset is being observed.
The constructor looks like this:

```C++
ManualZoneSpecifier(
    TimeOffset stdOffset = TimeOffset(),
    bool isDst = false,
    const char* stdAbbrev = "",
    const char* dstAbbrev = "",
    TimeOffset deltaOffset = TimeOffset::forHour(1));
```
All the parameters are theoretically optional, but most applications will
set at least the `stdOffset` parameter:

* `stdOffset`: the standard time UTC offset (default: 00:00)
* `isDst`: whether the DST is being observed (default: false)
* `stdAbbrev`: the abbreviation during standard time (default: "")
* `dstAbbrev`: the abbreviation during DST (default: "")
* `deltaOffset`: the time shift during DST (default: +01:00)

When `ManualZoneSpecifier::getUtcOffset()` is called, it will normally return
the value of `stdOffset`. However, if the user sets the `isDst` flag to `true`
using `ManualZoneSpecifier::isDst(true)`, then `getUtcOffset()` will return
`stdOffset + deltaOffset`.

The `ManualZoneSpecifier` is expected to be created once at the beginning of
the application. The `TimeZone` object can be created on demand by pointing it
to the `ManualZoneSpecifier` instance. For example, the following creates a
`TimeZone` set to be UTC-08:00 normally, but can change to UTC-07:00 when the
`ManualZoneSpecifier::isDst(true)` is called:

```C++
ManualZoneSpecifier zoneSpecifier(TimeOffset::forHour(-8), false, "PST", "PDT");

void someFunction() {
  ...
  TimeZone tz = TimeZone::forZoneSpecifier(&zoneSpecifier);
  TimeOffset offset = tz.getUtcOffset(0); // returns -08:00
  tz.isDst(true);
  offset = tz.getUtcOffset(0); // returns -07:00
  ...
}
```

The `TimeZone::isDst()` and `TimeZone::isDst(bool)` methods are convenience
methods that work only if the `TimeZone` instance refers to a
`ManualZoneSpecifier`. They simply call the underying `ManualZoneSpecifier`. If
the underlying `ZoneSpecifier` is a different type, the `TimeZone::isDst()` does
nothing.

#### BasicZoneSpecifier (kTypeBasic)

The `BasicZoneSpecifier` represents a time zone defined by the TZ Database. The
constructor accepts a pointer to a `basic::ZoneInfo`:

```C++
BasicZoneSpecifier(const basic::ZoneInfo* zoneInfo);
```

The supported `basic::ZoneInfo` data objects are contained in
[zonedb/zone_info.h](src/ace_time/zonedb/zone_infos.h) which was generated by
a script using the TZ Database. This header file is already included in
`<AceTime.h>`. As of version 2019a of the database, it contains 231 zones whose
time change rules are simple enough to be supported by `BasicZoneSpecifier`. The
bottom of the `zone_infos.h` header file lists 128 zones whose zone rules are
too complicated for `BasicZoneSpecifier`. Some examples of `ZoneInfo` entries
supported by `zonedb` are:

* `zonedb::kZoneAmerica_Los_Angeles`
* `zonedb::kZoneAmerica_New_York`
* `zonedb::kZoneEurope_London`
* ...


The following example creates a `TimeZone` using a `BasicZoneSpecifier` which
describes `America/Los_Angeles`:

```C++
#include <AceTime.h>
using namespace ace_time;
...

BasicZoneSpecifier zoneSpecifier(&zonedb::kZoneAmerica_Los_Angeles);

void someFunction() {
  ...
  TimeZone tz = TimeZone::forZoneSpecifier(&zoneSpecifier);

  // 2018-03-11T01:59:59-08:00 was still in STD time
  {
    OffsetDateTime dt = OffsetDateTime::forComponents(2018, 3, 11, 1, 59, 59,
      TimeOffset::forHour(-8));
    acetime_t epochSeconds = dt.toEpochSeconds();
    TimeOffset offset = tz.getUtcOffset(epochSeconds); // returns -08:00
  }

  // 2018-03-11T02:00:00-08:00 was in DST time
  {
    OffsetDateTime dt = OffsetDateTime::forComponents(2018, 3, 11, 2, 0, 0,
      TimeOffset::forHour(-8));
    acetime_t epochSeconds = dt.toEpochSeconds();
    TimeOffset offset = tz.getUtcOffset(epochSeconds); // returns -07:00
  }
  ...
}
```

#### ExtendedZoneSpecifier (kTypeExtended)

The `ExtendedZoneSpecifier` is very similar to `BasicZoneSpecifier` except that
it supports (almost) all zones in the TZ Database instead of a subset. The
supported zones are given in
[zonedbx/zone_infos.h](src/ace_time/zonedbx/zone_infos.h).
As of version 2019a of TZ Database, there are 348 supported time zones. We`
ignore the 11 zones are those whose zone names do **not** contain a `/`
character (CET, CST6CDT, EET, EST, EST5EDT, HST, MET, MST, MST7MDT, PST8PDT,
WET) because they don't correspond to an actual geographical zone.

The zone infos which can be used by `ExtendedZoneSpecifier` are in the
`zonedbx::` namespace instead of the `zonedb::` namespace. Some examples of the
zone infos are:
* `zonedbx::kZoneAmerica_Los_Angeles`
* `zonedbx::kZoneAmerica_Indiana_Indianapolis`
* `zonedbx::kZoneAmerica_New_York`
* `zonedbx::kZoneEurope_London`
* `zonedbx::kZoneAfrica_Casablanca`
* ...

The usage is the same as `BasicZoneSpecifier`:

```C++
ExtendedZoneSpecifier zoneSpecifier(&zonedbx::kZoneAmerica_Los_Angeles);

void someFunction() {
  ...
  TimeZone tz = TimeZone::forZoneSpecifier(&zoneSpecifier);

  // 2018-03-11T01:59:59-08:00 was still in STD time
  {
    OffsetDateTime dt = OffsetDateTime::forComponents(2018, 3, 11, 1, 59, 59,
      TimeOffset::forHour(-8));
    acetime_t epochSeconds = dt.toEpochSeconds();
    TimeOffset offset = tz.getUtcOffset(epochSeconds); // returns -08:00
  }

  // 2018-03-11T02:00:00-08:00 was in DST time
  {
    OffsetDateTime dt = OffsetDateTime::forComponents(2018, 3, 11, 2, 0, 0,
      TimeOffset::forHour(-8));
    acetime_t epochSeconds = dt.toEpochSeconds();
    TimeOffset offset = tz.getUtcOffset(epochSeconds); // returns -07:00
  }
  ...
}
```

The advantage of `ExtendedZoneSpecifier` over `BasicZoneSpecifier` is that
`ExtendedZoneSpecifier` supports all (actual geographical) time zones in the TZ
Database. The cost is that it consumes 5 times more memory and is slower. If
`BasicZoneSpecifier` supports the zone that you want using the zone files in the
`zonedb::` namespace, you should normally use that instead of
`ExtendedZoneSpecifier`. The one other advatnage of `ExtendedZoneSpecifier`
over `BasicZoneSpecifier` is that `ExtendedZoneSpecifier::forComponents()`
is more accurate than `BasicZoneSpecifier::forComponents()` because the
`zonedbx::` data files contain transition information which are missing in the
`zonedb::` data files due to space constraints.

### ZonedDateTime

A `ZonedDateTime` is a `LocalDateTime` associated with a given `TimeZone`. This
is analogous to an`OffsetDateTime` being a `LocalDateTime` associated with a
`TimeOffset`. All 4 types of `TimeZone` are supported, the `ZonedDateTime`
class itself does not care which one is used.

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
  TimeZone tz = TimeZone::forZoneSpecifier(&zoneSpecifier);

  // 2018-01-01 00:00:00+00:15
  ZonedDateTime zonedDateTime = ZonedDateTime::forComponents(
      2018, 1, 1, 0, 0, 0, tz);
  acetime_t epochDays = zonedDateTime.toEpochDays();
  acetime_t epochSeconds = zonedDateTime.toEpochSeconds();

  zonedDateTime.printTo(Serial); // prints "2018-01-01 00:00:00-08:00"
  Serial.println(epochDays); // prints 6574 [TODO: Check]
  Serial.println(epochSeconds); // prints 568079100 [TODO: Check]
  ...
}
```

#### Conversion to Other Time Zones

You can convert a given `ZonedDateTime` object into a representation in a
different time zone using the `DateTime::convertToTimeZone()` method:

```C++
static BasicZoneSpecifier zspecLosAngeles(&zonedb::kZoneAmerica_Los_Angeles);
static BasicZoneSpecifier zspecZurich(&zonedb::kZoneEurope_Zurich);

void someFunction() {
  ...
  TimeZone tzLosAngeles = TimeZone::forZoneSpecifier(&zspecLosAngeles);
  TimeZone tzZurich = TimeZone::forZoneSpecifier(&zspecZurich);

  // Europe/Zurich, 2018-01-01T09:20:00+01:00
  ZonedDateTime zurichTime = ZonedDateTime::forComponents(
      2018, 1, 1, 9, 20, 0, tzZurich);

  // Convert to America/Los_Angeles, 2018-01-01T01:20:00-08:00
  ZonedDateTime losAngelesTime = zurichTime.convertToTimeZone(tzLosAngeles);
  ...
}
```

The two `ZonedDateTime` objects will return the same value for `epochSeconds()`
because that is not affected by the time zone. However, the various date time
components (year, month, day, hour, minute, seconds) will be different.

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
we can print out a countdown to a target `ZonedDateTime` from the current
`ZonedDateTime` like this:

```C++
ZonedDateTime currentDate = ...;
ZonedDateTime targetDate = ...;
acetime_t diffSeconds = targetDate.toEpochSeconds()
    - currentDate.toEpochSeconds();
TimePeriod timePeriod(diffSeconds);
timePeriod.printTo(Serial)
```

## Error Handling

Many features of the date and time classes have explicit or implicit
range of validity in their inputs and outputs. The Arduino programming
environment does not use C++ exceptions, so we encode error states in the return
value of various classes. Most date and time classes have an `isError()` method
on them, for example:

```C++
bool LocalDate::isError() const;
bool LocalTime::isError() const;
bool LocalDateTime::isError() const;
bool OffsetDatetime::isError() const;
bool ZonedDateTime::isError() const;
bool TimeOffset::isError() const;
```

A well-crafted application should check for these error conditions before
writing or displaying the objects to the user. For example, the `zonedb::` and
`zonedbx::` files are valid from year 2000 until 2050. If you try to create a
date outside of this range, the `ZonedDateTime` object will return true of
`isError()`. The following snippet will print "true" because the year 1998 is
outside of the valid range of the `::zonedb` files.

```C++
BasicZoneSpecifier zoneSpecifier(&zonedb::kZoneAmerica_Los_Angeles);
TimeZone tz = TimeZone::forZoneSpecifier(&zoneSpecifier);
ZonedDateTime dt = ZonedDateTime::forComponents(1998, 3, 11, 1, 59, 59, tz);
Serial.println(dt.isError() ? "true" : "false");
```


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
                        1
           TimeProvider ------------.
          ^     ^                   |
         /      |      1            |
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
`ZonedDateTime` and `TimeZone` classes desribed in the previous section. For
example, to print the current time in UTC, use something like:

```C++
TimeProvider timeProvider = ...;
acetime_t nowSeconds = timeProvider.getNow();
LocalDateTime now = LocalDateTime::forEpochSeconds(nowSeconds);
now.printTo(Serial);
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
  OffsetDateTime odt = OffsetDateTime::forEpochSeconds(
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

### System Clock Heartbeat and Syncing

The `SystemClock` requires 2 maintenance tasks to run periodically
to help it keep proper time.

First, the `SystemClock::getNow()` method must be called peridically
before an internal integer overflow occurs, even if the `getNow()` is not
needed. The internal integer overflow happens every 65.536 seconds.
Even if your application is *guaranteed* to call `SystemClock::getNow()`
faster than every 65 seconds, it is probably prudent to implement a "heartbeat"
mechanism which simply calls the `SystemClock::getNow()` periodically
regardless of how often the application makes that call.

Secondly, since the internal `millis()` clock is not very accurate, we must
synchronize the `SystemClock` periodically with a more accurate time
source. The frequency of this syncing depends on the accuracy of the `millis()`
(which depends on the hardware oscillator of the chip) and the cost of the call
to the `getNow()` method of the syncing time clock. If the syncing time
source is the DS3231 chip, syncing once every 1-10 minutes might be sufficient
since talking to the RTC chip is relatively cheap. If the syncing time source is
the `NtpTimeProvider`, the network connection is fairly expensive so maybe once
every 1-12 hours might be advisable.

The `SystemClock` provides 2 ways to perform these periodic maintenance
actions. By default, the heartbeat happens every 5 seconds and the syncing
happens every 3600 seconds. Those parameters are configurable in the
constructors of the following classes.

**Method 1: Using SystemClockSyncLoop and SystemClockHeartbeatLoop**

You can use the `SystemClockSyncLoop` and `SystemClockHeartbeatLoop` classes and
insert them somewhere into the global `loop()` method, like this:

```C++
#include <AceTime.h>
using namespace ace_time;
using namespace ace_time::clock;
...

SystemClock systemClock(...);
SystemClockSyncLoop systemClockSyncLoop(systemClock);
SystemClockHeartbeatLoop systemClockHeartbeatLoop(systemClock);

void loop() {
  ...
  systemClockSyncLoop.loop();
  systemClockHeartbeatLoop.loop();
  ...
}
```

We have assumed here that the global `loop()` function executes sufficiently
frequently, for example, faster than 10 or 100 times a second.

**Method 2: Using SystemClockSyncCoroutine and SystemClockHeartbeatCoroutine**

You can use two [AceRoutine](https://github.com/bxparks/AceRoutine) coroutines
to perform the heartbeat and sync. First, `#include <AceRoutine.h>` *before* the
`#include <AceTime.h>` (which activates the `SystemClockSyncCoroutine` and
`SystemClockHeartbeatCoroutine` classes). Then create the 2 coroutines, and
configure it to run using the `CoroutineScheduler`:

```C++
#include <AceRoutine.h> // include this before <AceTime.h>
#include <AceTime.h>
using namespace ace_time;
using namespace ace_time::clock;
using namespace ace_routine;
...

SystemClock systemClock(...);
SystemClockSyncCoroutine systemClockSync(systemClock);
SystemClockHeartbeatCoroutine systemClockHeartbeat(systemClock);

void setup() {
  ...
  systemClockSync.setupCoroutine(F("systemClockSync"));
  systemClockHeartbeat.setupCoroutine(F("systemClockHeartbeat"));
  CoroutineScheduler::setup();
  ...
}

void loop() {
  CoroutineScheduler::loop();
  ...
}
```

The biggest advantage of using AceRoutine coroutines is that the syncing process
becomes non-blocking. In other words, if you are using the `NtpTimeProvider` to
provide syncing, the `SystemClockSyncLoop` object calls its `getNow()` method,
which blocks the execution of the program until the NTP server returns a
response (or the request times out after 1000 milliseconds). If you use the
`SystemClockSyncCoroutine`, the program continues to do other things (e.g. update
displays, scan for buttons) while the `NtpTimeProvider` is waiting for a
response from the NTP server.

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
classes:

**8-bit processors**

```
sizeof(LocalDate): 3
sizeof(LocalTime): 3
sizeof(LocalDateTime): 6
sizeof(TimeOffset): 1
sizeof(OffsetDateTime): 7
sizeof(ManualZoneSpecifier): 10
sizeof(BasicZoneSpecifier): 95
sizeof(ExtendedZoneSpecifier): 366
sizeof(TimeZone): 3
sizeof(ZonedDateTime): 10
sizeof(TimePeriod): 4
sizeof(SystemClock): 17
sizeof(DS3231TimeKeeper): 3
sizeof(SystemClockSyncLoop): 14
sizeof(SystemClockHeartbeatLoop): 8
sizeof(SystemClockSyncCoroutine): 31
sizeof(SystemClockHeartbeatCoroutine): 18
```

**32-bit processors**
```
sizeof(LocalDate): 3
sizeof(LocalTime): 3
sizeof(LocalDateTime): 6
sizeof(TimeOffset): 1
sizeof(OffsetDateTime): 7
sizeof(ManualZoneSpecifier): 20
sizeof(BasicZoneSpecifier): 140
sizeof(ExtendedZoneSpecifier): 472
sizeof(TimeZone): 8
sizeof(ZonedDateTime): 16
sizeof(TimePeriod): 4
sizeof(SystemClock): 24
sizeof(NtpTimeProvider): 88 (ESP8266), 116 (ESP32)
sizeof(SystemClockSyncLoop): 20
sizeof(SystemClockHeartbeatLoop): 12
sizeof(SystemClockSyncCoroutine): 52
sizeof(SystemClockHeartbeatCoroutine): 36
```

## Comparisons to Other Time Libraries

The [ComparisonBenchmark.ino](examples/ComparisonBenchmark/) program compares
the CPU time of AceTime methods with the equilvalent methods [Arduino Time
Library](https://github.com/PaulStoffregen/Time). The functionality tested was
the roundtrip conversion from `ZonedDateTime::forEpochSeconds()` to the date
time components, then back to `ZonedDateTime::toEpochSeconds()` again. Details
are given in the [README.md](examples/ComparisonBenchmark/README.md) file in
that folder, but here is a summary for various boards (all times in
microseconds):

```
----------------------------+---------+----------+
Board or CPU                | AceTime | Time Lib |
----------------------------+---------+----------+
ATmega328P 16MHz (Nano)     | 353.000 |  931.000 |
ESP8266 80MHz               |  21.600 |   68.100 |
ESP32 240MHz                |   2.145 |    9.355 |
Teensy 3.2 96MHz            |   2.330 |   22.390 |
----------------------------+---------+----------+
```

The [AVR libc time
library](https://www.nongnu.org/avr-libc/user-manual/group__avr__time.html), is
based on the UNIX/POSIX time library. I have not tried to use it on an Arduino
platform. There are 2 things going against it: First it works only on AVR
processors, and I wanted a time library that worked across multiple processors
(like the ESP8266 and ESP32). Second, the AVR time library is based on the
[traditional C/Unix library methods](http://www.catb.org/esr/time-programming/)
which can be difficult to understand.

The [ezTime](https://github.com/ropg/ezTime) is a library that seems to be
composed of 2 parts: A client library that runs on the microcontroller and a
server part that provides a translation from the timezone name to the POSIX DST
transition string. Unfortunately, this means that the controller must have
network access for this library to work. I wanted to create a library that was
self-contained and could run on an Arduino Nano with just an RTC chip without a
network shield.

The [Micro Time Zone](https://github.com/evq/utz) is a pure-C library
that can compile on the Arduino platform. It contains a limited subset of the TZ
Database encoded as C structs and determines the DST transitions using the
encoded structs. It supports roughly of 45 zones with just a 3kB tzinfo
database. The initial versions of AceTime, particularly the `BasicZoneSpecifier`
class was directly inspired by this library. I would be interesting to run this
library to the same set of "validation" unit tests that checks the AceTime logic
and see how accurate this library is. One problem with Micro Time Zone library
is that it loads the entire tzinfo database for all 45 time zones, even if only
one zone is used. Therefore, the AceTime library will consume less resources if
only a handful of zones are used, which is the expected use case of AceTime.

## Bugs and Limitations

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
* `LocalDate`, `LocalDateTime`
    * These classes (and all other Date classes which are based on these) use
      a single 8-bit signed byte to represent the 'year' internally. This saves
      memory, at the cost of restricting the range.
    * The value of -128 (`INT8_MIN`) is used to indicate an "invalid" value, so
      the actual range is [-127, 127]. This restricts the year range to [1873,
      2127].
    * It is possible to construct a `LocalDate` or `LocalDateTime` object with a
      `year` component greater than 2127, but such an object may not be very
      useful because the `toSecondsSincEpoch()` method would exceed the range of
      `acetime_t, so would return an incorrect value.
* `toUnixSeconds()`
    * [Unix time](https://en.wikipedia.org/wiki/Unix_time) uses an epoch of
      1970-01-01T00:00:00Z. On 32-bit Unix systems that use a signed 32-bit
      integer to represent the seconds field, the unix time will rollover just
      after 2038-01-19T03:14:07Z.
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
* `BasicValidationTest` and `ExtendedValidationTest`
    * These tests compare the transition times calculated by AceTime to Python's
      [pytz](https://pypi.org/project/pytz/) library. Unfortunately, pytz does
      not support dates after Unix signed 32-bit `time_t` rollover at
      (2038-01-19T03:14:07Z).
    * These are too big to run on any Arduino controller. They are designed to
      run on a Linux or MacOS machine through the Makefiles using the
      [unitduino](https://github.com/bxparks/AUnit/tree/develop/unitduino)
      emulator.
* `BasicValidationMoreTest` and `ExtendedValidationMoreTest`
    * These tests compare the transition times calculated by AceTime to Java's
      `java.time` package which should support the entire range of dates that
      AceTime can represent. We have artificially limited the range of testing
      from 2000 to 2050.
    * These are too big to run on any Arduino controller. They are designed to
      run on a Linux or MacOS machine through the Makefiles using the
      [unitduino](https://github.com/bxparks/AUnit/tree/develop/unitduino)
      emulator.
* `zonedb::` and `zonedbx::` zone files
    * These statically defined data structures are loaded into flash memory
      then copied to RAM on the Arduino chips. They do not use the `PROGMEM`
      keyword to store them only on flash. Fortunately, most `ZoneInfo`
      instances are only 40-60 bytes and the corresponding `ZonePolicy`
      instances are 50-100 bytes.
* TZ Database
    * The TZ data files contain 3 types of records: Zone, Rule and Link.
      We do not yet support Link entries which are essentiallly symbolic links
      of one timezone identifier to another. This will be added in a later
      version.