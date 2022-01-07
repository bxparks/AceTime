# Auto Benchmark

Here are the results from `AutoBenchmark.ino` for various boards.
These results show that integer division and modulus operations are incredibly
slow on 8-bit AVR processors.

**Version**: AceTime v1.9.0

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

In v1.9.0:
* Extract `BasicZoneProcessorCache<SIZE>` and `ExtendedZoneProcessorCache<SIZE>`
  from `BasicZoneManager` and `ExtendedZoneManager`. Remove all pure `virtual`
  methods from `ZoneManager`, making ZoneManager hierarchy non-polymorphic.
    * Saves 1100-1300 of flash on AVR.
    * No signficant changes to CPU performance.

In v1.9.0+:
* Remove support for SAMD21 boards.
    * Arduino IDE 1.8.19 with SparkFun SAMD 1.8.6 can no longer upload binaries
      to these boards. Something about bossac 1.7.0 not found.
* Upgrade tool chain:
    * Arduino IDE from 1.8.13 to 1.8.19
    * Arduino AVR from 1.8.3 to 1.8.4
    * STM32duino from 2.0.0 to 2.2.0
    * ESP32 from 1.0.6 to 2.0.2
    * Teensyduino from 1.55 to 1.56
* Add support for `fold` parameter in `LocalDateTime`, `OffsetDateTime`,
  `ZonedDateTime`, and `ExtendedZoneProcessor`. No significant change in
  runtime.
* Add benchmarks for `ZonedDateTime::forComponents()`.

## Arduino Nano

* 16MHz ATmega328P
* Arduino IDE 1.8.19, Arduino CLI 0.19.2
* Arduino AVR Boards 1.8.4

```
Sizes of Objects:
sizeof(LocalDate): 3
sizeof(LocalTime): 3
sizeof(LocalDateTime): 6
sizeof(TimeOffset): 2
sizeof(OffsetDateTime): 8
sizeof(TimeZone): 5
sizeof(TimeZoneData): 5
sizeof(ZonedDateTime): 13
sizeof(TimePeriod): 4
sizeof(BasicZoneProcessor): 116
sizeof(ExtendedZoneProcessor): 432
sizeof(BasicZoneProcessorCache<1>): 120
sizeof(ExtendedZoneProcessorCache<1>): 436
sizeof(BasicZoneManager): 7
sizeof(ExtendedZoneManager): 7
sizeof(BasicLinkManager): 5
sizeof(ExtendedLinkManager): 5
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
| LocalDate::forEpochDays()                        |  219.000 |
| LocalDate::toEpochDays()                         |   54.000 |
| LocalDate::dayOfWeek()                           |   49.000 |
|--------------------------------------------------+----------|
| OffsetDateTime::forEpochSeconds()                |  323.000 |
| OffsetDateTime::toEpochSeconds()                 |   86.000 |
|--------------------------------------------------+----------|
| ZonedDateTime::toEpochSeconds()                  |   85.000 |
| ZonedDateTime::toEpochDays()                     |   73.000 |
| ZonedDateTime::forEpochSeconds(UTC)              |  338.000 |
| ZonedDateTime::forEpochSeconds(Basic_nocache)    | 1187.000 |
| ZonedDateTime::forEpochSeconds(Basic_cached)     |  616.000 |
| ZonedDateTime::forEpochSeconds(Extended_nocache) | 2136.000 |
| ZonedDateTime::forEpochSeconds(Extended_cached)  |  615.000 |
|--------------------------------------------------+----------|
| ZonedDateTime::forComponents(Extended_nocache)   | 1990.000 |
| ZonedDateTime::forComponents(Extended_cached)    |  474.000 |
|--------------------------------------------------+----------|
| BasicZoneManager::createForZoneName(binary)      |  119.000 |
| BasicZoneManager::createForZoneId(binary)        |   45.000 |
| BasicZoneManager::createForZoneId(linear)        |  304.000 |
+--------------------------------------------------+----------+
Iterations_per_run: 1000

```

## Sparkfun Pro Micro

* 16 MHz ATmega32U4
* Arduino IDE 1.8.19, Arduino CLI 0.19.2
* SparkFun AVR Boards 1.1.13

