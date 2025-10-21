# Developer Notes

Information which are useful for developers and maintainers of the AceTime
library.

## Table of Contents

- [Project Dependency](#project-dependency)
- [Architectural Layers](#architectural-layers)
    - [Storage Layer](#storage-layer)
    - [Broker Layer](#broker-layer)
    - [ZoneProcessor Storage Layer](#zoneprocessor-storage-layer)
    - [Zone Database Layer](#zone-database-layer)
- [Storage Layer Classes](#storage-layer-classes)
    - [ZoneContext](#zonecontext)
    - [ZoneInfo and ZoneEra](#zoneinfo-and-zoneera)
    - [ZonePolicy and ZoneRule](#zonepolicy-and-zonerule)
    - [ZoneRegistry](#zoneregistry)
    - [Offset Encoding](#offset-encoding)
    - [TinyYear Encoding](#tinyyear-encoding)
- [BasicZoneProcessor](#basiczoneprocessor)
- [ExtendedZoneProcessor](#extendedzoneprocessor)
    - [Search by Component or EpochSeconds](#search-by-component-or-epochseconds)
    - [Stea 1: Find Matches](#step-1-find-matches)
    - [Step 2: Create Transitions](#step-2-create-transitions)
    - [Step 2A: Transition Time versus StartDateTime](#step-2a-transition-time-versus-startdatetime)
    - [Step 3: Fix Transition Times](#step-3-fix-transition-times)
    - [Step 4: Generate Start Until Times](#step-4-generate-until-times)
    - [Step 5: Calculate Abbreviations](#step-5-calculate-abbreviations)

## Project Dependency

This repo was programmatically generated from the
[AceTimeSuite](https://github.com/bxparks/AceTimeSuite) project.

## Architectural Layers

The AceTime library is organized roughly in layers, and they are related in the
following way, where the arrow means "depends on":

```
           Storage layer (src/zoneinfo)
           ace_time::ZoneInfoLow::ZoneXXX
           ace_time::ZoneInfoMid::ZoneXXX
           ace_time::ZoneInfoHigh::ZoneXXX
                   ^
                   |
           Broker layer (src/zoneinfo)
           ace_time::ZoneInfoLow::ZoneXXXBroker
           ace_time::ZoneInfoMid::ZoneXXXBroker
           ace_time::ZoneInfoHigh::ZoneXXXBroker
           ace_time::basic::Info
           ace_time::extended::Info
           ace_time::complete::Info
                   ^      ^
                   |       \
                   |      Zone Database layer (src/zonedbYYY)
                   |      ace_time::zonedb
                   |      ace_time::zonedbx
                   |      ace_time::zonedbc
                   |       ^
                   |      /
                ZoneProcessors (src/ace_time/*Processor)
                ace_time::basic
                ace_time::extended
                ace_time::complete
                ace_time::BasicZoneProcessorTemplate<>
                ace_time::ExtendedZoneProcessorTemplate<>
                ace_time::BasicZoneProcessor
                ace_time::ExtendedZoneProcessor
                ace_time::CompleteZoneProcessor
                                         ^
                                         |
DateTime classes (src/ace_time)     TimeZone classes (src/ace_time)
ace_time::PlainDate                 ace_time::TimeZone
ace_time::PlainTime                 ace_time::ZoneManager
ace_time::OffsetDateTime            ace_time::ZoneProcessorCache
                      ^             ^
                       \           /
                        \         /
                     ZonedDateTime classes (src/ace_time)
                     ace_time::ZonedDateTime
                     ace_time::ZonedExtra
                             ^
                             |
                        Testing (src/testing)
                        ace_time::testing
```

The data structures are defined [src/zoneinfo](src/zoneinfo).

### Storage Layer

The `zoneinfo` storage layer provides low-level data structures that define the
storage formats of various fields, records, and tables used by the zoneinfo
databases in the AceTime library. These should be treated as opaque objects by
the client application which can use the `const ZoneInfo *` pointer as the
zone identifier.

There are 5 major data types that are stored in the zoneinfo database:
`ZoneContext`, `ZoneRule`, `ZonePolicy`, `ZoneEra` and `ZoneInfo`. These data
types are analogous to tables in a relational database.

The implementation data classes are defined as inner-classes within
container classes. This allows the container classes to be used as template
parameters to select the lower-level storage classes as a whole batch.

There are 3 container classes defined, corresponding to different resolution
levels supported by each set.

- `ZoneInfoLow.h`
    - low resolution persistence format
        - 1-minute resolution for AT, UNTIL, STDOFF
        - 15-minute resolution for DSTOFF
        - year fields using 1-byte offset from a `baseYear` of 2100,
          supporting the years `[1973,2226]`
    - `ZoneInfoLow::ZoneContext`
    - `ZoneInfoLow::ZoneRule`
    - `ZoneInfoLow::ZonePolicy`
    - `ZoneInfoLow::ZoneEra`
    - `ZoneInfoLow::ZoneInfo`
- `ZoneInfoMid.h`
    - medium resolution persistence format
        - 1-minute resolution for AT, UNTIL, STDOFF
        - 15-minute resolution for DSTOFF
        - 2-byte year fields supporting years `[-32767,32765]`
    - `ZoneInfoMid::ZoneContext`
    - `ZoneInfoMid::ZoneRule`
    - `ZoneInfoMid::ZonePolicy`
    - `ZoneInfoMid::ZoneEra`
    - `ZoneInfoMid::ZoneInfo`
- `ZoneInfoHigh.h`
    - high resolution persistence format
        - 1-second resolution for AT, UNTIL, STDOFF, and DSTOFF
        - 2-byte year fields supporting years `[-32767,32765]`
    - `ZoneInfoHigh::ZoneContext`
    - `ZoneInfoHigh::ZoneRule`
    - `ZoneInfoHigh::ZonePolicy`
    - `ZoneInfoHigh::ZoneEra`
    - `ZoneInfoHigh::ZoneInfo`

### Broker Layer

Wrapping each of these low-level persistent classes are the "broker" layer
classes. They convert the low-level storage formats into a consistent API using
identical types and integer sizes. The allows the code in the ZoneProcessor
layer to be agnostic to the exact storage format of the zoneinfo database.

- `ZoneInfoLow.h`
    - `ZoneInfoLow::ZoneContextBroker`
    - `ZoneInfoLow::ZoneRuleBroker`
    - `ZoneInfoLow::ZonePolicyBroker`
    - `ZoneInfoLow::ZoneEraBroker`
    - `ZoneInfoLow::ZoneInfoBroker`
    - `ZoneInfoLow::ZoneRegistryBroker`
    - `ZoneInfoLow::ZoneInfoStore`
- `ZoneInfoMid.h`
    - `ZoneInfoMid::ZoneContextBroker`
    - `ZoneInfoMid::ZoneRuleBroker`
    - `ZoneInfoMid::ZonePolicyBroker`
    - `ZoneInfoMid::ZoneEraBroker`
    - `ZoneInfoMid::ZoneInfoBroker`
    - `ZoneInfoMid::ZoneRegistryBroker`
    - `ZoneInfoMid::ZoneInfoStore`
- `ZoneInfoHigh.h`
    - `ZoneInfoHigh::ZoneContextBroker`
    - `ZoneInfoHigh::ZoneRuleBroker`
    - `ZoneInfoHigh::ZonePolicyBroker`
    - `ZoneInfoHigh::ZoneEraBroker`
    - `ZoneInfoHigh::ZoneInfoBroker`
    - `ZoneInfoHigh::ZoneRegistryBroker`
    - `ZoneInfoHigh::ZoneInfoStore`

The Broker layer adds 2 higher-level brokers on top of the 5 low-level storage
classes:

- `ZoneRegistryBroker`
    - provides access to the zone registry consisting of a list of pointers to
      `ZoneInfo` objects
- `ZoneInfoStore`
    - provides the high level interface to create a `ZoneInfoBroker` given an
    identifier for a zone (either a direct pointer, or an offset index)
    - (TODO: It might be possible to wrap the raw timezone `const ZoneInfo*`
      pointer into a `ZoneInfoBroker` object, and use the broker object as the
      timezone identifier. This would completely hide the low-level `const
      ZoneInfo*` pointer from the client application.)

### ZoneProcessor Storage Layer

There are 3 concrete implementations of the `ZoneProcessor` interface:

- `BasicZoneProcessor`
- `ExtendedZoneProcessor`
- `CompleteZoneProcessor`

In theory, the each concrete ZoneProcessor can choose to use any of the
underlying storage format. However, for simplicity, we restrict each
ZoneProcessor to a specific storage format, as defined in
[zoneinfo/infos.h](src/zoneinfo/infos.h):

- BasicZoneProcessor -> ZoneInfoLow
    - `basic::Info = ZoneInfoLow`
- ExtendedZoneProcessor -> ZoneInfoLow
    - `extended::Info = ZoneInfoLow`
- CompleteZoneProcessor -> ZoneInfoHigh
    - `complete::Info = ZoneInfoHigh`

In theory, the `basic::`, `extended::`, and `complete::` namespaces provide a
layer of indirection that would allow changing a particular storage format with
another storage format, without changing the higher-level code (since the Broker
Layer provides a consistent API).

### Zone Database Layer

The Zone Database files are programmatically generated by the
[AceTimeSuite/compiler/tzcompiler.sh](../../compiler/tzcompiler.sh) script using
the TZDB files from the [IANA TZ Data](https://www.iana.org/time-zones)
project. They use the low-level data structures defined by
[src/zoneinfo](src/zoneinfo) files.

There are several `zonedb` directories which are pre-generated and included in
the library:

- `src/zonedb/`
    - contains transition rules from year >= 2000
    - used by `BasicZoneProcessor`
    - uses the `zonedb::` namespace
    - uses `basic::Info` classes (i.e. ZoneInfoLow)
- `src/zonedb2025/`
    - same as `zonedb` but for transition rules with year >= 2025
    - uses the `zonedb2025` namespace
- `src/zonedbx/`
    - contains transition rules from year >= 2000
    - used by `ExtendedZoneProcessor`
    - uses the `zonedbx::` namespace
    - uses `extended::Info` classes (i.e. ZoneInfoLow)
- `src/zonedbx2025/`
    - same as `zonedbx` but for transition rules with year >= 2025
    - uses the `zonedbx2025` namespace
- `src/zonedbc/`
    - contains all transitions from TZDB (year >= 1844)
    - used by `CompleteZoneProcessor`
    - uses the `zonedbc::` namespace
    - uses `complete::Info` classes (i.e. ZoneInfoHigh)

Each `zonedb*/` directory contains the following files:

- `zone_infos.cpp`, `zone_infos.h`
- `zone_policies.cpp`, `zone_policies.h`
- `zone_registry.cpp`, `zone_registry.h`

## Storage Layer Classes

### ZoneContext

There is a single `ZoneContext kZoneContext` record included in the
`zone_infos.h` and `zone_infos.cpp` files. The record and its related data could
have been placed in a separate file (e.g. `zone_context.h,cpp`), but they are
relatively small so I avoided creating the overhead of generating separate
files.

The `ZoneContext` provides a number of common parameters for all `ZoneInfo`
records. Each `ZoneInfo` record (see below) has a reference to a `ZoneContext`
record. The fields of the `ZoneContext` class are defined in

- [ZoneInfoLow.h](src/zoneinfo/ZoneInfoLow.h)
- [ZoneInfoMid.h](src/zoneinfo/ZoneInfoMid.h)
- [ZoneInfoHigh.h](src/zoneinfo/ZoneInfoHigh.h)

The `ZoneContext::tzVersion` field is also available as a direct variable named
`kTzDatabaseVersion` in the appropriate namespace for the database: (`zonedb::`,
`zonedbx::`, `zonedbc::`). This allows the TZ version to be retrieved without
having to having to hop across multiple wrapper classes: first get a `ZoneInfo`,
then wrap it around its  `ZoneInfoBroker`, call `ZoneInfoBroker::zoneContext()`,
to retrieve a `ZoneContextBroker`, which finally allows calling the
`ZoneContextBroker::tzVersion()` method.

The `letters[]` and `fragments[]` string arrays are stored in flash memory using
`PROGMEM`, in addition to storing the actual strings themselves in `PROGMEM`.

There is no equivalent `formats[]` array in `ZoneContext`. Instead each `format`
string is stored in the `ZoneEra` record as a pointer. This is because each
`ZoneEra.format` string is relatively unique, so there did not seem to be much
room to save memory through deduplication. Also, adding the `formats[]` array
into `ZoneContext` would incorporate the entire set of `formats[]` into flash
memory even if only a limited number of timezones were selected through a custom
zone registry. That would increase the flash size for applications using a small
number of zones, while decreasing the flash size for applications using the
default zone registries (`kZoneRegistry` or `kZoneAndLinkRegistry`). But the
advantage of collecting all the `format` strings into a common pool would be
that all those strings would be in `PROGMEM` flash memory, instead of normal
static memory. This would save RAM memory on AVR processors when large number of
zones are compiled into the application. It might be worth investigating the 2
implementations and comparing the flash and static memory usage patterns to
determine if it's worth moving the `format` strings to `formats[]` array in the
`ZoneContext`.

### ZoneInfo and ZoneEra

The `zone_infos.h` and `zone_infos.cpp` files contain a `ZoneInfo` record for
each supported time zone corresponding to a `Zone` or `Link` entry in the IANA
TZ database. Each `ZoneInfo` aggregates one ore more `ZoneEra` records.

The zone identifier has the form `kZone{region}_{city}`, where the
`{region}_{city}` comes directly from the TZ Database. Some minor character
transformations are applied to create an valid C++ identifier. For example, all
dashes `-` are converted to underscore `_`. As an exception, the `+` character
is converted to `_PLUS_` to differentiate "Etc/GMT+1" from "Etc/GMT-1", and so
on.

Near end of the `zone_info.h` file, we list the zones which were deliberately
excluded by the tool. Also at the end of the `zone_info.h` file, there may be
warnings about known inaccuracies for a particular zone.

### ZonePolicy and ZoneRule

The `zone_policies.h` and `zone_policies.cpp` hole the `RULE` entries from the
IANA TZ database. A `ZonePolicy` is a collection of one or more `ZoneRule`
records, which each `ZoneRule` corresponding to a single line of the `RULE`
entry in the TZ database. A `ZoneEra` record may hold a pointer to a
`ZonePolicy` record. For example, the `kZoneAmerica_Los_Angeles` has a pointer
to a `kZonePolicyUS` record.

### Zone Registry

The `zone_registry.h` and `zone_registry.cpp` files contain 2 pre-defined
registries of timezones:

- `const ZoneInfo- const kZoneRegistry[kZoneRegistrySize]`
    - contains a list of all `Zone` entries
- `const ZoneInfo- const kZoneAndLinkRegistry[kZoneAndLinkRegistrySize]`
    - contains a list of all `Zone` and `Link` entries

Due to reasons which are too complicated to explain here, `Zone` and `Link`
entries should treated with the same priority. Client applications should almost
always use the `kZoneAndLinkRegistry`. The only exception may be testing
applications which may want to use the smaller `kZoneRegistry` to achieve
complete coverage of all timezones with the same set of rules, without
duplicates.

### Offset Encoding

The `ZoneInfoHigh` format encodes various formats using 1-second resolution for
all offsets (STDOFF, DSTOFF) and time instants (AT, UNTIL).

The `ZoneInfoLow` and `ZoneInfoMid` storage formats try to reduce the memory
footprint by encoding the offsets (STDOFF, DSTOFF) in 15-minute resolution, and
the time instants (AT, UNTIL) with a 1-minute resolution.

A number of fields which required only 4-bits of space were combined together to
save memory space. There are 5 offsets and moment-in-time quantities from the TZ
zoneinfo files which are captured in the `zone_info.{h,cpp}` and
`zone_policies.{h,cpp}` files:

- `STDOFF` field in `Zone` entry (previously `OFFSET`), 1-minute resolution
- `RULES` (DSTOFF) field in `Zone` entry when numeric (e.g. "1:00"), 15-minute
  resolution
- `UNTIL` field in `Zone` entry, 1-minute resolution
- `SAVE` (DSTOFF) field in `Rule` entry, 15-minute resolution
- `AT` field in `Rule` entry, 1-minute resolution

To reduce flash memory size, these fields are encoded in non-obvious ways which
are difficult to remember. Here is my attempt to document the encoding. In the
following diagram, a field labeled `code` (e.g. `offsetCode`) has a unit of 15
minutes. For example, an `offsetCode` of 2 means 30 minutes. To capture time
offsets or moments with a 1-minute resolution, we store the remaining 15-minutes
(0 to 14 inclusive), using 4-bits in the upper 4-bits or the lower 4-bits of one
of the other fields. For the "extended" zoneinfo files (i.e. `zonedbx`), the
`code` value is shifted by +4 (in other words, 1 hour), so that a 4-bit field is
able to represent offsets from -1:00 to +2:45. This allows us to capture zones
which have a negative -1:00 hour offset, and zones which undergo a "double DST"
shift of +2:00 hours.

```
TZ DB Fields                            ZoneEra
============                            =======

STDOFF (1-min resolution)
-> offset_seconds                       +--------------------+
  -> offset_code ---------------------> |     offsetCode     |
     offset_minute                      |     (8 bits)       |
                  \                     +--------------------+
                   \
                    \                   +--------------------+
                     \                  |     deltaCode      |
                      \                 |--------------------|
                       ---------------> | minute  | code     |
                                        | (4-bits)| (4-bits) |
                                        +--------------------+
                                                    ^
RULES (numeric, 15-min resolution)                 /
-> era_delta_seconds                              /
  -> era_delta_minutes                           /
    -> era_delta_code + 4------------------------


UNTIL (1-min resolution)
-> until_time_seconds                   +--------------------+
   until_time_suffix (w, s, u)          |   untilTimeCode    |
   -> until_time_code   --------------> |   (8-bits)         |
      until_time_minute -------.        +--------------------+
      until_time_suffix --.     \
                           \     --------------------------------.
                             \                                    \
                              \         +--------------------+     |
                               \        | untilTimeModifier  |     |
                                \       |--------------------|    /
                                 -----> | suffix  | minute   | <-'
                                        | (4-bits)| (4-bits) |
                                        +--------------------+


                                        ZoneRule
                                        ========
AT (1-min resolution)
-> at_time_seconds                      +--------------------+
   at_time_suffix (w, s, u)             |   atTimeCode       |
   -> at_time_code   -----------------> |   (8-bits)         |
      at_time_minute -----------.       +--------------------+
      at_time_suffix -----.      \
                           \      -------------------------------.
                             \                                    \
                              \         +--------------------+     |
                               \        |   atTimeModifier   |     |
                                \       |--------------------|    /
                                 -----> | suffix  | minute   | <-'
                                        | (4-bits)| (4-bits) |
                                        +--------------------+

SAVE (15-min resolution)
-> delta_seconds
  -> delta_minutes                      +--------------------+
    -> delta_code + 4 ----------------> |     deltaCode      |
                                        |     (8 bits)       |
                                        +--------------------+
```

### TinyYear Encoding

The `ZoneInfoLow` storage format also implements a space saving measure by
encoding the year fields using a one-byte `int8_t` signed integer offset from
the `baseYear` field specified in the `ZoneContext` object. The range of a
one-byte signed integer is `[-128,127]`, but `-128` is used to represent
`kInvalidYear`, `-127` indicates `-Infinity`, and `+126` and `+127` are used to
represent `+Infinity` (for the `TO` and `UNTIL` fields). So the actual range of
the year offset is `[-126,125]`.

The base year for the `zonedb` and `zonedbx` databases is set to 2100, which
means that these databases can represent all transition rules over the years
`[1974,2225]`. So the `zonedb`, `zonedb2025`, `zonedbx`, and `zonedbx2025`
databases can store DST transitions for the next 200 years, which should be more
than enough for the foreseeable future. If the range needs to be extended, then
the `baseYear` field in `ZoneContext` can be updated, the `zonedb` and `zonedbx`
databases can be either regenerated or new versions of them can be created.

## BasicZoneProcessor

**TBD**: Add information about how the `BasicZoneProcessor` works.

- supports only a limited subset of timezones from the IANA TZ database (448
  zones and links, out of a total of 596, as of TZDB 2023c).
- able to detect gap conditions
- *not* able to detect overlap conditions

## ExtendedZoneProcessor

The `CompleteZoneProcessor` is currently *identical* to the
`ExtendedZoneprocessor` (using a template class). This avoid having to maintain
and test 2 slightly different code bases. This subsection applies to both
`ExtendedZoneProcessor` and `CompleteZoneProcessor`.

The low-level storage formats of the 2 databases (`zonedbx` and `zonedbc`) used
by these 2 classes are actually significantly different (using `ZoneInfoLow` and
`ZoneInfoHigh` formats respectively). But the encapsulation provided by the
Broker layer (i.e. the `ZoneXxxBroker` classes) allow the 2 databases to be
processed by the exactly the same C++ code.

### Search By Component or EpochSeconds

There are 2 ways that a `ZonedDateTime` can be constructed:

1) Using its human-readable components and its timezone through the
`ZonedDateTime::forComponents()` factory method. The human-readable date can be:
    * An undefined time in a DST gap, or
    * A duplicate time during a DST overlap.
2) Using the epochSeconds and its timezone through the
`ZoneDateTime::forEpochSeconds()` factory method.
    * The epochSeconds always specifies a well-defined unique time.

The call stack of the first method looks like this:

```
ZoneDateTime::forComponents()
  -> TimeZone::getOffsetDateTime(PlainDateTime&)
    -> ExtendeZoneProcessor::findByPlainDateTime(PlainDateTime&)
      -> TransitionStorage::findTransitionForDateTime(PlainDateTime&)
```

The call stack of the second method looks like this:

```
ZoneDateTime::forEpochSeconds(acetime_t)
  -> TimeZone::getOffsetDateTime(acetime_t)
    -> ExtendedZoneProcessor::findByEpochSeconds(acetime_t)
      -> TransitionStorage::findTransitionForSeconds(acetime_t)
```

Both the `findTransitionForDateTime()` and `findTransitionForSeconds()` methods
search the list of Transitions of the specified TimeZone for a matching
Transition. Most Date-Time libraries precalculate these Transitions for all
Zones, for a certain range of years. Precalculation is definitely faster, and
easier because the logic for calculating the Transition objects resides in only
a single place.

The precalculation of Transition objects for all zones and years consumes too
much memory on an embedded microcontrollers. To save memory, the
`ExtendedZoneProcessor` class in the AceTime library calculates the `Transition`
objects lazily, for only a single zone and for a single year (technically, for a
14-month interval covering the 12 months of the specified year) when the
`initForEpochSeconds(acetime_t)` or `initForYear(int16_t)` method is called. The
list of Transitions for a single year is cached, so that subsequent requests in
the same year avoid the work of recalculating the Transitions.

The calculation of the Transitions for a given zone and year is very complex,
and I find that I can no longer understand my own code after a few months away
of this code base. So here are some notes to help my future-self. The code is in
`ExtendedZoneProcessor::initForYear(int16_t)` and is organized into 5 steps as
described below.

### Step 1: Find Matches

The `ExtendedZoneProcessor::findMatches()` finds all `ZoneEra` objects which
overlap with the 14-month interval from Dec 1 of the prior year until Feb 1 of
the following year. For example, `initForYear(2010)` means that the interval is
from 2009-12-01T00:00 until 2011-02-01T00:00.

A 14-month interval is chosen because a plain date time of Jan 1 could land in
the prior year after the correct UTC offset is calculated, so we need to pick up
Transitions in the prior year. Similarly, a plain date time of Dec 31 could land
in the following year after correcting for UTC offset.

A `MatchingEra` is a wrapper around a `ZoneEra`, with its startDateTime and
untilDateTime truncated to be within the 14-month interval of interest.

### Step 2: Create Transitions

The class creates an array of `Transition` objects spanning 14 months that
covers the given `year`, from 12/1 of the previous year until 2/1 of the
following year. The extra month at the start and end of the one-year interval is
to account for the fact that a plain DateTime of 1/1 for a given year may
actually occur in the previous year after shifting to UTC time. But the amount
of shift is not known until we calculate the Transitions. By starting the
interval of interest at 12/1, we make sure that correct Transition is determined
for 12/31 if needed. Similarly, a plain DateTime of 12/31 may actually occur on
1/1 of the following year, so we extend our time interval of interest to 2/1 of
the following year.

Here is an example of a Zone where for the given year `y`, there are 3 matching
ZoneEras: E1, E2, E3.

```
                                <--------------------------------- E1 --
      1/1                                                 12/1       12/31
       +----------------------------------------------------[----------+
     E1|                                                    [..........|
       |----------------------------------------------------[----------|
y-1  R2|             ^                      v               [          |
       |----------------------------------------------------[----------|
     R3|                                                 ^  [   v      |
       +----------------------------------------------------[----------+


       <--- E1 --)[--------------- E2 ------------)[------ E3 --------->
                m1/d1                            m2/d2
       +---------)[-------------------------------)[-------------------+
     E1|.........)[                               )[                   |
       |---------)[-------------------------------)[-------------------|
 y   R2|         )[..^......................v.....)[                   |
       |---------)[-------------------------------)[-------------------|
     R3|         )[                               )[.....^......v......|
       +---------)[-------------------------------)[-------------------+


       -- E3 ---------------------------------------->
                2/1
       +---------)-----------------------------------------------------+
     E1|         )                                                     |
       |---------)-----------------------------------------------------|
y+1  R2|         )   ^                      v                          |
       |---------)-----------------------------------------------------|
     R3|.........)                                       ^      v      |
       +---------)-----------------------------------------------------+
```

The `findMatches()` returns the list of `ZoneEra` which overlap with the
14-month interval from 12/1 to 2/1. In this case, it will return E1, E2 and E3.
For each ZoneEra, the algorithm creates the appropriate Transition objects at
the appropriate boundaries.

In the example above, the E1 era is a simple `ZoneEra` which does not use a
ZoneRule. The `STDOFF` and the `RULES` columns directly give the UTC offset and
the DST offset. The Transition object can be created directly for 12/1.

The E2 era contains various `ZoneRule` entries, R2, which are recurring rules
that occur every year. This particular Zone switches from E1 to E2 at m/d, which
is slightly different than one of the Transition rules specified in R2. To
determine the Transition that applies at m1/d1, we must go back to the "most
recent prior" Transition of R2, which occurred in the prior year. We take that
Transition, the shift it to m1/d1 to get the effective Transition at m1/d1.

Sometimes (often?), the switch from one ZoneEra to another happens at exactly
the same time as one of the Transitions specified by the ZoneRule of the next
ZoneEra. The code that checks for this situation is located at
`ExtendedZoneProcessor::compareTransitionToMatch()` and was recently (v1.8)
updated so that equality between a `Transition` and its `ZoneEra` is accepted
if *any* of the `w`, `s` or `u` times match each other, instead of relying on
just the `w` time. This prevents extraneous Transition objects are not created.
(We don't want to switch to a new ZoneEra with a new ZonePolicy, then
immediately switch to a different Transition due to the Rule in the new
ZonePolicy.)

The E3 era for this Zone begins at m2/d2 in the above example, which is
different than the Transitions defined by the R3 rules. Similar to E2, we must
calculate the Transition at m2/d2 using the "most recent prior" Transition,
which happens to occur in the previous year. Note that the "most recent prior"
Transition may have happened many years prior to the current year if the
matching ZoneRule happened many years prior.

Putting all this together, here is the final list of 7 Transitions which are
needed for this Zone, for this given year `y`:

```
                                <--------------------------------- E1 --
      1/1                                                 12/1       12/31
       +----------------------------------------------------[----------+
     E1|                                                    x          |
       |----------------------------------------------------[----------|
y-1  R2|                                                    [          |
       |----------------------------------------------------[----------|
     R3|                                                    [          |
       +----------------------------------------------------[----------+


       <--- E1 --)[--------------- E2 ------------)[------ E3 --------->
                m1/d1                            m2/d2
       +---------)[-------------------------------)[-------------------+
     E1|.........)[                               )[                   |
       |---------)[-------------------------------)[-------------------|
 y   R2|          x  ^                      v     )[                   |
       |---------)[-------------------------------)[-------------------|
     R3|         )[                               )x     ^      v      |
       +---------)[-------------------------------)[-------------------+


       -- E3 ------------------------------------->
                2/1
       +---------)-----------------------------------------------------+
     E1|         )                                                     |
       |---------)-----------------------------------------------------|
y+1  R2|         )                                                     |
       |---------)-----------------------------------------------------|
     R3|         )                                                     |
       +---------)-----------------------------------------------------+
```

### Step 2A: Transition Time versus StartDateTime

Many time stamps in the zonedb database are tricky to handle because they
must be interpreted with the correct UTC offsets.

Suppose we have 2 ZoneEras, which get converted into 2 MatchingEra (with
truncated start and until years). Each MatchingEra contains only its
untilDateTime which becomes copied over to the next MatchingEra's
startDateTime. For example, in the diagram below, S2 is set to be equal to U1.
The tricky part is that the startDateTime of a MatchingEra must be interpreted
using the UTC offset of the *previous* MatchingEra.

MatchingEra (E1) contains its Rules (R1) which generate the set of Transitions
(T1X). MatchingEra (E2) contains Rules (R2) which generate Transitions (T2X).
Similar to the startDateTime of the MatchingEra, the transitionTime (T1X, T2X)
of each Transition must be interpreted using the UTC offset of the *previous*
Transition.

Given 2 MatchingEra objects, the set of Transitions may initially start like
this:

```
 S1    E1                    U1S2      E2                            U2
  |---------------------------)|-------------------------------------)

                      T1b             T1c
R1   ----------------)|--------------)|-------------->

          T2b                  T2c             T2d
R2  -----}|-------------------)|--------------)|------------------------>
```

When a Zone switches from one MatchingEra (E1) to another (E2), the Transition
objects must be collapsed in a manner that makes sense. One of the most tricky
part of the merge process is determining whether a Transition happens at the
same time as a switch to a different MatchingEra.

If T2c is determined to happen at the same time as U1/S2, then the combined
Transition graph looks like this, where T1b is truncated to the end of U1, and
immediately moves into T2c.

```
 S1    E1                    U1S2      E2                            U2
  |---------------------------)|-------------------------------------)

                      T1b      T2c             T2d
     ----------------)|-------)|--------------)|------------------------>
```

However, if T2c is determined to happen *after* U1/S2, then an extra Transition
(T2b) is picked up, like this:

```
 S1    E1                    U1S2      E2                            U2
  |---------------------------)|-------------------------------------)

                      T1b      T2b T2c         T2d
     ----------------)|-------)|--)|----------)|------------------------>
```

### Step 3: Fix Transition Times

After obtaining the combined list of Transitions, a final pass converts the
transitionTime of all Transition (with any additional truncations and
adjustments) into the wall time using the UTC offset of the *previous*
Transition.

### Step 4: Generate Start Until Times

After the list of Transitions is created, the `Transition.startDateTime`
and `Transition.untilDateTime` created using the transtionTime field.

- The `untilDateTime` of the previous Transition is the current `transitionTime`
  shifted into the UTC offset of the -previous- Transition.
- The `startDateTime` of the current Transition is the current `transitionTime`
  shifted into the UTC offset of the -current- Transition.

### Step 5: Calculate Abbreviations

Finally, the time zone abbreviations (e.g. "PST", "CET") are calculated for each
Transition and recorded into the `Transition.abbrev` fixed sized array.

The original TZDB specification used to say that the abbreviation could be 3-6
characters, so this field used to be an array 7 characters (to account for the
terminating NUL character). That blub about 3-6 characters no longer seems to
exist.

When support for `%z` was added, we needed to support formats of the form
`(+/-)hh[mm[ss]]` which can be 7 characters long. So the size of `abbrev` is now
8 (as specified in [kAbbrevSize](src/ace_time/common/common.h)).
