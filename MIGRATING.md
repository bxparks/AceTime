# Migrating to Releases with Breaking Changes

## Table of Contents

* [Migrating to v1.8.0](#MigratingToVersion180)
    * [Migrating to AceTimeClock](#MigratingToAceTimeClock)
    * [Migrating the DS3231Clock](#MigratingTheDS3231Clock)
    * [Migrating to LinkManagers](#MigratingToLinkManagers)

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
   the flash memory consumption by at least 1300 bytes on AVR for applications
   which use only the AceTime portion of the library, and increases the
   flexibility of the `DS3231Clock` class.
3) Support for *thin links* was moved out of `BasicZoneManager` and
   `ExtendedZoneManager` into the new `BasicLinkManager` and
   `ExtendedLinkManager`. This simplifies the ZoneManagers, and reduces the
   flash memory consumption of applications which do not use this feature by
   200-500 bytes.

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

Here is the migration process. For all occurrences of the `DS3231Clock` class
like this:

```C++
#include <AceTimeClock.h>
using namespace ace_time;
using namespace ace_time::clock;
...
DS3231Clock dsClock;
```

Replace that with a template instance of the `DS3231Clock<T>` class:

```C++
#include <AceTimeClock.h>
#include <AceWire.h> // TwoWireInterface
#include <Wire.h> // TwoWire, Wire
using namespace ace_time;
using namespace ace_time::clock;

using WireInterface = ace_wire::TwoWireInterface<TwoWire>;
WireInterface wireInterface(Wire);
DS3231Clock<WireInterface> dsClock(wireInterface);
```

The new version requires more configuration. But in return, we gain more
flexibility and potentially a large reduction of flash memory consumption. Here
is another example where the `SimpleWireInterface` class is swapped in place of
the `TwoWireInterface` class, without any changes to the `DS3231Clock` class:

```C++
#include <AceTimeClock.h>
#include <AceWire.h>
using namespace ace_time;

const uint8_t SCL_PIN = SCL;
const uint8_t SDA_PIN = SDA;
const uint8_t DELAY_MICROS = 4;
using WireInterface = ace_wire::SimpleWireInterface;
WireInterface wireInterface(SDA_PIN, SCL_PIN, DELAY_MICROS);
DS3231Clock<WireInterface> dsClock(wireInterface);
```

According to the benchmarks at
[AceWire/MemoryBenchmark](https://github.com/bxparks/AceWire/tree/develop/examples/MemoryBenchmark),
using `SimpleWireInterface` instead of the `TwoWire` class reduces flash
consumption by 1500 bytes on an AVR processor. The flash consumption can be
reduced by 2000 bytes if the "fast" version `SimpleWireFastInterface` is used
instead.

<a name="MigratingToLinkManagers"></a>
### Migrating to LinkManagers

In v1.7.5, thin links were activated by adding the `kLinkRegistrySize` and
`kLinkRegistry` parameters to the constructor of `BasicZoneManager` and
`ExtendedZoneManager`, like this:

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

See the [Thin Links](USER_GUIDE.md#ThinLinks) section in the User Guide for
additional information.
