# Validation Tests

These tests compare the algorithm implemented by `ZonedDateTime` and
`ZoneProcessor` classes with the equivalent functionalty from the Python
[pytz](https://pypi.org/project/pytz/) library and the [Java 11
Time](https://docs.oracle.com/en/java/javase/11/docs/api/java.base/java/time/package-summary.html)
library.

These unit tests require a desktop-class machine running Linux or MacOS. They
are too big to run on any Arduino microcontroller that I know of. They use the
[UnixHostDuino](https://github.com/bxparks/UnixHostDuino) emulation layer. They
also use various files (e.g. `validation_data.h`, `validation_data.cpp`,
`validation_tests.cpp`) which are *generated* dynamically by the Makefile.
(These files used to be manually generated, then checked into source control.
But after it was clear that no Arduino microcontroller would be able to run
these tests, it did not seem worth checking in the generated code.)

## Compiling and Running

Prerequite:

1. You need to compile the
[TestDataGenerator.java](../../tools/java/TestDataGenerator.java) program:
    * `$ (cd ../../tools/java; make)`
1. Install [UnixHostDuino](https://github.com/bxparks/UnixHostDuino) as
  a sibling project to AceTime if you have not already done so:
    * `$ (cd ../../..; git clone https://github.com/bxparks/UnixHostDuino)`

You can run the tests in this directory by running the following commands:

```
$ make clean
$ make tests
$ make runtests
```
