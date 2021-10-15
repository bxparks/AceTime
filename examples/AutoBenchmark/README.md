# Auto Benchmark

Here are the results from `AutoBenchmark.ino` for various boards.
These results show that integer division and modulus operations are incredibly
slow on 8-bit AVR processors.

**Version**: AceTime v1.8

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

v0.8 to v1.4:
* The CPU time did not change much from

In v1.5:
* No significant changes to CPU time.
* Zone registries (kZoneRegistry, kZoneAndLinkRegistry) are now sorted by zoneId
  instead of zoneName, and the `ZoneManager::createForZoneId()` will use a
  binary search, instead of a linear search. This makes it 10-15X faster for
  ~266 entries.
* The `ZoneManager::createForZoneName()` also converts to a zoneId, then
  performs a binary search, instead of doing a binary search on the zoneName
  directly. Even with the extra level of indirection, the `createForZoneName()`
  is between 1.5-2X faster than the previous version.

In v1.6:
* BasicZoneManager and ExtendedZoneManager can take an optional
  LinkRegistry which will be searched if a zoneId is not found. The
  `BasicZoneManager::createForZoneId(link)` benchmark shows that if the zoneId
  is not found, the total search time is roughly double, because the
  LinkRegistry must be search as a fallback.
* On some compilers, the `BasicZoneManager::createForZoneName(binary)` becames
  slightly slower (~10%?) because the algorithm was moved into the
  `ace_common::binarySearchByKey()` template function, and the compiler is not
  able to optimize the resulting function as well as the hand-rolled version.
  The slightly decrease in speed seemed acceptable cost to reduce duplicate code
  maintenance.

In v1.7.2:
* `SystemClock::clockMillis()` became non-virtual after incorporating
  AceRoutine v1.3. The sizeof `SystemClockLoop` and `SystemClockCoroutine`
  decreases 4 bytes on AVR, and 4-8 bytes on 32-bit processors. No signficant
  changes in CPU time.

In v1.7.5:
* significant changes to size of `ExtendedZoneProcessor`
    * 8-bit processors
        * increases by 24 bytes on AVR, due adding 1 pointer and 2
            `uint16_t` to MatchingEra
        * decreases by 48 bytes on AVR, by disabling
            `originalTransitionTime` unless
            `ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG` is enabled.
    * 32-bit processors
        * increases by 32 bytes on 32-bit processors due to adding
            a pointer and 2 `uint16_t` to MatchingEra
        * decreases by 32 bytes on 32-bit processors due to disabling
            `originalTransitionTime` in Transition
* Upgrade ESP8266 Core from 2.7.4 to 3.0.2.
    * AutoBenchmark indicate that things are a few percentage faster.

In v1.8.0:
* Remove `sizeof()` Clock classes which were moved to AceTimeClock library.
* No significant changes to excution times of various benchmarks.

## Arduino Nano

* 16MHz ATmega328P
* Arduino IDE 1.8.16, Arduino CLI 0.19.2
* Arduino AVR Boards 1.8.3

```
Sizes of Objects:
sizeof(LocalDate): 3
sizeof(LocalTime): 3
sizeof(LocalDateTime): 6
sizeof(TimeOffset): 2
sizeof(OffsetDateTime): 8
sizeof(BasicZoneProcessor): 116
sizeof(ExtendedZoneProcessor): 432
sizeof(BasicZoneManager<1>): 129
sizeof(ExtendedZoneManager<1>): 445
sizeof(TimeZoneData): 5
sizeof(TimeZone): 5
sizeof(ZonedDateTime): 13
sizeof(TimePeriod): 4
sizeof(internal::ZoneContext): 9
sizeof(basic::ZoneEra): 11
sizeof(basic::ZoneInfo): 11
sizeof(basic::ZoneRule): 9
sizeof(basic::ZonePolicy): 6
sizeof(basic::ZoneRegistrar): 5
sizeof(basic::LinkRegistrar): 5
sizeof(BasicZoneProcessor::Transition): 21
sizeof(ExtendedZoneProcessor::Transition): 40
sizeof(ExtendedZoneProcessor::TransitionStorage): 340
sizeof(ExtendedZoneProcessor::MatchingEra): 20

CPU:
+--------------------------------------------------+----------+
| Method                                           |   micros |
|--------------------------------------------------+----------|
| EmptyLoop                                        |    4.000 |
|--------------------------------------------------+----------|
| LocalDate::forEpochDays()                        |  221.000 |
| LocalDate::toEpochDays()                         |   56.000 |
| LocalDate::dayOfWeek()                           |   48.000 |
| OffsetDateTime::forEpochSeconds()                |  325.000 |
| OffsetDateTime::toEpochSeconds()                 |   84.000 |
| ZonedDateTime::toEpochSeconds()                  |   85.000 |
| ZonedDateTime::toEpochDays()                     |   71.000 |
| ZonedDateTime::forEpochSeconds(UTC)              |  339.000 |
| ZonedDateTime::forEpochSeconds(Basic_nocache)    | 1186.000 |
| ZonedDateTime::forEpochSeconds(Basic_cached)     |  615.000 |
| ZonedDateTime::forEpochSeconds(Extended_nocache) | 2137.000 |
| ZonedDateTime::forEpochSeconds(Extended_cached)  |  616.000 |
| BasicZoneManager::createForZoneName(binary)      |  119.000 |
| BasicZoneManager::createForZoneId(binary)        |   48.000 |
| BasicZoneManager::createForZoneId(linear)        |  307.000 |
+--------------------------------------------------+----------+
Iterations_per_run: 1000

```

