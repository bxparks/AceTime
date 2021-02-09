# Auto Benchmark

Here are the results from `AutoBenchmark.ino` for various boards.
These results show that integer division and modulus operations are incredibly
slow on 8-bit AVR processors.

**Version**: AceTime v1.5

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

* The CPU time did not change much from v0.8 to v1.4.
* The CPU time of most classes did not change much from v1.4 to v1.5. The
  big difference is that the Zone registries (kZoneRegistry,
  kZoneAndLinkRegistry) are now sorted by zoneId instead of zoneName, and the
  `ZoneManager::createForZoneId()` will use a binary search, instead of a linear
  search. This makes it 10-15X faster for ~266 entries. The
  `ZoneManager::createForZoneName()` also converts to a zoneId, then performs a
  binary search, instead of doing a binary search on the zoneName directly. Even
  with the extra level of indirection, the `createForZoneName()` is between
  1.5-2X faster than the previous version.

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
sizeof(internal::ZoneContext): 9
sizeof(basic::ZoneEra): 11
sizeof(basic::ZoneInfo): 11
sizeof(basic::ZoneRule): 9
sizeof(basic::ZonePolicy): 6
sizeof(basic::Transition): 21
sizeof(extended::Transition): 46
sizeof(extended::ZoneMatch): 14

CPU:
+--------------------------------------------------+----------+
| Method                                           |   micros |
|--------------------------------------------------+----------|
| EmptyLoop                                        |      5.0 |
|--------------------------------------------------+----------|
| LocalDate::forEpochDays()                        |    219.0 |
| LocalDate::toEpochDays()                         |     55.0 |
| LocalDate::dayOfWeek()                           |     50.0 |
| OffsetDateTime::forEpochSeconds()                |    324.0 |
| OffsetDateTime::toEpochSeconds()                 |     86.0 |
| ZonedDateTime::toEpochSeconds()                  |     83.0 |
| ZonedDateTime::toEpochDays()                     |     73.0 |
| ZonedDateTime::forEpochSeconds(UTC)              |    338.0 |
| ZonedDateTime::forEpochSeconds(Basic_nocache)    |   1180.0 |
| ZonedDateTime::forEpochSeconds(Basic_cached)     |    617.0 |
| ZonedDateTime::forEpochSeconds(Extended_nocache) |   2022.0 |
| ZonedDateTime::forEpochSeconds(Extended_cached)  |    618.0 |
| BasicZoneManager::createForZoneName(binary)      |    110.0 |
| BasicZoneManager::createForZoneId(binary)        |     36.0 |
| BasicZoneManager::createForZoneId(linear)        |    392.0 |
+--------------------------------------------------+----------+
Iterations_per_run: 1000

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
sizeof(internal::ZoneContext): 9
sizeof(basic::ZoneEra): 11
sizeof(basic::ZoneInfo): 11
sizeof(basic::ZoneRule): 9
sizeof(basic::ZonePolicy): 6
sizeof(basic::Transition): 21
sizeof(extended::Transition): 46
sizeof(extended::ZoneMatch): 14

CPU:
+--------------------------------------------------+----------+
| Method                                           |   micros |
|--------------------------------------------------+----------|
| EmptyLoop                                        |      3.0 |
|--------------------------------------------------+----------|
| LocalDate::forEpochDays()                        |    219.0 |
| LocalDate::toEpochDays()                         |     56.0 |
| LocalDate::dayOfWeek()                           |     49.0 |
| OffsetDateTime::forEpochSeconds()                |    325.0 |
| OffsetDateTime::toEpochSeconds()                 |     85.0 |
| ZonedDateTime::toEpochSeconds()                  |     83.0 |
| ZonedDateTime::toEpochDays()                     |     72.0 |
| ZonedDateTime::forEpochSeconds(UTC)              |    341.0 |
| ZonedDateTime::forEpochSeconds(Basic_nocache)    |   1187.0 |
| ZonedDateTime::forEpochSeconds(Basic_cached)     |    619.0 |
| ZonedDateTime::forEpochSeconds(Extended_nocache) |   2034.0 |
| ZonedDateTime::forEpochSeconds(Extended_cached)  |    621.0 |
| BasicZoneManager::createForZoneName(binary)      |    110.0 |
| BasicZoneManager::createForZoneId(binary)        |     38.0 |
| BasicZoneManager::createForZoneId(linear)        |    398.0 |
+--------------------------------------------------+----------+
Iterations_per_run: 1000

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
sizeof(BasicZoneRegistrar): 8
sizeof(ExtendedZoneRegistrar): 8
sizeof(BasicZoneManager<1>): 176
sizeof(ExtendedZoneManager<1>): 552
sizeof(TimeZoneData): 8
sizeof(TimeZone): 12
sizeof(ZonedDateTime): 20
sizeof(TimePeriod): 4
sizeof(clock::DS3231Clock): 8
sizeof(clock::SystemClock): 24
sizeof(clock::SystemClockLoop): 44
sizeof(clock::SystemClockCoroutine): 68
sizeof(internal::ZoneContext): 16
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
| LocalDate::forEpochDays()                        |     23.7 |
| LocalDate::toEpochDays()                         |      9.4 |
| LocalDate::dayOfWeek()                           |     12.2 |
| OffsetDateTime::forEpochSeconds()                |     35.3 |
| OffsetDateTime::toEpochSeconds()                 |     16.6 |
| ZonedDateTime::toEpochSeconds()                  |     18.1 |
| ZonedDateTime::toEpochDays()                     |     15.8 |
| ZonedDateTime::forEpochSeconds(UTC)              |     41.4 |
| ZonedDateTime::forEpochSeconds(Basic_nocache)    |    234.5 |
| ZonedDateTime::forEpochSeconds(Basic_cached)     |     73.0 |
| ZonedDateTime::forEpochSeconds(Extended_nocache) |    435.8 |
| ZonedDateTime::forEpochSeconds(Extended_cached)  |     73.2 |
| BasicZoneManager::createForZoneName(binary)      |     15.5 |
| BasicZoneManager::createForZoneId(binary)        |      3.5 |
| BasicZoneManager::createForZoneId(linear)        |     14.4 |
+--------------------------------------------------+----------+
Iterations_per_run: 10000

