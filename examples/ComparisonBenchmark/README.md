# Compare Time Libraries

This program compares the runtime performance of AceTime library versus the
[Arduino Time Library](https://github.com/PaulStoffregen/Time) for the
functions that perform the round trip conversion from the set of human-readable
date/time components (i.e. year, month, day, hour, minute, second) into the
number of seconds since a particular epoch (2000-01-01 for AceTime and
1970-01-01 for Arduino Time library), then back to the human-readable date/time
components.

From the benchmarks below on various processors, I conclude that the AceTime
library is:

* 2-5X faster on an 8-bit AVR processor
* 3-4X faster on an ESP8266
* 3-5X faster on an ESP32
* 7-20X faster on Teensy 3.2 controller

The main culprit for the slow speed of the `makeTime()` and `breakTime()`
methods in the Arduino Time library is that they use a loop to count the number
of days and seconds between the given date/time and the Unix epoch (1970-01-01).
For dates around the current date/time (year 2018), these methods must add up 47
years worth of days, then loop over the remaining months, days and seconds in
the current year. As the years pass, the number of cycles in those methods will
increase and the performance will become worse.

In contrast, the AceTime library uses arithmetic formulas to convert between
date/time components and epochSeconds. These expressions take the same amount of
time regardless of the value of the year. They contain a fair number of integer
division operations which are notoriously slow on 8-bit AVR processors because
divisions are emulated in software instead of implemented in hardware. Even with
the slow integer division operations, the AceTime library is faster on the AVR
boards than the Arduino Time Library. On processors that support hardware
divisions, the AceTime library can become an order of magnitude (10X) faster
than the Arduino Time Library.

## Arduino Nano

```
+--------------------------------------------+---------+
| Method                                     |  micros |
|--------------------------------------------|---------|
| Empty loop                                 |   4.500 |
|--------------------------------------------|---------|
| AceTime - ZonedDateTime::forEpochSeconds() | 277.000 |
| Arduino Time - breakTime()                 | 595.000 |
|--------------------------------------------|---------|
| AceTime - ZonedDateTime::toEpochSeconds()  |  76.500 |
| Arduino Time - makeTime()                  | 336.000 |
+--------------------------------------------+---------+
Number of iterations per run: 2000
Delta seconds: 236682
```

## ESP8266

```
+--------------------------------------------+---------+
| Method                                     |  micros |
|--------------------------------------------|---------|
| Empty loop                                 |   0.800 |
|--------------------------------------------|---------|
| AceTime - ZonedDateTime::forEpochSeconds() |  16.500 |
| Arduino Time - breakTime()                 |  45.700 |
|--------------------------------------------|---------|
| AceTime - ZonedDateTime::toEpochSeconds()  |   5.100 |
| Arduino Time - makeTime()                  |  22.400 |
+--------------------------------------------+---------+
Number of iterations per run: 10000
Delta seconds: 47336
```

## ESP32

```
+--------------------------------------------+---------+
| Method                                     |  micros |
|--------------------------------------------|---------|
| Empty loop                                 |   0.275 |
|--------------------------------------------|---------|
| AceTime - ZonedDateTime::forEpochSeconds() |   1.295 |
| Arduino Time - breakTime()                 |   5.050 |
|--------------------------------------------|---------|
| AceTime - ZonedDateTime::toEpochSeconds()  |   0.850 |
| Arduino Time - makeTime()                  |   4.305 |
+--------------------------------------------+---------+
Number of iterations per run: 100000
Delta seconds: 2366
```

## Teensy 3.2

```
+--------------------------------------------+---------+
| Method                                     |  micros |
|--------------------------------------------|---------|
| Empty loop                                 |   0.510 |
|--------------------------------------------|---------|
| AceTime - ZonedDateTime::forEpochSeconds() |   1.870 |
| Arduino Time - breakTime()                 |  12.610 |
|--------------------------------------------|---------|
| AceTime - ZonedDateTime::toEpochSeconds()  |   0.460 |
| Arduino Time - makeTime()                  |   9.780 |
+--------------------------------------------+---------+
Number of iterations per run: 100000
Delta seconds: 4733
```
