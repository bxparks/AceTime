# Validation Tests

These tests compare the algorithm implemented by `ZonedDateTime` and
`ZoneProcessor` classes with the equivalent functionalty from 3
other libraries:
* Python [pytz](https://pypi.org/project/pytz/) library
* [Java 11 Time](https://docs.oracle.com/en/java/javase/11/docs/api/java.base/java/time/package-summary.html) library.
* [Hinnant date](https://github.com/HowardHinnant/date) C++ library

These unit tests require a desktop-class machine running Linux or MacOS. They
are too big to run on any Arduino microcontroller that I know of. They use the
[UnixHostDuino](https://github.com/bxparks/UnixHostDuino) emulation layer. They
also use various files (e.g. `validation_data.h`, `validation_data.cpp`,
`validation_tests.cpp`) which are *generated* dynamically by the Makefile.
(These files used to be manually generated, then checked into source control.
But after it was clear that no Arduino microcontroller would be able to run
these tests, it did not seem worth checking in the generated code.)

## Compiling and Running

Prerequites:

1. The Java and C++ tools in `$ACE_TIME_DIR/tools/compare_java` and
`compare_cpp` need some setup. The best source of this information are
in the respective README.md files:
    * [compare_java](../../tools/compare_java/)
    * [compare_cpp](../../tools/compare_cpp/)
    * (The various `Makefile` in the subdirectories here will run `make -C` in
      those directories to build the Java and C++ binaries if necessary.)
1. Install [UnixHostDuino](https://github.com/bxparks/UnixHostDuino) as
  a sibling project to AceTime if you have not already done so:
    * `$ (cd ../../..; git clone https://github.com/bxparks/UnixHostDuino)`

You can run the tests in this directory by running the following commands:

```
$ make clean
$ make tests
$ make runtests
```
