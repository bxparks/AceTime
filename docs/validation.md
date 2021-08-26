# Testing and Validation

Writing tests for this library was challenging, probably taking up 2-3X more
effort than the core of the library. I think the reason is that the number of
input variables into the library and the number of output variables are
substantially large, making it difficult to write isolated unit tests. Secondly,
the TZ Database zone files are deceptively easy to read by humans, but contain
many implicit rules that are difficult to translate into computer algorithms,
creating a large number of code paths to test.

## Table of Contents

* [Approach](#Approach)
* [Python pytz](#TestPythonPytz)
* [Python dateutil](#TestPythonDateUtil)
* [Java java.time](#TestJavaTime)
* [C++ Hinnant Date](#TestHinnantDate)
* [Noda Time](#TestNodaTime)

<a name="Approach"></a>
## Approach

It is simply impractical to manually create the inputs and expected outputs
using the TZ database. The calculation of one data point can take several
minutes manually. The solution would be to programmatically generate the data
points. To that end, I wrote the 2 different implementations of `ZoneProcessor`
(`BasicZoneProcessor` and `ExtendedZoneProcessor`) partially as an attempt to
write different versions of the algorithms to validate them against each other.
(I think I wrote 4-5 different versions altogether, of which only 2 made it into
this library). However, it turned out that the number of timezones supported by
the `ExtendedZoneProcessor` was much larger than the ones supported by
`BasicZoneProcessor` so it became infeasible to test the non-overlapping
timezones.

My next idea was to validate AceTime against a known, independently created,
timezone library that also supports the TZ Database. Currently, I validate
the AceTime library against 4 other timezone libraries:

* Python [pytz](https://pypi.org/project/pytz/)
* Python [dateutil](https://pypi.org/project/python-dateutil)
* Java 11 [java.time](https://docs.oracle.com/en/java/javase/11/docs/api/java.base/java/time/package-summary.html)
* C++11/14/17 [Hinnant date](https://github.com/HowardHinnant/date)
* [Noda Time](https://nodatime.org) C# library

When these tests pass, I become confident that AceTime is producing the correct
results, but it is entirely expected that some obscure edge-case bugs will be
found in the future.

<a name="TestPythonPytz"></a>
## Python pytz

The Python pytz library was a natural choice since the `tzcompiler.py` was
already written in Python. I created:

* [BasicPythonTest](../tests/validation/BasicPythonTest/)
* [ExtendedPythonTest](../tests/validation/ExtendedPythonTest/)

The `pytz` library is used to generate various C++ source code
(`validation_data.cpp`, `validation_data.h`, `validation_tests.cpp`) which
contain a list of epochSeconds, the UTC offset, the DST offset, at DST
transition points, for all timezones. The integration test then compiles in the
`ZonedDateTime` and verifies that the expected DST transitions and date
components are identical.

The resulting data test set contains between 150k to 220k data points, and can
no longer fit in any Arduino microcontroller that I am aware of. They can be
executed only on desktop-class Linux or MacOS machines through the use of the
[EpoxyDuino](https://github.com/bxparks/EpoxyDuino) emulation framework.

The `pytz` library supports [dates only until
2038](https://answers.launchpad.net/pytz/+question/262216). It is also tricky to
match the `pytz` version to the TZ Database version used by AceTime. The
following versions of `pytz` have been tested:

* pytz 2019.1, containing TZ Datbase 2019a
* pytz 2019.2, containing TZ Datbase 2019b
* pytz 2019.3, containing TZ Datbase 2019c

A number of zones did not match between pytz and AceTime. Those
have been listed in the `compare_pytz/blacklist.json` file.

<a name="TestPythonDateUtil"></a>
## Python dateutil

Validation against the Python dateutil library is similar to pytz. I created:

* [BasicDateUtilTest](../tests/validation/BasicDateUtilTest/)
* [ExtendedDateUtilTest](../tests/validation/ExtendedDateUtilTest/)

Similar to the `pytz` library, the `dateutil` library supports [dates only until
2038](https://github.com/dateutil/dateutil/issues/462). The
following versions of `dateutil` have been tested:

* dateutil 2.8.1, containing TZ Datbase 2019c

A number of zones did not match between dateutil and AceTime. Those
have been listed in the `compare_dateutil/blacklist.json` file.

<a name="TestJavaTime"></a>
## Java java.time

The Java 11 `java.time` library is not limited to 2038 but supports years
through the [year 1,000,000,000
(billion)](https://docs.oracle.com/en/java/javase/11/docs/api/java.base/java/time/class-use/Instant.html).
I wrote the
[compare_java](https://github.com/bxparks/AceTimeTools/tree/develop/compare_java)
program to generate a `validation_data.cpp` file in exactly the same format as
the `tzcompiler.py` program, and produced data points from year 2000 to year
2050, which is the exact range of years supported by the `zonedb::` and
`zonedbx::` zoneinfo files.

The result is 2 validation programs under `tests/validation`:

* [BasicJavaTest](../tests/validation/BasicJavaTest/)
* [ExtendedJavaTest](../tests/validation/ExtendedJavaTest/)

The most difficult part of using Java is figuring out how to install it
and figuring out which of the many variants of the JDK to use. On Ubuntu 18.04,
I used `openjdk 11.0.4 2019-07-16` which seems to use TZ Database 2018g. I have
no recollection how I installed it, I think it was something like `$ sudo apt
install openjdk-11-jdk:amd64`.

The underlying timezone database used by the `java.time` package seems to be
locked to the release version of the JDK. I have not been able to figure out a
way to upgrade the timezone database independently (it's something to do with
the
[TZUpdater](https://www.oracle.com/technetwork/java/javase/documentation/tzupdater-readme-136440.html)
but I haven't figured it out.)

A number of zones did not match between java.time and AceTime. Those
have been listed in the `compare_java/blacklist.json` file.

<a name="TestHinnantDate"></a>
## C++ Hinnant Date

I looked for a timezone library that allowed me to control the specific
version of the TZ Database. This led me to the C++11/14/17 [Hinnant
date](https://github.com/HowardHinnant/date) library, which has apparently been
accepted into the C++20 standard. This date and timezone library is incredible
powerful, complex and difficult to use. I managed to incorporate it into 2 more
validation tests, and verified that the AceTime library matches the Hinnant date
library for all timezones from 2000 to 2049 (inclusive):

* [BasicHinnantDateTest](../tests/validation/BasicHinnantDateTest/)
* [ExtendedHinnantDateTest](../tests/validation/ExtendedHinnantDateTest/)

I have validated the AceTime library with the Hinnant date library for the
following TZ Dabase versions:
* TZ DB version 2019a
* TZ DB version 2019b
* TZ DB version 2019c
* TZ DB version 2020a

AceTime matches Hinnant Date on all data points from the year 2000 to 2050. No
`blacklist.json` file was needed.

<a name="TestNodaTime"></a>
## Noda Time

I wrote the test data generator
[compare_noda](https://github.com/bxparks/AceTimeTools/tree/develop/compare_noda)
in C# to generate a `validation_data.cpp` using the
[Noda Time](https://nodatime.org) library. The result is 2 validation programs
under `tests/validation`:

* [BasicNodaTest](../tests/validation/BasicNodaTest/)
* [ExtendedNodaTest](../tests/validation/ExtendedNodaTest/)

AceTime matches Noda Time on all data points from the year 2000 to 2050. No
`blacklist.json` file was needed.
