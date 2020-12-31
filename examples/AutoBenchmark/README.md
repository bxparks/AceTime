# Auto Benchmark

Here are the results from `AutoBenchmark.ino` for various boards.
These results show that integer division and modulus operations are incredibly
slow on 8-bit AVR processors.

**Version**: AceTime v1.4

**NOTE**: This file was auto-generated using `make README.md`. DO NOT EDIT.

## Dependencies

This program depends on the following libraries:

* [AceTime](https://github.com/bxparks/AceTime)
* [AceRoutine](https://github.com/bxparks/AceRoutine)

## How to Generate

This requires the [AUniter](https://github.com/bxparks/AUniter) script
to execute the Arduino IDE programmatically.

The `Makefile` has rules to generate the `*.txt` results file for several
microcontrollers that I usually support, but the `$ make benchmarks` command
does not work very well because the USB port of the microcontroller is a
dynamically changing parameter. I created a semi-automated way of collect the
`*.txt` files:

1. Connect the microcontroller to the serial port. I usually do this through a
USB hub with individually controlled switch.
2. Type `$ auniter ports` to determine its `/dev/ttyXXX` port number (e.g.
`/dev/ttyUSB0` or `/dev/ttyACM0`).
3. If the port is `USB0` or `ACM0`, type `$ make nano.txt`, etc.
4. Switch off the old microontroller.
5. Go to Step 1 and repeat for each microcontroller.

The `generate_table.awk` program reads one of `*.txt` files and prints out an
ASCII table that can be directly embedded into this README.md file. For example
the following command produces the table in the Nano section below:

```
$ ./generate_table.awk < nano.txt
```

Fortunately, we no longer need to run `generate_table.awk` for each `*.txt`
file. The process has been automated using the `generate_readme.py` script which
will be invoked by the following command:
```
$ make README.md
```

The CPU times below are given in microseconds.

## CPU Time Changes

The CPU time did not change much from v0.8 to v1.4.

## Arduino Nano

* 16MHz ATmega328P
* Arduino IDE 1.8.13
* Arduino AVR Boards 1.8.3

```
Sizes of Objects:
sizeof(LocalDate): 3
sizeof(LocalTime): 3
sizeof(LocalDateTime): 6
sizeof(TimeOffset): 2
sizeof(OffsetDateTime): 8
sizeof(BasicZoneProcessor): 113
sizeof(ExtendedZoneProcessor): 453
sizeof(BasicZoneRegistrar): 5
sizeof(ExtendedZoneRegistrar): 5
sizeof(BasicZoneManager<1>): 123
sizeof(ExtendedZoneManager<1>): 463
sizeof(TimeZoneData): 5
sizeof(TimeZone): 5
sizeof(ZonedDateTime): 13
sizeof(TimePeriod): 4
sizeof(clock::DS3231Clock): 3
sizeof(clock::SystemClock): 17
sizeof(clock::SystemClockLoop): 34
sizeof(clock::SystemClockCoroutine): 43
sizeof(basic::ZoneContext): 6
sizeof(basic::ZoneEra): 11
sizeof(basic::ZoneInfo): 12
sizeof(basic::ZoneRule): 9
sizeof(basic::ZonePolicy): 6
sizeof(basic::Transition): 21
sizeof(extended::Transition): 46
sizeof(extended::ZoneMatch): 14

CPU:
+--------------------------------------------------+----------+
| Method                                           |   micros |
|--------------------------------------------------+----------|
| EmptyLoop                                        |      6.0 |
|--------------------------------------------------+----------|
| LocalDate::forEpochDays()                        |    215.6 |
| LocalDate::toEpochDays()                         |     56.8 |
| LocalDate::dayOfWeek()                           |     51.2 |
| OffsetDateTime::forEpochSeconds()                |    318.8 |
| OffsetDateTime::toEpochSeconds()                 |     87.2 |
| ZonedDateTime::toEpochSeconds()                  |     85.6 |
| ZonedDateTime::toEpochDays()                     |     73.2 |
| ZonedDateTime::forEpochSeconds(UTC)              |    334.8 |
| ZonedDateTime::forEpochSeconds(Basic_nocache)    |   1185.6 |
| ZonedDateTime::forEpochSeconds(Basic_cached)     |    625.2 |
| ZonedDateTime::forEpochSeconds(Extended_nocache) |   2028.4 |
| ZonedDateTime::forEpochSeconds(Extended_cached)  |    625.6 |
+--------------------------------------------------+----------+
Iterations_per_run: 2500

```

## Sparkfun Pro Micro

* 16 MHz ATmega32U4
* Arduino IDE 1.8.13
* SparkFun AVR Boards 1.1.13

```
Sizes of Objects:
sizeof(LocalDate): 3
sizeof(LocalTime): 3
sizeof(LocalDateTime): 6
sizeof(TimeOffset): 2
sizeof(OffsetDateTime): 8
sizeof(BasicZoneProcessor): 113
sizeof(ExtendedZoneProcessor): 453
sizeof(BasicZoneRegistrar): 5
sizeof(ExtendedZoneRegistrar): 5
sizeof(BasicZoneManager<1>): 123
sizeof(ExtendedZoneManager<1>): 463
sizeof(TimeZoneData): 5
sizeof(TimeZone): 5
sizeof(ZonedDateTime): 13
sizeof(TimePeriod): 4
sizeof(clock::DS3231Clock): 3
sizeof(clock::SystemClock): 17
sizeof(clock::SystemClockLoop): 34
sizeof(clock::SystemClockCoroutine): 43
sizeof(basic::ZoneContext): 6
sizeof(basic::ZoneEra): 11
sizeof(basic::ZoneInfo): 12
sizeof(basic::ZoneRule): 9
sizeof(basic::ZonePolicy): 6
sizeof(basic::Transition): 21
sizeof(extended::Transition): 46
sizeof(extended::ZoneMatch): 14

CPU:
+--------------------------------------------------+----------+
| Method                                           |   micros |
|--------------------------------------------------+----------|
| EmptyLoop                                        |      5.2 |
|--------------------------------------------------+----------|
| LocalDate::forEpochDays()                        |    216.4 |
| LocalDate::toEpochDays()                         |     57.6 |
| LocalDate::dayOfWeek()                           |     50.8 |
| OffsetDateTime::forEpochSeconds()                |    320.8 |
| OffsetDateTime::toEpochSeconds()                 |     87.2 |
| ZonedDateTime::toEpochSeconds()                  |     87.2 |
| ZonedDateTime::toEpochDays()                     |     73.2 |
| ZonedDateTime::forEpochSeconds(UTC)              |    336.0 |
| ZonedDateTime::forEpochSeconds(Basic_nocache)    |   1191.6 |
| ZonedDateTime::forEpochSeconds(Basic_cached)     |    627.6 |
| ZonedDateTime::forEpochSeconds(Extended_nocache) |   2039.2 |
| ZonedDateTime::forEpochSeconds(Extended_cached)  |    627.6 |
+--------------------------------------------------+----------+
Iterations_per_run: 2500

```

## SAMD21 M0 Mini

* 48 MHz ARM Cortex-M0+
* Arduino IDE 1.8.13
* Sparkfun SAMD Core 1.8.1

```
Sizes of Objects:
sizeof(LocalDate): 3
sizeof(LocalTime): 3
sizeof(LocalDateTime): 6
sizeof(TimeOffset): 2
sizeof(OffsetDateTime): 8
sizeof(BasicZoneProcessor): 156
sizeof(ExtendedZoneProcessor): 532
sizeof(BasicZoneRegistrar): 12
sizeof(ExtendedZoneRegistrar): 12
sizeof(BasicZoneManager<1>): 180
sizeof(ExtendedZoneManager<1>): 556
sizeof(TimeZoneData): 8
sizeof(TimeZone): 12
sizeof(ZonedDateTime): 20
sizeof(TimePeriod): 4
sizeof(clock::DS3231Clock): 8
sizeof(clock::SystemClock): 24
sizeof(clock::SystemClockLoop): 44
sizeof(clock::SystemClockCoroutine): 68
sizeof(basic::ZoneContext): 8
sizeof(basic::ZoneEra): 16
sizeof(basic::ZoneInfo): 20
sizeof(basic::ZoneRule): 9
sizeof(basic::ZonePolicy): 12
sizeof(basic::Transition): 28
sizeof(extended::Transition): 52
sizeof(extended::ZoneMatch): 16

CPU:
+--------------------------------------------------+----------+
| Method                                           |   micros |
|--------------------------------------------------+----------|
| EmptyLoop                                        |      1.3 |
|--------------------------------------------------+----------|
| LocalDate::forEpochDays()                        |     23.2 |
| LocalDate::toEpochDays()                         |      9.7 |
| LocalDate::dayOfWeek()                           |     12.3 |
| OffsetDateTime::forEpochSeconds()                |     35.0 |
| OffsetDateTime::toEpochSeconds()                 |     17.1 |
| ZonedDateTime::toEpochSeconds()                  |     18.2 |
| ZonedDateTime::toEpochDays()                     |     16.3 |
| ZonedDateTime::forEpochSeconds(UTC)              |     41.3 |
| ZonedDateTime::forEpochSeconds(Basic_nocache)    |    235.2 |
| ZonedDateTime::forEpochSeconds(Basic_cached)     |     73.8 |
| ZonedDateTime::forEpochSeconds(Extended_nocache) |    435.1 |
| ZonedDateTime::forEpochSeconds(Extended_cached)  |     74.1 |
+--------------------------------------------------+----------+
Iterations_per_run: 10000

```

## ESP8266

* NodeMCU 1.0 clone, 80MHz ESP8266
* Arduino IDE 1.8.13
* ESP8266 Boards 2.7.4

```
Sizes of Objects:
sizeof(LocalDate): 3
sizeof(LocalTime): 3
sizeof(LocalDateTime): 6
sizeof(TimeOffset): 2
sizeof(OffsetDateTime): 8
sizeof(BasicZoneProcessor): 156
sizeof(ExtendedZoneProcessor): 532
sizeof(BasicZoneRegistrar): 12
sizeof(ExtendedZoneRegistrar): 12
sizeof(BasicZoneManager<1>): 180
sizeof(ExtendedZoneManager<1>): 556
sizeof(TimeZoneData): 8
sizeof(TimeZone): 12
sizeof(ZonedDateTime): 20
sizeof(TimePeriod): 4
sizeof(clock::DS3231Clock): 8
sizeof(clock::NtpClock): 88
sizeof(clock::SystemClock): 24
sizeof(clock::SystemClockLoop): 44
sizeof(clock::SystemClockCoroutine): 68
sizeof(basic::ZoneContext): 8
sizeof(basic::ZoneEra): 16
sizeof(basic::ZoneInfo): 20
sizeof(basic::ZoneRule): 9
sizeof(basic::ZonePolicy): 12
sizeof(basic::Transition): 28
sizeof(extended::Transition): 52
sizeof(extended::ZoneMatch): 16

CPU:
+--------------------------------------------------+----------+
| Method                                           |   micros |
|--------------------------------------------------+----------|
| EmptyLoop                                        |      5.1 |
|--------------------------------------------------+----------|
| LocalDate::forEpochDays()                        |      7.8 |
| LocalDate::toEpochDays()                         |      4.3 |
| LocalDate::dayOfWeek()                           |      4.1 |
| OffsetDateTime::forEpochSeconds()                |     12.2 |
| OffsetDateTime::toEpochSeconds()                 |      7.0 |
| ZonedDateTime::toEpochSeconds()                  |      7.0 |
| ZonedDateTime::toEpochDays()                     |      6.0 |
| ZonedDateTime::forEpochSeconds(UTC)              |     13.1 |
| ZonedDateTime::forEpochSeconds(Basic_nocache)    |     97.7 |
| ZonedDateTime::forEpochSeconds(Basic_cached)     |     27.1 |
| ZonedDateTime::forEpochSeconds(Extended_nocache) |    177.1 |
| ZonedDateTime::forEpochSeconds(Extended_cached)  |     27.3 |
+--------------------------------------------------+----------+
Iterations_per_run: 10000

```

## ESP32

* ESP32-01 Dev Board, 240 MHz Tensilica LX6
* Arduino IDE 1.8.13
* ESP32 Boards 1.0.4

```
Sizes of Objects:
sizeof(LocalDate): 3
sizeof(LocalTime): 3
sizeof(LocalDateTime): 6
sizeof(TimeOffset): 2
sizeof(OffsetDateTime): 8
sizeof(BasicZoneProcessor): 156
sizeof(ExtendedZoneProcessor): 532
sizeof(BasicZoneRegistrar): 12
sizeof(ExtendedZoneRegistrar): 12
sizeof(BasicZoneManager<1>): 180
sizeof(ExtendedZoneManager<1>): 556
sizeof(TimeZoneData): 8
sizeof(TimeZone): 12
sizeof(ZonedDateTime): 20
sizeof(TimePeriod): 4
sizeof(clock::DS3231Clock): 8
sizeof(clock::NtpClock): 116
sizeof(clock::SystemClock): 24
sizeof(clock::SystemClockLoop): 44
sizeof(clock::SystemClockCoroutine): 68
sizeof(basic::ZoneContext): 8
sizeof(basic::ZoneEra): 16
sizeof(basic::ZoneInfo): 20
sizeof(basic::ZoneRule): 9
sizeof(basic::ZonePolicy): 12
sizeof(basic::Transition): 28
sizeof(extended::Transition): 52
sizeof(extended::ZoneMatch): 16

CPU:
+--------------------------------------------------+----------+
| Method                                           |   micros |
|--------------------------------------------------+----------|
| EmptyLoop                                        |      1.4 |
|--------------------------------------------------+----------|
| LocalDate::forEpochDays()                        |      0.5 |
| LocalDate::toEpochDays()                         |      0.4 |
| LocalDate::dayOfWeek()                           |      0.4 |
| OffsetDateTime::forEpochSeconds()                |      0.9 |
| OffsetDateTime::toEpochSeconds()                 |      1.4 |
| ZonedDateTime::toEpochSeconds()                  |      1.4 |
| ZonedDateTime::toEpochDays()                     |      1.1 |
| ZonedDateTime::forEpochSeconds(UTC)              |      1.2 |
| ZonedDateTime::forEpochSeconds(Basic_nocache)    |     15.3 |
| ZonedDateTime::forEpochSeconds(Basic_cached)     |      2.6 |
| ZonedDateTime::forEpochSeconds(Extended_nocache) |     30.0 |
| ZonedDateTime::forEpochSeconds(Extended_cached)  |      2.5 |
+--------------------------------------------------+----------+
Iterations_per_run: 100000

```

Note: Once the benchmark of the function under test becomes smaller than the
duration of an empty loop, the numbers become unreliable.

## Teensy 3.2

* 96 MHz ARM Cortex-M4
* Arduino IDE 1.8.13
* Teensyduino 1.53
* Compiler options: "Faster"

```
TBD
```

