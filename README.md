# AceTime

Date and time classes for Arduino with basic time zone support using offsets
from UTC. It also supports an enhanced "system clock" that can be synchronized
from an external time source, such as an
[NTP](https://en.wikipedia.org/wiki/Network_Time_Protocol) server
or a DS3231 RTC chip. This library is meant to be an alternative to the
venerable 
[Arduino Time Library](https://github.com/PaulStoffregen/Time).

Compared to the Arduino Time Library, the AceTime library is more
object-oriented and uses an epoch that starts on 2000-01-01T00:00:00Z instead of
the Unix epoch of 1970-01-01T00:00:00Z. By using an internal 2-digit year
starting with the 2000 and counting the number of seconds from the year 2000
epoch using an `uint32_t`, the AceTime library can handle all dates from the
year 2000 to the end of year 2099. This date range corresponds to the date range
of the DS3231 RTC chip.

There are roughly 2 categories of classes provided by the AceTime library:

* date and time primitives
    * `DateTime`, `TimeZone`, `TimePeriod`
    * Borrowing design elements from the 
      [Joda-Time](http://www.joda.org/joda-time/quickstart.html) Java library.
* system clock classes
    * `SystemTimeKeeper`, `NtpTimeProvider`, `DS3231TimeKeeper`
    * Implements the system clock syncing of the Arduino Time library.

Time zones are handled as offsets from UTC in 15-minute increments which
supports every time zone offset currently used today. This allows the time zone
to be stored as a single `uint8_t` integer. The AceTime library does **not**
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
* `src/ace_time/hw/` - internal classes for interfacing with hardware
* `src/ace_time/testing/` - internal testing files
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

```
#include <AceTime.h>
using namespace ace_time;
```

### Date Time Primitives

*TBD*

### TimeProviders and TimeKeepers

*TBD*

### NTP Time Provider

*TBD*

### DS3231 Time Keeper

*TBD*

### System Time Keeper

*TBD*
### System Clock Syncing and Heartbeat

*TBD*

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
