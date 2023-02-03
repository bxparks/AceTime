# Auto Benchmark

Here are the results from `AutoBenchmark.ino` for various boards.
These results show that integer division and modulus operations are incredibly
slow on 8-bit AVR processors.

**Version**: AceTime v2.1.1

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

**v0.8 to v1.4:**
* The CPU time did not change much from

**v1.5:**
* No significant changes to CPU time.
* Zone registries (kZoneRegistry, kZoneAndLinkRegistry) are now sorted by zoneId
  instead of zoneName, and the `ZoneManager::createForZoneId()` will use a
  binary search, instead of a linear search. This makes it 10-15X faster for
  ~266 entries.
* The `ZoneManager::createForZoneName()` also converts to a zoneId, then
  performs a binary search, instead of doing a binary search on the zoneName
  directly. Even with the extra level of indirection, the `createForZoneName()`
  is between 1.5-2X faster than the previous version.

**v1.6:**
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

**v1.7.2:**
* `SystemClock::clockMillis()` became non-virtual after incorporating
  AceRoutine v1.3. The sizeof `SystemClockLoop` and `SystemClockCoroutine`
  decreases 4 bytes on AVR, and 4-8 bytes on 32-bit processors. No signficant
  changes in CPU time.

**v1.7.5:**
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

**v1.8.0:**
* Remove `sizeof()` Clock classes which were moved to AceTimeClock library.
* No significant changes to excution times of various benchmarks.

**v1.9.0:**
* Extract `BasicZoneProcessorCache<SIZE>` and `ExtendedZoneProcessorCache<SIZE>`
  from `BasicZoneManager` and `ExtendedZoneManager`. Remove all pure `virtual`
  methods from `ZoneManager`, making ZoneManager hierarchy non-polymorphic.
    * Saves 1100-1300 of flash on AVR.
    * No signficant changes to CPU performance.

**v1.10.0:**
* Remove support for SAMD21 boards.
    * Arduino IDE 1.8.19 with SparkFun SAMD 1.8.6 can no longer upload binaries
      to these boards. Something about bossac 1.7.0 not found.
* Upgrade tool chain:
    * Arduino IDE from 1.8.13 to 1.8.19
    * Arduino AVR from 1.8.3 to 1.8.4
    * STM32duino from 2.0.0 to 2.2.0
    * ESP32 from 1.0.6 to 2.0.2
    * Teensyduino from 1.55 to 1.56
* Add benchmarks for `ZonedDateTime::forComponents()`.
* Add support for `fold` parameter in `LocalDateTime`, `OffsetDateTime`,
  `ZonedDateTime`, and `ExtendedZoneProcessor`.
    * The `ZonedDateTime::forComponents()` can be made much faster using 'fold'.
    * We know exactly when we must normalize and when we can avoid
      normalization.
    * 5X faster on AVR processors when cached, and
    * 1.5-3X faster on 32-bit processors.

**v1.11.0:**
* Upgrade ZoneInfo database so that Links are symbolic links to Zones, instead
  of hard links to Zones.
    * No significant changes to CPU benchmarks.

**v1.11.5**
* Upgrade tool chain
    * Arduino CLI from 0.20.2 to 0.27.1
    * Arduino AVR Core from 1.8.4 to 1.8.5
    * STM32duino from 2.2.0 to 2.3.0
    * ESP32 Core from 2.0.2 to 2.0.5
    * Teensyduino from 1.56 to 1.57
* Upgrade TZDB from 2022b to 2022d

**v2.0**
* Use `int16_t` year fields.
* Implement adjustable epoch year.
* Upgrade to TZDB 2022f.
* AVR:
    * sizeof(LocalDate) increases from 3 to 4
    * sizeof(BasicZoneProcessor) increases from 116 to 122
    * sizeof(ExtendedZoneProcessor) increases from 436 to 468
    * sizeof(TransitionStorage) increases from 340 to 364
    * ZonedDateTime::forEpochSeconds() slower by 5-10%
* ESP8266
    * sizeof(LocalDate) increases from 3 to 4
    * sizeof(BasicZoneProcessor) remains at 164
    * sizeof(ExtendedZoneProcessor) increases from 540 to 588
    * sizeof(TransitionStorage) increases from 420 to 452
    * ZonedDateTime::forEpochSeconds() slower by 0-10%

