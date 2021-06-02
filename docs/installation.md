# AceTime Library Installation

The latest stable release is available in the Arduino IDE Library Manager. Two
libraries need to be installed since v1.2:

* Search for "AceTime". Click Install.
* Search for "AceCommon". Click Install.

The development version can be installed by cloning the 2 git repos:

* AceTime (https://github.com/bxparks/AceTime)
* AceCommon (https://github.com/bxparks/AceCommon)

You can copy over the contents to the `./libraries` directory used by the
Arduino IDE. (The result is a directory named `./libraries/AceTime` and
`./libraries/AceCommon`). Or you can create symlinks from `./libraries` to these
directories. Or you can `git clone` directly into the `./libraries` directory.

The `develop` branch contains the latest development.
The `master` branch contains the stable releases.

**Version**: 1.7.2 (2021-06-02, TZ DB version 2021a)

<a name="SourceCode"></a>
## Source Code

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
   [EpoxyDuino](https://github.com/bxparks/EpoxyDuino)
* `examples/` - example programs
* `tools/` - parser for the TZ Database files, code generators for `zonedb::`
  and `zonedbx::` zone files, and code generators for various unit tests

<a name="Dependencies"></a>
## Dependencies

The AceTime library depends on:

* AceCommon (https://github.com/bxparks/AceCommon)
* AceRoutine (https://github.com/bxparks/AceRoutine)
    * Optional. Needed if you use the `SystemClockCoroutine` class for automatic
      syncing. This is recommended but not strictly necessary.

Various programs in the `examples/` directory have one or more of the following
external dependencies. The comment section near the top of the `*.ino` file will
usually have more precise dependency information:

* AceRoutine (https://github.com/bxparks/AceRoutine)
* Arduino Time Lib (https://github.com/PaulStoffregen/Time)
* Arduino Timezone (https://github.com/JChristensen/Timezone)

Various scripts in the `tools/` directory depend on:

* IANA TZ Database (https://github.com/eggert/tz)
* Python pytz library (https://pypi.org/project/pytz/)
* Python dateutil library (https://pypi.org/project/python-dateutil)
* Hinnant date library (https://github.com/HowardHinnant/date)
* Python 3.6 or greater
* Java OpenJDK 11

If you want to run the unit tests or some of the command line examples using a
Linux or MacOS machine, you need:

* AUnit (https://github.com/bxparks/AUnit)
* EpoxyDuino (https://github.com/bxparks/EpoxyDuino)

<a name="Examples"></a>
## Examples

The following programs are provided in the `examples/` directory:

* [HelloDateTime](examples/HelloDateTime/)
    * demo program of various date and time classes
* [HelloSystemClock](examples/HelloSystemClock/)
    * demo program of `SystemClock`
* [HelloSystemClockCoroutine](examples/HelloSystemClockCoroutine/)
    * same as `HelloSystemClock` but using AceRoutine coroutines
* [HelloNtpClock](examples/HelloNtpClock/)
    * demo program of `NtpClock`
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

Various fully-featured hardware clocks can be found in the
https://github.com/bxparks/clocks repo (previously hosted under `examples/`).

* [CommandLineClock](https://github.com/bxparks/clocks/tree/master/CommandLineClock)
    * a clock with using the serial port for receiving commands and printing
      results
    * various system clock options: `millis()`, DS3231, or NTP client
    * useful for debugging or porting AceTime to a new hardware platform
* [OneZoneClock](https://github.com/bxparks/clocks/tree/master/OneZoneClock)
    * a digital clock showing one timezone selected from a menu of timezones
    * typical hardware includes:
        * DS3231 RTC chip
        * 2 buttons
        * an SSD1306 OLED display or PCD8544 LCD display
* [MultiZoneClock](https://github.com/bxparks/clocks/tree/master/MultiZoneClock)
    * similar to OneZoneClock but showing multiple timezones on the display,
      selected from a menu of timezones.
* [WorldClock](https://github.com/bxparks/clocks/tree/master/WorldClock)
    * a clock with 3 OLED screens showing the time at 3 different time zones