## Sparkfun Pro Micro

* 16 MHz ATmega32U4
* Arduino IDE 1.8.16, Arduino CLI 0.19.2
* SparkFun AVR Boards 1.1.13

```
Sizes of Objects:
sizeof(LocalDate): 3
sizeof(LocalTime): 3
sizeof(LocalDateTime): 6
sizeof(TimeOffset): 2
sizeof(OffsetDateTime): 8
sizeof(BasicZoneProcessor): 116
sizeof(ExtendedZoneProcessor): 432
sizeof(BasicZoneManager<1>): 129
sizeof(ExtendedZoneManager<1>): 445
sizeof(TimeZoneData): 5
sizeof(TimeZone): 5
sizeof(ZonedDateTime): 13
sizeof(TimePeriod): 4
sizeof(internal::ZoneContext): 9
sizeof(basic::ZoneEra): 11
sizeof(basic::ZoneInfo): 11
sizeof(basic::ZoneRule): 9
sizeof(basic::ZonePolicy): 6
sizeof(basic::ZoneRegistrar): 5
sizeof(basic::LinkRegistrar): 5
sizeof(BasicZoneProcessor::Transition): 21
sizeof(ExtendedZoneProcessor::Transition): 40
sizeof(ExtendedZoneProcessor::TransitionStorage): 340
sizeof(ExtendedZoneProcessor::MatchingEra): 20

CPU:
+--------------------------------------------------+----------+
| Method                                           |   micros |
|--------------------------------------------------+----------|
| EmptyLoop                                        |    4.000 |
|--------------------------------------------------+----------|
| LocalDate::forEpochDays()                        |  221.000 |
| LocalDate::toEpochDays()                         |   54.000 |
| LocalDate::dayOfWeek()                           |   50.000 |
| OffsetDateTime::forEpochSeconds()                |  324.000 |
| OffsetDateTime::toEpochSeconds()                 |   80.000 |
| ZonedDateTime::toEpochSeconds()                  |   81.000 |
| ZonedDateTime::toEpochDays()                     |   70.000 |
| ZonedDateTime::forEpochSeconds(UTC)              |  339.000 |
| ZonedDateTime::forEpochSeconds(Basic_nocache)    | 1172.000 |
| ZonedDateTime::forEpochSeconds(Basic_cached)     |  619.000 |
| ZonedDateTime::forEpochSeconds(Extended_nocache) |   -1.000 |
| ZonedDateTime::forEpochSeconds(Extended_cache)   |   -1.000 |
| BasicZoneManager::createForZoneName(binary)      |  119.000 |
| BasicZoneManager::createForZoneId(binary)        |   48.000 |
| BasicZoneManager::createForZoneId(linear)        |  307.000 |
+--------------------------------------------------+----------+
Iterations_per_run: 1000

```

## SAMD21 M0 Mini

* 48 MHz ARM Cortex-M0+
* Arduino IDE 1.8.16, Arduino CLI 0.19.2
* Sparkfun SAMD Core 1.8.4