**v2.1.1**
* Upgrade to TZDB 2022g.
* Add `ZonedExtra`.
* Unify fat and symbolic links.
* Not much difference in execution times, except:
    * `ZonedDateTime::forComponents()` using the `BasicZoneProcessor`
      becomes ~50% slower due to the extra work needed to resolve gaps and
      overlaps.
    * `ZonedDateTime::forEpochSeconds()` using `BasicZoneProcessors` remains
      unchanged.
    * `ExtendedZoneProcessor` is substantially faster on AVR processors.
       Maybe it should be recommended ove `BasicZoneProcessor` even on AVR.

## Arduino Nano

* 16MHz ATmega328P
* Arduino IDE 1.8.19, Arduino CLI 0.27.1
* Arduino AVR Boards 1.8.5

```
Sizes of Objects:
sizeof(LocalDate): 4
sizeof(LocalTime): 4
sizeof(LocalDateTime): 8
sizeof(TimeOffset): 2
sizeof(OffsetDateTime): 10
sizeof(TimeZone): 5
sizeof(TimeZoneData): 5
sizeof(ZonedDateTime): 15
sizeof(ZonedExtra): 16
sizeof(TimePeriod): 4
sizeof(BasicZoneProcessor): 122
sizeof(ExtendedZoneProcessor): 464
sizeof(BasicZoneProcessorCache<1>): 126
sizeof(ExtendedZoneProcessorCache<1>): 468
sizeof(BasicZoneManager): 7
sizeof(ExtendedZoneManager): 7
sizeof(internal::ZoneContext): 9
sizeof(basic::ZoneEra): 12
sizeof(basic::ZoneInfo): 13
sizeof(basic::ZoneRule): 11
sizeof(basic::ZonePolicy): 6
sizeof(basic::ZoneRegistrar): 5
sizeof(BasicZoneProcessor::Transition): 22
sizeof(ExtendedZoneProcessor::Transition): 43
sizeof(ExtendedZoneProcessor::TransitionStorage): 364
sizeof(ExtendedZoneProcessor::MatchingEra): 22

CPU:
+--------------------------------------------------+----------+
| Method                                           |   micros |
|--------------------------------------------------+----------|
| EmptyLoop                                        |    3.000 |
|--------------------------------------------------+----------|
| LocalDate::forEpochDays()                        |  243.000 |
| LocalDate::toEpochDays()                         |   52.000 |
| LocalDate::dayOfWeek()                           |   49.000 |
|--------------------------------------------------+----------|
| OffsetDateTime::forEpochSeconds()                |  363.000 |
| OffsetDateTime::toEpochSeconds()                 |   77.000 |
|--------------------------------------------------+----------|
| ZonedDateTime::toEpochSeconds()                  |   78.000 |
| ZonedDateTime::toEpochDays()                     |   66.000 |
| ZonedDateTime::forEpochSeconds(UTC)              |  391.000 |
|--------------------------------------------------+----------|
| ZonedDateTime::forEpochSeconds(Basic_nocache)    | 1224.000 |
| ZonedDateTime::forEpochSeconds(Basic_cached)     |  699.000 |
| ZonedDateTime::forEpochSeconds(Extended_nocache) |   -1.000 |
| ZonedDateTime::forEpochSeconds(Extended_cached)  |   -1.000 |
|--------------------------------------------------+----------|
| ZonedDateTime::forComponents(Basic_nocache)      | 1769.000 |
| ZonedDateTime::forComponents(Basic_cached)       | 1225.000 |
| ZonedDateTime::forComponents(Extended_nocache)   |   -1.000 |
| ZonedDateTime::forComponents(Extended_cached)    |   -1.000 |
|--------------------------------------------------+----------|
| ZonedExtra::forEpochSeconds(Basic_nocache)       |  852.000 |
| ZonedExtra::forEpochSeconds(Basic_cached)        |  325.000 |
| ZonedExtra::forEpochSeconds(Extended_nocache)    |   -1.000 |
| ZonedExtra::forEpochSeconds(Extended_cached)     |   -1.000 |
|--------------------------------------------------+----------|
| ZonedExtra::forComponents(Basic_nocache)         | 1746.000 |
| ZonedExtra::forComponents(Basic_cached)          | 1202.000 |
| ZonedExtra::forComponents(Extended_nocache)      |   -1.000 |
| ZonedExtra::forComponents(Extended_cached)       |   -1.000 |
|--------------------------------------------------+----------|
| BasicZoneManager::createForZoneName(binary)      |  115.000 |
| BasicZoneManager::createForZoneId(binary)        |   47.000 |
| BasicZoneManager::createForZoneId(linear)        |  299.000 |
+--------------------------------------------------+----------+
Iterations_per_run: 1000

```

