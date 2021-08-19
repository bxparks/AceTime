# Validation Tests

These tests compare the algorithm implemented by `ZonedDateTime` and
`ZoneProcessor` classes with the equivalent functionality from 5
other libraries:

* [Python pytz](https://pypi.org/project/pytz/) library
* [Python dateutil](https://pypi.org/project/python-dateutil/) library
* [Java 11 Time](https://docs.oracle.com/en/java/javase/11/docs/api/java.base/java/time/package-summary.html) library.
* [Hinnant date](https://github.com/HowardHinnant/date) C++ library
* [Noda Time](https://nodatime.org) C# library

These unit tests require a desktop-class machine running Linux or MacOS. They
are too big to run on any Arduino microcontroller that I know of. They use the
[EpoxyDuino](https://github.com/bxparks/EpoxyDuino) emulation layer to run
these programs on the desktop machine. They also use various files (e.g.
`validation_data.h`, `validation_data.cpp`, `validation_tests.cpp`) which are
*generated* dynamically by the various `Makefile` files. (These files used to be
manually generated, then checked into source control. But after it was clear
that no Arduino microcontroller would be able to run these tests, it did not
seem worth checking in the generated code.)

## Compiling and Running

### Prerequisites

The required Python, Java and C++ tools and libraries are explained in:

* [compare_pytz](../../tools/compare_pytz/)
* [compare_dateutil](../../tools/compare_dateutil/)
* [compare_java](../../tools/compare_java/)
* [compare_cpp](../../tools/compare_cpp/)
* [compare_noda](../../tools/compare_noda/)

The various `Makefile` files under the subdirectories here will run `make -C` in
those directories to build the Java and C++ binaries as necessary. Here is a
(potentially out of date) summary of the 3rd party prerequisites:

1. Install EpoxyDuino as a sibling project to `AceTime`:
    * `$ git clone https://github.com/bxparks/EpoxyDuino`
1. Clone the IANA TZ database as a sibling project to `AceTime`:
    * `$ git clone https://github.com/eggert/tz`
1. Install the Python `pytz` and `dateutil` libraries:
    * `$ pip3 install --user pytz python-dateutil`
1. Install the Java 11 JDK:
    * `$ sudo apt install openjdk-11-jdk` (Ubuntu 18.04 or 20.04)
1. Clone the Hinnant date library as a sibling to the `AceTime` directory, and
   install the `libcurl4` library that it requires to download the tzfiles:
    * `$ git clone https://github.com/HowardHinnant/date`
    * `$ sudo apt install libcurl4-openssl-dev`
1. Install Microsoft .NET 5.0 on Linux:
    * See https://docs.microsoft.com/en-us/dotnet/core/install/linux
    * The `compare_noda.csproj` file already contains the NuGet dependency
      to Noda Time and will be automatically retrieved by the `dotnet build`
      command.

### Running the Tests

You can run the tests in this directory by running the following commands:

```
$ make clean
$ make tests
$ make runtests
```
