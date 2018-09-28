# AceTime

Date and time classes for Arduino with basic time zone support using offsets
from UTC. It also supports an enhanced "system clock" that can be synchronized
from an external time source, such as an
[NTP](https://en.wikipedia.org/wiki/Network_Time_Protocol) server
or a DS3231 RTC chip. This library is meant to be an alternative to the
venerable [Arduino Time Library](https://github.com/PaulStoffregen/Time).

Compared to the Arduino Time Library, here are the main differences:
1. AceTime is more object-oriented. For example, you can create multiple
   instances of the system clock if you need to.
1. AceTime uses an epoch that starts on 2000-01-01T00:00:00Z instead of the Unix
   epoch of 1970-01-01T00:00:00Z.
    * Using an `uint32_t` to track the number of seconds since the Epoch, the
      AceTime library can handle all dates from **2000** to the **end of 2099**.
    * This date range corresponds to the range of the DS3231 RTC chip.
1. AceTime is **2-3X** faster on an ATmega328P, **4X** faster on the ESP8266,
   and **10-20X** faster on the ARM (Teensy) and ESP32 processors.

There are roughly 2 categories of classes provided by the AceTime library:

* date and time primitives
    * e.g. `DateTime`, `TimeZone`, `TimePeriod`
    * Borrowing some concepts from the
      [Joda-Time](http://www.joda.org/joda-time/quickstart.html) Java library.
* system clock classes
    * e.g. `SystemTimeKeeper`, `NtpTimeProvider`, `DS3231TimeKeeper`
    * Implements the system clock syncing feature of the Arduino Time library.

Time zones are handled as offsets from UTC in 15-minute increments which
supports every time zone offset currently used today. This allows the time zone
to be stored as a single `int8_t` integer. The AceTime library does **not**
support the [tz database](https://en.wikipedia.org/wiki/Tz_database) because of
the limited flash memory capacity of most Arduino boards. This means that
Daylight Saving time must be handled manually.

Version: 0.1 (2018-09-25)

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
* `src/ace_time/` - implementation files
* `src/ace_time/common/` - internal shared files
* `src/ace_time/hw/` - very thin hardware abstraction layer
* `src/ace_time/testing/` - files used in unit tests
* `tests/` - unit tests using [AUnit](https://github.com/bxparks/AUnit)
* `examples/` - example programs

### Dependencies

The main AceTime library itself has no external dependency. There is an
an optional dependency to
[AceRoutine](https://github.com/bxparks/AceRoutine)
if you want to use the `SystemTimeSyncCoroutine` and
`SystemTimeHeartbeatCoroutine` classes for automatic syncing and freshening.
(This is recommended but not strictly necessary).

Various sketches in the `examples/` directory have a number of external
dependencies (not all of the sketches require all of the following):

* [AceRoutine](https://github.com/bxparks/AceRoutine)
* [AceButton](https://github.com/bxparks/AceButton)
* [AceSegment](https://github.com/bxparks/AceSegment)
* [FastCRC](https://github.com/FrankBoesing/FastCRC)
* [SSD1306Ascii](https://github.com/greiman/SSD1306Ascii)

### Docs

The [docs/](docs/) directory contains the
[Doxygen docs on GitHub Pages](https://bxparks.github.io/AceTime/html).

### Examples

The following example sketches are provided:

* [AutoBenchmark.ino](examples/AutoBenchmark/)
    * a program that performs CPU and memory benchmarking and print a report
* [Clock.ino](examples/Clock/)
    * a digital clock using a DS3231 RTC chip, an NTP client, 2 buttons, and a
      SSD1306 OLED display
* [CommandLineClock.ino](examples/CommandLineClock/)
    * a clock using a DS3231 RTC chip, an NTP client, and serial port command
      line interface,
* [ComparisonBenchmark.ino](examples/ComparisonBenchmark/)
    * a program that compares the speed of AceTime with the
      [Arduino Time Library](https://github.com/PaulStoffregen/Time).
* [CrcEepromDemo.ino](examples/CrcEepromDemo/)
    * a program that verifies the `CrcEeprom` class

## Usage

### Include Header and Namespace

Only a single header file `AceTime.h` is required to use this library.
To prevent name clashes with other libraries that the calling code may use, all
classes are defined in the `ace_time` namespace. To use the code without
prepending the `ace_time::` prefix, use the `using` directive:

```C++
#include <AceTime.h>
using namespace ace_time;
```

### Date Time Primitives

The `DateTime` class holds the components of a date, time and its associated
time zone. This includes the `year`, `month`, `day`, `hour`, `minute`, `second`,
and the UTC offset represented as an integral multiple of 15 minutes. Each of
these components are stored internally as an unsigned `uint8_t` integer, except
for the `TimeZone` which is stored as a signed `int8_t` integer. The year is
represented by the last 2 digits of the years between 2000 and 2099.

The following is basic usage guide. More details can be found in the Doxygen
docs.

#### TimeZone

The `TimeZone` class is a thin wrapper around an `int8_t` integer that
represents the number of 15-minute offsets from the UTC time zone. A time zone
object can created using a constructor or one of 3 factory methods.

For example, the Pacific Standard Time is `UTC-08:00`, which is `-32` units away
from UTC. Here are the 4 ways to create the TimeZone object:

```C++
TimeZone tz(-32);
TimeZone tz = TimeZone::forHour(-8);
TimeZone tz = TimeZone::forHourMinute(-1, 8, 0);
TimeZone tz = TimeZone::forOffsetString("-08:00");
```

As another example, the Central European Time Zone `UTC+01:00` can be
constructed using one of the following:
```C++
TimeZone tz(4);
TimeZone tz = TimeZone::forHour(1);
TimeZone tz = TimeZone::forHourMinute(1, 1, 0);
TimeZone tz = TimeZone::forOffsetString("+01:00");
```
The `+` symbol in `"+01:00"` is currently required by the string parser if
the time zone is a positive offset from UTC.

#### DateTime Components

You can create an instance of the `DateTime` object using the constructor that
takes the (year, month, day, hour, minute, second, timeZone) parameters:
```C++
DateTime dt;

// 2001-01-01 00:00:00Z
dt = DateTime(1, 1, 1, 0, 0, 0, TimeZone(0));

// 2018-08-30T06:45:01-07:00
dt = DateTime(18, 8, 30, 6, 45, 1, TimeZone::forHour(-7));

// 2099-12-31 23:59:59-16:00
dt = DateTime(99, 12, 31, 23, 59, 59, TimeZone::forHour(-16));
```

Once a `DateTime` object is created you can access the individual date/time
components using the accessor methods:
```C++
dt = DateTime(18, 8, 30, 6, 45, 1, TimeZone::forHour(-7));

uint8_t year = dt.year(); // 0 - 99 (actually 0 - 136)
uint8_t month = dt.month(); // 1 - 12
uint8_t day = dt.day(); // 1 - 31
uint8_t hour = dt.hour(); // 0 - 23
uint8_t minute = dt.minute(); // 0 - 59
uint8_t second = dt.second(); // 0 - 59
uint8_t dayOfWeek = dt.dayOfWeek(); // 1=Sunday, 7=Saturday
TimeZone tz = dt.timeZone();
```

The `dayOfWeek()` method calculates the correct day of the week from the (year,
month, day) information. The method does not use the time zone information
because the day of the week is unaffected by it.

#### Date Strings

The `dayOfweek()` method returns a numerical code where `1=Sunday` and
`7=Saturday`. The code can be translated into their English names using the
`common::DateStrings` class:

```C++
#include <AceTime.h>
using namespace ace_time;
...

common::DateStrings dateStrings;
uint8_t dayOfWeek = dt.dayOfWeek();
const char* longName = dateStrings.weekDayLongString(dayOfWeek);
const char* shortName = dateStrings.weekDayShortString(dayOfWeek);
```
The `DateStrings` class is inside the `ace_time::common` namespace, so you need
to prefix it with `common::`. The `weekDayShortString()` method returns the
first 3 characters of the week day (i.e. "Sun", "Mon", "Tue", "Wed", "Thu",
"Fri", "Sat").

Similarly the `month()` method returns a code where `1=January` and
`12=December`, which can be translated into English strings using:

```C++
common::DateStrings dateStrings;
uint8_t month = dt.dayOfWeek();
const char* longName = dateStrings.monthLongString(month);
const char* shortName = dateStrings.monthShortString(month);
```
The `monthShortString()` method returns the first 3 characters of the month
(i.e. "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct",
"Nov", "Dec").

**Caution**: The `DateStrings` object uses an internal buffer to hold the
generated human-readable strings. The strings must be used before the
`DateStrings` object is destructed (e.g. goes out of scope if it is created on
the local stack).

#### Days and Seconds From Epoch

The `DateTime` object can calculate the number of seconds since the AceTime
Epoch which is **2000-01-01T00:00:00Z** at the UTC time zone. For example:
```C++
DateTime dt(18, 1, 1, 0, 0, 0, TimeZone(1)); // 2018-01-01 00:00:00+00:15
uint32_t daysSinceEpoch = dt.toDaysSinceEpoch();
uint32_t secondsSinceEpoch = dt.toSecondsSinceEpoch();

Serial.println(daysSinceEpoch);
Serial.println(secondsSinceEpoch);
```

This prints `6574` and `568079100` respectively. The `toDaysSinceEpoch()` method
counts the number of whole days since the Epoch, including leap days.

You can go the other way and create a `DateTime` object from the seconds from
Epoch:
```C++
DateTime dt(568079100, TimeZone(1));
```
This will produce the same object as
`DateTime(18, 1, 1, 0, 0, 0, TimeZone(1))`.

#### Invalid DateTime Objects

A value of `0` for the `secondsSinceEpoch` is used internally as an ERROR
marker, so users should never create a`DateTime` object with this value. In
other words,
```C++
DateTime dt(0);
```
creates a `dt` object which returns `true` for `dt.isError()`.

More generally, users should avoid creating any `DateTime` object corresponding
to the first day of the year 2000 (2000-01-01). On that day, there is a danger
that the object may exceed the range of its internal variables. There may exists
a time zone on the first day of year 2000 when the object's
`toSecondsSinceEpoch()` is `<= 0`. Such a `DateTime` cannot be properly
represented using a `DateTime` object and will produce invalid `DateTime`
components (year, month, day, hour, minute, seconds). Stated another way, users
should create `DateTime` objects where the `toSecondsSinceEpoch() >= 86400`.
Smaller values *may* work for some time zones, but not for others.

#### Conversion to Other Time Zones

You can convert a given `DateTime` object into a representation in a different
time zone using the `DateTime::convertToTimeZone()` method:
```C++
// Central European Time
// 2018-01-01T09:20:00+01:00
DateTime dt(18, 1, 1, 9, 20, 0, TimeZone::forHour(1));

// Convert to Pacific Daylight Time.
// 2018-01-01T01:20:00-07:00
DateTime pacificTime = dt.convertToTimeZone(TimeZone::forHour(-7));
```
The two `DateTime` objects return the same value for `secondsSinceEpoch()`
because that is not affected by the time zone. However, the various date time
components (year, month, day, hour, minute, seconds) will be different.

#### TimePeriod

The `TimePeriod` class can be used to represents a difference between two
`DateTime` objects, if the difference is not too large. Internally, it is
implemented as 3 unsigned `uint8_t` integers representing the hour, minute and
seconds. There is a 4th signed `int8_t` integer that holds the sign (-1 or +1)
of the time period. The largest (or smallest) time period that can be
represented by this class is +/- 255h59m59s, corresponding to +/- 921599
seconds.

This class is intended to be used when the difference between 2 dates need
to be presented to the user broken down into hours, minutes and seconds. For
example, we can print out a count down to a target `DateTime` from the current
`DateTime` like this:
```C++
DateTime currentDate(...);
DateTime targetDate(...);
int32_t diffSeconds =
        targetDate.toSecondsSinceEpoch() - currentDate.toSecondsSincEpoch();
TimePeriod diff(diffSeconds);
diff.printTo(Serial)
```

### TimeProviders and TimeKeepers

The `TimeProvider` class and its subclasses implement the `getNow()` method
which returns a `uint32_t` that represents the number of seconds since the
AceTime Epoch (2000-01-01T00:00:00Z). The relevant part of the `TimeProvider`
class is:

```C++
class TimeProvider {
  public:
    ...
    virtual uint32_t getNow() const = 0;
    ...
};
```
To obtain the human-readable version of the current time, create a
`DateTime` object from the seconds from Epoch returned by `getNow()`:
```C++
TimeProvider* timeProvider = ...;

uint32_t nowSeconds = timeProvider->getNow();
DateTime now(nowSeconds);
```

The `TimeKeeper` class and its subclasses are also subclasses of`TimeProvider`
but the time keepers implement the `setNow()` method which allows their time to
be set.
```C++
class TimeKeeper: public TimeProvider {
  public:
    virtual void setNow(uint32_t secondsSinceEpoch) = 0;
};
```

In other words, the `TimeKeeper` can be set to the current time. It is then
expected to have an internal clock that continues to update the current time.

The AceTime library comes with a number of time providers and keepers, as
described in the following subsections.

#### DS3231 Time Keeper

The `DS3231TimeKeeper` is backed by a DS3231 RTC chip which is normally backed
by a battery or a supercapacitor to survive power failures. The DS3231 chip does
not contain the concept of a time zone. Therefore, I recommend that the
`DS3231TimeKeeper` class is used to store only the UTC date/time components,
instead of the local time. When the time is read back in using the
`DS3231TimeKeeper::getNow()`, you can convert that to the appropriate time zone
using the `DateTime::convertToTimeZone()` method.

```C++
DS3231TimeKeeper dsTimeKeeper;
...
void setup() {
  ...
  dsTimeKeeper.setup();
  ...

}
void loop() {
  ...
}
```

#### NTP Time Provider

The `NtpTimeProvider` is available on the ESP8266 and ESP32 have builtin WiFi
capability. This class uses an NTP client to fetch the current time from the
specified NTP server. The constructor takes 5 parameters, 2 of them
related to the WiFi authentication, and 3 of them related to timing parameters.
Only the SSID and PASSWORD parameters are required:
```C++
NtpTimeProvider ntpTimeProvider(SSID, PASSWORD);
...
void setup() {
  ...
  ntpTimeProvider.setup();
  ...
}

void loop() {
  ...
}
```
**Security Warning**: You should avoid committing your SSID and PASSWORD into a
public repository like GitHub because it will become public to anyone. Even if
you delete the commit, it will be accessible through the git history.

#### System Time Keeper

The `SystemTimeKeeper` is a special `TimeKeeper` that uses the Arduino built-in
`millis()` method as the source of its time. The biggest advantage of
`SystemTimeKeeper` is that its `getNow()` has very little overhead _(TBD: insert
benchmark)_ so it can be called as frequently as needed. The `getNow()` method
of other `TimeProviders` can consume a significant amount of time. For example,
the `DS3231TimeKeeper` must talk to the DS3231 RTC chip over an I2C bus. Even
worse, the `NtpTimeProvider` must the talk to the NTP server over the network
which can be unpredictably slow.

Unfortunately, the `millis()` internal clock of most (all?) Arduino boards is
not accurate. Therefore, the `SystemTimeKeeper` provides a mechanism to
synchronize its clock to an external (and presumably more accurate clock)
`TimeProvider`.

The `SystemTimeKeeper` also provides a way to save the current time to a
`backupTimeKeeper` (e.g. the `DS3231TimeKeeper` using an RTC chip with battery
backup). When the `SystemTimeKeeper` starts up, it will read the backup
`TimeKeeper` and set the current time. Then it can synchronize with an external
clock source (e.g. the `NtpTimeProvider`). The time is saved to the backup time
keeper whenever the `SystemTimeKeeper` is synced with the external time
provider.

Here is how to set up the `SystemTimeKeeper`:
```C++
DS3231TimeKeeper dsTimeKeeper;
NtpTimeProvider ntpTimeProvider(SSID, PASSWORD);
SystemTimeKeeper systemTimeKeeper(
  &ntpTimeProvider /*sync*/, &dsTimeKeeper /*backup*/);
...

void setup() {
  dsTimeKeeper.setup();
  ntpTimeProvider.setup();
  systemTimeKeeper.setup();
  ...
}

void loop() {
}
```

If you wanted to use the `DS3231TimeKeeper` as *both* the backup and sync
time sources, then the setup would something like this:
```C++
DS3231TimeKeeper dsTimeKeeper;
SystemTimeKeeper systemTimeKeeper(
    &dsTimeKeeper /*sync*/, &dsTimeKeeper /*backup*/);
...

void setup() {
  dsTimeKeeper.setup();
  systemTimeKeeper.setup();
  ...
}
```

You could also choose not to have either the backup or sync time sources, in
which case you can give `nullptr` as the correspond argument. For example,
to use no backup time keeper:
```C++
DS3231TimeKeeper dsTimeKeeper;
SystemTimeKeeper systemTimeKeeper(&dsTimeKeeper /*sync*/, nullptr /*backup*/);
...

void setup() {
  dsTimeKeeper.setup();
  systemTimeKeeper.setup();
  ...
}
```

#### System Clock Syncing and Heartbeat

For technical reasons &mdash; see the implementation of
`SystemTimeKeeper::getNow()`) &mdash; the `getNow()` method must be called
periodically to avoid an integer overflow. The maximum period between 2
consecutive calls to `getNow()` is 65.535 seconds. We need to implement a
"heartbeat" mechanism which simply calls the `getNow()` of the system time
keeper.

Secondly, since the internal `millis()` clock is not very accurate, we must
synchronize the system time keeper periodically. The frequency depends on the
accurate of the `millis()` and the cost of the call to the `getNow()` method of
the syncing time provider. For a DS3231 time source, syncing once every 1-10
minutes might work since talking to the RTC chip is cheap. For syncing with the
`NtpTimeProvider` with an accurate `millis()` maybe once every 1-12 hours might
be advisable.

The `SystemTimeKeeper` provides 2 ways to perform these periodic maintenance
actions.

**Method 1: Using the Global Loop()**

You can use the `SystemTimeLoop` class and insert that into the global `loop()`
method.
```C++
SystemTimeKeeper systemTimeKeeper(...);
SystemTimeLoop systemTimeLoop(
    systemTimeKeeper, syncPeriodSeconds, heartbeatPeriodMillis);

void loop() {
  systemTimeLoop.loop();
}
```

**Method 2: Using AceRoutine Coroutines**

You can use 2 AceRoutine coroutines to perform the sync and heartbeat. First,
`#include <AceRoutine.h>` before the `<AceTime.h>` (to activate the
`SystemTimeSyncCoroutine` and `SystemTimeHeartbeatCoroutine` classes). Then
create the 2 coroutines, and configure it to run using the `CoroutineScheduler`:

```C++
#include <AceRoutine.h>
#include <AceTime.h>
...
SystemTimeKeeper systemTimeKeeper(...);
SystemTimeSyncCoroutine systemTimeSync(systemTimeKeeper, syncPeriodSeconds,
    initialSyncPeriodSeconds, requestTimeoutMillis);
SystemTimeHeartbeatCoroutine systemTimeHeartbeat(systemTimeKeeper,
    heartbeatPeriodMillis);

void setup() {
  ...
  systemTimeSync.setupCoroutine(F("systemTimeSync"));
  systemTimeHeartbeat.setupCoroutine(F("systemTimeHeartbeat"));
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
provide syncing, the `SystemTimeLoop` object calls its `getNow()` method, which
blocks the execution of the program until the NTP server returns a response (or
the request times out after 1000 milliseconds). If you use the coroutines, the
program continues to do other things (e.g. update displays, scan for buttons)
during the time that the `NtpTimeProvider` has issued a request and is waiting
for a response from the NTP server.

## System Requirements

This library was developed and tested using:
* [Arduino IDE 1.8.6](https://www.arduino.cc/en/Main/Software)
* [ESP8266 Arduino Core 2.4.1](https://arduino-esp8266.readthedocs.io/en/2.4.1/)
* [arduino-esp32](https://github.com/espressif/arduino-esp32)

I used Ubuntu 17.10 for most of my development.

The library is tested on the following hardware before each release:

* Arduino Nano clone (16 MHz ATmega328P)
* Arduino Pro Micro clone (16 MHz ATmega32U4)
* WeMos D1 Mini clone (ESP-12E module, 80 MHz ESP8266)
* ESP32 dev board (ESP-WROOM-32 module, 240 MHz dual core Tensilica LX6)

I will occasionally test on the following hardware as a sanity check:

* Teensy 3.2 (72 MHz ARM Cortex-M4)

## Benchmarks

### CPU

The [AutoBenchmark.ino](examples/AutoBenchmark/) program measures the
amount of CPU cycles taken by some of the more expensive `DateTime`
methods. The most expensive method is the constructor `DateTime(seconds)`.
Here is a summary of how long that constructor takes for some Arduino boards
that I have access to:
```
----------------------------+---------+
Board or CPU                |  micros |
----------------------------+---------+
ATmega328P 16MHz (Nano)     | 401.800 |
ESP8266 80MHz               |  12.080 |
ESP32 240MHz                |   0.705 |
Teensy 3.2 96MHz            |   2.750 |
----------------------------+---------+
```

### Memory

Here is a quick summary of the amount of static RAM consumed by various
classes:

**8-bit processors**

```
sizeof(DateTime): 8
sizeof(TimeZone): 1
sizeof(TimePeriod): 4
sizeof(SystemTimeKeeper): 17
sizeof(DS3231TimeKeeper): 4
sizeof(SystemTimeLoop): 10
sizeof(SystemTimeSyncCoroutine): 29
sizeof(SystemTimeHeartbeatCoroutine): 18
```

**32-bit processors**
```
sizeof(DateTime): 8
sizeof(TimeZone): 1
sizeof(TimePeriod): 4
sizeof(SystemTimeKeeper): 24
sizeof(DS3231TimeKeeper): 8
sizeof(NtpTimeProvider): 96 (ESP8266), 120 (ESP32)
sizeof(SystemTimeLoop): 12
sizeof(SystemTimeSyncCoroutine): 52
sizeof(SystemTimeHeartbeatCoroutine): 36
```

### Comparison to Arduino Time Library

The [ComparisonBenchmark.ino](examples/ComparisonBenchmark/) program compares
the CPU time of AceTime methods with the equilvalent methods
[Arduino Time Library](https://github.com/PaulStoffregen/Time).
The functionality tested was the roundtrip conversion from `secondsFromEpoch` to
the date time components, then back to `secondsFromEpoch` again. Details are
given in the README.md file in that folder, but here is a summary for various
boards (all times in microseconds):
```
----------------------------+---------+----------+
Board or CPU                | AceTime | Time Lib |
----------------------------+---------+----------+
ATmega328P 16MHz (Nano)     | 364.000 |  879.000 |
ESP8266 80MHz               |  20.900 |   68.500 |
ESP32 240MHz                |   0.570 |    9.330 |
Teensy 3.2 96MHz            |   1.980 |   22.570 |
----------------------------+---------+---------+
```

## Bugs and Limitations

* The `NtpTimeProvider` on an ESP8266 calls `WiFi.hostByName()` to resolve
  the IP address of the NTP server. Unfortunately, this seems to be blocking
  call. When the DNS resolver is working properly, this call returns in ~10ms or
  less. But sometimes, the DNS resolver seems to get into a state where it takes
  4-5 **seconds** to time out. Even if you use coroutines, the entire program
  will block for those 4-5 seconds.
* AceTime uses an epoch of 2000-01-01T00:00:00Z. `DateTime::getNow()` returns
  the number of seconds since the epoch as a 32-bit unsigned integer. So it will
  rollover just after 2136-02-07T06:23:15Z.
* It is possible to construct a `DateTime` object with a `year` component
  greater than 136, but such an object may not be very useful because the
  `toSecondsSincEpoch()` method would return an incorrect number.
* [NTP](https://en.wikipedia.org/wiki/Network_Time_Protocol) uses an epoch
  of 1900-01-01T00:00:00Z, with 32-bit unsigned integer as the seconds counter.
  It will overflow just after 2036-02-07T06:28:15Z.
* [Unix time](https://en.wikipedia.org/wiki/Unix_time) uses an epoch of
  1970-01-01T00:00:00Z. On 32-bit Unix systems that use a signed 32-bit integer
  to represent the seconds field, the unix time will rollover just after
  2038-01-19T03:14:07Z. The AceTime `DateTime::toUnixSeconds()` method returns
  an *unsigned* 32-bit integer, so it will rollover about 70 later, just after
  2106-02-07T06:23:15Z.

## Changelog

See [CHANGELOG.md](CHANGELOG.md).

## License

[MIT License](https://opensource.org/licenses/MIT)

## Feedback and Support

If you have any questions, comments, bug reports, or feature requests, please
file a GitHub ticket or send me an email. I'd love to hear about how this
software and its documentation can be improved. Instead of forking the
repository to modify or add a feature for your own projects, let me have a
chance to incorporate the change into the main repository so that your external
dependencies are simpler and so that others can benefit. I can't promise that I
will incorporate everything, but I will give your ideas serious consideration.

## Authors

Created by Brian T. Park (brian@xparks.net).
