# Migrating to Releases with Breaking Changes

## Table of Contents

* [Migrating to v1.9.0](#MigratingToVersion190)
    * [Configuring the Zone Managers](#ConfiguringZoneManagers)
    * [Using the Zone Managers](#UsingZoneManagers)
    * [Link Managers](#LinkManagers)
* [Migrating to v1.8.0](#MigratingToVersion180)
    * [Migrating to AceTimeClock](#MigratingToAceTimeClock)
    * [Migrating the DS3231Clock](#MigratingTheDS3231Clock)
    * [Migrating to LinkManagers](#MigratingToLinkManagers)

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