```
Sizes of Objects:
sizeof(LocalDate): 3
sizeof(LocalTime): 3
sizeof(LocalDateTime): 6
sizeof(TimeOffset): 2
sizeof(OffsetDateTime): 8
sizeof(BasicZoneProcessor): 164
sizeof(ExtendedZoneProcessor): 540
sizeof(BasicZoneManager<1>): 188
sizeof(ExtendedZoneManager<1>): 564
sizeof(TimeZoneData): 8
sizeof(TimeZone): 12
sizeof(ZonedDateTime): 20
sizeof(TimePeriod): 4
sizeof(internal::ZoneContext): 16
sizeof(basic::ZoneEra): 16
sizeof(basic::ZoneInfo): 20
sizeof(basic::ZoneRule): 9
sizeof(basic::ZonePolicy): 12
sizeof(basic::ZoneRegistrar): 8
sizeof(basic::LinkRegistrar): 8
sizeof(BasicZoneProcessor::Transition): 28
sizeof(ExtendedZoneProcessor::Transition): 48
sizeof(ExtendedZoneProcessor::TransitionStorage): 420
sizeof(ExtendedZoneProcessor::MatchingEra): 24

CPU:
+--------------------------------------------------+----------+
| Method                                           |   micros |
|--------------------------------------------------+----------|
| EmptyLoop                                        |    1.400 |
|--------------------------------------------------+----------|
| LocalDate::forEpochDays()                        |   23.400 |
| LocalDate::toEpochDays()                         |    8.600 |
| LocalDate::dayOfWeek()                           |   11.000 |
| OffsetDateTime::forEpochSeconds()                |   35.000 |
| OffsetDateTime::toEpochSeconds()                 |   18.600 |
| ZonedDateTime::toEpochSeconds()                  |   18.200 |
| ZonedDateTime::toEpochDays()                     |   16.000 |
| ZonedDateTime::forEpochSeconds(UTC)              |   37.000 |
| ZonedDateTime::forEpochSeconds(Basic_nocache)    |  235.400 |
| ZonedDateTime::forEpochSeconds(Basic_cached)     |   71.800 |
| ZonedDateTime::forEpochSeconds(Extended_nocache) |  477.000 |
| ZonedDateTime::forEpochSeconds(Extended_cached)  |   71.600 |
| BasicZoneManager::createForZoneName(binary)      |   16.000 |
| BasicZoneManager::createForZoneId(binary)        |    4.600 |
| BasicZoneManager::createForZoneId(linear)        |   14.800 |
+--------------------------------------------------+----------+
Iterations_per_run: 5000

```

## STM32 Blue Pill

* STM32F103C8, 72 MHz ARM Cortex-M3
* Arduino IDE 1.8.16, Arduino CLI 0.19.2
* STM32duino 2.0.0

```
Sizes of Objects:
sizeof(LocalDate): 3
sizeof(LocalTime): 3
sizeof(LocalDateTime): 6
sizeof(TimeOffset): 2
sizeof(OffsetDateTime): 8
sizeof(BasicZoneProcessor): 164
sizeof(ExtendedZoneProcessor): 540
sizeof(BasicZoneManager<1>): 188
sizeof(ExtendedZoneManager<1>): 564
sizeof(TimeZoneData): 8
sizeof(TimeZone): 12
sizeof(ZonedDateTime): 20
sizeof(TimePeriod): 4
sizeof(internal::ZoneContext): 16
sizeof(basic::ZoneEra): 16
sizeof(basic::ZoneInfo): 20
sizeof(basic::ZoneRule): 9
sizeof(basic::ZonePolicy): 12
sizeof(basic::ZoneRegistrar): 8
sizeof(basic::LinkRegistrar): 8
sizeof(BasicZoneProcessor::Transition): 28
sizeof(ExtendedZoneProcessor::Transition): 48
sizeof(ExtendedZoneProcessor::TransitionStorage): 420
sizeof(ExtendedZoneProcessor::MatchingEra): 24

CPU:
+--------------------------------------------------+----------+
| Method                                           |   micros |
|--------------------------------------------------+----------|
| EmptyLoop                                        |    1.200 |
|--------------------------------------------------+----------|
| LocalDate::forEpochDays()                        |    2.400 |
| LocalDate::toEpochDays()                         |    1.000 |
| LocalDate::dayOfWeek()                           |    1.300 |
| OffsetDateTime::forEpochSeconds()                |    3.700 |
| OffsetDateTime::toEpochSeconds()                 |    4.700 |
| ZonedDateTime::toEpochSeconds()                  |    4.800 |
| ZonedDateTime::toEpochDays()                     |    3.600 |
| ZonedDateTime::forEpochSeconds(UTC)              |    4.700 |
| ZonedDateTime::forEpochSeconds(Basic_nocache)    |   75.200 |
| ZonedDateTime::forEpochSeconds(Basic_cached)     |   10.300 |
| ZonedDateTime::forEpochSeconds(Extended_nocache) |  147.400 |
| ZonedDateTime::forEpochSeconds(Extended_cached)  |   10.300 |
| BasicZoneManager::createForZoneName(binary)      |   12.800 |
| BasicZoneManager::createForZoneId(binary)        |    3.400 |
| BasicZoneManager::createForZoneId(linear)        |   17.800 |
+--------------------------------------------------+----------+
Iterations_per_run: 10000

```