## Sparkfun Pro Micro

* 16 MHz ATmega32U4
* Arduino IDE 1.8.19, Arduino CLI 0.27.1
* SparkFun AVR Boards 1.1.13

```
Sizes of Objects:
sizeof(LocalDate): 4
sizeof(LocalTime): 4
sizeof(LocalDateTime): 8
sizeof(TimeOffset): 2
sizeof(OffsetDateTime): 10
sizeof(TimeZone): 5
sizeof(TimeZoneData): 5
sizeof(ZonedDateTime): 15
sizeof(ZonedExtra): 16
sizeof(TimePeriod): 4
sizeof(BasicZoneProcessor): 122
sizeof(ExtendedZoneProcessor): 464
sizeof(BasicZoneProcessorCache<1>): 126
sizeof(ExtendedZoneProcessorCache<1>): 468
sizeof(BasicZoneManager): 7
sizeof(ExtendedZoneManager): 7
sizeof(internal::ZoneContext): 9
sizeof(basic::ZoneEra): 12
sizeof(basic::ZoneInfo): 13
sizeof(basic::ZoneRule): 11
sizeof(basic::ZonePolicy): 6
sizeof(basic::ZoneRegistrar): 5
sizeof(BasicZoneProcessor::Transition): 22
sizeof(ExtendedZoneProcessor::Transition): 43
sizeof(ExtendedZoneProcessor::TransitionStorage): 364
sizeof(ExtendedZoneProcessor::MatchingEra): 22

CPU:
+--------------------------------------------------+----------+
| Method                                           |   micros |
|--------------------------------------------------+----------|
| EmptyLoop                                        |    3.000 |
|--------------------------------------------------+----------|
| LocalDate::forEpochDays()                        |  245.000 |
| LocalDate::toEpochDays()                         |   51.000 |
| LocalDate::dayOfWeek()                           |   50.000 |
|--------------------------------------------------+----------|
| OffsetDateTime::forEpochSeconds()                |  367.000 |
| OffsetDateTime::toEpochSeconds()                 |   78.000 |
|--------------------------------------------------+----------|
| ZonedDateTime::toEpochSeconds()                  |   78.000 |
| ZonedDateTime::toEpochDays()                     |   68.000 |
| ZonedDateTime::forEpochSeconds(UTC)              |  393.000 |
|--------------------------------------------------+----------|
| ZonedDateTime::forEpochSeconds(Basic_nocache)    | 1230.000 |
| ZonedDateTime::forEpochSeconds(Basic_cached)     |  702.000 |
| ZonedDateTime::forEpochSeconds(Extended_nocache) |   -1.000 |
| ZonedDateTime::forEpochSeconds(Extended_cached)  |   -1.000 |
|--------------------------------------------------+----------|
| ZonedDateTime::forComponents(Basic_nocache)      | 1777.000 |
| ZonedDateTime::forComponents(Basic_cached)       | 1231.000 |
| ZonedDateTime::forComponents(Extended_nocache)   |   -1.000 |
| ZonedDateTime::forComponents(Extended_cached)    |   -1.000 |
|--------------------------------------------------+----------|
| ZonedExtra::forEpochSeconds(Basic_nocache)       |  856.000 |
| ZonedExtra::forEpochSeconds(Basic_cached)        |  325.000 |
| ZonedExtra::forEpochSeconds(Extended_nocache)    |   -1.000 |
| ZonedExtra::forEpochSeconds(Extended_cached)     |   -1.000 |
|--------------------------------------------------+----------|
| ZonedExtra::forComponents(Basic_nocache)         | 1754.000 |
| ZonedExtra::forComponents(Basic_cached)          | 1209.000 |
| ZonedExtra::forComponents(Extended_nocache)      |   -1.000 |
| ZonedExtra::forComponents(Extended_cached)       |   -1.000 |
|--------------------------------------------------+----------|
| BasicZoneManager::createForZoneName(binary)      |  116.000 |
| BasicZoneManager::createForZoneId(binary)        |   50.000 |
| BasicZoneManager::createForZoneId(linear)        |  298.000 |
+--------------------------------------------------+----------+
Iterations_per_run: 1000

```

