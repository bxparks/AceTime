# Validation Tests

These tests compare the algorithm implemented by `ZonedDateTime` and
`ZoneSpecifier` classes with the equivalent functionalty of the Python
[pytz](https://pypi.org/project/pytz/) library and the [Java 11
Time](https://docs.oracle.com/en/java/javase/11/docs/api/java.base/java/time/package-summary.html)
library.

These unit tests will **only** run on a Linux or MacOS machine, not on any
Arduino, because they consume too much memory. They also use various files which
are *generated* by the Makefile. (They used to be manually generated, then
checked into source control, but once it was clear that no Arduino
microcontroller would be able to run these tests, it did not seem worth checking
in the generated code.)

## Compiling and Running

You can run the entire test suite by running the following commands:

```
$ make clean
$ make tests
$ make runtests
```