## ESP8266

* NodeMCU 1.0 clone, 80MHz ESP8266
* Arduino IDE 1.8.16, Arduino CLI 0.19.2
* ESP8266 Boards 3.0.2

```
Sizes of Objects:
sizeof(LocalDate): 3
sizeof(LocalTime): 3
sizeof(LocalDateTime): 6
sizeof(TimeOffset): 2
sizeof(OffsetDateTime): 8
sizeof(BasicZoneProcessor): 164
sizeof(ExtendedZoneProcessor): 540
sizeof(BasicZoneManager<1>): 188
sizeof(ExtendedZoneManager<1>): 564
sizeof(TimeZoneData): 8
sizeof(TimeZone): 12
sizeof(ZonedDateTime): 20
sizeof(TimePeriod): 4
sizeof(internal::ZoneContext): 16
sizeof(basic::ZoneEra): 16
sizeof(basic::ZoneInfo): 20
sizeof(basic::ZoneRule): 9
sizeof(basic::ZonePolicy): 12
sizeof(basic::ZoneRegistrar): 8
sizeof(basic::LinkRegistrar): 8
sizeof(BasicZoneProcessor::Transition): 28
sizeof(ExtendedZoneProcessor::Transition): 48
sizeof(ExtendedZoneProcessor::TransitionStorage): 420
sizeof(ExtendedZoneProcessor::MatchingEra): 24

CPU:
+--------------------------------------------------+----------+
| Method                                           |   micros |
|--------------------------------------------------+----------|
| EmptyLoop                                        |    4.800 |
|--------------------------------------------------+----------|
| LocalDate::forEpochDays()                        |    7.900 |
| LocalDate::toEpochDays()                         |    3.800 |
| LocalDate::dayOfWeek()                           |    3.600 |
| OffsetDateTime::forEpochSeconds()                |   12.100 |
| OffsetDateTime::toEpochSeconds()                 |    6.700 |
| ZonedDateTime::toEpochSeconds()                  |    6.900 |
| ZonedDateTime::toEpochDays()                     |    5.800 |
| ZonedDateTime::forEpochSeconds(UTC)              |   12.900 |
| ZonedDateTime::forEpochSeconds(Basic_nocache)    |   95.400 |
| ZonedDateTime::forEpochSeconds(Basic_cached)     |   26.000 |
| ZonedDateTime::forEpochSeconds(Extended_nocache) |  189.600 |
| ZonedDateTime::forEpochSeconds(Extended_cached)  |   25.800 |
| BasicZoneManager::createForZoneName(binary)      |   14.800 |
| BasicZoneManager::createForZoneId(binary)        |    6.500 |
| BasicZoneManager::createForZoneId(linear)        |   44.200 |
+--------------------------------------------------+----------+
Iterations_per_run: 10000

```

## ESP32

* ESP32-01 Dev Board, 240 MHz Tensilica LX6
* Arduino IDE 1.8.16, Arduino CLI 0.19.2
* ESP32 Boards 1.0.6