```

## STM32 Blue Pill

* STM32F103C8, 72 MHz ARM Cortex-M3
* Arduino IDE 1.8.13
* STM32duino 1.9.0

```
Sizes of Objects:
sizeof(LocalDate): 3
sizeof(LocalTime): 3
sizeof(LocalDateTime): 6
sizeof(TimeOffset): 2
sizeof(OffsetDateTime): 8
sizeof(BasicZoneProcessor): 156
sizeof(ExtendedZoneProcessor): 532
sizeof(BasicZoneRegistrar): 8
sizeof(ExtendedZoneRegistrar): 8
sizeof(BasicZoneManager<1>): 176
sizeof(ExtendedZoneManager<1>): 552
sizeof(TimeZoneData): 8
sizeof(TimeZone): 12
sizeof(ZonedDateTime): 20
sizeof(TimePeriod): 4
sizeof(clock::DS3231Clock): 8
sizeof(clock::SystemClock): 24
sizeof(clock::SystemClockLoop): 44
sizeof(clock::SystemClockCoroutine): 68
sizeof(internal::ZoneContext): 16
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
| EmptyLoop                                        |      1.1 |
|--------------------------------------------------+----------|
| LocalDate::forEpochDays()                        |      2.5 |
| LocalDate::toEpochDays()                         |      0.8 |
| LocalDate::dayOfWeek()                           |      1.2 |
| OffsetDateTime::forEpochSeconds()                |      3.6 |
| OffsetDateTime::toEpochSeconds()                 |      4.2 |
| ZonedDateTime::toEpochSeconds()                  |      4.2 |
| ZonedDateTime::toEpochDays()                     |      3.3 |
| ZonedDateTime::forEpochSeconds(UTC)              |      4.9 |
| ZonedDateTime::forEpochSeconds(Basic_nocache)    |     66.3 |
| ZonedDateTime::forEpochSeconds(Basic_cached)     |     10.3 |
| ZonedDateTime::forEpochSeconds(Extended_nocache) |    128.4 |
| ZonedDateTime::forEpochSeconds(Extended_cached)  |     10.1 |
| BasicZoneManager::createForZoneName(binary)      |     11.8 |
| BasicZoneManager::createForZoneId(binary)        |      2.6 |
| BasicZoneManager::createForZoneId(linear)        |     17.9 |
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
sizeof(BasicZoneRegistrar): 8
sizeof(ExtendedZoneRegistrar): 8
sizeof(BasicZoneManager<1>): 176
sizeof(ExtendedZoneManager<1>): 552
sizeof(TimeZoneData): 8
sizeof(TimeZone): 12
sizeof(ZonedDateTime): 20
sizeof(TimePeriod): 4
sizeof(clock::DS3231Clock): 8
sizeof(clock::NtpClock): 88
sizeof(clock::SystemClock): 24
sizeof(clock::SystemClockLoop): 44
sizeof(clock::SystemClockCoroutine): 68
sizeof(internal::ZoneContext): 16
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
| EmptyLoop                                        |      5.0 |
|--------------------------------------------------+----------|
| LocalDate::forEpochDays()                        |      7.9 |
| LocalDate::toEpochDays()                         |      4.1 |
| LocalDate::dayOfWeek()                           |      4.0 |
| OffsetDateTime::forEpochSeconds()                |     12.3 |
| OffsetDateTime::toEpochSeconds()                 |      7.0 |
| ZonedDateTime::toEpochSeconds()                  |      6.9 |
| ZonedDateTime::toEpochDays()                     |      5.9 |
| ZonedDateTime::forEpochSeconds(UTC)              |     13.4 |
| ZonedDateTime::forEpochSeconds(Basic_nocache)    |     97.2 |
| ZonedDateTime::forEpochSeconds(Basic_cached)     |     26.5 |
| ZonedDateTime::forEpochSeconds(Extended_nocache) |    200.2 |
| ZonedDateTime::forEpochSeconds(Extended_cached)  |     26.6 |
| BasicZoneManager::createForZoneName(binary)      |     15.9 |
| BasicZoneManager::createForZoneId(binary)        |      7.4 |
| BasicZoneManager::createForZoneId(linear)        |     50.6 |
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
sizeof(BasicZoneRegistrar): 8
sizeof(ExtendedZoneRegistrar): 8
sizeof(BasicZoneManager<1>): 176
sizeof(ExtendedZoneManager<1>): 552
sizeof(TimeZoneData): 8
sizeof(TimeZone): 12
sizeof(ZonedDateTime): 20
sizeof(TimePeriod): 4
sizeof(clock::DS3231Clock): 8
sizeof(clock::NtpClock): 116
sizeof(clock::SystemClock): 24
sizeof(clock::SystemClockLoop): 44
sizeof(clock::SystemClockCoroutine): 68
sizeof(internal::ZoneContext): 16
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
| LocalDate::forEpochDays()                        |      0.6 |
| LocalDate::toEpochDays()                         |      0.3 |
| LocalDate::dayOfWeek()                           |      0.3 |
| OffsetDateTime::forEpochSeconds()                |      1.0 |
| OffsetDateTime::toEpochSeconds()                 |      1.3 |
| ZonedDateTime::toEpochSeconds()                  |      1.3 |
| ZonedDateTime::toEpochDays()                     |      1.0 |
| ZonedDateTime::forEpochSeconds(UTC)              |      1.3 |
| ZonedDateTime::forEpochSeconds(Basic_nocache)    |     15.5 |
| ZonedDateTime::forEpochSeconds(Basic_cached)     |      2.5 |
| ZonedDateTime::forEpochSeconds(Extended_nocache) |     30.1 |
| ZonedDateTime::forEpochSeconds(Extended_cached)  |      2.4 |
| BasicZoneManager::createForZoneName(binary)      |      2.9 |
| BasicZoneManager::createForZoneId(binary)        |      0.6 |
| BasicZoneManager::createForZoneId(linear)        |      2.6 |
+--------------------------------------------------+----------+
Iterations_per_run: 50000