## STM32 Blue Pill

* STM32F103C8, 72 MHz ARM Cortex-M3
* Arduino IDE 1.8.19, Arduino CLI 0.27.1
* STM32duino 2.3.0

```
Sizes of Objects:
sizeof(LocalDate): 4
sizeof(LocalTime): 4
sizeof(LocalDateTime): 8
sizeof(TimeOffset): 2
sizeof(OffsetDateTime): 10
sizeof(TimeZone): 12
sizeof(TimeZoneData): 8
sizeof(ZonedDateTime): 24
sizeof(ZonedExtra): 16
sizeof(TimePeriod): 4
sizeof(BasicZoneProcessor): 164
sizeof(ExtendedZoneProcessor): 588
sizeof(BasicZoneProcessorCache<1>): 172
sizeof(ExtendedZoneProcessorCache<1>): 596
sizeof(BasicZoneManager): 12
sizeof(ExtendedZoneManager): 12
sizeof(internal::ZoneContext): 16
sizeof(basic::ZoneEra): 16
sizeof(basic::ZoneInfo): 24
sizeof(basic::ZoneRule): 12
sizeof(basic::ZonePolicy): 12
sizeof(basic::ZoneRegistrar): 8
sizeof(BasicZoneProcessor::Transition): 28
sizeof(ExtendedZoneProcessor::Transition): 52
sizeof(ExtendedZoneProcessor::TransitionStorage): 452
sizeof(ExtendedZoneProcessor::MatchingEra): 28

CPU:
+--------------------------------------------------+----------+
| Method                                           |   micros |
|--------------------------------------------------+----------|
| EmptyLoop                                        |    1.200 |
|--------------------------------------------------+----------|
| LocalDate::forEpochDays()                        |    2.800 |
| LocalDate::toEpochDays()                         |    1.300 |
| LocalDate::dayOfWeek()                           |    1.500 |
|--------------------------------------------------+----------|
| OffsetDateTime::forEpochSeconds()                |    4.300 |
| OffsetDateTime::toEpochSeconds()                 |    5.100 |
|--------------------------------------------------+----------|
| ZonedDateTime::toEpochSeconds()                  |    5.100 |
| ZonedDateTime::toEpochDays()                     |    3.700 |
| ZonedDateTime::forEpochSeconds(UTC)              |    6.400 |
|--------------------------------------------------+----------|
| ZonedDateTime::forEpochSeconds(Basic_nocache)    |   76.700 |
| ZonedDateTime::forEpochSeconds(Basic_cached)     |   12.400 |
| ZonedDateTime::forEpochSeconds(Extended_nocache) |  193.500 |
| ZonedDateTime::forEpochSeconds(Extended_cached)  |   16.600 |
|--------------------------------------------------+----------|
| ZonedDateTime::forComponents(Basic_nocache)      |  101.200 |
| ZonedDateTime::forComponents(Basic_cached)       |   36.600 |
| ZonedDateTime::forComponents(Extended_nocache)   |  148.100 |
| ZonedDateTime::forComponents(Extended_cached)    |    7.300 |
|--------------------------------------------------+----------|
| ZonedExtra::forEpochSeconds(Basic_nocache)       |   72.400 |
| ZonedExtra::forEpochSeconds(Basic_cached)        |    7.700 |
| ZonedExtra::forEpochSeconds(Extended_nocache)    |  188.700 |
| ZonedExtra::forEpochSeconds(Extended_cached)     |   11.700 |
|--------------------------------------------------+----------|
| ZonedExtra::forComponents(Basic_nocache)         |  100.600 |
| ZonedExtra::forComponents(Basic_cached)          |   35.900 |
| ZonedExtra::forComponents(Extended_nocache)      |  147.500 |
| ZonedExtra::forComponents(Extended_cached)       |    6.700 |
|--------------------------------------------------+----------|
| BasicZoneManager::createForZoneName(binary)      |   11.500 |
| BasicZoneManager::createForZoneId(binary)        |    2.300 |
| BasicZoneManager::createForZoneId(linear)        |   17.700 |
+--------------------------------------------------+----------+
Iterations_per_run: 10000

```

