# Developer Notes

Information which are useful for developers and maintainers of the AceTime
library.

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
dashes `-` are converted to underscore `_`.

The `kTzDatabaseVersion` string constant identifies the version of the TZ
Database that was used to generate these files, e.g. "2019a".

The `kZoneContext` variable points to an instance of `common::ZoneContext`
which identifies the `startYear` and `endYear` of the current database. These
will normally be 2000 and 2050 respectively, but a custom version of the
zonedb files could be generated whose startYear and endYear could be smaller (to
produce smaller ZoneDB files).

Near end of the `zone_info.h` file, we list the zones which were
deliberately excluded by the tool.

Also at the end of the `zone_info.h` file, there may be warnings about
known inaccuracies for a particular zone.

### `zone_infos.cpp`

Each zone entry in the `zone_info.cpp` contains a comment section that
describes some metadata about the given entry. For example, the entry for
`zonedb::kZoneEraAmerica_Los_Angeles` contains the following, showing how
much memory it will consume on the 8-bit Arduino controllers and the 32-bit
Arduino controllers:

* `// Zone name: America/Los_Angeles`
* `// Era count: 1`
* `// Strings: 24`
* `// Memory (8-bit): 43`
* `// Memory (32-bit): 53`

### `zone_policies.h`

An entry in `zone_info.cpp` may refer to a zone policy defined in
`zone_policies.h`. For example, the `kZoneAmerica_Los_Angeles` has a pointer
to a `kPolicyUS` data structure which is defined in `zone_policies.h`.

Each policy entry starts with a comment secion that contains some metadata
about the policy. For example:
```
// Policy name: US
// Rule count: 5
// Memory (8-bit): 51
// Memory (32-bit): 55
```
Just like `zone_infos.cpp`, the Memory section describes the amount of static
RAM consumed by the particular `ZonePolicy` data structure (and associated
`ZoneRule`.

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
* Verify that `AceTime` and Hinnant `date` agree with each other using the
  same TZDB version.
    * BasicHinnantDateTest
        * `$ cd tests/validation/BasicHinnantDateTest`
        * Update the `TZ_VERSION` variable in the `Makefile` with the commit tag
          of the new TZDB version. (e.g. `TZ_VERSION = 2020c`).
        * `$ make clean`
        * `$ make`
        * `$ ./BasicHinnantDateTest.out | grep failed`
        * There should be no failures: `TestRunner summary: 268 passed, 0
          failed, 0 skipped, 0 timed out, out of 268 test(s).`
    * ExtendedHinnantDateTest
        * `$ cd tests/validation/ExtendedHinnantDateTest`
        * Update the `TZ_VERSION` variable in the `Makefile` with the commit tag
          of the new TZDB version. (e.g. `TZ_VERSION = 2020c`).
        * `$ make clean`
        * `$ make`
        * `$ ./ExtendedHinnantDateTest.out | grep failed`
        * There should be no failures: `TestRunner summary: 387 passed, 0
          failed, 0 skipped, 0 timed out, out of 387 test(s).`
* Update the `src/ace_time/zonedb` and `zonedbx` zone info files:
    * zonedb
        * `$ cd src/ace_time/zonedb`
        * Update the `TZ_VERSION` in the `Makefile`.
        * `$ make`
    * zonedbx
        * `$ cd src/ace_time/zonedbx`
        * Update the `TZ_VERSION` in the `Makefile`.
        * `$ make`
* Update the CHANGELOG.md.
* Commit the changes to git
    * `$ git add ...`
    * `$ git commit -m "..."`

There are 6 other validation tests in `tests/validation` that compare AceTime
with other 3rd party librarties (Python pytz, Python dateutil, and Java date).
Unfortunately, they all seem to use the underlying TZDB version provided by the
Operating System, and I have not been able to figure out how to manually update
this dependency manually. When a new TZDB is released, all of these other tests
will fail until the underying timezone database of the OS is updated.

## Release Process

* Update and commit the version numbers in various files:
    * `src/AceTime.h`
    * `README.md`
    * `USER_GUIDE.md`
    * `docs/doxygen.cfg`
    * `library.properties`
    * `CHANGELOG.md`
    * Bump the version numbers in the unit tests:
        * `BasicZoneProcessorTest/BasicZoneProcessorTest.ino`
        * `ExtendedZoneProcessorTest/ExtendedZoneProcessorTest.ino`
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
