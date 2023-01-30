# Migrating to Releases with Breaking Changes

## Table of Contents

* [Migrating to v2.1.0](#MigratingToVersion210)
    * [Unified Links](#UnifiedLinks)
    * [ZonedExtra](#ZonedExtra)
* [Migrating to v2.0.0](#MigratingToVersion200)
    * [High Level](#HighLevel200)
    * [Details](#Details200)
    * [Background Motivation](#Motivation200)
* [Migrating to v1.9.0](#MigratingToVersion190)
    * [Configuring the Zone Managers](#ConfiguringZoneManagers)
    * [Using the Zone Managers](#UsingZoneManagers)
    * [Link Managers](#LinkManagers)
* [Migrating to v1.8.0](#MigratingToVersion180)
    * [Migrating to AceTimeClock](#MigratingToAceTimeClock)
    * [Migrating the DS3231Clock](#MigratingTheDS3231Clock)
    * [Migrating to LinkManagers](#MigratingToLinkManagers)

<a name="MigratingToVersion210"></a>
## Migrating to v2.1

<a name="UnifiedLinks"></a>
### Unified Links

Over the years, I implemented 4 different versions of the Link entries:

* Ghost Links (`< v1.5`)
* Fat Links (`>= v1.5`)
* Thin Links (`>= v1.6`)
* Symbolic Links (`>= v1.11`)

I had mistakenly assumed that TZDB Link entries were somehow less important
than the Zone entries. Often Link entries were old spellings of
zones which were replaced by new zone names (e.g. "Asia/Calcutta" replaced by
"Asia/Kolkata"), or zones using an older naming convention pre-1993 (e.g.
"UTC" replaced by "Etc/UTC").

The code in AceTime reflected my assumption of the second-class status of Link
entries. Recently however the IANA TZDB project has aggressively merged
unrelated Zones (in different countries) into a single zone if they all happen
to have the same DST transition rules since 1970. The duplicate zones become
Link entries to the "canonical" zone (e.g. "Arctic/Longyearbyen",
"Europe/Copenhagen", "Europe/Oslo", "Europe/Stockholm", "Atlantic/Jan_Mayen" are
all links to "Europe/Berlin").

It is now more clear the Link entries should be considered first-class entries,
just like Zone entries. The v2.1 release implements this change in semantics.
All previous implementations of Links are now merged into a single
implementation which treats Links equal to Zones, and all the usual operations
on Zones are also valid on Links.

1) The "Thin Link" feature has been removed, along with the Link Manager
   classes. The code was too complex, and did not provide enough value.
    * `LinkManager`
    * `BasicLinkManager`
    * `ExtendedLinkManager`.
2) The `followLink` parameter on various `TimeZone` and `ZoneProcessor` methods
   has been removed.
3) The `zonedb` and `zonedbx` databases no longer contain the link registries:
    * `basic::kLinkRegistry`
    * `extended::kLinkRegistry`
4) The `kZoneRegistry` is still generated for historical reasons.
    * This registry contains the minimum complete dataset of the IANA timezones.
    * Most applications should use `kZoneAndLinkRegistry` which contains the
      Link entries.

Most application code are expected to treat Links and Zones equally. There are
only 2 methods which apply only to Link time zones:

* `TimeZone::isLink()`
    * Returns true if the current time zone is a Link.
* `TimeZone::printTargetZoneNameTo()`
    * Prints the name of the target Zone if the current zone is a link.
    * It prints nothing is `isLink()` is false.

<a name="ZonedExtra"></a>
### ZonedExtra

The `ZonedExtra` class was created to replace 3 ad-hoc query methods on the
`TimeZone` object:

* Removed: `TimeZone::getUtcOffset()`
* Removed: `TimeZone::getDeltaOffset()`
* Removed: `TimeZone::getAbbrev()`

Once the `ZonedExtra` object has been objected for a particular point in time,
the following methods on `ZonedExtra` are the replacements for the above:

* `ZonedExtra::timeOffset()`
* `ZonedExtra::dstOffset()`
* `ZonedExtra::abbrev()`

The `ZonedExtra` object will normally be created through 2 factory methods:

* `ZonedExtra::forEpochSeconds(epochSeconds, tz)`
* `ZonedExtra::forLocalDateTime(ldt, tz)`

The `ZonedExtra` object provides access to other meta-information about the time
zone at that particular time. See the [ZonedExtra](USER_GUIDE.md#ZonedExtra)
section in the `USER_GUIDE.md` for more detailed information about this class.

<a name="MigratingToVersion200"></a>
## Migrating to v2.0

<a name="HighLevel200"></a>
### High Level

The primary purpose of AceTime v2 is to extend the range of years supported by
the library from `[2000,2050)` to `[2000,10000)`, while remaining compact enough
to be useful on resource constrained environments like 8-bit AVR processors.
AceTime uses a 32-bit integer to represent the "epoch seconds". This means that
it has a range about 136 years. In version v1, the epoch was hardcoded to be
2000-01-01T00:00:00, which allowed the "epoch seconds" to support years from
about 1950 to 2050, which got truncated to about 2000 to 2050 for simplicity.
However, in the year 2022, the upper limit of 2050 seems too close for comfort
to use in embedded devices which could last more than 25 years.

One solution for extending the range of the "epoch seconds" is to use a 64-bit
integer instead of a 32-bit integer. This solution is used by many other time
zone libraries. However, 64-bit operations are extremely resource intensive on
8-bit processors in terms of both flash memory and CPU cycles, and did not seem
appropriate for AceTime which hopes to remain usable on 8-bit processors.

The solution used by AceTime v2 is to keep the "epoch seconds" as a 32-bit
integer, but allow the **epoch year** to be adjustable instead of being
hardcoded to the year 2000. Downstream applications can select an epoch year
that is appropriate for their use case, allowing AceTime to be valid over
roughly a 100-year interval straddling the epoch year. For example, if the epoch
year is set to 2100 (so that the epoch is 2100-01-01T00:00:00), AceTime will
work for the years 2050 to 2150. The assumption is that a 100-year interval is
sufficient for most embedded applications.

With the epoch year being adjustable, it becomes necessary to decouple the
various `year` fields in the TZDB database (implemented by the `zonedb` and
`zonedbx` packages) from the `year` fields in the AceTime library code itself.
Previously in v1, the `year` fields were encoded as 8-bit `int8_t` integers
which were interpreted to be offsets from the hardcoded epoch year of 2000. To
allow the zone databases to be valid until the year 10000, the internal `year`
fields are changed from an `int8_t` to a 16-bit `int16_t` type. The AceTime API
hides most of the impact of this change from client applications, so the biggest
noticeable change may be the increase in flash size of the `zonedb` and
`zonedbx` databases which are now 2.5 kiB to 3.5 kiB larger.

The increase in flash size for `zonedb` and `zonedbx` seems acceptable for the
following reasons: The full impact of the size increase would be felt only if
the application incorporated the entire `zonedb` or `zonedbx` database to
support all 595 time zones in the TZDB database. But most 8-bit processors do
not have enough flash memory to use the full database (e.g. 23 kiB for `zonedb`,
38 kiB for `zonedbx`), so it is likely that these 8-bit applications would use
only a small subset (1-10) of the timezones available in those databases. The
flash size increase would be far smaller when using only small number of time
zones. On 32-bit processors where the full database would likely be used, they
often have far more flash memory (0.5 MiB to 4 MiB on ESP8266 or ESP32), so the
increase of 2.5-3.5 kiB of flash memory would be negligible on those processors.

Some backwards incompatible changes were necessary from v1 to v2. These are
explained in detail in the next section.

<a name="Details200"></a>
### Details

AceTime v2 implements the following major changes and features:

* the internal `year` field in various classes (`LocalDate`, `LocalDateTime`,
  `OffsetDateTime`, `ZonedDateTime`) changes from `int8_t` to an `int16_t`
    * the range increases from `[1873,2127]` to `[1,9999]`
    * the various `year()` methods in these classes were already using `int16_t`
      so this internal change should be mostly invisible to client applications
* the `year` fields in the `zonedb` and `zonedbx` databases also change from
  `int8_t` to `int16_t`
    * the year range increases from `[2000,2049]` to `[2000,9999]`
    * decouples the TZ database from the adjustable current epoch year
* removed constants
    * `LocalDate::kEpochYear`
        * replacement: `Epoch::currentEpochYear()` function
        * reason: no longer a constant
    * `LocalDate::kSecondsSinceUnixEpoch`
        * purpose: number of seconds from 1970 to the AceTime epoch (2000-01-01
          in v1, but adjustable in v2)
        * replacement: `Epoch::secondsToCurrentEpochFromUnixEpoch64()`
        * reasons:
            * `int32_t` seconds can overflow, so use `int64_t`
            * epoch year is now adjustable, not a constant
    * `LocalDate::kDaysSinceUnixEpoch`
        * purpose: number of days from 1970-01-01 to AceTime epoch (2000-01-01
          in v1, but adjustable in v2)
        * replacement: `Epoch::daysToCurrentEpochFromUnixEpoch()`
        * reason: epoch is now adjustable, so must become a function
    * `LocalDate::kMinYearTiny`
        * replacement: `LocalDate::kMinYear`
        * reason: 8-bit offset no longer used, replaced by 16-bit integer
    * `LocalDate::kMaxYearTiny`
        * replacement: `LocalDate::kMaxYear`
        * reason: 8-bit offset no longer used, replaced by 16-bit integer
    * `LocalDate::kInvalidUnixDays`
        * replacement: `kInvalidEpochDays`
        * reason: simplification, both had the same value `INT32_MIN`
    * `LocalDate::kInvalidUnixSeconds`
        * replacement: `LocalDate::kInvalidUnixSeconds64`
        * reason: 32-bit versions of `toUnixSeconds()` removed
* removed functions
    * `LocalDate::toUnixSeconds()`
        * reason: 32-bit Unix seconds will overflow in the year 2038
        * replacement: `LocalDate::toUnixSeconds64()`
    * `LocalDate::forUnixSeconds()`
        * reason: 32-bit Unix seconds will overflow in the year 2038
        * replacement: `LocalDate::forUnixSeconds64()`
    * `LocalDate::yearTiny()`
        * reason: `int8_t` year fields replaced by `int16_t` type
    * `LocalDate::forTinyComponents()` (undocumented)
        * reason: `int8_t` year fields replaced by `int16_t` type
    * `OffsetDateTime::toUnixSeconds()`
    * `OffsetDateTime::forUnixSeconds()`
    * `OffsetDateTime::yearTiny()`
    * `ZonedDateTime::toUnixSeconds()`
    * `ZonedDateTime::forUnixSeconds()`
    * `ZonedDateTime::yearTiny()`
* new functions
    * `Epoch::currentEpochYear(epochYear)`
        * purpose: set the current epoch year
    * `Epoch::currentEpochYear()`
        * purpose: get the current epoch year
    * `Epoch::daysToCurrentEpochFromUnixEpoch()`
        * purpose: number of days from Unix epoch (1970-01-01) to
          the current epoch ({yyyy}-01-01) where `yyyy` is set by
          `currentEpochYear(yyyy)`
    * `Epoch::daysToCurrentEpochFromConverterEpoch()`
        * purpose: number of days from the converter epoch
          (2000-01-01T00:00:00) to the current epoch ({yyyy}-01-01T00:00:00)
          where `yyyy` is set by `currentEpochYear(yyyy)`
        * comment: should not normally be needed by client applications
    * `Epoch::secondsToCurrentEpochFromUnixEpoch64()`
        * purpose: number of seconds from the Unix epoch (1970-01-01T00:00:00)
          to the current epoch ({yyyy}-01-01T00:00:00)
        * comment: useful for converting between AceTime epoch and Unix epoch
    * `Epoch::epochValidYearLower()`
        * purpose: defines lower limit of valid years (`valid_year >= lower`)
          for features related to epochSeconds and timezones
    * `Epoch::epochValidYearUpper()`
        * purpose: defines upper limit of valid years (`valid_year < upper`)
          for features related to epochSeconds and timezones

The epochSeconds that was generated by AceTime v1 using the epoch year of 2000
will be incompatible with AceTime v2 using a different epoch year. Client
applications which need read old epochSeconds value using AceTime v2 have a
number of options:

1) Call `Epoch::currentEpochYear(2000)` at the beginning of the application,
   so that the v2 epoch year is the same as the v1 epoch year. The disadvantage
   is that the 32-bit epochSeconds will stop working with this library sometime
   around the year 2065-2066.
2) Perform a conversion of the v1 epochSeconds to the v2 epochSeconds by
   setting `Epoch::currentEpochYear(year)` first, then calculating the new
   epochSeconds using `newEpochSeconds = oldEpochSeconds -
   Epoch::daysToCurrentEpochFromConverterEpoch() * 86400`.
3) Do no conversion. Just reset the date and time using the new epoch year.
   The next time the device is rebooted, the date and time will use the
   new epoch year instead of the old epoch year.

<a name="Motivation200"></a>
### Background Motivation

Using 32-bit integer field for epochSeconds gives a range of about 136 years.
This is the cause of the famous Unix [Year 2038
problem](https://en.wikipedia.org/wiki/Year_2038_problem) which uses a 32-bit
signed integer starting from the epoch year of 1970 (1970-01-01 00:00:00 UTC).

When the AceTime project started in 2018, using the year 2000 as the epoch year
pushed the theoretical maximum year of epochSeconds to about 2068 which seemed
sufficiently far enough away. The epoch year of 2000 also seemed convenient
because it is the same value used by the [AVR
libc](https://avr-libc.nongnu.org/) in its
[time.h](https://avr-libc.nongnu.org/user-manual/group__avr__time.html)
implementation. The actual upper limit was restricted to 2050 to provide some
headroom before calculations would overflow in the year 2068.

Now in the year 2022, the upper limit of 2050 feels too low, since embedded
devices could be reasonably expected to keep working for the next 25 years.
The updated AceTime v2 is designed to support a 100-year interval from
`[2000,2100)` by default. To prevent the need to change the source code when the
range needs to extended even further in the future, the "current epoch year" is
made adjustable by the client application.

<a name="MigratingToVersion190"></a>
## Migrating to v1.9.0

The `ZoneManager` hierarchy (containing `ManualZoneManager`, `BasicZoneManager`,
and `ExtendedZoneManager`) was refactored from v1.8.0 to v1.9.0.

<a name="ConfiguringZoneManagers"></a>
### Configuring the Zone Managers

In v1.8, the `ZoneManager` was an abstract interface class with 7 pure virtual
methods that was the base class of the class hierarchy of all ZoneManager
subclasses. This was convenient because the `TimeZone` related parts of the
client application code could be written against the `ZoneManager` base class
and the specific implementation could be configured in a small section of the
application code. The problem with such a polymorphic class hierarchy is that
the virtual methods consume significant amounts of flash memory, especially on
8-bit AVR processors with limited flash. The
[examples/MemoryBenchmark](examples/MemoryBenchmark) program showed that this
design consumed an extra 1100-1300 bytes of flash.

In v1.9, several changes were made to reduce the flash memory size:

1. All virtual methods were removed from the `ZoneManager` and its
   subclasses.
2. The `BasicZoneManager` and `ExtendedZoneManager` classes are no longer
   template classes, making them easier to use (e.g. in the `ZoneSorterByName`
   and `ZoneSorterByOffsetAndName` classes).
3. The internal `BasicZoneProcessorCache` and `ExtendedZoneProcessorCache`
   member variables were extracted out from the respective ZoneManager classes.
   These are now expected to be created separately, and passed into the
   constructors of the `BasicZoneManager` and `ExtendedZoneManager` classes.

The migration path is relatively simple. In v1.8, the `BasicZoneManager` was
configured like this:

```C++
static const uint8_t CACHE_SIZE = 4;
BasicZoneManager<CACHE_SIZE> zoneManager(
    kZoneRegistrySize,
    kZoneRegistry);
```

In v1.9, this should be replaced with code that looks like:

```C++
static const uint8_t CACHE_SIZE = 4;
BasicZoneProcessorCache<CACHE_SIZE> zoneProcessorCache;
BasicZoneManager zoneManager(
    kZoneRegistrySize,
    kZoneRegistry,
    zoneProcessorCache);
```

Similarly, in v1.8, the `ExtendedZoneManager` was configured like following:

```C++
static const uint8_t CACHE_SIZE = 4;
ExtendedoneManager<CACHE_SIZE> zoneManager(
    zonedb::kZoneRegistrySize,
    zonedb::kZoneRegistry);
```

In v1.9, this should be replaced with code that looks like this:

```C++
static const uint8_t CACHE_SIZE = 4;
ExtendedoneProcessorCache<CACHE_SIZE> zoneProcessorCache;
ExtendedoneManager zoneManager(
    zonedbx::kZoneRegistrySize,
    zonedbx::kZoneRegistry,
    zoneProcessorCache);
```

<a name="UsingZoneManagers"></a>
### Using the Zone Managers

In v1.8, the `ZoneManager` was the parent interface class of all polymorphic
subclasses. So the client code that needed a specific subclass of `ZoneManager`
could do something like this:

```C++
class Controller {
  public:
    Controller(
      ZoneManager* zoneManager,
      ...
    ) :
      mZoneManager(zoneManager),
      ...
    {}

  private:
    ZoneManager* mZoneManager;
};
```

Any instance of `BasicZoneManager<SIZE>` or `ExtendedZoneManager<SIZE>` could be
passed into the constructor. This provided some runtime flexibility and code
simplicity. However, the runtime flexibility did not seem useful for the vast
majority of cases and the simplicity offered by the single parent interface
class was paid for by an extra 1100-1300 bytes of flash memory.

In v1.9, the application still has the ability to choose between a
`BasicZoneManager` and an `ExtendedZoneManager` at compile time. The same
`Controller` constructor should look something like this:

```C++
// Define the various TIME_ZONE_TYPE macros in config.h.
#include "config.h"

class Controller {
  public:
    Controller(
    #if TIME_ZONE_TYPE == TIME_ZONE_TYPE_BASIC
      BasicZoneManager* zoneManager,
    #elif TIME_ZONE_TYPE == TIME_ZONE_TYPE_EXTENDED
      ExtendedZoneManager* zoneManager,
    #endif
      ...
    ) :
      mZoneManager(zoneManager),
      ...
    {}

  private:
  #if TIME_ZONE_TYPE == TIME_ZONE_TYPE_BASIC
    BasicZoneManager* mZoneManager;
  #elif TIME_ZONE_TYPE == TIME_ZONE_TYPE_EXTENDED
    ExtendedZoneManager* mZoneManager;
  #endif
};
```

It is assumed that most applications will hard code either the
`BasicZoneManager` or the `ExtendedZoneManager`, and will not need this level
of configuration.

<a name="LinkManagers"></a>
### Link Managers

In v1.8, the `LinkManager` was an interface class with pure virtual methods:

```C++
class LinkManager {
  public:
    static const uint16_t kInvalidZoneId = 0x0;
    virtual uint32_t zoneIdForLinkId(uint32_t linkId) const = 0;
    virtual uint16_t linkRegistrySize() const = 0;
};
```

But allowing the `BasicLinkManager` and `ExtendedLinkManager` to be polymorphic
did not seem worth the extra flash usage, similar to the `ZoneManager`
hierarchy.

In v1.9, the pure `virtual` methods were removed, but the static constant
was retained for backwards compatibility:

```C++
class LinkManager {
  public:
    static const uint16_t kInvalidZoneId = 0x0;
};
```

The `BasicLinkManager` and `ExtendedLinkManager` should be used directly,
instead of through the `LinkManager` interface. Since Link Managers were
introduced only in v1.8, I expect almost no one to be affected by this.

<a name="MigratingToVersion180"></a>
## Migrating to v1.8.0

Three breaking changes were made from v1.7.5 to v1.8.0:

1) The `SystemClock` and other clock classes were moved to
   [AceTimeClock](https://github.com/bxparks/AceTimeClock). This improves the
   decoupling between the AceTime and AceTimeClock libraries and allows
   faster development of each library.
2) The `DS3231Clock` class was converted into a template class to replace a
   direct dependency to the I2C `<Wire.h>` library with an indirect dependency
   to the [AceWire](https://github.com/bxparks/AceWire) library. This reduces
   the flash memory consumption between 1300-2500 bytes on AVR processor
   on applications which use only the AceTime portion of the library, and
   increases the flexibility of the `DS3231Clock` class.
3) Support for [thin links](USER_GUIDE.md#ThinLinks) was moved out of
   `BasicZoneManager` and `ExtendedZoneManager` into the new `BasicLinkManager`
   and `ExtendedLinkManager` classes. This simplifies the ZoneManagers, and
   reduces the flash memory consumption of applications which do not use this
   feature by 200-500 bytes.

The following subsections show how to migrate client application from
AceTime v1.7.5 to AceTime v1.8.0.

<a name="MigratingToAceTimeClock"></a>
### Migrating to AceTimeClock

For AceTime v1.8.0, the clock classes under the `ace_time::clock` namespace have
been moved to the `AceTimeClock` library. To help backwards compatibility,
the namespace of the clock classes remain in the `ace_time::clock` namespace.

To migrate your old code, install `AceTimeClock` using the Arduino Library
Manager. (See [AceTimeClock
Installation](https://github.com/bxparks/AceTimeClock#Installation) for more
details). Then update the client code to add the `<AceTimeClock.h>` header
file just after the exiting `<AceTime.h>` header.

For example, if the original code looks like this:

```C++
#include <AceTime.h>
using namespace ace_time;
using namespace ace_time::clock;
```

Replace that with this:

```C++
#include <AceTime.h>
#include <AceTimeClock.h>
using namespace ace_time;
using namespace ace_time::clock;
```

<a name="MigratingTheDS3231Clock"></a>
### Migrating the DS3231Clock

For AceTime v1.8.0, the `DS3231Clock` class was converted into a template class
to replace a direct dependency to the `<Wire.h>` library with an indirect
dependency to to the [AceWire](https://github.com/bxparks/AceWire) library.
There were 2 primary motivation for the change.

One, simply including the `<Wire.h>` header file increases the flash memory
usage by ~1300 bytes on AVR, *even* if the `Wire` object is never used. The
`DS3231Clock` class is the *only* class in the AceTime library that depended on
the `<Wire.h>`. So any application that pulled in `<AceTime.h>` for the time
zone classes would suffer the increased flash usage of the `<Wire.h>` library,
even if the `Wire` was never referenced or used in the client application.

Two, the `TwoWire` class from `<Wire.h>` is not designed to be used
polymorphically (see
[SoftwareWire#28](https://github.com/Testato/SoftwareWire/issues/28) for more
details). In other words, it cannot be subclassed and cannot be replaced with
a different implementation of the I2C protocol. If a 3rd party library contains
a direct dependency to the `<Wire.h>` directly, it is impossible to replace the
`Wire` object with a different I2C implementation (for example, one of the
alternative I2C implementations listed in this [Overview of Arduino I2C
libraries](https://github.com/Testato/SoftwareWire/wiki/Arduino-I2C-libraries).
The `<AceWire.h>` library solves this problem by using compile-time polymorphism
through C++ templates.

Here is the migration process. For all occurrences of the old `DS3231Clock`
class that look like this:

```C++
#include <AceTimeClock.h>
using namespace ace_time;
using namespace ace_time::clock;
...
DS3231Clock dsClock;
```

Replace that with a template instance of the `DS3231Clock<T>` class, and its I2C
interface classes (`TwoWire` and `TwoWireInterface`):

```C++
#include <AceTimeClock.h>
#include <AceWire.h> // TwoWireInterface
#include <Wire.h> // TwoWire, Wire
using namespace ace_time;
using namespace ace_time::clock;

using WireInterface = ace_wire::TwoWireInterface<TwoWire>;
WireInterface wireInterface(Wire);
DS3231Clock<WireInterface> dsClock(wireInterface);

void setup() {
  ...
  Wire.begin();
  wireInterface.begin();
  dsClock.setup();
  ...
}
```

The new `DS3231Clock<T>` class requires more configuration. But in return, we
gain more flexibility and potentially a large reduction of flash memory
consumption if the pre-installed `<Wire.h>` is replaced with a different I2C
implementation.  For example, here is a version of the `DS3231Clock` object
where the `SimpleWireInterface` software I2C class replaces the hardware
`TwoWireInterface` class, without any changes to the `DS3231Clock` class:

```C++
#include <AceTimeClock.h>
#include <AceWire.h>
using namespace ace_time;
using namespace ace_time::clock;

const uint8_t SCL_PIN = SCL;
const uint8_t SDA_PIN = SDA;
const uint8_t DELAY_MICROS = 4;
using WireInterface = ace_wire::SimpleWireInterface;
WireInterface wireInterface(SDA_PIN, SCL_PIN, DELAY_MICROS);
DS3231Clock<WireInterface> dsClock(wireInterface);

void setup() {
  ...
  wireInterface.begin();
  dsClock.setup();
  ...
}
```

According to the benchmarks at
[AceWire/MemoryBenchmark](https://github.com/bxparks/AceWire/tree/develop/examples/MemoryBenchmark),
using `SimpleWireInterface` instead of the `TwoWire` class reduces flash
consumption by 1500 bytes on an AVR processor. The flash consumption can be
reduced by 2000 bytes if the "fast" version `SimpleWireFastInterface` is used
instead.

<a name="MigratingToLinkManagers"></a>
### Migrating to LinkManagers

In v1.7.5, [thin links](USER_GUIDE.md#ThinLinks) were activated by adding the
`kLinkRegistrySize` and `kLinkRegistry` parameters to the constructor of
`BasicZoneManager` and `ExtendedZoneManager`, like this:

```C++
BasicZoneManager zoneManager(
    zonedb::kZoneRegistrySize, zonedb::kZoneRegistry,
    zonedb::kLinkRegistrySize, zonedb::kLinkRegistry);

ExtendedZoneManager zoneManager(
    zonedbx::kZoneRegistrySize, zonedbx::kZoneRegistry,
    zonedbx::kLinkRegistrySize, zonedbx::kLinkRegistry);
```

This caused `BasicZoneManager::createForZoneId()` and
`ExtendedZoneManager::createForZoneId()` to perform an additional lookup in the
`kLinkRegistry` if a `zoneId` was not found in the `kZoneRegistry`.

In v1.8.0, that fall-back functionality has been moved to the LinkManagers,
and the ZoneManager constructors no longer accept the link registry parameters:

```C++
BasicLinkManager linkManager(
    zonedb::kLinkRegistrySize, zonedb::kLinkRegistry);

ExtendedLinkManager linkManager(
    zonedbx::kLinkRegistrySize, zonedbx::kLinkRegistry);
```

The client application is now responsible for activating the link registry, and
performing the fallback lookup:

```C++
TimeZone findTimeZone(uint32_t zoneId) {
  TimeZone tz = zoneManager.createForZoneId(zoneId);
  if (tz.isError()) {
    // Search the link registry.
    zoneId = linkManager.zoneIdForLinkId(zoneId);
    if (zoneId != LinkManager::kInvalidZoneId) {
      tz = zoneManager.createForZoneId(zoneId);
    }
  }
  return tz;
}
```

This change allows the ZoneManagers to provide a consistent API for some
upcoming features, and prevents unnecessary flash consumption (200-500 bytes) if
the client application does not use the thin link feature.