## ESP8266

* NodeMCU 1.0 clone, 80MHz ESP8266
* Arduino IDE 1.8.19, Arduino CLI 0.27.1
* ESP8266 Boards 3.0.2

```
Sizes of Objects:
sizeof(LocalDate): 4
sizeof(LocalTime): 4
sizeof(LocalDateTime): 8
sizeof(TimeOffset): 2
sizeof(OffsetDateTime): 10
sizeof(TimeZone): 12
sizeof(TimeZoneData): 8
sizeof(ZonedDateTime): 24
sizeof(ZonedExtra): 16
sizeof(TimePeriod): 4
sizeof(BasicZoneProcessor): 164
sizeof(ExtendedZoneProcessor): 588
sizeof(BasicZoneProcessorCache<1>): 172
sizeof(ExtendedZoneProcessorCache<1>): 596
sizeof(BasicZoneManager): 12
sizeof(ExtendedZoneManager): 12
sizeof(internal::ZoneContext): 16
sizeof(basic::ZoneEra): 16
sizeof(basic::ZoneInfo): 24
sizeof(basic::ZoneRule): 12
sizeof(basic::ZonePolicy): 12
sizeof(basic::ZoneRegistrar): 8
sizeof(BasicZoneProcessor::Transition): 28
sizeof(ExtendedZoneProcessor::Transition): 52
sizeof(ExtendedZoneProcessor::TransitionStorage): 452
sizeof(ExtendedZoneProcessor::MatchingEra): 28

CPU:
+--------------------------------------------------+----------+
| Method                                           |   micros |
|--------------------------------------------------+----------|
| EmptyLoop                                        |    4.800 |
|--------------------------------------------------+----------|
| LocalDate::forEpochDays()                        |    6.200 |
| LocalDate::toEpochDays()                         |    3.000 |
| LocalDate::dayOfWeek()                           |    3.800 |
|--------------------------------------------------+----------|
| OffsetDateTime::forEpochSeconds()                |   12.400 |
| OffsetDateTime::toEpochSeconds()                 |    7.000 |
|--------------------------------------------------+----------|
| ZonedDateTime::toEpochSeconds()                  |    6.600 |
| ZonedDateTime::toEpochDays()                     |    5.800 |
| ZonedDateTime::forEpochSeconds(UTC)              |   15.000 |
|--------------------------------------------------+----------|
| ZonedDateTime::forEpochSeconds(Basic_nocache)    |   93.800 |
| ZonedDateTime::forEpochSeconds(Basic_cached)     |   22.200 |
| ZonedDateTime::forEpochSeconds(Extended_nocache) |  232.000 |
| ZonedDateTime::forEpochSeconds(Extended_cached)  |   29.200 |
|--------------------------------------------------+----------|
| ZonedDateTime::forComponents(Basic_nocache)      |  116.600 |
| ZonedDateTime::forComponents(Basic_cached)       |   47.400 |
| ZonedDateTime::forComponents(Extended_nocache)   |  165.200 |
| ZonedDateTime::forComponents(Extended_cached)    |    3.600 |
|--------------------------------------------------+----------|
| ZonedExtra::forEpochSeconds(Basic_nocache)       |   83.800 |
| ZonedExtra::forEpochSeconds(Basic_cached)        |    8.600 |
| ZonedExtra::forEpochSeconds(Extended_nocache)    |  217.600 |
| ZonedExtra::forEpochSeconds(Extended_cached)     |   15.000 |
|--------------------------------------------------+----------|
| ZonedExtra::forComponents(Basic_nocache)         |  115.200 |
| ZonedExtra::forComponents(Basic_cached)          |   46.400 |
| ZonedExtra::forComponents(Extended_nocache)      |  164.200 |
| ZonedExtra::forComponents(Extended_cached)       |    2.200 |
|--------------------------------------------------+----------|
| BasicZoneManager::createForZoneName(binary)      |   14.800 |
| BasicZoneManager::createForZoneId(binary)        |    6.400 |
| BasicZoneManager::createForZoneId(linear)        |   42.600 |
+--------------------------------------------------+----------+
Iterations_per_run: 5000

```

