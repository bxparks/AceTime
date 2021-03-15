# Comparisons to Other Time Libraries

## Table of Content

* [Arduino Time Library](#ArduinoTimeLibrary)
* [C Time Library](#CLibrary)
* [ezTime](#EzTime)
* [Micro Time Zone](#MicroTimeZone)
* [Java Time, Joda-Time, Noda Time](#JavaTime)
* [Howard Hinnant Date Library](#HinnantDate)
* [Google cctz](#Cctz)

<a name="ArduinoTimeLibrary"></a>
## Arduino Time Library

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

<a name="CLibrary"></a>
## C Time Library (time.h)

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
## ezTime

The [ezTime](https://github.com/ropg/ezTime) is a library that seems to be
composed of 2 parts: A client library that runs on the microcontroller and a
server part that provides a translation from the timezone name to the POSIX DST
transition string. Unfortunately, this means that the controller must have
network access for this library to work. I wanted to create a library that was
self-contained and could run on an Arduino Nano with just an RTC chip without a
network shield.

<a name="MicroTimeZone"></a>
## Micro Time Zone

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
## Java Time, Joda-Time, Noda Time

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
## Howard Hinnant Date Library

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
[BasicHinnantDateTest](tests/validation/BasicHinnantDateTest/)
and
[ExtendedHinnantDateTest](tests/validation/ExtendedHinnantDateTest/)
validation tests which compare the AceTime algorithms to the Hinnant Date
algorithms. For all times zones between the years 2000 until 2050, the AceTime
UTC offsets (`TimeZone::getUtcOffset()`), timezone abbreviations
(`TimeZone::getAbbrev()`), and epochSecond conversion to date components
(`ZonedDateTime::fromEpochSeconds()`) match the results from the Hinannt Date
libraries.

<a name="Cctz"></a>
## Google cctz

The [cctz](https://github.com/google/cctz) library from Google is also based on
the `<chrono>` library. I have not looked at this library closely because I
assumed that it would *not* fit inside an Arduino controller. Hopefully I will
get some time to take a closer look in the future.
