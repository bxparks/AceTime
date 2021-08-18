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

**Version**: 1.7.2 (2021-06-02, TZ DB version 2021a)

## Table of Contents

* [Overview](#Overview)
* [Headers and Namespaces](#Headers)
* [Clock Class](#ClockClass)
* [NTP Clock](#NtpClock)
* [DS3231 Clock](#DS3231Clock)
* [STM RTC Clock](#StmRtcClock)
* [STM32F1 Clock](#Stm32F1Clock)
* [Unix Clock](#UnixClock)
* [System Clock](#SystemClock)
    * [System Clock Maintenance Tasks](#SystemClockMaintenance)
    * [Reference Clock And Backup Clock](#ReferenceClockAndBackupClock)
    * [System Clock Loop](#SystemClockLoop)
    * [System Clock Coroutine](#SystemClockCoroutine)
    * [System Clock Status Inspection](#SystemClockStatus)
* [System Clock Examples](#SystemClockExamples)
    * [No Reference And No Backup](#NoReferenceAndNoBackup)
    * [DS3231 Reference](#DS3231Reference)
    * [NTP Reference With DS3231 Backup](#NtpReferenceWithDS3231Backup)
    * [DS3231 Reference and Backup](#DS3231ReferenceAndBackup)
* [System Clock Configurable Parameters](#SystemClockConfigurableParameters)

**Related Documents**:

* [README.md](../README.md): introductory background.
* [docs/installation.md](../docs/installation.md): how to install the library
* [docs/date_time_timezone.md](../docs/date_time_timezone.md): the
  Date, Time and TimeZone classes
* [Doxygen docs](https://bxparks.github.io/AceTime/html) hosted on GitHub Pages

<a name="Overview"></a>
## Overview

The main purpose of the `Clock` classes in this module is to provide a 32-bit
signed integer (`acetime_t` typedef'ed to `int32_t`) that represents the number
of seconds since a fixed point in the past called the "Epoch". The AceTime Epoch
is defined to be 2000-01-01 00:00:00 UTC.

The `epochSeconds` can come from many different sources and different subclasses
of `Clock` provide access to these different sources:

* the `DS3231Clock` uses the DS3231 RTC chip over I2C,
* the `NtpClock` talks to an NTP server, sing the ESP8266 and ESP32 chips,
* the `StmRtcClock` reads the built-in RTC module on STM32 chips,
* the `Stm32F1Clock` targets the special RTC module on the STM32F1 chip,
* the `UnixClock` uses the `time()` method provided by the C library, used
  mostly for testing and debugging.

The `SystemClock` is a special subclass of the `Clock` class. Its job
is to provide an auto-incrementing epochSeconds that can be access very quickly
and cheaply. At a minimum, it should handle at least 10 requests per second, but
the current implementation should be able to handle 1000 to 1M requests per
second, depending on the processor. It has a number of features:

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
`time()` function does not auto-increment. It must be incremented from an
*external* process (e.g. in an Interrupt Service Routine, ISR) which must call
the `system_tick()` function exactly once a second. On the SAMD21 and Teensy
platforms, the `time.h` header file does not exist. These substantial
differences among different Arduino platforms made it worthwhile to create the
`SystemClock` class that could be used across all Arduino platforms.

The class hierarchy diagram for these various classes looks like the following.
The upward arrow means "is-subclass-of", the side-ways arrow means "depends on",
and the diamond-line means "is-aggregation-of":

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

The classes in the `ace_time::hw` namespace provide a thin abstraction or
translation layer between the specific `Clock` subclass and the underlying
hardware or network device.

The `SystemClockLoop` and `SystemClockCoroutine` subclasses provide 2 different
ways of keeping the `SystemClock` continually synchronized with the Arduino
`millis()` function, with the `referenceClock`, and with the `backupClock`. (It
could be argued that these classes would be better implemented using
composition, as adapter classes around the `SystemClock` object, instead of
using inheritance. I went back and forth on this, there were pros and cons
either way, and finally decided that inheritance made these classes easier to
use.)

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
zone using the `ZonedDateTime` and `TimeZone` classes described in the previous
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
temperature-compensated oscillator that counts time in 1 second steps. It is
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
`hw::Stm32F1Rtc` class. The `Stm32F1Rtc` class bypasses the HAL code for
the STM32F1 because the RTC HAL layer for the STM32F1 has a bug which causes the
date components to be lost after a power reset:

* https://github.com/stm32duino/STM32RTC/issues/29
* https://github.com/stm32duino/Arduino_Core_STM32/issues/266
* https://github.com/stm32duino/STM32RTC/issues/32

Instead, the `hw::Stm32F1Rtc` class writes the AceTime epochSeconds directly
into the 32-bit RTC counter on the STM32F1. More information can be found in the
class docstring on Doxygen (TBD: Add direct link after regenerating the docs.)

I also recommend that nothing should be connected to the PC14 and PC15 pins of
the Blue Pill board, not even the male header pins. The male header pins changed
the capacitance of the oscillator circuit enough to cause my `LSE_CLOCK` to run
5-10% too slow. Removing the pins fixed the problem, giving me a clock That is
accurate to better than 1 second per 48 hours. See for example:

* https://github.com/rogerclarkmelbourne/Arduino_STM32/issues/572
* https://www.stm32duino.com/viewtopic.php?t=143

The `Stm32F1Clock` and `Stm32F1Rtc` classes are new for v1.7 and should be
considered experimental. They seem to work great for me on my Blue Pill for what
it is worth.

<a name="UnixClock"></a>
## Unix Clock

The `UnixClock` is a version of `Clock` that retrieves the epochSeconds from the
`time()` function on a POSIX or Unix compatible environment with a `time.h`
header file. It is currently activated only for EpoxyDuino
(https://github.com/bxparks/EpoxyDuino) but it could possibly to useful in other
environments, I don't know. Currently, it's used only for testing.

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
    static const uint8_t kSyncStatusOk = 0;
    static const uint8_t kSyncStatusError = 1;
    static const uint8_t kSyncStatusTimedOut = 2;

    void setup();

    acetime_t getNow() const override;
    void setNow(acetime_t epochSeconds) override;

    bool isInit() const;
    acetime_t getLastSyncTime() const;
    uint8_t getSyncStatusCode() const;
    int32_t getSecondsSinceSyncAttempt() const;
    int32_t getSecondsToSyncAttempt() const;
    int16_t getClockSkew() const;

  protected:
    explicit SystemClock(
        Clock* referenceClock /* nullable */,
        Clock* backupClock /* nullable */);

    Clock* getReferenceClock() const { return mReferenceClock; }

    unsigned long clockMillis() const { return ::millis(); }

    void keepAlive();

    void backupNow(acetime_t nowSeconds);

    void syncNow(acetime_t epochSeconds);
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
        uint16_t requestTimeoutMillis = 1000);

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

<a name="ReferenceClockAndBackupClock"></a>
### Reference Clock and Backup Clock

The constructor of both `SystemClockLoop` and `SystemClockCoroutine`
take 2 parameters which are required but are *nullable*:

* the `referenceClock`
    * an instance of the `Clock` class which provides an external clock of high
      accuracy
    * assumed to be expensive to use (e.g. the `NtpClock` requires a network
      request and can take multiple seconds.)
    * the `SystemClock` will synchronize against the `referenceClock` on a
      periodic basis using the non-blocking API of the `Clock` interface
    * the synchronization interval is a configurable parameter in the
      constructor (default, every 1 hour).
* the `backupClock`
    * an instance of the `Clock` class which preserves the time and *continues
      to tick* after the power is lost
    * e.g. `DS3231Clock` backed by a battery or a super capacitor
    * upon initialized, the `SystemClock` retrieves the current time
      from the `backupClock` so that the current time available in the
      `SystemClock` right away, without having to wait for synchronization with
      the slower `referenceClock` (e.g. the `NtpClock`).

Since both parameters are nullable, there are 4 combinations:

* `SystemClock{Loop,Coroutine}(nullptr, nullptr)`
    * no referenceClock or backupClock
    * only the `millis()` function is used
* `SystemClock{Loop,Coroutine}(referenceClock, nullptr)`
    * performs periodic syncing with referenceClock
    * if the referenceClock does not keep time after power loss, then the
      date/time much be reset after reinitialization
* `SystemClock{Loop,Coroutine}(nullptr, backupClock)`
    * `millis()` used as the reference
    * date and time retrieve from backupClock upon initial startup
    * `backupClock` updated when `SystemClock::setNow()` is called
    * no further syncing happens to the backupClock
    * *Not Recommended*: It is difficult to see this configuration being useful.
* `SystemClock{Loop,Coroutine}(referenceClock, backupClock)`
    * the `referenceClock` and `backupClock` will usually be different objects
    * using both provides redundancy and rapid initialization
    * see example below where the `referenceClock` is an `NtpClock` which can
      take many seconds to initialize, and the `backupClock` is a `DS3231` which
      can initialize the `SystemClock` quickly when the board restarts
    * this configuration allows the clock to keep working if the network goes
      down
* `SystemClock{Loop,Coroutine}(referenceClock, referenceClock)`
    * if the `backupClock` is defined to be identical to the `referenceClock`
      then the code takes special precautions to avoid updating the
      `backupClock` when `syncNow()` is called
    * this avoids writing the same value back into the RTC, which would cause
      progressive loss of accuracy due to the overwriting of sub-second
      information
    * this configuration has the benefit of guaranteeing that the `SystemClock`
      returns a valid epochSeconds as soon as the `setup()` method returns
      successfully, because the `backupClock` is called in the `setup()` to
      initialize the `SystemClock`
    * see [DS3231 As Both Reference and Backup](#DS3231ReferenceAndBackup)
      example below

<a name="SystemClockMaintenance"></a>
### SystemClock Maintenance Tasks

There are 2 internal maintenance tasks that must be performed periodically.

First, the `SystemClock` advances the internal `epochSeconds` counter using the
`millis()` function when the `getNow()` method is called. This functionality
is needed because on the AVR platform, the `time()` function does not
automatically advance. On other platforms, the `time()` function does not even
exist. For cross-platform compatibility, We are forced to use the `millis()`
function as a substitute. The synchronization with `millis()` must happen every
65.536 seconds or faster. Most of the time, this will not be problem because the
`getNow()` method will be called very frequently, say 10 times a second, to
detect a transition of the time from one second to the next second. But there
may be applications where `getNow()` is not called frequently, so the
`SystemClock` maintenance task must make sure that `getNow()` is called
frequently enough even if the calling application does not do so.

Second, if the `referenceClock` is given, the `SystemClock` should synchronize
its internal `epochSeconds` with the reference clock periodically.
The frequency of this sync'ing will likely depend on the accuracy of the
`millis()`, and how expensive the call to the `referenceClock` is. If the
`referenceClock` is a DS3231 chip, syncing once every 1-10 minutes might be
acceptable since talking to the RTC chip over I2C is relatively cheap. If the
`referenceClock` is the `NtpClock`, the network connection is fairly expensive
so maybe once every 1-12 hours might be advisable. Since the `SystemClock` does
not actually know what kind of `referenceClock` is attached to it, it needs to
be given a some reasonable default value. The current default value is every
**one hour**. This value can be changed in the constructor of one of the
following subclasses.

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

I suspect that most people will feel more comfortable using the
`SystemClockLoop` class. But if you are already using the AceRoutine library, it
may be more convenient to use the `SystemClockCoroutine` class instead.

<a name="SystemClockStatus"></a>
### SystemClock Status Inspection

The `SystemClock` exposes a number of methods that allow inspection of its sync
status with the referenceClock.

```C++
class SystemClock: public Clock {
  public:
    static const uint8_t kSyncStatusOk = 0;
    static const uint8_t kSyncStatusError = 1;
    static const uint8_t kSyncStatusTimedOut = 2;

    ...

    acetime_t getLastSyncTime() const;
    uint8_t getSyncStatusCode() const;
    int32_t getSecondsSinceSyncAttempt() const;
    int32_t getSecondsToSyncAttempt() const;
    int16_t getClockSkew() const;
};
```

* `getLastSyncTime()`
    * the `acetime_t` time of the most recent **successful** syncing with the
      referenceClock
    * (there is no equilvalent `getLastFailedSyncTime()` method, because if the
      sync fails, we don't get the correct time, so we don't know what time it
      failed, but see `getSecondsSinceSyncAttempt()` below)
* `getSyncStatusCode()`
    * status code of the most recent attempt to sync with the referenceClock
* `getSecondsSinceSyncAttempt()`
    * number of seconds since the most recent sync attempt with the
      referenceClock, regardless whether it failed or succeeded
    * this a positive number, but it sometimes makes sense to display this
      number as a negative value, to indicate that it occurred in the past
    * elapsed time is extracted from the global `millis()` function
* `getSecondsToSyncAttempt()`
    * number of seconds estimated until the next sync attempt with the
      referenceClock
    * elapsed time is extracted from the global `millis()` function
* `getClockSkew()`
    * number of seconds that the `SystemClock` was slow (negative) or fast
      (postive) compared to the referenceClock, at the time of the most recent
      *successful* sync
    * if the syncing is done often enough, and the `millis()` function is
      reasonably calibrated, this value should be almost always `0`.

The quantities returned by the above methods can be fed into the `TimePeriod`
object to extract the hour, minute and second components.

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

**Note**: This configuration does *not* provide fail-over. In other words, if
the `referenceClock` is unreachable, then the code does not automatically start
using the `backupClock` as the reference clock. The `backupClock` is used *only*
during initial startup to initialize the `SystemClock`. If the network continues
to be unreachable for a long time, then the `SystemClock` will be only as
accurate as the `millis()` function.

<a name="DS3231ReferenceAndBackup"></a>
### DS3231 As Both Reference and Backup

The `DS3231Clock` for example can be given as *both* the reference and backup
clock sources, like this:

```C++
#include <AceTime.h>
using namespace ace_time;
using namespace ace_time::clock;

DS3231Clock dsClock;

SystemClockLoop systemClock(&dsClock /*reference*/, &dsClock /*backup*/);
```

The `SystemClock` will notice that the `referenceClock` is the same as the
`backupClock`, and will take precautions to avoid writing to the  `backupClock`
in the `syncNow()` method. Otherwise, there would be progressive skewing the
`referenceClock`. To see how this would happen, recall that the `referenceClock`
is the original source of the `epochSeconds` given to `syncNow()`. If the
`epochSeconds` is written back into the `referenceClock` (through the
`backupClock` reference), then the internal subsecond resolution of the RTC
would be lost, and the RTC would lose a small fraction of a second each time
`syncNow()` is called.

The biggest advantage of using this configuration (where the same clock is used
as `referenceClock` and `backupClock`) is guarantee a valid state of
`SystemClock` after a successful call to `setup()`. Without a `backupClock`, the
`SystemClock::getNow()` returns a `kInvalidSeconds` error condition until the
first successful `syncNow()` complete. With a `backupClock`, the
`SystemClock::setup()` blocks until a valid time is retrieved from the
`backupClock`, uses that value to initialize the `SystemClock`. The `getNow()`
method will always return a valid vlaue (as long as both reference and backup
clock remain valid).

In summary, if your application can tolerate a short period (order of seconds or
less) where the `SystemClock::getNow()` can return `kInvalidSeconds`, then you
can use just define the reference clock without reusing it as a backup clock,
`SystemClockLoop(&dsClock, nullptr)`. For robustness, most applications should
be written to tolerate and correctly handle this situation anyways, but I
understand that people want to do the least amount of work, and handling error
conditions is more work. Configuring the `referenceClock` as the `backupClock`
provides a slightly better well-behaved `SystemClock`, at the expense of having
possibility that the setup process of the application could take longer.

<a name="SystemClockConfigurableParameters"></a>
## System Clock Configurable Parameters

The constructors of both `SystemClockLoop` and `SystemClockCoroutine` take an
identical list of parameters, some of which have default values. This section
explains the meaning of those parameters. Both construtors look exactly like
this:

```C++
explicit SystemClockLoop(
    Clock* referenceClock /* nullable */,
    Clock* backupClock /* nullable */,
    uint16_t syncPeriodSeconds = 3600,
    uint16_t initialSyncPeriodSeconds = 5,
    uint16_t requestTimeoutMillis = 1000,
    ace_common::TimingStats* timingStats = nullptr);
```

* `syncPeriodSeconds`
    * Number of seconds between successive requests to the `referenceClock` to
      obtain the current time.
    * If requests to the `referenceClock` is cheap (e.g. `DS3231Clock` over
      I2C), then this value could be lowered (e.g. 1 minute) without much
      detriment.
    * If requests to the `referenceClock` is expensive (e.g. `NtpClock`) then
      this value should remain relatively large.
* `initialSyncPeriodSeconds`
    * Number of seconds to wait between the very first request to the
      `referenceClock` just after reboot, and the *second* request to the
      `referenceClock` if the first request fails.
    * If the request fails again, the `SystemClockLoop::loop()` and
      `SystemClockCoroutine::runCoroutine()` methods will use an [exponential
      backoff](https://en.wikipedia.org/wiki/Exponential_backoff) algorithm and
      double the number of seconds between each successive retry attempt.
    * The retry interval keeps increasing until it becomes greater than or equal
      to `syncPeriodSeconds`, after which the retry interval becomes pegged to
      `syncPeriodSeconds`.
* `requestTimeoutMillis`
    * Number of milliseconds to wait after making a request to the
      `referenceClock` before marking the request as timed out, and therefore,
      failed.
    * Note that since the non-blocking request API of `Clock` is used, other
      tasks continue to run while we wait for a response from the
      `referenceClock`.
* `timingStats`
    * An instance of `ace_common::TimingStats` used for debugging and
      benchmarking.
    * Application developers are not expected to use this normally.