## ESP32

* ESP32-01 Dev Board, 240 MHz Tensilica LX6
* Arduino IDE 1.8.19, Arduino CLI 0.27.1
* ESP32 Boards 2.0.5

```
Sizes of Objects:
sizeof(LocalDate): 4
sizeof(LocalTime): 4
sizeof(LocalDateTime): 8
sizeof(TimeOffset): 2
sizeof(OffsetDateTime): 10
sizeof(TimeZone): 12
sizeof(TimeZoneData): 8
sizeof(ZonedDateTime): 24
sizeof(ZonedExtra): 16
sizeof(TimePeriod): 4
sizeof(BasicZoneProcessor): 164
sizeof(ExtendedZoneProcessor): 588
sizeof(BasicZoneProcessorCache<1>): 172
sizeof(ExtendedZoneProcessorCache<1>): 596
sizeof(BasicZoneManager): 12
sizeof(ExtendedZoneManager): 12
sizeof(internal::ZoneContext): 16
sizeof(basic::ZoneEra): 16
sizeof(basic::ZoneInfo): 24
sizeof(basic::ZoneRule): 12
sizeof(basic::ZonePolicy): 12
sizeof(basic::ZoneRegistrar): 8
sizeof(BasicZoneProcessor::Transition): 28
sizeof(ExtendedZoneProcessor::Transition): 52
sizeof(ExtendedZoneProcessor::TransitionStorage): 452
sizeof(ExtendedZoneProcessor::MatchingEra): 28

CPU:
+--------------------------------------------------+----------+
| Method                                           |   micros |
|--------------------------------------------------+----------|
| EmptyLoop                                        |    1.200 |
|--------------------------------------------------+----------|
| LocalDate::forEpochDays()                        |    0.750 |
| LocalDate::toEpochDays()                         |    0.350 |
| LocalDate::dayOfWeek()                           |    0.400 |
|--------------------------------------------------+----------|
| OffsetDateTime::forEpochSeconds()                |    1.650 |
| OffsetDateTime::toEpochSeconds()                 |    1.550 |
|--------------------------------------------------+----------|
| ZonedDateTime::toEpochSeconds()                  |    1.350 |
| ZonedDateTime::toEpochDays()                     |    1.200 |
| ZonedDateTime::forEpochSeconds(UTC)              |    3.150 |
|--------------------------------------------------+----------|
| ZonedDateTime::forEpochSeconds(Basic_nocache)    |   16.600 |
| ZonedDateTime::forEpochSeconds(Basic_cached)     |    3.550 |
| ZonedDateTime::forEpochSeconds(Extended_nocache) |   45.850 |
| ZonedDateTime::forEpochSeconds(Extended_cached)  |    4.850 |
|--------------------------------------------------+----------|
| ZonedDateTime::forComponents(Basic_nocache)      |   24.350 |
| ZonedDateTime::forComponents(Basic_cached)       |   11.300 |
| ZonedDateTime::forComponents(Extended_nocache)   |   34.600 |
| ZonedDateTime::forComponents(Extended_cached)    |    2.100 |
|--------------------------------------------------+----------|
| ZonedExtra::forEpochSeconds(Basic_nocache)       |   15.250 |
| ZonedExtra::forEpochSeconds(Basic_cached)        |    1.250 |
| ZonedExtra::forEpochSeconds(Extended_nocache)    |   43.650 |
| ZonedExtra::forEpochSeconds(Extended_cached)     |    2.650 |
|--------------------------------------------------+----------|
| ZonedExtra::forComponents(Basic_nocache)         |   23.150 |
| ZonedExtra::forComponents(Basic_cached)          |   10.100 |
| ZonedExtra::forComponents(Extended_nocache)      |   33.400 |
| ZonedExtra::forComponents(Extended_cached)       |    0.900 |
|--------------------------------------------------+----------|
| BasicZoneManager::createForZoneName(binary)      |    3.000 |
| BasicZoneManager::createForZoneId(binary)        |    0.600 |
| BasicZoneManager::createForZoneId(linear)        |    2.800 |
+--------------------------------------------------+----------+
Iterations_per_run: 20000

```