```
Sizes of Objects:
sizeof(LocalDate): 3
sizeof(LocalTime): 3
sizeof(LocalDateTime): 6
sizeof(TimeOffset): 2
sizeof(OffsetDateTime): 8
sizeof(TimeZone): 5
sizeof(TimeZoneData): 5
sizeof(ZonedDateTime): 13
sizeof(TimePeriod): 4
sizeof(BasicZoneProcessor): 116
sizeof(ExtendedZoneProcessor): 432
sizeof(BasicZoneProcessorCache<1>): 120
sizeof(ExtendedZoneProcessorCache<1>): 436
sizeof(BasicZoneManager): 7
sizeof(ExtendedZoneManager): 7
sizeof(BasicLinkManager): 5
sizeof(ExtendedLinkManager): 5
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
| EmptyLoop                                        |    3.000 |
|--------------------------------------------------+----------|
| LocalDate::forEpochDays()                        |  220.000 |
| LocalDate::toEpochDays()                         |   56.000 |
| LocalDate::dayOfWeek()                           |   49.000 |
|--------------------------------------------------+----------|
| OffsetDateTime::forEpochSeconds()                |  325.000 |
| OffsetDateTime::toEpochSeconds()                 |   85.000 |
|--------------------------------------------------+----------|
| ZonedDateTime::toEpochSeconds()                  |   85.000 |
| ZonedDateTime::toEpochDays()                     |   70.000 |
| ZonedDateTime::forEpochSeconds(UTC)              |  340.000 |
| ZonedDateTime::forEpochSeconds(Basic_nocache)    | 1194.000 |
| ZonedDateTime::forEpochSeconds(Basic_cached)     |  620.000 |
| ZonedDateTime::forEpochSeconds(Extended_nocache) | 2148.000 |
| ZonedDateTime::forEpochSeconds(Extended_cached)  |  618.000 |
|--------------------------------------------------+----------|
| ZonedDateTime::forComponents(Extended_nocache)   | 2002.000 |
| ZonedDateTime::forComponents(Extended_cached)    |  477.000 |
|--------------------------------------------------+----------|
| BasicZoneManager::createForZoneName(binary)      |  120.000 |
| BasicZoneManager::createForZoneId(binary)        |   49.000 |
| BasicZoneManager::createForZoneId(linear)        |  307.000 |
+--------------------------------------------------+----------+
Iterations_per_run: 1000

```

## STM32 Blue Pill

* STM32F103C8, 72 MHz ARM Cortex-M3
* Arduino IDE 1.8.19, Arduino CLI 0.19.2
* STM32duino 2.2.0

```
Sizes of Objects:
sizeof(LocalDate): 3
sizeof(LocalTime): 3
sizeof(LocalDateTime): 6
sizeof(TimeOffset): 2
sizeof(OffsetDateTime): 8
sizeof(TimeZone): 12
sizeof(TimeZoneData): 8
sizeof(ZonedDateTime): 20
sizeof(TimePeriod): 4
sizeof(BasicZoneProcessor): 164
sizeof(ExtendedZoneProcessor): 540
sizeof(BasicZoneProcessorCache<1>): 172
sizeof(ExtendedZoneProcessorCache<1>): 548
sizeof(BasicZoneManager): 12
sizeof(ExtendedZoneManager): 12
sizeof(BasicLinkManager): 8
sizeof(ExtendedLinkManager): 8
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
| EmptyLoop                                        |    1.100 |
|--------------------------------------------------+----------|
| LocalDate::forEpochDays()                        |    2.300 |
| LocalDate::toEpochDays()                         |    1.100 |
| LocalDate::dayOfWeek()                           |    1.200 |
|--------------------------------------------------+----------|
| OffsetDateTime::forEpochSeconds()                |    3.600 |
| OffsetDateTime::toEpochSeconds()                 |    4.800 |
|--------------------------------------------------+----------|
| ZonedDateTime::toEpochSeconds()                  |    4.500 |
| ZonedDateTime::toEpochDays()                     |    3.500 |
| ZonedDateTime::forEpochSeconds(UTC)              |    4.800 |
| ZonedDateTime::forEpochSeconds(Basic_nocache)    |   74.600 |
| ZonedDateTime::forEpochSeconds(Basic_cached)     |   10.500 |
| ZonedDateTime::forEpochSeconds(Extended_nocache) |  144.800 |
| ZonedDateTime::forEpochSeconds(Extended_cached)  |   10.300 |
|--------------------------------------------------+----------|
| ZonedDateTime::forComponents(Extended_nocache)   |  149.000 |
| ZonedDateTime::forComponents(Extended_cached)    |   14.800 |
|--------------------------------------------------+----------|
| BasicZoneManager::createForZoneName(binary)      |   11.600 |
| BasicZoneManager::createForZoneId(binary)        |    2.600 |
| BasicZoneManager::createForZoneId(linear)        |   17.800 |
+--------------------------------------------------+----------+
Iterations_per_run: 10000

```