```

Note: Once the benchmark of the function under test becomes smaller than the
duration of an empty loop, the numbers become unreliable.

## Teensy 3.2

* 96 MHz ARM Cortex-M4
* Arduino IDE 1.8.13
* Teensyduino 1.53
* Compiler options: "Faster"

```
Sizes of Objects:
sizeof(LocalDate): 3
sizeof(LocalTime): 3
sizeof(LocalDateTime): 6
sizeof(TimeOffset): 2
sizeof(OffsetDateTime): 8
sizeof(BasicZoneProcessor): 156
sizeof(ExtendedZoneProcessor): 532
sizeof(BasicZoneRegistrar): 8
sizeof(ExtendedZoneRegistrar): 8
sizeof(BasicZoneManager<1>): 176
sizeof(ExtendedZoneManager<1>): 552
sizeof(TimeZoneData): 8
sizeof(TimeZone): 12
sizeof(ZonedDateTime): 20
sizeof(TimePeriod): 4
sizeof(clock::DS3231Clock): 8
sizeof(clock::SystemClock): 24
sizeof(clock::SystemClockLoop): 44
sizeof(clock::SystemClockCoroutine): 68
sizeof(internal::ZoneContext): 16
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
| EmptyLoop                                        |      0.4 |
|--------------------------------------------------+----------|
| LocalDate::forEpochDays()                        |      1.9 |
| LocalDate::toEpochDays()                         |      0.1 |
| LocalDate::dayOfWeek()                           |      1.0 |
| OffsetDateTime::forEpochSeconds()                |      2.5 |
| OffsetDateTime::toEpochSeconds()                 |      0.3 |
| ZonedDateTime::toEpochSeconds()                  |      0.3 |
| ZonedDateTime::toEpochDays()                     |      0.2 |
| ZonedDateTime::forEpochSeconds(UTC)              |      2.5 |
| ZonedDateTime::forEpochSeconds(Basic_nocache)    |     30.0 |
| ZonedDateTime::forEpochSeconds(Basic_cached)     |      6.0 |
| ZonedDateTime::forEpochSeconds(Extended_nocache) |     67.6 |
| ZonedDateTime::forEpochSeconds(Extended_cached)  |      5.8 |
| BasicZoneManager::createForZoneName(binary)      |      6.6 |
| BasicZoneManager::createForZoneId(binary)        |      2.3 |
| BasicZoneManager::createForZoneId(linear)        |     10.5 |
+--------------------------------------------------+----------+
Iterations_per_run: 50000

```