Note: Once the benchmark of the function under test becomes smaller than the
duration of an empty loop, the numbers become unreliable.

## Teensy 3.2

* 96 MHz ARM Cortex-M4
* Arduino IDE 1.8.19, Arduino CLI 0.27.1
* Teensyduino 1.57
* Compiler options: "Faster"

```
Sizes of Objects:
sizeof(LocalDate): 4
sizeof(LocalTime): 4
sizeof(LocalDateTime): 8
sizeof(TimeOffset): 2
sizeof(OffsetDateTime): 10
sizeof(TimeZone): 12
sizeof(TimeZoneData): 8
sizeof(ZonedDateTime): 24
sizeof(ZonedExtra): 16
sizeof(TimePeriod): 4
sizeof(BasicZoneProcessor): 164
sizeof(ExtendedZoneProcessor): 588
sizeof(BasicZoneProcessorCache<1>): 172
sizeof(ExtendedZoneProcessorCache<1>): 596
sizeof(BasicZoneManager): 12
sizeof(ExtendedZoneManager): 12
sizeof(internal::ZoneContext): 16
sizeof(basic::ZoneEra): 16
sizeof(basic::ZoneInfo): 24
sizeof(basic::ZoneRule): 12
sizeof(basic::ZonePolicy): 12
sizeof(basic::ZoneRegistrar): 8
sizeof(BasicZoneProcessor::Transition): 28
sizeof(ExtendedZoneProcessor::Transition): 52
sizeof(ExtendedZoneProcessor::TransitionStorage): 452
sizeof(ExtendedZoneProcessor::MatchingEra): 28

CPU:
+--------------------------------------------------+----------+
| Method                                           |   micros |
|--------------------------------------------------+----------|
| EmptyLoop                                        |    0.400 |
|--------------------------------------------------+----------|
| LocalDate::forEpochDays()                        |    2.300 |
| LocalDate::toEpochDays()                         |    0.500 |
| LocalDate::dayOfWeek()                           |    1.000 |
|--------------------------------------------------+----------|
| OffsetDateTime::forEpochSeconds()                |    3.550 |
| OffsetDateTime::toEpochSeconds()                 |    1.150 |
|--------------------------------------------------+----------|
| ZonedDateTime::toEpochSeconds()                  |    1.050 |
| ZonedDateTime::toEpochDays()                     |    0.950 |
| ZonedDateTime::forEpochSeconds(UTC)              |    3.350 |
|--------------------------------------------------+----------|
| ZonedDateTime::forEpochSeconds(Basic_nocache)    |   37.500 |
| ZonedDateTime::forEpochSeconds(Basic_cached)     |    8.650 |
| ZonedDateTime::forEpochSeconds(Extended_nocache) |  109.200 |
| ZonedDateTime::forEpochSeconds(Extended_cached)  |   10.350 |
|--------------------------------------------------+----------|
| ZonedDateTime::forComponents(Basic_nocache)      |   51.400 |
| ZonedDateTime::forComponents(Basic_cached)       |   22.000 |
| ZonedDateTime::forComponents(Extended_nocache)   |   84.200 |
| ZonedDateTime::forComponents(Extended_cached)    |    4.450 |
|--------------------------------------------------+----------|
| ZonedExtra::forEpochSeconds(Basic_nocache)       |   35.050 |
| ZonedExtra::forEpochSeconds(Basic_cached)        |    6.150 |
| ZonedExtra::forEpochSeconds(Extended_nocache)    |  106.900 |
| ZonedExtra::forEpochSeconds(Extended_cached)     |    7.950 |
|--------------------------------------------------+----------|
| ZonedExtra::forComponents(Basic_nocache)         |   51.500 |
| ZonedExtra::forComponents(Basic_cached)          |   22.150 |
| ZonedExtra::forComponents(Extended_nocache)      |   84.300 |
| ZonedExtra::forComponents(Extended_cached)       |    4.600 |
|--------------------------------------------------+----------|
| BasicZoneManager::createForZoneName(binary)      |    6.000 |
| BasicZoneManager::createForZoneId(binary)        |    1.850 |
| BasicZoneManager::createForZoneId(linear)        |   10.200 |
+--------------------------------------------------+----------+
Iterations_per_run: 20000

```

