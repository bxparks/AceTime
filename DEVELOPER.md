# Developer Notes

Information which are useful for developers and maintainers of the AceTime
library.

## Table of Contents

* [Project/Repo Dependency](#ProjectRepoDependency)
* [Namespace Dependency](#NamespaceDependency)
* [Zone Info Database](#ZoneInfoDatabase)
    * [Template Layer](#TemplateLayer)
    * [Storage Layer](#StorageLayer)
    * [Broker Layer](#BrokerLayer)
    * [ZoneDb Files](#ZoneDbFiles)
        * [ZoneInfo and ZoneEra](#ZoneInfoZoneEra)
        * [ZonePolicy and ZoneRule](#ZonePolicyZoneRule)
        * [ZoneRegistry](#ZoneRegistry)
    * [Offset Encoding](#OffsetEncoding)
    * [TinyYear Encoding](#TinyYearEncoding)
* [BasicZoneProcessor](#BasicZoneProcessor)
* [ExtendedZoneProcessor](#ExtendedZoneProcessor)
    * [Search by Component or EpochSeconds](#SearchByComponentOrEpochSeconds)
    * [Stea 1: Find Matches](#Step1FindMatches)
    * [Step 2: Create Transitions](#Step2CreateTransitions)
    * [Step 2A: Transition Time versus StartDateTime](#Step2ATransitionTimeAndStartDateTime)
    * [Step 3: Fix Transition Times](#Step3FixTransitionTimes)
    * [Step 4: Generate Start Until Times](#Step4GenerateStartUntilTimes)
    * [Step 5: Calculate Abbreviations](#Step5CalculateAbbreviations)
* [Upgrading ZoneInfo Files to a New TZDB Version](#UpgradingZoneInfoFiles)
* [Release Process](#ReleaseProcess)

<a name="ProjectRepoDependency"></a>
## Project/Repo Dependency

On 2021-08-25, the scripts under `./tools` were moved into the
[AceTimeTools](https://github.com/bxparks/AceTimeTools/) project, and the
integration tests under `./tests/validation` were moved into the
[AceTimeValidations](https://github.com/bxparks/AceTimeValidation) project. Then
on 2021-09-08, the Python timezone classes (`zone_processor.py`, `acetz.py`,
etc) were moved into the
[acetimepy](https://github.com/bxparks/acetimepy) project.

Here is the dependency diagram among these projects.

```
                    AceTimeTools --------
                    ^    ^   ^           \ artransformer.py
        creating   /     |    \ creating  \ -> bufestimator.py
        zonedb*   /      |     \ zonedb   /  -> zone_processor.py
                 /       |      \        v
             AceTime     |      acetimepy
             ^    ^      |      ^
            /      \     |     /
           /        \    |    /
AceTimeClock    AceTimeValidation
```

There is slight circular dependency between `AceTimeTools` and `acetimepy`.

AceTimeTools needs acetimepy when generating the C++ zoneinfo files under
`AceTime/src/zonedb[x]`. The `tzcompiler.py` calls `bufestimator.py` to generate
the buffer sizes needed by the C++ `ExtendedZoneProcessor` class. The
`AceTimeTools/bufestimator.py` module needs `acetimepy/zone_processor.py`
module to calculate those buffer sizes.

On the other hand, acetimepy needs AceTimeTools to generate the zoneinfo
files under `acetimepy/zonedb/`, which are consumed by the `acetz` and
`ZoneManager` classes. Fortunately, acetimepy does *not* need AceTimeTools
during runtime, so 3rd party consumers can incorporate acetimepy without pulling
in AceTimeTools.

Both AceTime and acetimepy can be used as runtime libraries **without**
pulling in the dependency to AceTimeTools (which is required only to generated
the zoneinfo database files).

<a name="NamespaceDependency"></a>
## Namespace Dependency

The various AceTime namespaces are related in the following way, where the arrow
means "depends on":

```
           (storage layer)
           ace_time::zoneinfolow
           ace_time::zoneinfomid
           ace_time::zoneinfohigh
                   ^
                   |
             (broker layer)
             ace_time::basic
             ace_time::extended
             ace_time::complete
                   ^      ^
                   |       \
                   |      (datafile layer)
                   |      ace_time::zonedb
                   |      ace_time::zonedbx
ace_time::internal |      ace_time::zonedbc
              ^    |       ^
               \   |      /
ace_time::hw    ace_time::
      ^         ^      ^
      |        /        \
ace_time::clock     ace_time::testing
```

<a name="ZoneInfoDatabase"></a>
## Zone Info Database

<a name="TemplateLayer"></a>
### Template Layer

The template layer provides low-level building blocks that define the storage
formats of various fields, records, and tables used by the zoneinfo databases
in the AceTime library. These should never be seen directly by the client
application.

There are 5 major data types that are stored in the zoneinfo database:
`ZoneContext`, `ZoneRule`, `ZonePolicy`, `ZoneEra` and `ZoneInfo`. These data
types are analogous to tables in a relational database. There are 3
implementations defined, corresponding to different resolution levels supported
by each set:

* `ZoneInfoLow.h`
    * low resolution
        * 1-minute resolution for AT, UNTIL, STDOFF; 15-minute resolution for
          DST offsets
        * year fields using 1-byte offset from a `baseYear` of 2100,
          supporting the years `[1973,2226]`
    * `zoneinfolow::ZoneContext<>`
    * `zoneinfolow::ZoneRule<>`
    * `zoneinfolow::ZonePolicy<>`
    * `zoneinfolow::ZoneEra<>`
    * `zoneinfolow::ZoneInfo<>`
* `ZoneInfoMid.h`
    * medium resolution persistence format
        * 1-minute resolution for AT, UNTIL, STDOFF; 15-minute resolution for
          DST offset)
        * 2-byte year fields supporting years `[-32767,32766]`
    * `zoneinfomid::ZoneContext<>`
    * `zoneinfomid::ZoneRule<>`
    * `zoneinfomid::ZonePolicy<>`
    * `zoneinfomid::ZoneEra<>`
    * `zoneinfomid::ZoneInfo<>`
* `ZoneInfoHigh.h`
    * high resolution persistence format
        * 1-second resolution for AT, UNTIL, STDOFF, and DST offsets
        * 2-byte year fields supporting years `[-32767,32766]`
    * `zoneinfohigh::ZoneContext<>`
    * `zoneinfohigh::ZoneRule<>`
    * `zoneinfohigh::ZonePolicy<>`
    * `zoneinfohigh::ZoneEra<>`
    * `zoneinfohigh::ZoneInfo<>`

Wrapping each of these low-level persistent classes are the "broker" layer
classes. They convert the low-level storage formats into a consistent API using
identical types and integer sizes. The allows the code in the `src/ace_time`
layer to agnostic to the exact storage format of the zoneinfo database.

* `BrokersLow.h`
    * `zoneinfolow::ZoneContextBroker<>`
    * `zoneinfolow::ZoneRuleBroker<>`
    * `zoneinfolow::ZonePolicyBroker<>`
    * `zoneinfolow::ZoneEraBroker<>`
    * `zoneinfolow::ZoneInfoBroker<>`
    * `zoneinfolow::ZoneRegistryBroker`
    * `zoneinfolow::ZoneInfoStore`
* `BrokersMid.h`
    * `zoneinfomid::ZoneContextBroker<>`
    * `zoneinfomid::ZoneRuleBroker<>`
    * `zoneinfomid::ZonePolicyBroker<>`
    * `zoneinfomid::ZoneEraBroker<>`
    * `zoneinfomid::ZoneInfoBroker<>`
    * `zoneinfomid::ZoneRegistryBroker`
    * `zoneinfomid::ZoneInfoStore`
* `BrokersHigh.h`
    * `zoneinfohigh::ZoneContextBroker<>`
    * `zoneinfohigh::ZoneRuleBroker<>`
    * `zoneinfohigh::ZonePolicyBroker<>`
    * `zoneinfohigh::ZoneEraBroker<>`
    * `zoneinfohigh::ZoneInfoBroker<>`
    * `zoneinfohigh::ZoneRegistryBroker`
    * `zoneinfohigh::ZoneInfoStore`

All of these classes are templatized, so that custom instantiations can be
created for different zoneinfo databases, which can be verified by the compiler
to be used together in the proper way.

<a name="StorageLayer"></a>
### Storage Layer

This is the actual storage layer used by library, instantiated from the template
classes. These classes are defined by the
[zoneinfo/infos.h](src/zoneinfo/infos.h) file:

* Basic
    * `basic::ZoneContext`
    * `basic::ZoneRule`
    * `basic::ZonePolicy`
    * `basic::ZoneEra`
    * `basic::ZoneInfo`
* Extended
    * `extended::ZoneContext`
    * `extended::ZoneRule`
    * `extended::ZonePolicy`
    * `extended::ZoneEra`
    * `extended::ZoneInfo`
* Complete
    * `complete::ZoneContext`
    * `complete::ZoneRule`
    * `complete::ZonePolicy`
    * `complete::ZoneEra`
    * `complete::ZoneInfo`

These classes are intended to be hidden from the client application. The only
exception is the `const xxx::ZoneInfo*` pointer, which is used as an opaque
identifier for a timezone. From this pointer, a `TimeZone` object can be
created.

<a name="BrokerLayer"></a>
### Broker Layer

This is the actual broker layer used by library, instantiated from the template
classes. These classes are defined by the
[zoneinfo/brokers.h](src/zoneinfo/brokers.h) file:

* Basic
    * uses `zoneinfolow::` classes
    * `basic::ZoneContextBroker`
    * `basic::ZoneRuleBroker`
    * `basic::ZonePolicyBroker`
    * `basic::ZoneEraBroker`
    * `basic::ZoneInfoBroker`
    * `basic::ZoneRegistryBroker`
    * `basic::ZoneInfoStore`
* Extended
    * uses `zoneinfolow::` classes (previously used `zoneinfomid::`)
    * `extended::ZoneContextBroker`
    * `extended::ZoneRuleBroker`
    * `extended::ZonePolicyBroker`
    * `extended::ZoneEraBroker`
    * `extended::ZoneInfoBroker`
    * `extended::ZoneRegistryBroker`
    * `extended::ZoneInfoStore`
* Complete
    * uses `zoneinfohigh::` classes
    * `complete::ZoneContextBroker`
    * `complete::ZoneRuleBroker`
    * `complete::ZonePolicyBroker`
    * `complete::ZoneEraBroker`
    * `complete::ZoneInfoBroker`
    * `complete::ZoneRegistryBroker`
    * `complete::ZoneInfoStore`

(TODO: It might be possible to wrap the raw timezone `const ZoneInfo*` pointer
into a `ZoneInfoBroker` object, and use the broker object as the timezone
identifier. This would completely hide the low-level `const ZoneInfo*` pointer
from the client application.)

<a name="ZoneDbFiles"></a>
### ZoneDb Files

The AceTime library comes with 3 sets of zoneinfo files, which were
programmatically generated by the scripts in
[AceTimeTools](https://github.com/bxparks/AceTimeTools/) from the [IANA TZ
Data](https://www.iana.org/time-zones):

* `src/zonedb/*`
    * uses the `zonedb::` namespace
    * used by `BasicZoneProcessor`
    * uses `basic::ZoneXxx` classes
* `src/zonedbx/*`
    * uses the `zonedbx::` namespace
    * used by `ExtendedZoneProcessor`
    * uses `extended::ZoneXxx` classes
* `src/zonedbc/*`
    * uses the `zonedbc::` namespace
    * used by `CompleteZoneProcessor`
    * uses `complete::ZoneXxx` classes

Each `zonedb*/` directory contains the following files:

* `zone_infos.h', `zone_infos.cpp`
* `zone_policies.h`, `zone_policies.cpp`
* `zone_registry.h`, `zone_registry.cpp`

<a name="ZoneInfoZoneEra"></a>
#### ZoneInfo and ZoneEra

The `zone_infos.h` and `zone_infos.cpp` files contain a `ZoneInfo` record for
each supported time zone corresponding to a `Zone` or `Link` entry in the IANA
TZ database. Each `ZoneInfo` aggregates one ore more `ZoneEra` records.

The zone identifier has the form `kZone{region}_{city}`, where the
`{region}_{city}` comes directly from the TZ Database. Some minor character
transformations are applied to create an valid C++ identifier. For example, all
dashes `-` are converted to underscore `_`. As an exception, the `+` character
is converted to `_PLUS_` to differentiate "Etc/GMT+1" from "Etc/GMT-1", and so
on.

The `kTzDatabaseVersion` string constant identifies the version of the TZ
Database that was used to generate these files, e.g. "2023c".

The `kZoneContext` variable points to an instance of `xxx::ZoneContext`
which identifies the `startYear` and `untilYear` of the current database.

Near end of the `zone_info.h` file, we list the zones which were deliberately
excluded by the tool. Also at the end of the `zone_info.h` file, there may be
warnings about known inaccuracies for a particular zone.

<a name="ZonePolicyZoneRule"></a>
#### ZonePolicy and ZoneRule

The `zone_policies.h` and `zone_policies.cpp` hole the `RULE` entries from the
IANA TZ database. A `ZonePolicy` is a collection of one or more `ZoneRule`
records, which each `ZoneRule` corresponding to a single line of the `RULE`
entry in the TZ database. A `ZoneEra` record may hold a pointer to a
`ZonePolicy` record. For example, the `kZoneAmerica_Los_Angeles` has a pointer
to a `kZonePolicyUS` record.

<a name="ZoneRegistry"></a>
#### Zone Registry

The `zone_registry.h` and `zone_registry.cpp` files contain 2 pre-defined
registries of timezones:

* `const ZoneInfo* const kZoneRegistry[kZoneRegistrySize]`
    * contains a list of all `Zone` entries
* `const ZoneInfo* const kZoneAndLinkRegistry[kZoneAndLinkRegistrySize]`
    * contains a list of all `Zone` and `Link` entries

Due to reasons which are too complicated to explain here, `Zone` and `Link`
entries should treated with the same priority. Client applications should almost
always use the `kZoneAndLinkRegistry`. The only exception may be testig
applications which may want to use the smaller `kZoneRegistry` to achieve
complete coverage of all timezones with the same set of rules, without
duplicates.

<a name="OffsetEncoding"></a>
### Offset Encoding

The `zoneinfolow` storage format was optimized for small size. A number of
fields which required only 4-bits of space were combined together to save memory
space. There are 5 offsets and moment-in-time quantities from the TZ zoneinfo
files which are captured in the `zone_info.{h,cpp}` and `zone_policies.{h,cpp}`
files:

* `STDOFF` field in `Zone` entry (previously `OFFSET`), 1-minute resolution
* `RULES` field in `Zone` entry when numeric (e.g. "1:00"), 15-minute resolution
* `UNTIL` field in `Zone` entry, 1-minute resolution
* `SAVE` field in `Rule` entry, 15-minute resolution
* `AT` field in `Rule` entry, 1-minute resolution

To reduce flash memory size, these fields are encoded in non-obvious ways which
are difficult to remember. Here is my attempt to document the encoding. In the
following diagram, a field labled `code` (e.g. `offsetCode`) has a unit of 15
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

<a name="TinyYearEncoding"></a>
### TinyYear Encoding

The `zoneinfolow` storage format also implements a space saving measure by
encoding the year fields using a one-byte `int8_t` signed integer offset from
the `baseYear` field specified in the `ZoneContext` object. The range of a
one-byte signed integer is `[-128,127]`, but `-128` is used to represent
`kInvalidYear`, `-127` indicates `-Infinity`, and `+126` and `+127` are used to
represent `+Infinity` (for the `TO` and `UNTIL` fields). So the actual range of
the year offset is `[-126,125]`. The base year for the `zonedb` and `zonedbx`
databases is set to 2100, which means that these databases can represent all
transition rules over the years `[1974,2225]`.

So the `zonedb` and `zonedbx` databases can store DST transitions for the next
200 years, which should be more than enough for the foreseeable future. If the
range needs to be extended, then the `baseYear` field in `ZoneContext` can be
updated, the `zonedb` and `zonedbx` databases can be either regenerated or new
versions of them can be created.

<a name="BasicZoneProcessor"></a>
## BasicZoneProcessor

**TBD**: Add information about how the `BasicZoneProcessor` works.

- supports only a limited subset of timezones from the IANA TZ database (448
  zones and links, out of a total of 596, as of TZDB 2023c).
- able to detect gap conditions
- *not* able to detect overlap conditions

<a name="ExtendedZoneProcessor"></a>
## ExtendedZoneProcessor

The `CompleteZoneProcessor` is currently *identical* to the
`ExtendedZoneprocessor` (using a template class). This avoid having to maintain
and test 2 slightly different code bases. This subsection that describes the
workings of `ExtendedZoneProcessor` applies without modification to
`CompleteZoneProcessor`.

The low-level storage formats of the 2 databases (`zonedbx` and `zonedbc`) used
by these 2 classes are actually signficantly different (using `zoneinfolow` and
`zoneinfohigh` formats respectively). But the encapsulation provided by the
Broker layer (i.e. the `ZoneXxxBroker` classes) allow the 2 databases to be
processed by the exactly the same C++ code.

<a name="SearchByComponentOrEpochSeconds"></a>
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
  -> TimeZone::getOffsetDateTime(LocalDateTime&)
    -> ExtendeZoneProcessor::findByLocalDateTime(LocalDateTime&)
      -> TransitionStorage::findTransitionForDateTime(LocalDateTime&)
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

<a name="Step1FindMatches"></a>
### Step 1: Find Matches

The `ExtendedZoneProcessor::findMatches()` finds all `ZoneEra` objects which
overlap with the 14-month interval from Dec 1 of the prior year until Feb 1 of
the following year. For example, `initForYear(2010)` means that the interval is
from 2009-12-01T00:00 until 2011-02-01T00:00.

A 14-month interval is chosen because a local date time of Jan 1 could land in
the prior year after the correct UTC offset is calculated, so we need to pick up
Transitions in the prior year. Similarly, a local date time of Dec 31 could land
in the following year after correcting for UTC offset.

A `MatchingEra` is a wrapper around a `ZoneEra`, with its startDateTime and
untilDateTime truncated to be within the 14-month interval of interest.

<a name="Step2CreateTransitions"></a>
### Step 2: Create Transitions

The class creates an array of `Transition` objects spanning 14 months that
covers the given `year`, from 12/1 of the previous year until 2/1 of the
following year. The extra month at the start and end of the one-year interval is
to account for the fact that a local DateTime of 1/1 for a given year may
actually occur in the previous year after shifting to UTC time. But the amount
of shift is not known until we calculate the Transitions. By starting the
interval of interest at 12/1, we make sure that correct Transition is determined
for 12/31 if needed. Similarly, a local DateTime of 12/31 may actually occur on
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

<a name="Step2ATransitionTimeAndStartDateTime"></a>
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

<a name="Step3FixTransitionTimes"></a>
### Step 3: Fix Transition Times

After obtaining the combined list of Transitions, a final pass converts the
transitionTime of all Transition (with any additional truncations and
adjustments) into the wall time using the UTC offset of the *previous*
Transition.

<a name="Step4GenerateStartUntilTimes"></a>
### Step 4: Generate Start Until Times

After the list of Transitions is created, the `Transition.startDateTime`
and `Transition.untilDateTime` created using the transtionTime field.

* The `untilDateTime` of the previous Transition is the current `transitionTime`
  shifted into the UTC offset of the *previous* Transition.
* The `startDateTime` of the current Transition is the current `transitionTime`
  shifted into the UTC offset of the *current* Transition.

<a name="Step5CalculateAbbreviations"></a>
### Step 5: Calculate Abbreviations

Finally, the time zone abbreviations (e.g. "PST", "CET") are calculated for each
Transition and recorded into the `Transition.abbrev` fixed sized array. The TZDB
specification says that the maximum length of an abbreviation is 6, so this
field is a static array of 7 characters (to account for the terminating NUL
character).

<a name="UpgradingZoneInfoFiles"></a>
## Upgrading ZoneInfo Files to a New TZDB Version

About 2-4 times a year, a new TZDB version is released. Here are some notes
(mostly for myself) on how to create a new release after a new TZDB version is
available.

* Update the TZDB repo (https://github.com/eggert/tz). This should be a
  sibling to the `AceTime` repo (since it will cause the least problems
  for various internal scripts):
    * `$ cd ../tz`
    * `$ git pull`
    * Check that the correct tag is pulled (e.g. `2020c` tag if that's the
      version that we want to upgrade to).
* Update the Hinnant date repo (https://github.com:HowardHinnant/date). This
  should be a sibling to the `AceTime` repo:
    * `$ cd ../date`
    * `$ git pull`
* Update the zonedb files for acetimepy (needed by AcetzBasicTest and
  AcetzExtendedTest):
    * `$ cd acetimepy/src/acetime/zonedb`
    * Update the `TZ_VERSION` variable in `Makefile`.
    * `$ make`
* Verify that `AceTimeValidation` passes (which compares AceTime with
  acetimepy and the Hinnant `date` library):
    * `$ cd ../AceTimeValidation`
    * Update the `TZ_VERSION` variable in the following files:
        * `tests/AcetzBasicTest/Makefile`
        * `tests/AcetExtendedzTest/Makefile`
        * `tests/HinnantBasicTest/Makefile`
        * `tests/HinnantExtendedTest/Makefile`
    * `$ make clean`
    * `$ make validations`
    * `$ make runvalidations`
* Update the various zoneinfo files for AceTime:
    * `$ cd AceTime/src`
    * Update the `TZ_VERSION` variable in `Makefile`.
    * `$ make`
* Update CHANGELOGs
    * AceTime/CHANGELOG.md
    * acetimepy/CHANGELOG.md
    * AceTimeValidation/CHANGELOG.md
* Commit the changes to git
    * `$ git add ...`
    * `$ git commit -m "..."`

There are 12 other validation tests in the AceTimeValidation project that
compare AceTime with various other third party libraries:

* `DateUtilBasicTest`
* `DateUtilExtendedTest`
* `GoBasicTest`
* `GoExtendedTest`
* `JavaBasicTest`
* `JavaExtendedTest`
* `NodaBasicTest`
* `NodaExtendedTest`
* `PytzBasicTest`
* `PytzExtendedTest`
* `ZoneInfoBasicTest`
* `ZoneInfoExtendedTest`

Unfortunately, they all seem to use the underlying TZDB version provided by the
Operating System, and I have not been able to figure out how to manually update
this dependency manually. When a new TZDB is released, all of these tests will
fail until the underlying timezone database of the OS is updated.

<a name="ReleaseProcess"></a>
## Release Process

* Update `examples/MemoryBenchmark` and `examples/AutoBenchmark`.
* Update and commit the version numbers in various files:
    * `src/AceTime.h`
    * `README.md`
    * `USER_GUIDE.md`
    * `MIGRATING.md`
    * `CHANGELOG.md`
    * `docs/doxygen.cfg`
    * `library.properties`
    * `$ git commit -m "..."`
    * `$ git push`
* Update and commit the Doxygen docs. This is done as a separate git commit
  because the Doxygen changes are often so large that they obscure all other
  important changes to the code base:
    * `$ cd docs`
    * `$ make clean`
    * `$ make`
    * `$ git add .`
    * `$ git commit -m "..."`
    * `$ git push`
* (Optional) Create a new Release of acetimepy
    * (This should be done first, before AceTime)
    * Go to https://github.com/bxparks/acetimepy
    * Bump version number on `develop`.
    * Merge `develop` into `master`.
    * Click on "Releases"
    * Click on "Draft a new release"
    * Enter a tag version (e.g. `v1.2`), targeting the `master` branch.
    * Enter the release title.
    * Enter the release notes. I normally just copy and paste the latest changes
      from `CHANGELOG.md`.
    * Click Publish release.
* (Optional) Create a new Release of AceTimeTools
    * (Depends on acetimepy)
    * Go to https://github.com/bxparks/AceTimeTools
    * Click on "Releases"
    * Click on "Draft a new release"
    * Enter a tag version (e.g. `v1.2`), targeting the `master` branch.
    * Enter the release title.
    * Enter the release notes. I normally just copy and paste the latest changes
      from `CHANGELOG.md`.
    * Click Publish release.
* (Optional) Create a new Release of AceTimeValidation.
    * (Depends on acetimepy)
    * Go to https://github.com/bxparks/AceTimeTools
    * Click on "Releases"
    * Click on "Draft a new release"
    * Enter a tag version (e.g. `v1.2`), targeting the `master` branch.
    * Enter the release title.
    * Enter the release notes. I normally just copy and paste the latest changes
      from `CHANGELOG.md`.
    * Click Publish release.
* Create a new Release of AceTime (third, depends on AceTimeValidation).
    * (Depends on acetimepy, AceTimeValidation)
    * Go to https://github.com/bxparks/AceTime
    * Merge the `develop` branch into `master` by creating a Pull Request.
    * Approve and merge the PR.
    * Click on "Releases"
    * Click on "Draft a new release"
    * Enter a tag version (e.g. `v1.2`), targeting the `master` branch.
    * Enter the release title.
    * Enter the release notes. I normally just copy and paste the latest changes
      from `CHANGELOG.md`.
    * Click Publish release.
* Add corresponding tags on acetimepy, AceTimeTools and AceTimeValidation
  for reference.
    * acetimepy
        * `$ git tag -a 'atX.Y.Z' -m 'AceTime vX.Y.Z'`
        * `$ git push --tags`
    * AceTimeTools
        * `$ git tag -a 'atX.Y.Z' -m 'AceTime vX.Y.Z'`
        * `$ git push --tags`
    * AceTimeValidation
        * `$ git tag -a 'atX.Y.Z' -m 'AceTime vX.Y.Z'`
        * `$ git push --tags`