## ESP8266

* NodeMCU 1.0 clone, 80MHz ESP8266
* Arduino IDE 1.8.19, Arduino CLI 0.19.2
* ESP8266 Boards 3.0.2

```
Sizes of Objects:
sizeof(LocalDate): 3
sizeof(LocalTime): 3
sizeof(LocalDateTime): 6
sizeof(TimeOffset): 2
sizeof(OffsetDateTime): 8
sizeof(TimeZone): 12
sizeof(TimeZoneData): 8
sizeof(ZonedDateTime): 20
sizeof(TimePeriod): 4
sizeof(BasicZoneProcessor): 164
sizeof(ExtendedZoneProcessor): 540
sizeof(BasicZoneProcessorCache<1>): 172
sizeof(ExtendedZoneProcessorCache<1>): 548
sizeof(BasicZoneManager): 12
sizeof(ExtendedZoneManager): 12
sizeof(BasicLinkManager): 8
sizeof(ExtendedLinkManager): 8
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
| LocalDate::forEpochDays()                        |    7.800 |
| LocalDate::toEpochDays()                         |    3.200 |
| LocalDate::dayOfWeek()                           |    3.600 |
|--------------------------------------------------+----------|
| OffsetDateTime::forEpochSeconds()                |   12.000 |
| OffsetDateTime::toEpochSeconds()                 |    6.800 |
|--------------------------------------------------+----------|
| ZonedDateTime::toEpochSeconds()                  |    6.600 |
| ZonedDateTime::toEpochDays()                     |    5.400 |
| ZonedDateTime::forEpochSeconds(UTC)              |   13.000 |
| ZonedDateTime::forEpochSeconds(Basic_nocache)    |   95.400 |
| ZonedDateTime::forEpochSeconds(Basic_cached)     |   25.800 |
| ZonedDateTime::forEpochSeconds(Extended_nocache) |  188.600 |
| ZonedDateTime::forEpochSeconds(Extended_cached)  |   26.000 |
|--------------------------------------------------+----------|
| ZonedDateTime::forComponents(Extended_nocache)   |  180.200 |
| ZonedDateTime::forComponents(Extended_cached)    |   18.800 |
|--------------------------------------------------+----------|
| BasicZoneManager::createForZoneName(binary)      |   14.600 |
| BasicZoneManager::createForZoneId(binary)        |    6.400 |
| BasicZoneManager::createForZoneId(linear)        |   44.400 |
+--------------------------------------------------+----------+
Iterations_per_run: 5000

```

## ESP32

* ESP32-01 Dev Board, 240 MHz Tensilica LX6
* Arduino IDE 1.8.19, Arduino CLI 0.19.2
* ESP32 Boards 2.0.2

