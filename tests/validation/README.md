# Validation Tests

These tests compare the algorithm implemented by `ZonedDateTime` and
`ZoneSpecifier` classes with the equivalent functionalty from the Python
[pytz](https://pypi.org/project/pytz/) library and the [Java 11
Time](https://docs.oracle.com/en/java/javase/11/docs/api/java.base/java/time/package-summary.html)
library.

These unit tests require a desktop-class machine running Linux or MacOS, not on
any Arduino microcontroller, because they consume too much memory. They use
the [unitduino](https://github.com/bxparks/AUnit/tree/develop/unitduino)
emulation layer provided by [AUnit](https://github.com/bxparks/AUnit).
They also use various files (e.g. `validation_data.h`, `validation_data.cpp`,
`validation_tests.cpp`) which are *generated* dynamically by the Makefile. (They
used to be manually generated, then checked into source control. But after it
was clear that no Arduino microcontroller would be able to run these tests, it
did not seem worth checking in the generated code.)

## Compiling and Running

You can run the entire test suite by running the following commands:

```
$ make clean
$ make tests
$ make runtests
```