```
Sizes of Objects:
sizeof(LocalDate): 3
sizeof(LocalTime): 3
sizeof(LocalDateTime): 6
sizeof(TimeOffset): 2
sizeof(OffsetDateTime): 8
sizeof(BasicZoneProcessor): 164
sizeof(ExtendedZoneProcessor): 540
sizeof(BasicZoneManager<1>): 188
sizeof(ExtendedZoneManager<1>): 564
sizeof(TimeZoneData): 8
sizeof(TimeZone): 12
sizeof(ZonedDateTime): 20
sizeof(TimePeriod): 4
sizeof(internal::ZoneContext): 16
sizeof(basic::ZoneEra): 16
sizeof(basic::ZoneInfo): 20
sizeof(basic::ZoneRule): 9
sizeof(basic::ZonePolicy): 12
sizeof(basic::ZoneRegistrar): 8
sizeof(basic::LinkRegistrar): 8
sizeof(BasicZoneProcessor::Transition): 28
sizeof(ExtendedZoneProcessor::Transition): 48
sizeof(ExtendedZoneProcessor::TransitionStorage): 420
sizeof(ExtendedZoneProcessor::MatchingEra): 24

CPU:
+--------------------------------------------------+----------+
| Method                                           |   micros |
|--------------------------------------------------+----------|
| EmptyLoop                                        |    1.400 |
|--------------------------------------------------+----------|
| LocalDate::forEpochDays()                        |    0.600 |
| LocalDate::toEpochDays()                         |    0.250 |
| LocalDate::dayOfWeek()                           |    0.350 |
| OffsetDateTime::forEpochSeconds()                |    1.050 |
| OffsetDateTime::toEpochSeconds()                 |    1.400 |
| ZonedDateTime::toEpochSeconds()                  |    1.350 |
| ZonedDateTime::toEpochDays()                     |    0.950 |
| ZonedDateTime::forEpochSeconds(UTC)              |    1.350 |
| ZonedDateTime::forEpochSeconds(Basic_nocache)    |   16.100 |
| ZonedDateTime::forEpochSeconds(Basic_cached)     |    2.600 |
| ZonedDateTime::forEpochSeconds(Extended_nocache) |   34.450 |
| ZonedDateTime::forEpochSeconds(Extended_cached)  |    2.500 |
| BasicZoneManager::createForZoneName(binary)      |    2.950 |
| BasicZoneManager::createForZoneId(binary)        |    0.750 |
| BasicZoneManager::createForZoneId(linear)        |    2.550 |
+--------------------------------------------------+----------+
Iterations_per_run: 20000

```

Note: Once the benchmark of the function under test becomes smaller than the
duration of an empty loop, the numbers become unreliable.

## Teensy 3.2

* 96 MHz ARM Cortex-M4
* Arduino IDE 1.8.16, Arduino CLI 0.19.2
* Teensyduino 1.55
* Compiler options: "Faster"

```
Sizes of Objects:
sizeof(LocalDate): 3
sizeof(LocalTime): 3
sizeof(LocalDateTime): 6
sizeof(TimeOffset): 2
sizeof(OffsetDateTime): 8
sizeof(BasicZoneProcessor): 164
sizeof(ExtendedZoneProcessor): 540
sizeof(BasicZoneManager<1>): 188
sizeof(ExtendedZoneManager<1>): 564
sizeof(TimeZoneData): 8
sizeof(TimeZone): 12
sizeof(ZonedDateTime): 20
sizeof(TimePeriod): 4
sizeof(internal::ZoneContext): 16
sizeof(basic::ZoneEra): 16
sizeof(basic::ZoneInfo): 20
sizeof(basic::ZoneRule): 9
sizeof(basic::ZonePolicy): 12
sizeof(basic::ZoneRegistrar): 8
sizeof(basic::LinkRegistrar): 8
sizeof(BasicZoneProcessor::Transition): 28
sizeof(ExtendedZoneProcessor::Transition): 48
sizeof(ExtendedZoneProcessor::TransitionStorage): 420
sizeof(ExtendedZoneProcessor::MatchingEra): 24

CPU:
+--------------------------------------------------+----------+
| Method                                           |   micros |
|--------------------------------------------------+----------|
| EmptyLoop                                        |    0.400 |
|--------------------------------------------------+----------|
| LocalDate::forEpochDays()                        |    1.850 |
| LocalDate::toEpochDays()                         |    0.200 |
| LocalDate::dayOfWeek()                           |    1.000 |
| OffsetDateTime::forEpochSeconds()                |    2.400 |
| OffsetDateTime::toEpochSeconds()                 |    0.550 |
| ZonedDateTime::toEpochSeconds()                  |    0.300 |
| ZonedDateTime::toEpochDays()                     |    0.550 |
| ZonedDateTime::forEpochSeconds(UTC)              |    2.600 |
| ZonedDateTime::forEpochSeconds(Basic_nocache)    |   34.650 |
| ZonedDateTime::forEpochSeconds(Basic_cached)     |    6.550 |
| ZonedDateTime::forEpochSeconds(Extended_nocache) |   82.350 |
| ZonedDateTime::forEpochSeconds(Extended_cached)  |    5.600 |
| BasicZoneManager::createForZoneName(binary)      |    5.950 |
| BasicZoneManager::createForZoneId(binary)        |    2.150 |
| BasicZoneManager::createForZoneId(linear)        |   10.300 |
+--------------------------------------------------+----------+
Iterations_per_run: 20000

```

