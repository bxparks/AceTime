# AceTime Clock User Guide

The classes in this section of the AceTime library are in the following
namespaces:

* `ace_time::clock`
* `ace_time::hw`

When I first created the AceTime library, there were only few classes in the
Clock module of the library. There are several more now, and it would make
sense for these classes to be in a separate library because the other parts of
AceTime library are not dependent of the Clock classes. But for now, these
classes live within the AceTime library.

**Version**: 1.6+ (2021-03-14, TZ DB version 2021a)

**Related documents**:

* [README.md](README.md): introductory background.
* [INSTALLATION.md](INSTALLATION.md): how to install the library
* [USER_GUIDE.md](USER_GUIDE.md): the Date, Time and TimeZone classes
* [CLOCK_GUIDE.md](CLOCK_GUIDE.md): the clock classes
* [VALIDATION.md](VALIDATION.md): testing and validating the library
* [Doxygen docs](https://bxparks.github.io/AceTime/html) hosted on GitHub Pages

## Table of Contents

* [Overview](#Overview)
* [Headers and Namespaces](#Headers)
* [Clock Class](#ClockClass)
* [NTP Clock](#NtpClock)
* [DS3231 Clock](#DS3231Clock)
* [STM RTC Clock](#StmRtcClock)
* [STM32F1 Clock](#Stm32F1Clock)
* [System Clock](#SystemClock)
    * [System Clock Maintenance Tasks](#SystemClockMaintenance)
    * [Reference And Backup Clocks](#ReferenceAndBackupClocks)
    * [System Clock Loop](#SystemClockLoop)
    * [System Clock Coroutine](#SystemClockCoroutine)
* [System Clock Examples](#SystemClockExamples)
    * [No Reference And No Backup](#NoReferenceAndNoBackup)
    * [DS3231 Reference](#DS3231Reference)
    * [NTP Reference With DS3231 Backup](#NtpReferenceWithDS3231Backup)
    * [DS3231 Reference and Backup](#DS3231ReferenceAndBackup)

<a name="Overview"></a>
## Overview

The main purpose of the `Clock` classes in this module is to provide a 32-bit
signed integer (`acetime_t` typedefed to `int32_t`) that represents the number
of seconds since a fixed point in the past called the "Epoch". The `SystemClock`
ensures that this integer increments by one every second.

The `acetime::clock` namespace contains classes needed to implement various
types of clocks using different sources:

* the `DS3231Clock` uses the DS3231 RTC chip,
* the `NtpClock` uses an NTP server,
* the `StmRtcClock` should be used for all other STM32 chips,
* the `Stm32F1Clock` uses the RTC built-into the STM32F1 chip,
* the `UnixClock` uses the `time()` method provided by the
  C library, used mostly for testing and debugging.

The `SystemClock` is a special subclass of the `Clock` class. It has a number of
features:

* It is powered by the internal `millis()` function that is provided by all
  Arduino platforms. The `millis()` function can be accessed extremely cheaply
  and quickly, and it auto-increments. However, it may not be highly accurate
  over long time periods, and it definitely does not preserve its information
  across power resets.
* It can be given an optional `referenceClock` (an instance of the `Clock`
  class) which provides an external clock of high accuracy. The `referenceClock`
  is assumed to be expensive to use (e.g. the `NtpClock` requires a network
  request and can take multiple-seconds.) Therefore, the `SystemClock` will
  synchronize against the `referenceClock` on a periodic basis that is
  configurable (default, every 1 hour).
* It can be also be given an optional `backupClock` (an instance of the `Clock`
  class) which preserves the time and *continues to tick* after the power is
  lost. For example, the `DS3231Clock` can be backed by a battery or a super
  capacitor. Similarly, the `Stm32F1Clock` wil remember the date and time when a
  battery is connected to the `VBat` pin of the processor. When the
  `SystemClock` is re-initialized, it can retrieve the current time from the
  `backupClock` so that the current time available from the `SystemClock` right
  away, without having to wait to resychronize with a very slow reference clock
  (e.g. the `NtpClock`).

Those familiar with the `time.h` functions in the C-library may be wondering why
we can't just use the `time()` function. On the AVR platform, the value of the
`time()` function does not auto-increment. On the SAMD21 and Teensy platforms,
the `time.h` header file does not exist. These substantial differences among
different Arduino platforms made it worthwhile to create the `SystemClock` class
that could be used across all Arduino platforms.

The class hierarchy diagram for these various classes looks like this, where the
upward arrow means "is-subclass-of", the side-ways arrow means "depends on", and
the diamond-line means "is-aggregation-of":

```
     (0..2)
.---------- Clock
|            ^  ^
|            |   \
|            |    NtpClock     ----> WiFi, ESP8266WiFi
|            |    DS3231Clock  ----> hw::DS3231
|            |    StmRtcClock  ----> hw::StmRtc
|            |    Stm32F1Clock ----> hw::Stm32F1Rtc
|            |    UnixClock    ----> time()
|            |
`-----<>  SystemClock
           ^       ^
          /         \
SystemClockLoop      SystemClockCoroutine
```

The classes in the `ace_time::hw` namespace provide a very thin abstraction
layer between the specific `Clock` subclass and the underlying hardware or
network device.

The `SystemClockLoop` and `SystemClockCoroutine` subclasses provide 2 different
ways of keeping the `SystemClock` constantly synchronized with the Arduino
`millis()` function, the `referenceClock` and the `backupClock`. (It could be
argued that these classes would be better implemented using composition, as
adapter classes around the `SystemClock` object, instead of using inheritance. I
went back and forth and finally decided that inheritance made these classes
easier to use, but I agree that composition would lead to better "separation of
concerns".)

<a name="Headers"></a>
## Headers and Namespaces

Only a single header file `AceTime.h` is required to use this library.
To use the Clock classes without prepending the namespace prefixes, use one or
more of the following `using` directives:

```C++
#include <AceTime.h>
using namespace ace_time::clock;
using namespace ace_time::hw;
```

<a name="ClockClass"></a>
## Clock Class

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

<a name="NtpClock"></a>
## NTP Clock

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

<a name="DS3231Clock"></a>
## DS3231 Clock

The `DS3231Clock` class uses the DS3231 RTC chip. It contains an internal
temperature-compensated osciallator that counts time in 1 second steps. It is
often connected to a battery or a supercapacitor to survive power failures. The
DS3231 chip stores the time broken down by various date and time components
(i.e. year, month, day, hour, minute, seconds). It contains internal logic that
knows about the number of days in an month, and leap years. It supports dates
from 2000 to 2099. It does *not* contain the concept of a time zone. Therefore,
The `DS3231Clock` assumes that the date/time components stored on the chip is in
**UTC** time.

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

It has been claimed that the DS1307 and DS3232 RTC chips have exactly same
interface as DS3231 when accessing the time and date functionality. I don't have
these chips so I cannot confirm that. Contact @Naguissa
(https://github.com/Naguissa) for more info.

<a name="StmRtcClock"></a>
## STM32 RTC Clock

The `StmRtcClock` uses the `hw::StmRtc` class, which in turn uses the STM32RTC
library (https://github.com/stm32duino/STM32RTC) which provides access to the
internal RTC module inside various STM32 chips. On some dev boards, the `VBat`
pin is exposed and you can connect a battery or super capacitor to preserve the
date and time fields through a power reset.

```C++
class StmRtcClock: public Clock {
  public:
    explicit StmRtcClock() {}

    void setup() {}

    acetime_t getNow() const override;

    void setNow(acetime_t epochSeconds) override;

    bool isTimeSet() const;
};
```

This class is relatively new (added in v1.4) and may require more extensive
testing across various STM32 boards. It has one known problem on the STM32F1
chip, used by the popular "Blue Pill" boards. According to a few bug reports
(https://github.com/stm32duino/STM32RTC/issues/29,
https://github.com/stm32duino/STM32RTC/issues/32,
https://github.com/stm32duino/STM32RTC/pull/41), the date fields are lost upon a
power cycle on the STM32F1. This is because the RTC on the STM32F1 is different
than the RTC on other STM32 chips, and stores only a 32-bit integer counter. The
HAL (hardware abstraction layer) code in the STM32duino is able not able to
utilize this 32-bit counter to store the date fields, and instead stores them in
SRAM which is lost during a power-loss.

I created the `Stm32F1Clock` class to fix this problem below.

<a name="Stm32F1Clock"></a>
## STM32F1 Clock

The `Stm32F1Clock` is a class that is specifically designed to work on an
STM32F1 chip, and specifically using the `LSE_CLOCK` mode (low speed external).
The "Blue Pill" board already contains this external crystal chip attached to
pins PC14 and PC15. The `LSE_CLOCK` mode uses external crystal to drive the
internal 32-bit RTC counter, and has the nice property of being the only clock
that works while the chip is powered off, as long as a battery or super
capacitor is attached to the `VBat` terminal.

```C++
class Stm32F1Clock: public Clock {
  public:
    explicit Stm32F1Clock() {}

    void setup();

    acetime_t getNow() const override;

    void setNow(acetime_t epochSeconds) override;
};
```

Underneath the covers, the `Stm32F1Clock` delegates its functionality to the
`hw::Stm32F1Rtc` class. The `Stm32F1Rtc` class bypasses the (buggy) HAL code for
the STM32F1. and writes the AceTime epochSeconds directly into the 32-bit RTC
counter. More information can be found in the class docstring on Doxygen (TBD:
Add direct link after regenerating the docs.)

I also add a cautionary note that there should be nothing connected to the PC14
and PC15 pins of the Blue Pill board, not even the male header pins. The male
header pins changed the capacitance of the oscillator circuit enough to cause my
`LSE_CLOCK` to run 5-10% too slow. Removing the pins fixed the accuracy problem,
making the clock accurate to better than 1 second per 48 hours. See for example:

* https://github.com/rogerclarkmelbourne/Arduino_STM32/issues/572
* https://www.stm32duino.com/viewtopic.php?t=143

The `Stm32F1Clock` and `Stm32F1Rtc` classes are new for v1.7 and should be
considered experimental. They seem to work great for me on my Blue Pill if that
makes a difference.

<a name="UnixClock"></a>
## Unix Clock

The `UnixClock` is a version of `Clock` that retrieves the epochSeconds from the
`time()` function on a POSIX or Unix compatible environment with a `time.h`
header file. It is currently activated only for EpoxyDuino
(https://github.com/bxparks/EpoxyDuino) but it could possibly to useful in other
environments, I don't know. Currently, it's used only for testing purposes.

```C++
class UnixClock: public Clock {
  public:
    explicit UnixClock() {}

    void setup() {}

    acetime_t getNow() const override;
};
```

<a name="SystemClock"></a>
## System Clock

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
`referenceClock`.

The other problem with the `millis()` internal clock is that it does not survive
a power failure. The `SystemClock` provides a way to save the current time
to a `backupClock` (e.g. the `DS3231Clock` using the DS3231 chip with battery
backup). When the `SystemClock` starts up, it will read the `backupClock` and
set the current time. When it synchronizes with the `referenceClock`, (e.g. the
`NtpClock`), it saves a copy of it into the `backupClock`.

The `SystemClock` is an abstract class, and this library provides 2 concrete
implementations, `SystemClockLoop` and `SystemClockCoroutine`:

* The `SystemClockLoop` is designed to be called from the global `loop()`
  function.
* The `SystemClockCoroutine` is coroutine designed to be called through the
  AceRoutine (https://github.com/bxparks/AceRoutine) library.

They implement the periodic maintenance that is required on the `SystemClock`
(see the [SystemClockMaintenance](#SystemClockMaintenance) subsection below).
One of the maintenance task is to synchronize the system clock with an external
clock. But getting the time from the external clock is an expensive process
because, for example, it could go over the network to an NTP server. So the
`SystemClockCoroutine::runCoroutine()` and the `SystemClockLoop::loop()` methods
both use the non-block API of the `Clock` interface to retrieve the external
time, which allow other things to to continue to run on the microcontroller.

```C++
namespace ace_time {
namespace clock {

class SystemClock: public Clock {
  public:
    void setup();

    acetime_t getNow() const override;
    void setNow(acetime_t epochSeconds) override;

    bool isInit() const;
    acetime_t getLastSyncTime() const;

  protected:
    virtual unsigned long clockMillis() const { return ::millis(); }

    void keepAlive();

    void syncNow(acetime_t epochSeconds);

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
        ace_common::TimingStats* timingStats = nullptr);

    int runCoroutine() override;
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

<a name="ReferenceAndBackupClocks"></a>
### Reference and Backup Clocks

The constructor of both `SystemClockLoop` and `SystemClockCoroutine`
take 2 required but *nullable* parameters:

* the `referenceClock`
    * an instance of the `Clock` class which provides an external clock of high
      accuracy
    * assumed to be expensive to use (e.g. the `NtpClock` requires a network
      request and can take multiple seconds.)
    * the `SystemClock` will synchronize against the `referenceClock` on a
      periodic basis using the non-blocking API of the `Clock` interface
    * synchronized time is a configurable parameter in the constructor
      (default, every 1 hour).
* the `backupClock`
    * an instance of the `Clock` class which preserves the time and *continues
      to tick* after the power is lost
    * e.g. `DS3231Clock` backed by a battery or a super capacitor
    * upon initialized, the `SystemClock` can retrieve the current time
      from the `backupClock` so that the current time available in the
      `SystemClock` right away, without having to wait to resychronize with a
      slow `referenceClock` (e.g. the `NtpClock`).

Both parameters are required but each are nullable, so there are 4 combinations:

* `SystemClock{Loop,Coroutine}(nullptr, nullptr)`
    * no referenceClock or backupClock
    * only the `millis()` function is used
* `SystemClock{Loop,Coroutine}(referenceClock, nullptr)`
    * performs periodic syncing with referenceClock
    * if the referenceClock does not keep time after power loss, then the
      date/time much be reset after reinitialization
    * an example configuration would use the `NtpClock` or `DS3231Clock`
      as the referenceClock, and set the `backupClock` to `nullptr`
        * the `backupClock` is not needed because both the `NptClock` and
          `DS3231Clock` will preserve their time through a power loss
* `SystemClock{Loop,Coroutine}(nullptr, backupClock)`
    * `millis()` used as the reference
    * date and time retrieve from backupClock upon initial startup
    * I think the `backupClock` gets set only when the user manually
      calls `SystemClock::setNow()`, no further syncing happens to the
      backupClock.
    * It is difficult to see this configuration being useful in practice, so I
      don't recommend it.
* `SystemClock{Loop,Coroutine}(referenceClock, backupClock)`
    * the `referenceClock` and `backupClock` are assumed to be different
    * using both `referenceClock` and `backupClock` provides the most redundancy
      and rapid initialization
    * see example below where the `referenceClock` is an `NtpClock` which can
      take many seconds to initialize, and the `backupClock` is a `DS3231`
      which can initialize the `SystemClock` quickly when the board
      restarts
    * this configuration allows the clock to keep working if the network goes
      down

<a name="SystemClockMaintenance"></a>
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

<a name="SystemClockLoop"></a>
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

<a name="SystemClockCoroutine"></a>
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

<a name="SystemClockExamples"></a>
## SystemClock Examples

<a name="NoReferenceAndNoBackup"></a>
### No Reference And No Backup

This is the most basic example of a `SystemClockLoop` that uses no
`referenceClock` or a `backupClock`. The accuracy of this clock is limited by
the accuracy of the internal `millis()` function, and the clock has no backup
against power failure. Upon reboot, the `SystemClock::setNow()` must be called
to set the current time. The `SystemClock::loop()` must still be called to
perform a maintenance task of incrementing the AceTime epochSeconds returned by
`SystemClock::getNow()` using the progression of the Arduino `millis()`
function.

This configuration is not very practical, but it might be useful for quick
debugging.

```C++
#include <AceTime.h>
using namespace ace_time;
using namespace ace_time::clock;

SystemClock systemClock(nullptr /*reference*/, nullptr /*backup*/);
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

<a name="DS3231Reference"></a>
### DS3231 Reference

This `SystemClockLoop` uses a `DS3231Clock` as a `referenceClock`. No backup
clock is actually needed because the DS3231 RTC preserves its info as long as a
battery is connected to it. The `SystemClock::loop()` advances the internal
`epochSeconds` every second using the `millis()` function, and it synchronizes
the `epochSeconds` to the `DS3231` clock every one hour (by default).

```C++
#include <AceTime.h>
using namespace ace_time;
using namespace ace_time::clock;

DS3231Clock dsClock;
SystemClock systemClock(&dsClock /*reference*/, nullptr /*backup*/);
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

<a name="NtpReferenceWithDS3231Backup"></a>
### NTP Reference With DS3231 Backup

This is a more sophisticated example of a `SystemClockLoop` configured to use
the `NtpClock` as the `referenceClock` and the `DS3231Clock` as the
`backupClock`. Currently, the `NptClock` supports only the ESP8266 and the ESP32
microcontrollers, but it could be readily extended to support other controllers
which have network capabilities.

Every hour (by default), the `SystemClockLoop` makes a request to the `NtpClock`
to get the most accurate time. This is a network request that can potentially
take several seconds. Fortunately, `SystemClockLoop` uses the non-blocking API
of `NtpClock` when making this request, so everything else on the
microcontroller keeps running while the request is being fulfilled. When the
request to `NtpClock` is successful, the result is also written into the
`DS3231Clock` backup clock, just to keep it in sync as well.

(It just occurs to me that `Clock::setNow()` is a blocking call, so this code
assumes that updating the `backupClock` is a relatively quick operation. This
seems to me a reasonable assumption because a `backupClock` that takes a long
time to update does not seem like not a good candidate as a `backupClock`. But
let me know if my assumptions are incorrect.)

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
  while (!Serial); // wait for Leonardo/Micro

  dsClock.setup();
  ntpClock.setup();
  systemClock.setup();
}


void loop() {
  static acetime_t prevNow = systemClock.getNow();

  systemClock.loop();

  // Print the date/time every 10 seconds.
  acetime_t now = systemClock.getNow();
  if (now - prevNow >= 10) {
    auto odt = OffsetDateTime::forEpochSeconds(
        now, TimeOffset::forHours(-8)); // convert epochSeconds to UTC-08:00
    odt.printTo(Serial);
  }
}
```

**Note**: This is configuration does *not* provide fail-over. In other words, if
the `referenceClock` is unreachable, then the code does not automatically start
using the `backupClock` as the reference clock. The `backupClock` is used *only*
during initial startup to initialize the `SystemClock`. If the network continues
to be unreachable for a long time, then the `SystemClock` will be only as
accurate as the `millis()` function.

<a name="DS3231ReferenceAndBackup"></a>
### DS3231 Both Reference and Backup (don't do this)

It might be tempting to specify the `DS3231Clock` as *both* the reference and
backup clock sources like this (and an earlier version of this guide actually
had an example of this):

```C++
#include <AceTime.h>
using namespace ace_time;
using namespace ace_time::clock;

DS3231Clock dsClock;

// **Don't do this**
SystemClockLoop systemClock(&dsClock /*reference*/, &dsClock /*backup*/);
...
```

It turns out the code detects the situation where the `referenceClock` is the
same as the `backupClock`, and then ignores the `backupClock` completely in this
case because there is no benefit in this configuration, while actually harming
the accuracy of the time keeping. In practice, if something like the DS3231 with
a battery is used as the `referenceClock`, there is no need for a `backupClock`
because the DS3231 automatically retains its time info while the battery is
attached to it.
