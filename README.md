# AceTime

Date and time classes for Arduino with basic time zone support using offsets
from UTC. It also supports an enhanced "system clock" that can be synchronized
from an external time source, such as an
[NTP](https://en.wikipedia.org/wiki/Network_Time_Protocol) server
or a DS3231 RTC chip. This library is meant to be an alternative to the
venerable [Arduino Time Library](https://github.com/PaulStoffregen/Time).

Compared to the Arduino Time Library, here are differences:
1. AceTime is more object-oriented.
1. AceTime uses an epoch that starts on 2000-01-01T00:00:00Z instead of the Unix
   epoch of 1970-01-01T00:00:00Z.
    * Using an `uint32_t` to track the number of seconds since the Epoch, the
      AceTime library can handle all dates from **2000 to the end of 2099**.
    * This date range corresponds to the range of the DS3231 RTC chip.
1. AceTime is **2-3X** faster on an ATmega328P, **4X** faster on the ESP8266,
   and **10-20X** faster on the ARM (Teensy) and ESP32 processors.

There are roughly 2 categories of classes provided by the AceTime library:

* date and time primitives
    * `DateTime`, `TimeZone`, `TimePeriod`
    * Borrowing design elements from the
      [Joda-Time](http://www.joda.org/joda-time/quickstart.html) Java library.
* system clock classes
    * `SystemTimeKeeper`, `NtpTimeProvider`, `DS3231TimeKeeper`
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

Here are the basic usage. More details can be found in the Doxygen docs.

#### TimeZone

The `TimeZone` class is a thin wrapper around a `int8_t` integer that represents
the number of 15-minute offsets from the UTC time zone. For convenience, a
constructor is provided that takes a human-readable UTC offset.

For example, the Pacific Standard Time is `UTC-08:00`, which is `-32` units away
from UTC. It can be created using one of the following 4 methods (constructor
and 3 factory methods) in order of increasing complexity (and CPU time):
```C++
TimeZone tz(-32);
TimeZone tz = TimeZone::forHour(-8);
TimeZone tz = TimeZone::forHourMinute(-1, 8, 0);
TimeZone tz = TimeZone::forOffsetString("-08:00");
```

Another example, the Central European Time Zone `UTC+01:00` can be constructed
using one of the following:
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
takes all of these parameters:
```C++
DateTime dt;

// 2001-01-01 00:00:00Z
dt = DateTime(1, 1, 1, 0, 0, 0, TimeZone(0));

// 2018-08-30T06:45:01-07:00
dt = DateTime(18, 8, 30, 6, 45, 1, TimeZone::forHour(-7));

// 2099-12-31 23:59:59-16:00
dt = DateTime(99, 12, 31, 23, 59, 59, TimeZone::forHour(-16));
```

Once a `DateTime` object is created you can access that individual components
using the accessor methods:
```C++
dt = DateTime(18, 8, 30, 6, 45, 1, TimeZone::forHour(-7));

uint8_t year = dt.year(); // 0 - 99
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

#### Days and Seconds From Epoch

The `DateTime` object can be created from and converted to the number of seconds
since its **Epoch**. For the AceTime library, that Epoch is at the beginning of
this century, **2000-01-01T00:00:00Z** at the UTC time zone. For example:
```C++
DateTime dt(18, 1, 1, 0, 0, 0, TimeZone(1)); // 2018-01-01 00:00:00+00:15
uint32_t daysSinceEpoch = dt.toDaysSinceEpoch();
uint32_t secondsSinceEpoch = dt.toSecondsSinceEpoch();

Serial.println(daysSinceEpoch);
Serial.println(secondsSinceEpoch);
```

This prints `6574` and `568079100` respectively. The `toDaysSinceEpoch()` counts
the number of whole days since the Epoch, including leap days.

You can create a `DateTime` objects from the seconds from Epoch:
```C++
DateTime dt(568079100, TimeZone(1));
```
This will produce the same object as
`DateTime(18, 1, 1, 0, 0, 0, TimeZone(1))`.

#### Invalid DateTime Objects

A value of `0` for the `secondsSinceEpoch` is used internally as an ERROR
marker. In other words,
```C++
DateTime dt(0);
```
creates a `dt` object which returns `true` for `dt.isError()`.

More generally, you should avoid creating any `DateTime` object corresponding to
the first day of the year 2000 (2000-01-01). That's because there exists a time
zone on the first day of year 2000 when the object's `toSecondsSinceEpoch()` is
`<= 0`. Such a `DateTime` cannot be properly represented using a `DateTime`
object and will produce invalid `DateTime` components (year, month, day, hour,
minute, seconds). Stated another way, the safest thing is to make sure that the
`toSecondsSinceEpoch()` of a `DateTime` object is always greater than or equal
to 86400. Smaller values may work for some time zones, but not for others.

#### Convert to Another Time Zone

You can convert a given `DateTime` object into a representation in a different
time zone using the `DateTime::convertToTimeZone()` method:
```C++
DateTime dt(18, 1, 1, 0, 0, 0, TimeZone(1)); // 2018-01-01 00:00:00+00:15

DateTime pacificTime = dt.convertToTimeZone(TimeZone(-28));
```
The two `DateTime` objects return the same value for `secondsSinceEpoch()`
because that is not affected by the time zone. However, the various date time
components (year, month, day, hour, minute, seconds) will be different in the
target time zone.

### TimeProviders and TimeKeepers

The `TimeProvider` forms a set of classes which can be a source of time.
Subclasses are expected to implement the `getNow()` method which returns the
number of seconds since the AceTime Epoch (2000-01-01). This value can be used
to construct the `DateTime` object when needed.

The `TimeKeeper` is a subclass of `TimeProvider` which provides the `setNow()`
method in addition to `getNow()` method. In other words, the `TimeKeeper` can be
set to the current time and it is normally expected to have an internal clock
which allows it to continue to update the current time.

#### NTP Time Provider

The `NtpTimeProvider` is available on the ESP8266 microcontroller. It uses
an NTP client to fetch the current time from the specified NTP server. The
constructor takes 5, with 2 of them being required:
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
The `SSID` and `PASSWORD` are the credentials needed for the ESP8266 to get on
your wireless network. (**Security Warning**: Avoid checking in your
SSID/PASSWORD into a public repository like GitHub because it will become public
to anyone.)

#### DS3231 Time Keeper

The `DS3231TimeKeeper` is a `TimeKeeper` which is backed by a DS3231 RTC chip.
It is recommended that the `DS3231TimeKeeper` is used to store only UTC date
time, instead of the local time. When the current time is read back in using the
`DS3231TimeKeeper::getNow()`, you can convert that to the appropriate time zone
using the `DateTime::convertToTimeZone()` method.

```C++
DS3231TimeKeeper dsTimeKeeper;
...
void setup() {
  ...
  dsTimeProvider.setup();
  ...

}
void loop() {
  ...
}
```

#### System Time Keeper

The `SystemTimeKeeper` is a special `TimeKeeper` that uses the the
Arduino built-in `millis()` method as the source of its time. The biggest
advantage of `SystemTimeKeeper` is that its `getNow()` has very little overhead
_(TBD: insert benchmark)_ so can be called as frequently as needed. Other
`TimeProviders` can take a significant amount of time to talk to the DS3231 RTC
chip, or even worse, talk to the NTP server over thet network.

Unfortunately, the `millis()` internal clock of the Arduino boards is not
accurate. Therefore, the `SystemTimeKeeper` provides a mechanism to synchronize
its clock to an external (and presumably more accurate clock) `TimeProvider`.

The `SystemTimeKeeper` also provides a way to save the current time to a
`backupTimeKeeper` (e.g. the `DS3231TimeKeeper` using an RTC chip with battery
backup). When the `SystemTimeKeeper` starts up, it will read the backup
`TimeKeeper` and set the current time. Then it can synchronize with an external
clock source (e.g. the `NtpTimeProvider`). It could actually use the
`DS3231TimeKeeper` is *both* the backup `TimeKeeper` and the syncing
`TimeProvider`.

Here is how to set up the `SystemTimeKeeper`:
```C++
DS3231TimeKeeper dsTimeKeeper;
NtpTimeProvider ntpTimeProvider(SSID, PASSWORD);
SystemTimeKeeper systemTimeKeeper(&ntpTimeProvider, &dsTimeKeeper);
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

If you wanted to use the `DS3231TimeKeeper` as both the backup and sync
time sources, then the setup would something like this:
```C++
DS3231TimeKeeper dsTimeKeeper;
SystemTimeKeeper systemTimeKeeper(&dsTimeProvider, &dsTimeKeeper);
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
SystemTimeKeeper systemTimeKeeper(&dsTimeProvider, nullptr /*backup*/);
...

void setup() {
  dsTimeKeeper.setup();
  systemTimeKeeper.setup();
  ...
}
```

#### System Clock Syncing and Heartbeat

For reasons which are slightly obscure --- see the implementation of
`SystemTimeKeeper::getNow()`) for details --- the `getNow()` method must be
called periodically to avoid an integer overflow. The maximum period between 2
consecutive calls to `getNow()` is 65.535 seconds. We call this the "heartbeat"
of the system time keeper.

Secondly, since the internal `millis()` clock is not very accurate, we must
synchronize the system time keeper periodically. The frequency would dependend
on the accurate of the `millis()` and how expensive it is to call the `getNow()`
method of the syncing time provider. For a DS3231 time source, syncing once
every 10 minutes might work since talking to the RTC chip is cheap. For syncing
with the `NtpTimeProvider` with an accurate `millis()` maybe once every 12 hours
might be good enough.

The `SystemTimeKeeper` provides 2 ways to perform these periodic maintenance
actions.

**Using the Global Loop()**

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

**Using AceRoutine Coroutines**

You can use 2 AceRoutine coroutines to perform the sync and heartbeat. First,
`#include <AceRoutine.h>` before the `<AceTime.h>`. Then create the 2
coroutines, and set it up to run using the `CoroutineScheduler`:

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
* NodeMCU 1.0 clone (ESP-12E module, 80 MHz ESP8266)

## Bugs and Limitations

* The `NtpTimeProvider` should be able to run on an ESP32 but I haven't
  had the time to implement it.
* The `NtpTimeProvider` on an ESP8266 calls `WiFi.hostByName()` to resolve
  the IP address of the NTP server. Unfortunately, this seems to be blocking
  call. When the DNS resolver is working properly, this call returns in ~10ms or
  less. But sometimes, the DNS resolver seems to get into a state where it takes
  4-5 **seconds** to time out. Even if you use coroutines, the entire program
  will block for those 4-5 seconds.

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
