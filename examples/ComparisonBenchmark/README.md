# Compare Time Libraries

This program compares the runtime performance of
`LocalDateTime::forEpochSeconds` and the `LocalDateTime::toEpochSeconds()`
methods in the AceTime library versus the equivalent `breakTime()` and
`makeTime()` functions in the [Arduino Time
Library](https://github.com/PaulStoffregen/Time).

From the benchmarks below on various processors, I conclude that the
`LocalDateTime` methods are;

* 2-5X faster on an 8-bit AVR processor
* 3-5X faster on an ESP8266
* 7-8X faster on an ESP32
* 7-8X faster on Teensy 3.2 controller

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
+----------------------------------+---------+
| Method                           |  micros |
|----------------------------------|---------|
| Empty loop                       |   4.500 |
|----------------------------------|---------|
| LocalDateTime::forEpochSeconds() | 271.000 |
| breakTime()                      | 595.500 |
|----------------------------------|---------|
| LocalDateTime::toEpochSeconds()  |  66.500 |
| makeTime()                       | 336.000 |
+----------------------------------+---------+
Number of iterations per run: 2000
Delta seconds: 236682
```

## ESP8266

```
+----------------------------------+---------+
| Method                           |  micros |
|----------------------------------|---------|
| Empty loop                       |   0.900 |
|----------------------------------|---------|
| LocalDateTime::forEpochSeconds() |  15.200 |
| breakTime()                      |  45.600 |
|----------------------------------|---------|
| LocalDateTime::toEpochSeconds()  |   4.500 |
| makeTime()                       |  22.600 |
+----------------------------------+---------+
Number of iterations per run: 10000
Delta seconds: 47336
```

## ESP32

```
+----------------------------------+---------+
| Method                           |  micros |
|----------------------------------|---------|
| Empty loop                       |   0.280 |
|----------------------------------|---------|
| LocalDateTime::forEpochSeconds() |   0.620 |
| breakTime()                      |   5.070 |
|----------------------------------|---------|
| LocalDateTime::toEpochSeconds()  |   0.560 |
| makeTime()                       |   4.310 |
+----------------------------------+---------+
Number of iterations per run: 100000
Delta seconds: 2366
```

## Teensy 3.2

* CPU Speed: 96 MHz
* Compiler Optimization: "Faster"

```
+----------------------------------+---------+
| Method                           |  micros |
|----------------------------------|---------|
| Empty loop                       |   0.510 |
|----------------------------------|---------|
| LocalDateTime::forEpochSeconds() |   1.580 |
| breakTime()                      |  12.610 |
|----------------------------------|---------|
| LocalDateTime::toEpochSeconds()  |   1.170 |
| makeTime()                       |   9.780 |
+----------------------------------+---------+
Number of iterations per run: 100000
Delta seconds: 4733
```
