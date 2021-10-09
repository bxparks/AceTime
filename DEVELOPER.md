# Developer Notes

Information which are useful for developers and maintainers of the AceTime
library.

## Table of Contents

* [Project/Repo Dependency](#ProjectRepoDependency)
* [Namespace Dependency](#NamespaceDependency)
* [Zone DB Files](#ZoneDbFiles)
* [Encoding Offset and Time Fields](#EncodingOffsetAndTimeFields)
* [ExtendedZoneProcessor](#ExtendedZoneProcessor)
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
[AceTimePython](https://github.com/bxparks/AceTimePython) project.

Here is the dependency diagram among these projects.

```
                    AceTimeTools --------
                    ^    ^   ^           \ artransformer.py
        creating   /     |    \ creating  \ -> bufestimator.py
        zonedb[x] /      |     \ zonedbpy  \ -> zone_processor.py
                 /       |      \           v
              AceTime    |      AceTimePython
             ^    ^      |      ^
            /      \     |     /
           /        \    |    /
AceTimeClock    AceTimeValidation
```

There is slight circular dependency between `AceTimeTools` and `AceTimePython`.

AceTimeTools needs AceTimePython when generating the C++ zoneinfo files under
`AceTime/src/zonedb[x]`. The `tzcompiler.py` calls `bufestimator.py` to generate
the buffer sizes needed by the C++ `ExtendedZoneProcessor` class. The
`AceTimeTools/bufestimator.py` module needs `AceTimePython/zone_processor.py`
module to calculate those buffer sizes.

On the other hand, AceTimePython needs AceTimeTools to generate the zoneinfo
files under `AceTimePython/zonedbpy`, which are consumed by the `acetz.py`
module. Fortunately, AceTimePython does *not* need AceTimeTools during runtime,
so 3rd party consumers can incorporate AceTimePython without pulling in
AceTimeTools.

Both AceTime and AceTimePython can be used as runtime libraries **without**
pulling in the dependency to AceTimeTools (which is required only to generated
the zoneinfo database files).

<a name="NamespaceDependency"></a>
## Namespace Dependency

The various AceTime namespaces are related in the following way, where the arrow
means "depends on":

```
ace_time::clock     ace_time::testing
      |       \        /
      |        v      v
      |        ace_time
      |         |\     \
      |         | \     v
      |         |  \    ace_time::zonedb
      |         |   \   ace_time::zonedbx
      |         |    \     |
      v         |     v    v
ace_time::hw    |     ace_time::basic
          \     |     ace_time::extended
           \    |     /
            \   |    /
             v  v   v
           ace_time::logging
```

<a name="ZoneDbFiles"></a>
## Zone DB Files

As explained in the README.md, the AceTime library comes with 2 versions of the
[TZ Data](https://www.iana.org/time-zones):

* `src/ace_time/zonedb/*`: files used by the `BasicZoneProcessor` class
* `src/ace_time/zonedbx/*`: files used by the `ExtendedZoneProcessor` class

There are 2 main files in these directories:

* `zone_infos.h', `zone_infos.cpp`
* `zone_policies.h`, `zone_policies.cpp`

The format and meaning of these files are probably best explained in the header
files:

* `src/ace_time/internal/ZoneInfo.h`
* `src/ace_time/internal/ZonePolicy.h`

Some of the tricky encoding schemes were created to preserve resolution, while
keeping the binary size of the data structures as small as possible. For
example, these files are able to support DST transitions with a 1-minute
resolution, but the DST offsets have only a 15-minute resolution. Fortunately,
these limitations are sufficient to represent all time zones since about 1972.

Other bits of information are added by the code generator as comments in the
generated files and some of these are explained below.

### `zone_infos.h`

This file contains one entry of the `basic::ZoneInfo` or
`extended::ZoneInfo` data structure for each supported time zone.

The zone identifier is named `kZone{region}_{city}`, where the
`{region}_{city}` comes directly from the TZ Database. Some minor character
transformations are applied to create an valid C++ identifier. For example, all
dashes `-` are converted to underscore `_`. As an exception, the `+` character
is converted to `_PLUS_` to differentiate "Etc/GMT+1" from "Etc/GMT-1", and so
on.

The `kTzDatabaseVersion` string constant identifies the version of the TZ
Database that was used to generate these files, e.g. "2019a".

The `kZoneContext` variable points to an instance of `common::ZoneContext`
which identifies the `startYear` and `untilYear` of the current database. These
will normally be 2000 and 2050 respectively, but a custom version of the
zonedb files could be generated whose startYear and endYear could be smaller (to
produce smaller ZoneDB files).

Near end of the `zone_info.h` file, we list the zones which were
deliberately excluded by the tool.

Also at the end of the `zone_info.h` file, there may be warnings about
known inaccuracies for a particular zone.

### `zone_infos.cpp`

The top of this file contains a summary of the sizes of various parts of
the data structures in this file, and the 2 registries supplied by
`zone_registry.cpp`:

```
// Zones: 386
// Links: 207
// kZoneRegistry sizes (bytes):
//   Names: 3667 (originally 6100)
//   Formats: 597
//   Fragments: 122
//   Memory (8-bit): 16848
//   Memory (32-bit): 24464
// kZoneAndLinkRegistry sizes (bytes):
//   Names: 5620 (originally 9027)
//   Formats: 597
//   Fragments: 122
//   Memory (8-bit): 21492
//   Memory (32-bit): 31385
```

Each zone entry in the `zone_info.cpp` contains a comment section that
describes some metadata about the given entry. For example, the entry for
`zonedb::kZoneAmerica_Los_Angeles` contains the following, showing how
much memory it will consume on the 8-bit Arduino controllers and the 32-bit
Arduino controllers:

```
// Zone name: America/Los_Angeles
// Zone Eras: 1
// Strings (bytes): 17 (originally 24)
// Memory (8-bit): 39
// Memory (32-bit): 53
```

### `zone_policies.h`

An entry in `zone_info.cpp` may refer to a zone policy defined in
`zone_policies.h`. For example, the `kZoneAmerica_Los_Angeles` has a pointer
to a `kPolicyUS` data structure which is defined in `zone_policies.h`.

Each policy entry starts with a comment secion that contains some metadata
about the policy. For example:
```
// Policy name: US
// Rules: 5
// Memory (8-bit): 51
// Memory (32-bit): 72
```
Just like `zone_infos.cpp`, the Memory section describes the amount of static
RAM consumed by the particular `ZonePolicy` data structure (and associated
`ZoneRule`.

<a name="EncodingOffsetAndTimeFields"></a>
## Encoding Offset and Time Fields

There are 5 offsets and moment-in-time quantities from the TZ zoneinfo files
which are captured in the `zone_info.{h,cpp}` and `zone_policies.{h,cpp}` files:

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
RULES (numeric, 15-min resolution)                   /
-> rules_delta_seconds                              /
    -> rules_delta_code -- (basic) ----------------/
                        -- (extended) + 4 ---------


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
-> delta_seconds                        +--------------------+
    -> delta_code -- (basic) ---------> |     deltaCode      |
                  -- (extended) + 4 --> |     (8 bits)       |
                                        +--------------------+
```

<a name="ExtendedZoneProcessor"></a>
## ExtendedZoneProcessor

To save memory, the `ExtendedZoneProcessor` class calculates the `Transition`
objects on the fly when the `initForEpochSeconds(acetime_t)` or
`initForYear(int16_t)` method is called. The alternative used by most Date-Time
libraries to precalculate the Transitions for all Zones, for a certain range of
years. Precalculation is definitely faster, and easier because the logic for
calculating the Transition objects resides in only a single place. However, the
expansion of the Transition objects consumes too much memory on an embedded
microcontrollers.

When a request to create a `ZonedDateTime` object comes in, the
`ExtendedZoneProcessor.findTransition(epochSeconds)` method is called. It scans
the list of Transitions calculated above, looking for a match. The matching
Transition object contains the relevant standard offset and DST offset of that
Zone.

The calculation of the Transitions is very complex, and I find that I can no
longer understand my own code after a few months away of this code base. So here
are some notes to help my future-self. The code is in
`Transition::initForYear(int16_t)` and is organized into 5 steps as described
below.

<a name="Step1FindMatches"></a>
### Step 1: Find Matches

The `ExtendedZoneProcessor.findMatches()` finds all `ZoneEra` objects which
overlap with the 14-month interval from Dec 1 of the prior year until Feb 1 of
the following year. For example, `initForYear(2010)` means that the interval is
from 2009-12-01T00:00 until 2011-02-01T00:00.

A 14-month interval is chosen because a local date time of Jan 1 could land in
the prior year after the correct UTC offset is calculated, so we need to pick up
Transitions in the prior year. Similarly, a local date time of Dec 31 could land
in the following year after correcting for UTC offset.

A `MatchingEra` is a wrapper around a `ZoneEra`, with its startDateTimea and
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
* Verify that the `AceTime` library and the Hinnant `date` library agree with
  each other using the same TZDB version. This requires going into the
  [AceTimeValidation](https://github.com/bxparks/AceTimeValidation) project.
    * BasicHinnantDateTest
        * `$ cd .../AceTimeValidation/BasicHinnantDateTest`
        * Update the `TZ_VERSION` variable in the `Makefile` with the commit tag
          of the new TZDB version. (e.g. `TZ_VERSION = 2020c`).
        * `$ make clean`
        * `$ make`
        * `$ ./BasicHinnantDateTest.out | grep failed`
        * There should be no failures: `TestRunner summary: 268 passed, 0
          failed, 0 skipped, 0 timed out, out of 268 test(s).`
    * ExtendedHinnantDateTest
        * `$ cd .../AceTimeValidation/ExtendedHinnantDateTest`
        * Update the `TZ_VERSION` variable in the `Makefile` with the commit tag
          of the new TZDB version. (e.g. `TZ_VERSION = 2020c`).
        * `$ make clean`
        * `$ make`
        * `$ ./ExtendedHinnantDateTest.out | grep failed`
        * There should be no failures: `TestRunner summary: 387 passed, 0
          failed, 0 skipped, 0 timed out, out of 387 test(s).`
* Update the various zone info files:
    * `src/ace_time/zonedb`
        * `$ cd src/ace_time/zonedb`
        * Edit the `Makefile` and update the `TZ_VERSION`.
        * `$ make`
    * `src/ace_time/zonedbx`
        * `$ cd src/ace_time/zonedbx`
        * Edit the `Makefile` and update the `TZ_VERSION`.
        * `$ make`
    * `AceTimeTools/zonedbpy`
        * `$ cd AceTimeTools/zonedbpy`
        * Edit the `Makefile` and update the `TZ_VERSION`.
        * `$ make`
* Update the CHANGELOG.md.
* Commit the changes to git
    * `$ git add ...`
    * `$ git commit -m "..."`

There are 6 other validation tests in the AceTimeValidation project that compare
AceTime with other third party libraries (Python pytz, Python dateutil, and Java
date). Unfortunately, they all seem to use the underlying TZDB version provided
by the Operating System, and I have not been able to figure out how to manually
update this dependency manually. When a new TZDB is released, all of these other
tests will fail until the underlying timezone database of the OS is updated.

<a name="ReleaseProcess"></a>
## Release Process

* Update and commit the version numbers in various files:
    * `src/AceTime.h`
    * `README.md`
    * `docs/date_time_timezone.md`
    * `docs/clock_system_clock.md`
    * `docs/installation.md`
    * ...
    * `docs/doxygen.cfg`
    * `library.properties`
    * `CHANGELOG.md`
    * `$ git commit -m "..."`
* Update and commit the Doxygen docs. This is done as a separate git commit
  because the Doxygen changes are often so large that they obscure all other
  important changes to the code base:
    * `$ cd docs`
    * `$ make clean`
    * `$ make`
    * `$ git add .`
    * `$ git commit -m "..."`
* Merge the `develop` branch into `master`.
    * Normally I do this with a PR on GitHub to keep an audit trail.
    * Go to https://github.com/bxparks/AceTime.
    * Create a Pull Request from `develop` to `master`.
    * Approve and merge the PR.
* Create a new Release.
    * Go to https://github.com/bxparks/AceTime
    * Click on "Releases"
    * Click on "Draft a new release"
    * Enter a tag version (e.g. `v1.2`), targeting the `master` branch.
    * Enter the release title.
    * Enter the release notes. I normally just copy and paste the latest changes
      from `CHANGELOG.md`.
    * Click Publish release.
* Add corresponding tags on AceTimeTools and AceTimeValidation for reference.
    * Go to https://github.com/bxparks/AceTimeTools
    * `$ git tag -a 'atX.Y.Z' -m 'AceTime vX.Y.Z'`
    * `$ git push --tags`
    * Go to https://github.com/bxparks/AceTimeValidation
    * (Same as above)