```
Sizes of Objects:
sizeof(LocalDate): 3
sizeof(LocalTime): 3
sizeof(LocalDateTime): 6
sizeof(TimeOffset): 2
sizeof(OffsetDateTime): 8
sizeof(TimeZone): 12
sizeof(TimeZoneData): 8
sizeof(ZonedDateTime): 20
sizeof(TimePeriod): 4
sizeof(BasicZoneProcessor): 164
sizeof(ExtendedZoneProcessor): 540
sizeof(BasicZoneProcessorCache<1>): 172
sizeof(ExtendedZoneProcessorCache<1>): 548
sizeof(BasicZoneManager): 12
sizeof(ExtendedZoneManager): 12
sizeof(BasicLinkManager): 8
sizeof(ExtendedLinkManager): 8
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
| EmptyLoop                                        |    1.450 |
|--------------------------------------------------+----------|
| LocalDate::forEpochDays()                        |    0.550 |
| LocalDate::toEpochDays()                         |    0.250 |
| LocalDate::dayOfWeek()                           |    0.350 |
|--------------------------------------------------+----------|
| OffsetDateTime::forEpochSeconds()                |    1.000 |
| OffsetDateTime::toEpochSeconds()                 |    1.400 |
|--------------------------------------------------+----------|
| ZonedDateTime::toEpochSeconds()                  |    1.350 |
| ZonedDateTime::toEpochDays()                     |    0.950 |
| ZonedDateTime::forEpochSeconds(UTC)              |    1.300 |
| ZonedDateTime::forEpochSeconds(Basic_nocache)    |   16.050 |
| ZonedDateTime::forEpochSeconds(Basic_cached)     |    2.600 |
| ZonedDateTime::forEpochSeconds(Extended_nocache) |   34.100 |
| ZonedDateTime::forEpochSeconds(Extended_cached)  |    2.500 |
|--------------------------------------------------+----------|
| ZonedDateTime::forComponents(Extended_nocache)   |   34.400 |
| ZonedDateTime::forComponents(Extended_cached)    |    3.200 |
|--------------------------------------------------+----------|
| BasicZoneManager::createForZoneName(binary)      |    2.950 |
| BasicZoneManager::createForZoneId(binary)        |    0.650 |
| BasicZoneManager::createForZoneId(linear)        |    2.550 |
+--------------------------------------------------+----------+
Iterations_per_run: 20000

```

Note: Once the benchmark of the function under test becomes smaller than the
duration of an empty loop, the numbers become unreliable.

## Teensy 3.2

* 96 MHz ARM Cortex-M4
* Arduino IDE 1.8.19, Arduino CLI 0.19.2
* Teensyduino 1.56
* Compiler options: "Faster"

```
Sizes of Objects:
sizeof(LocalDate): 3
sizeof(LocalTime): 3
sizeof(LocalDateTime): 6
sizeof(TimeOffset): 2
sizeof(OffsetDateTime): 8
sizeof(TimeZone): 12
sizeof(TimeZoneData): 8
sizeof(ZonedDateTime): 20
sizeof(TimePeriod): 4
sizeof(BasicZoneProcessor): 164
sizeof(ExtendedZoneProcessor): 540
sizeof(BasicZoneProcessorCache<1>): 172
sizeof(ExtendedZoneProcessorCache<1>): 548
sizeof(BasicZoneManager): 12
sizeof(ExtendedZoneManager): 12
sizeof(BasicLinkManager): 8
sizeof(ExtendedLinkManager): 8
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
| LocalDate::forEpochDays()                        |    1.950 |
| LocalDate::toEpochDays()                         |    0.150 |
| LocalDate::dayOfWeek()                           |    0.900 |
|--------------------------------------------------+----------|
| OffsetDateTime::forEpochSeconds()                |    2.550 |
| OffsetDateTime::toEpochSeconds()                 |    0.500 |
|--------------------------------------------------+----------|
| ZonedDateTime::toEpochSeconds()                  |    0.250 |
| ZonedDateTime::toEpochDays()                     |    0.150 |
| ZonedDateTime::forEpochSeconds(UTC)              |    2.550 |
| ZonedDateTime::forEpochSeconds(Basic_nocache)    |   34.550 |
| ZonedDateTime::forEpochSeconds(Basic_cached)     |    6.400 |
| ZonedDateTime::forEpochSeconds(Extended_nocache) |   83.750 |
| ZonedDateTime::forEpochSeconds(Extended_cached)  |    5.700 |
|--------------------------------------------------+----------|
| ZonedDateTime::forComponents(Extended_nocache)   |   85.850 |
| ZonedDateTime::forComponents(Extended_cached)    |    8.050 |
|--------------------------------------------------+----------|
| BasicZoneManager::createForZoneName(binary)      |    6.050 |
| BasicZoneManager::createForZoneId(binary)        |    2.100 |
| BasicZoneManager::createForZoneId(linear)        |   10.450 |
+--------------------------------------------------+----------+
Iterations_per_run: 20000

```

