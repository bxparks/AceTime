# Auto Benchmark

Here are the results from `AutoBenchmark.ino` for various boards.
These results show that integer division and modulus operations are incredibly
slow on 8-bit AVR processors.

**Version**: AceTime v3.0.0

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
4. Switch off the old microcontroller.
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

**v2.2.0**
* Upgrade tool chain
    * Arduino AVR from 1.8.5 to 1.8.6
    * STM32duino from 2.3.0 to 2.4.0
    * ESP8266 from 3.0.2 to 3.1.2 failed, reverted back to 3.0.2
    * ESP32 from 2.0.5 to 2.0.7
* Add support for Seeed XIAO SAMD21
    * Seeeduino 1.8.3
* Upgrade to TZDB 2023b

**v2.2.2**
* Upgrade to TZDB 2023c

**v2.2.3**
* Add support for Adafruit ItsyBitsy M4
    * Using Adafruit SAMD Boards 1.7.11
* Remove Teensy 3.2
    * Nearing end of life. Moved to Tier 2 (should work).
* Upgrade tool chain
    * Seeeduino SAMD Boards 1.8.4
    * STM32duino Boards 2.5.0
    * ESP32 Boards 2.0.9

**v2.3.0**
* Add benchmarks for `CompleteZoneProcessor` and related classes
* Replace labels of `BasicZoneManager::createForXxx()` with
  `BasicZoneRegistrar::findIndexForXxx()`, because those are the methods which
  are actually being tested.

**v2.4.0**
* Support %z format.
* Upgrade to TZDB 2024b.
* Upgrade Arduino CLI to 1.1.1
* Almost no change in execution times.

## Arduino Nano

* 16MHz ATmega328P
* Arduino IDE 1.8.19, Arduino CLI 1.1.1
* Arduino AVR Boards 1.8.6

```
Sizes of Objects:
sizeof(LocalDate): 4
sizeof(LocalTime): 4
sizeof(LocalDateTime): 8
sizeof(TimeOffset): 4
sizeof(OffsetDateTime): 12
sizeof(TimeZone): 5
sizeof(TimeZoneData): 5
sizeof(ZonedDateTime): 17
sizeof(ZonedExtra): 25
sizeof(TimePeriod): 4
Basic:
  sizeof(basic::Info::ZoneContext): 20
  sizeof(basic::Info::ZoneEra): 11
  sizeof(basic::Info::ZoneInfo): 13
  sizeof(basic::Info::ZoneRule): 9
  sizeof(basic::Info::ZonePolicy): 3
  sizeof(basic::ZoneRegistrar): 5
  sizeof(BasicZoneProcessor): 148
  sizeof(BasicZoneProcessorCache<1>): 152
  sizeof(BasicZoneManager): 7
  sizeof(BasicZoneProcessor::Transition): 27
Extended:
  sizeof(extended::Info::ZoneContext): 20
  sizeof(extended::Info::ZoneEra): 11
  sizeof(extended::Info::ZoneInfo): 13
  sizeof(extended::Info::ZoneRule): 9
  sizeof(extended::Info::ZonePolicy): 3
  sizeof(extended::ZoneRegistrar): 5
  sizeof(ExtendedZoneProcessor): 561
  sizeof(ExtendedZoneProcessorCache<1>): 565
  sizeof(ExtendedZoneManager): 7
  sizeof(ExtendedZoneProcessor::Transition): 50
  sizeof(ExtendedZoneProcessor::TransitionStorage): 420
  sizeof(ExtendedZoneProcessor::MatchingEra): 32
Complete:
  sizeof(complete::Info::ZoneContext): 20
  sizeof(complete::Info::ZoneEra): 15
  sizeof(complete::Info::ZoneInfo): 13
  sizeof(complete::Info::ZoneRule): 12
  sizeof(complete::Info::ZonePolicy): 3
  sizeof(complete::ZoneRegistrar): 5
  sizeof(CompleteZoneProcessor): 561
  sizeof(CompleteZoneProcessorCache<1>): 565
  sizeof(CompleteZoneManager): 7
  sizeof(CompleteZoneProcessor::Transition): 50
  sizeof(CompleteZoneProcessor::TransitionStorage): 420
  sizeof(CompleteZoneProcessor::MatchingEra): 32

CPU:
+--------------------------------------------------+----------+
| Method                                           |   micros |
|--------------------------------------------------+----------|
| EmptyLoop                                        |    3.000 |
|--------------------------------------------------+----------|
| LocalDate::forEpochDays()                        |  243.000 |
| LocalDate::toEpochDays()                         |   51.000 |
| LocalDate::dayOfWeek()                           |   50.000 |
|--------------------------------------------------+----------|
| OffsetDateTime::forEpochSeconds()                |  363.000 |
| OffsetDateTime::toEpochSeconds()                 |   77.000 |
|--------------------------------------------------+----------|
| ZonedDateTime::toEpochSeconds()                  |   75.000 |
| ZonedDateTime::toEpochDays()                     |   62.000 |
| ZonedDateTime::forEpochSeconds(UTC)              |  393.000 |
|--------------------------------------------------+----------|
| ZonedDateTime::forEpochSeconds(Basic_nocache)    | 1728.000 |
| ZonedDateTime::forEpochSeconds(Basic_cached)     |  708.000 |
| ZonedDateTime::forEpochSeconds(Extended_nocache) |   -1.000 |
| ZonedDateTime::forEpochSeconds(Extended_cached)  |   -1.000 |
| ZonedDateTime::forEpochSeconds(Complete_nocache) |   -1.000 |
| ZonedDateTime::forEpochSeconds(Complete_cached)  |   -1.000 |
|--------------------------------------------------+----------|
| ZonedDateTime::forComponents(Basic_nocache)      | 2270.000 |
| ZonedDateTime::forComponents(Basic_cached)       | 1254.000 |
| ZonedDateTime::forComponents(Extended_nocache)   |   -1.000 |
| ZonedDateTime::forComponents(Extended_cached)    |   -1.000 |
| ZonedDateTime::forComponents(Complete_nocache)   |   -1.000 |
| ZonedDateTime::forComponents(Complete_cached)    |   -1.000 |
|--------------------------------------------------+----------|
| ZonedExtra::forEpochSeconds(Basic_nocache)       | 1405.000 |
| ZonedExtra::forEpochSeconds(Basic_cached)        |  380.000 |
| ZonedExtra::forEpochSeconds(Extended_nocache)    |   -1.000 |
| ZonedExtra::forEpochSeconds(Extended_cached)     |   -1.000 |
| ZonedExtra::forEpochSeconds(Complete_nocache)    |   -1.000 |
| ZonedExtra::forEpochSeconds(Complete_cached)     |   -1.000 |
|--------------------------------------------------+----------|
| ZonedExtra::forComponents(Basic_nocache)         | 2295.000 |
| ZonedExtra::forComponents(Basic_cached)          | 1279.000 |
| ZonedExtra::forComponents(Extended_nocache)      |   -1.000 |
| ZonedExtra::forComponents(Extended_cached)       |   -1.000 |
| ZonedExtra::forComponents(Complete_nocache)      |   -1.000 |
| ZonedExtra::forComponents(Complete_cached)       |   -1.000 |
|--------------------------------------------------+----------|
| BasicZoneRegistrar::findIndexForName(binary)     |  121.000 |
| BasicZoneRegistrar::findIndexForIdBinary()       |   48.000 |
| BasicZoneRegistrar::findIndexForIdLinear()       |  295.000 |
|--------------------------------------------------+----------|
| ExtendedZoneRegistrar::findIndexForName(binary)  |   -1.000 |
| ExtendedZoneRegistrar::findIndexForIdBinary()    |   -1.000 |
| ExtendedZoneRegistrar::findIndexForIdLinear()    |   -1.000 |
|--------------------------------------------------+----------|
| CompleteZoneRegistrar::findIndexForName(binary)  |   -1.000 |
| CompleteZoneRegistrar::findIndexForIdBinary()    |   -1.000 |
| CompleteZoneRegistrar::findIndexForIdLinear()    |   -1.000 |
+--------------------------------------------------+----------+
Iterations_per_run: 1000

```

## Sparkfun Pro Micro

* 16 MHz ATmega32U4
* Arduino IDE 1.8.19, Arduino CLI 1.1.1
* SparkFun AVR Boards 1.1.13

```
Sizes of Objects:
sizeof(LocalDate): 4
sizeof(LocalTime): 4
sizeof(LocalDateTime): 8
sizeof(TimeOffset): 4
sizeof(OffsetDateTime): 12
sizeof(TimeZone): 5
sizeof(TimeZoneData): 5
sizeof(ZonedDateTime): 17
sizeof(ZonedExtra): 25
sizeof(TimePeriod): 4
Basic:
  sizeof(basic::Info::ZoneContext): 20
  sizeof(basic::Info::ZoneEra): 11
  sizeof(basic::Info::ZoneInfo): 13
  sizeof(basic::Info::ZoneRule): 9
  sizeof(basic::Info::ZonePolicy): 3
  sizeof(basic::ZoneRegistrar): 5
  sizeof(BasicZoneProcessor): 148
  sizeof(BasicZoneProcessorCache<1>): 152
  sizeof(BasicZoneManager): 7
  sizeof(BasicZoneProcessor::Transition): 27
Extended:
  sizeof(extended::Info::ZoneContext): 20
  sizeof(extended::Info::ZoneEra): 11
  sizeof(extended::Info::ZoneInfo): 13
  sizeof(extended::Info::ZoneRule): 9
  sizeof(extended::Info::ZonePolicy): 3
  sizeof(extended::ZoneRegistrar): 5
  sizeof(ExtendedZoneProcessor): 561
  sizeof(ExtendedZoneProcessorCache<1>): 565
  sizeof(ExtendedZoneManager): 7
  sizeof(ExtendedZoneProcessor::Transition): 50
  sizeof(ExtendedZoneProcessor::TransitionStorage): 420
  sizeof(ExtendedZoneProcessor::MatchingEra): 32
Complete:
  sizeof(complete::Info::ZoneContext): 20
  sizeof(complete::Info::ZoneEra): 15
  sizeof(complete::Info::ZoneInfo): 13
  sizeof(complete::Info::ZoneRule): 12
  sizeof(complete::Info::ZonePolicy): 3
  sizeof(complete::ZoneRegistrar): 5
  sizeof(CompleteZoneProcessor): 561
  sizeof(CompleteZoneProcessorCache<1>): 565
  sizeof(CompleteZoneManager): 7
  sizeof(CompleteZoneProcessor::Transition): 50
  sizeof(CompleteZoneProcessor::TransitionStorage): 420
  sizeof(CompleteZoneProcessor::MatchingEra): 32

CPU:
+--------------------------------------------------+----------+
| Method                                           |   micros |
|--------------------------------------------------+----------|
| EmptyLoop                                        |    4.000 |
|--------------------------------------------------+----------|
| LocalDate::forEpochDays()                        |  244.000 |
| LocalDate::toEpochDays()                         |   52.000 |
| LocalDate::dayOfWeek()                           |   49.000 |
|--------------------------------------------------+----------|
| OffsetDateTime::forEpochSeconds()                |  364.000 |
| OffsetDateTime::toEpochSeconds()                 |   77.000 |
|--------------------------------------------------+----------|
| ZonedDateTime::toEpochSeconds()                  |   74.000 |
| ZonedDateTime::toEpochDays()                     |   64.000 |
| ZonedDateTime::forEpochSeconds(UTC)              |  394.000 |
|--------------------------------------------------+----------|
| ZonedDateTime::forEpochSeconds(Basic_nocache)    | 1736.000 |
| ZonedDateTime::forEpochSeconds(Basic_cached)     |  711.000 |
| ZonedDateTime::forEpochSeconds(Extended_nocache) |   -1.000 |
| ZonedDateTime::forEpochSeconds(Extended_cached)  |   -1.000 |
| ZonedDateTime::forEpochSeconds(Complete_nocache) |   -1.000 |
| ZonedDateTime::forEpochSeconds(Complete_cached)  |   -1.000 |
|--------------------------------------------------+----------|
| ZonedDateTime::forComponents(Basic_nocache)      | 2281.000 |
| ZonedDateTime::forComponents(Basic_cached)       | 1259.000 |
| ZonedDateTime::forComponents(Extended_nocache)   |   -1.000 |
| ZonedDateTime::forComponents(Extended_cached)    |   -1.000 |
| ZonedDateTime::forComponents(Complete_nocache)   |   -1.000 |
| ZonedDateTime::forComponents(Complete_cached)    |   -1.000 |
|--------------------------------------------------+----------|
| ZonedExtra::forEpochSeconds(Basic_nocache)       | 1411.000 |
| ZonedExtra::forEpochSeconds(Basic_cached)        |  381.000 |
| ZonedExtra::forEpochSeconds(Extended_nocache)    |   -1.000 |
| ZonedExtra::forEpochSeconds(Extended_cached)     |   -1.000 |
| ZonedExtra::forEpochSeconds(Complete_nocache)    |   -1.000 |
| ZonedExtra::forEpochSeconds(Complete_cached)     |   -1.000 |
|--------------------------------------------------+----------|
| ZonedExtra::forComponents(Basic_nocache)         | 2305.000 |
| ZonedExtra::forComponents(Basic_cached)          | 1284.000 |
| ZonedExtra::forComponents(Extended_nocache)      |   -1.000 |
| ZonedExtra::forComponents(Extended_cached)       |   -1.000 |
| ZonedExtra::forComponents(Complete_nocache)      |   -1.000 |
| ZonedExtra::forComponents(Complete_cached)       |   -1.000 |
|--------------------------------------------------+----------|
| BasicZoneRegistrar::findIndexForName(binary)     |  122.000 |
| BasicZoneRegistrar::findIndexForIdBinary()       |   49.000 |
| BasicZoneRegistrar::findIndexForIdLinear()       |  295.000 |
|--------------------------------------------------+----------|
| ExtendedZoneRegistrar::findIndexForName(binary)  |   -1.000 |
| ExtendedZoneRegistrar::findIndexForIdBinary()    |   -1.000 |
| ExtendedZoneRegistrar::findIndexForIdLinear()    |   -1.000 |
|--------------------------------------------------+----------|
| CompleteZoneRegistrar::findIndexForName(binary)  |   -1.000 |
| CompleteZoneRegistrar::findIndexForIdBinary()    |   -1.000 |
| CompleteZoneRegistrar::findIndexForIdLinear()    |   -1.000 |
+--------------------------------------------------+----------+
Iterations_per_run: 1000

```

## Seeed Studio XIAO SAMD21

* SAMD21, 48 MHz ARM Cortex-M0+
* Arduino IDE 1.8.19, Arduino CLI 1.1.1
* Seeeduino 1.8.4

```
Sizes of Objects:
sizeof(LocalDate): 4
sizeof(LocalTime): 4
sizeof(LocalDateTime): 8
sizeof(TimeOffset): 4
sizeof(OffsetDateTime): 12
sizeof(TimeZone): 12
sizeof(TimeZoneData): 8
sizeof(ZonedDateTime): 24
sizeof(ZonedExtra): 28
sizeof(TimePeriod): 4
Basic:
  sizeof(basic::Info::ZoneContext): 28
  sizeof(basic::Info::ZoneEra): 16
  sizeof(basic::Info::ZoneInfo): 24
  sizeof(basic::Info::ZoneRule): 9
  sizeof(basic::Info::ZonePolicy): 8
  sizeof(basic::ZoneRegistrar): 8
  sizeof(BasicZoneProcessor): 208
  sizeof(BasicZoneProcessorCache<1>): 216
  sizeof(BasicZoneManager): 12
  sizeof(BasicZoneProcessor::Transition): 36
Extended:
  sizeof(extended::Info::ZoneContext): 28
  sizeof(extended::Info::ZoneEra): 16
  sizeof(extended::Info::ZoneInfo): 24
  sizeof(extended::Info::ZoneRule): 9
  sizeof(extended::Info::ZonePolicy): 8
  sizeof(extended::ZoneRegistrar): 8
  sizeof(ExtendedZoneProcessor): 752
  sizeof(ExtendedZoneProcessorCache<1>): 760
  sizeof(ExtendedZoneManager): 12
  sizeof(ExtendedZoneProcessor::Transition): 64
  sizeof(ExtendedZoneProcessor::TransitionStorage): 548
  sizeof(ExtendedZoneProcessor::MatchingEra): 44
Complete:
  sizeof(complete::Info::ZoneContext): 28
  sizeof(complete::Info::ZoneEra): 20
  sizeof(complete::Info::ZoneInfo): 24
  sizeof(complete::Info::ZoneRule): 12
  sizeof(complete::Info::ZonePolicy): 8
  sizeof(complete::ZoneRegistrar): 8
  sizeof(CompleteZoneProcessor): 752
  sizeof(CompleteZoneProcessorCache<1>): 760
  sizeof(CompleteZoneManager): 12
  sizeof(CompleteZoneProcessor::Transition): 64
  sizeof(CompleteZoneProcessor::TransitionStorage): 548
  sizeof(CompleteZoneProcessor::MatchingEra): 44

CPU:
+--------------------------------------------------+----------+
| Method                                           |   micros |
|--------------------------------------------------+----------|
| EmptyLoop                                        |    1.400 |
|--------------------------------------------------+----------|
| LocalDate::forEpochDays()                        |   13.200 |
| LocalDate::toEpochDays()                         |    3.800 |
| LocalDate::dayOfWeek()                           |    6.600 |
|--------------------------------------------------+----------|
| OffsetDateTime::forEpochSeconds()                |   20.800 |
| OffsetDateTime::toEpochSeconds()                 |   11.800 |
|--------------------------------------------------+----------|
| ZonedDateTime::toEpochSeconds()                  |   11.800 |
| ZonedDateTime::toEpochDays()                     |    9.400 |
| ZonedDateTime::forEpochSeconds(UTC)              |   24.400 |
|--------------------------------------------------+----------|
| ZonedDateTime::forEpochSeconds(Basic_nocache)    |  219.000 |
| ZonedDateTime::forEpochSeconds(Basic_cached)     |   46.000 |
| ZonedDateTime::forEpochSeconds(Extended_nocache) |  548.600 |
| ZonedDateTime::forEpochSeconds(Extended_cached)  |   56.800 |
| ZonedDateTime::forEpochSeconds(Complete_nocache) |  668.800 |
| ZonedDateTime::forEpochSeconds(Complete_cached)  |   56.800 |
|--------------------------------------------------+----------|
| ZonedDateTime::forComponents(Basic_nocache)      |  277.600 |
| ZonedDateTime::forComponents(Basic_cached)       |   94.800 |
| ZonedDateTime::forComponents(Extended_nocache)   |  417.600 |
| ZonedDateTime::forComponents(Extended_cached)    |   17.000 |
| ZonedDateTime::forComponents(Complete_nocache)   |  538.000 |
| ZonedDateTime::forComponents(Complete_cached)    |   16.800 |
|--------------------------------------------------+----------|
| ZonedExtra::forEpochSeconds(Basic_nocache)       |  200.800 |
| ZonedExtra::forEpochSeconds(Basic_cached)        |   28.000 |
| ZonedExtra::forEpochSeconds(Extended_nocache)    |  530.000 |
| ZonedExtra::forEpochSeconds(Extended_cached)     |   38.800 |
| ZonedExtra::forEpochSeconds(Complete_nocache)    |  650.000 |
| ZonedExtra::forEpochSeconds(Complete_cached)     |   38.600 |
|--------------------------------------------------+----------|
| ZonedExtra::forComponents(Basic_nocache)         |  281.000 |
| ZonedExtra::forComponents(Basic_cached)          |   98.400 |
| ZonedExtra::forComponents(Extended_nocache)      |  421.000 |
| ZonedExtra::forComponents(Extended_cached)       |   20.600 |
| ZonedExtra::forComponents(Complete_nocache)      |  541.600 |
| ZonedExtra::forComponents(Complete_cached)       |   20.400 |
|--------------------------------------------------+----------|
| BasicZoneRegistrar::findIndexForName(binary)     |   16.000 |
| BasicZoneRegistrar::findIndexForIdBinary()       |    4.200 |
| BasicZoneRegistrar::findIndexForIdLinear()       |   13.600 |
|--------------------------------------------------+----------|
| ExtendedZoneRegistrar::findIndexForName(binary)  |   16.600 |
| ExtendedZoneRegistrar::findIndexForIdBinary()    |    4.400 |
| ExtendedZoneRegistrar::findIndexForIdLinear()    |   13.800 |
|--------------------------------------------------+----------|
| CompleteZoneRegistrar::findIndexForName(binary)  |   16.400 |
| CompleteZoneRegistrar::findIndexForIdBinary()    |    4.200 |
| CompleteZoneRegistrar::findIndexForIdLinear()    |   13.600 |
+--------------------------------------------------+----------+
Iterations_per_run: 5000

```

## STM32 Blue Pill

* STM32F103C8, 72 MHz ARM Cortex-M3
* Arduino IDE 1.8.19, Arduino CLI 1.1.1
* STM32duino 2.5.0

```
Sizes of Objects:
sizeof(LocalDate): 4
sizeof(LocalTime): 4
sizeof(LocalDateTime): 8
sizeof(TimeOffset): 4
sizeof(OffsetDateTime): 12
sizeof(TimeZone): 12
sizeof(TimeZoneData): 8
sizeof(ZonedDateTime): 24
sizeof(ZonedExtra): 28
sizeof(TimePeriod): 4
Basic:
  sizeof(basic::Info::ZoneContext): 28
  sizeof(basic::Info::ZoneEra): 16
  sizeof(basic::Info::ZoneInfo): 24
  sizeof(basic::Info::ZoneRule): 9
  sizeof(basic::Info::ZonePolicy): 8
  sizeof(basic::ZoneRegistrar): 8
  sizeof(BasicZoneProcessor): 208
  sizeof(BasicZoneProcessorCache<1>): 216
  sizeof(BasicZoneManager): 12
  sizeof(BasicZoneProcessor::Transition): 36
Extended:
  sizeof(extended::Info::ZoneContext): 28
  sizeof(extended::Info::ZoneEra): 16
  sizeof(extended::Info::ZoneInfo): 24
  sizeof(extended::Info::ZoneRule): 9
  sizeof(extended::Info::ZonePolicy): 8
  sizeof(extended::ZoneRegistrar): 8
  sizeof(ExtendedZoneProcessor): 752
  sizeof(ExtendedZoneProcessorCache<1>): 760
  sizeof(ExtendedZoneManager): 12
  sizeof(ExtendedZoneProcessor::Transition): 64
  sizeof(ExtendedZoneProcessor::TransitionStorage): 548
  sizeof(ExtendedZoneProcessor::MatchingEra): 44
Complete:
  sizeof(complete::Info::ZoneContext): 28
  sizeof(complete::Info::ZoneEra): 20
  sizeof(complete::Info::ZoneInfo): 24
  sizeof(complete::Info::ZoneRule): 12
  sizeof(complete::Info::ZonePolicy): 8
  sizeof(complete::ZoneRegistrar): 8
  sizeof(CompleteZoneProcessor): 752
  sizeof(CompleteZoneProcessorCache<1>): 760
  sizeof(CompleteZoneManager): 12
  sizeof(CompleteZoneProcessor::Transition): 64
  sizeof(CompleteZoneProcessor::TransitionStorage): 548
  sizeof(CompleteZoneProcessor::MatchingEra): 44

CPU:
+--------------------------------------------------+----------+
| Method                                           |   micros |
|--------------------------------------------------+----------|
| EmptyLoop                                        |    1.200 |
|--------------------------------------------------+----------|
| LocalDate::forEpochDays()                        |    2.800 |
| LocalDate::toEpochDays()                         |    1.200 |
| LocalDate::dayOfWeek()                           |    1.200 |
|--------------------------------------------------+----------|
| OffsetDateTime::forEpochSeconds()                |    4.200 |
| OffsetDateTime::toEpochSeconds()                 |    4.800 |
|--------------------------------------------------+----------|
| ZonedDateTime::toEpochSeconds()                  |    5.200 |
| ZonedDateTime::toEpochDays()                     |    3.600 |
| ZonedDateTime::forEpochSeconds(UTC)              |    6.200 |
|--------------------------------------------------+----------|
| ZonedDateTime::forEpochSeconds(Basic_nocache)    |   95.200 |
| ZonedDateTime::forEpochSeconds(Basic_cached)     |   12.600 |
| ZonedDateTime::forEpochSeconds(Extended_nocache) |  248.800 |
| ZonedDateTime::forEpochSeconds(Extended_cached)  |   17.000 |
| ZonedDateTime::forEpochSeconds(Complete_nocache) |  306.200 |
| ZonedDateTime::forEpochSeconds(Complete_cached)  |   17.000 |
|--------------------------------------------------+----------|
| ZonedDateTime::forComponents(Basic_nocache)      |  118.200 |
| ZonedDateTime::forComponents(Basic_cached)       |   37.200 |
| ZonedDateTime::forComponents(Extended_nocache)   |  198.000 |
| ZonedDateTime::forComponents(Extended_cached)    |    7.600 |
| ZonedDateTime::forComponents(Complete_nocache)   |  256.800 |
| ZonedDateTime::forComponents(Complete_cached)    |    7.600 |
|--------------------------------------------------+----------|
| ZonedExtra::forEpochSeconds(Basic_nocache)       |   92.600 |
| ZonedExtra::forEpochSeconds(Basic_cached)        |    9.200 |
| ZonedExtra::forEpochSeconds(Extended_nocache)    |  245.800 |
| ZonedExtra::forEpochSeconds(Extended_cached)     |   13.800 |
| ZonedExtra::forEpochSeconds(Complete_nocache)    |  303.000 |
| ZonedExtra::forEpochSeconds(Complete_cached)     |   13.800 |
|--------------------------------------------------+----------|
| ZonedExtra::forComponents(Basic_nocache)         |  119.000 |
| ZonedExtra::forComponents(Basic_cached)          |   38.000 |
| ZonedExtra::forComponents(Extended_nocache)      |  198.800 |
| ZonedExtra::forComponents(Extended_cached)       |    8.600 |
| ZonedExtra::forComponents(Complete_nocache)      |  257.800 |
| ZonedExtra::forComponents(Complete_cached)       |    8.400 |
|--------------------------------------------------+----------|
| BasicZoneRegistrar::findIndexForName(binary)     |   12.400 |
| BasicZoneRegistrar::findIndexForIdBinary()       |    2.400 |
| BasicZoneRegistrar::findIndexForIdLinear()       |   15.200 |
|--------------------------------------------------+----------|
| ExtendedZoneRegistrar::findIndexForName(binary)  |   12.800 |
| ExtendedZoneRegistrar::findIndexForIdBinary()    |    2.600 |
| ExtendedZoneRegistrar::findIndexForIdLinear()    |   15.600 |
|--------------------------------------------------+----------|
| CompleteZoneRegistrar::findIndexForName(binary)  |   13.000 |
| CompleteZoneRegistrar::findIndexForIdBinary()    |    2.600 |
| CompleteZoneRegistrar::findIndexForIdLinear()    |   15.400 |
+--------------------------------------------------+----------+
Iterations_per_run: 5000

```

## Adafruit ItsyBitsy M4 SAMD51

* SAMD51, 120 MHz ARM Cortex-M4
* Arduino IDE 1.8.19, Arduino CLI 1.1.1
* Adafruit SAMD 1.7.11

```
Sizes of Objects:
sizeof(LocalDate): 4
sizeof(LocalTime): 4
sizeof(LocalDateTime): 8
sizeof(TimeOffset): 4
sizeof(OffsetDateTime): 12
sizeof(TimeZone): 12
sizeof(TimeZoneData): 8
sizeof(ZonedDateTime): 24
sizeof(ZonedExtra): 28
sizeof(TimePeriod): 4
Basic:
  sizeof(basic::Info::ZoneContext): 28
  sizeof(basic::Info::ZoneEra): 16
  sizeof(basic::Info::ZoneInfo): 24
  sizeof(basic::Info::ZoneRule): 9
  sizeof(basic::Info::ZonePolicy): 8
  sizeof(basic::ZoneRegistrar): 8
  sizeof(BasicZoneProcessor): 208
  sizeof(BasicZoneProcessorCache<1>): 216
  sizeof(BasicZoneManager): 12
  sizeof(BasicZoneProcessor::Transition): 36
Extended:
  sizeof(extended::Info::ZoneContext): 28
  sizeof(extended::Info::ZoneEra): 16
  sizeof(extended::Info::ZoneInfo): 24
  sizeof(extended::Info::ZoneRule): 9
  sizeof(extended::Info::ZonePolicy): 8
  sizeof(extended::ZoneRegistrar): 8
  sizeof(ExtendedZoneProcessor): 752
  sizeof(ExtendedZoneProcessorCache<1>): 760
  sizeof(ExtendedZoneManager): 12
  sizeof(ExtendedZoneProcessor::Transition): 64
  sizeof(ExtendedZoneProcessor::TransitionStorage): 548
  sizeof(ExtendedZoneProcessor::MatchingEra): 44
Complete:
  sizeof(complete::Info::ZoneContext): 28
  sizeof(complete::Info::ZoneEra): 20
  sizeof(complete::Info::ZoneInfo): 24
  sizeof(complete::Info::ZoneRule): 12
  sizeof(complete::Info::ZonePolicy): 8
  sizeof(complete::ZoneRegistrar): 8
  sizeof(CompleteZoneProcessor): 752
  sizeof(CompleteZoneProcessorCache<1>): 760
  sizeof(CompleteZoneManager): 12
  sizeof(CompleteZoneProcessor::Transition): 64
  sizeof(CompleteZoneProcessor::TransitionStorage): 548
  sizeof(CompleteZoneProcessor::MatchingEra): 44

CPU:
+--------------------------------------------------+----------+
| Method                                           |   micros |
|--------------------------------------------------+----------|
| EmptyLoop                                        |    0.400 |
|--------------------------------------------------+----------|
| LocalDate::forEpochDays()                        |    1.400 |
| LocalDate::toEpochDays()                         |    0.600 |
| LocalDate::dayOfWeek()                           |    0.800 |
|--------------------------------------------------+----------|
| OffsetDateTime::forEpochSeconds()                |    2.000 |
| OffsetDateTime::toEpochSeconds()                 |    2.200 |
|--------------------------------------------------+----------|
| ZonedDateTime::toEpochSeconds()                  |    2.400 |
| ZonedDateTime::toEpochDays()                     |    1.800 |
| ZonedDateTime::forEpochSeconds(UTC)              |    3.000 |
|--------------------------------------------------+----------|
| ZonedDateTime::forEpochSeconds(Basic_nocache)    |   42.800 |
| ZonedDateTime::forEpochSeconds(Basic_cached)     |    6.200 |
| ZonedDateTime::forEpochSeconds(Extended_nocache) |  110.600 |
| ZonedDateTime::forEpochSeconds(Extended_cached)  |    8.000 |
| ZonedDateTime::forEpochSeconds(Complete_nocache) |  135.400 |
| ZonedDateTime::forEpochSeconds(Complete_cached)  |    8.000 |
|--------------------------------------------------+----------|
| ZonedDateTime::forComponents(Basic_nocache)      |   54.600 |
| ZonedDateTime::forComponents(Basic_cached)       |   17.800 |
| ZonedDateTime::forComponents(Extended_nocache)   |   88.800 |
| ZonedDateTime::forComponents(Extended_cached)    |    3.800 |
| ZonedDateTime::forComponents(Complete_nocache)   |  112.400 |
| ZonedDateTime::forComponents(Complete_cached)    |    3.600 |
|--------------------------------------------------+----------|
| ZonedExtra::forEpochSeconds(Basic_nocache)       |   40.200 |
| ZonedExtra::forEpochSeconds(Basic_cached)        |    4.400 |
| ZonedExtra::forEpochSeconds(Extended_nocache)    |  109.000 |
| ZonedExtra::forEpochSeconds(Extended_cached)     |    6.400 |
| ZonedExtra::forEpochSeconds(Complete_nocache)    |  132.000 |
| ZonedExtra::forEpochSeconds(Complete_cached)     |    6.400 |
|--------------------------------------------------+----------|
| ZonedExtra::forComponents(Basic_nocache)         |   54.400 |
| ZonedExtra::forComponents(Basic_cached)          |   18.600 |
| ZonedExtra::forComponents(Extended_nocache)      |   88.400 |
| ZonedExtra::forComponents(Extended_cached)       |    3.800 |
| ZonedExtra::forComponents(Complete_nocache)      |  112.000 |
| ZonedExtra::forComponents(Complete_cached)       |    3.800 |
|--------------------------------------------------+----------|
| BasicZoneRegistrar::findIndexForName(binary)     |    4.800 |
| BasicZoneRegistrar::findIndexForIdBinary()       |    1.400 |
| BasicZoneRegistrar::findIndexForIdLinear()       |    4.000 |
|--------------------------------------------------+----------|
| ExtendedZoneRegistrar::findIndexForName(binary)  |    5.000 |
| ExtendedZoneRegistrar::findIndexForIdBinary()    |    1.200 |
| ExtendedZoneRegistrar::findIndexForIdLinear()    |    4.000 |
|--------------------------------------------------+----------|
| CompleteZoneRegistrar::findIndexForName(binary)  |    5.000 |
| CompleteZoneRegistrar::findIndexForIdBinary()    |    1.200 |
| CompleteZoneRegistrar::findIndexForIdLinear()    |    3.800 |
+--------------------------------------------------+----------+
Iterations_per_run: 5000

```

## ESP8266

* NodeMCU 1.0 clone, 80MHz ESP8266
* Arduino IDE 1.8.19, Arduino CLI 1.1.1
* ESP8266 Boards 3.0.2

```
Sizes of Objects:
sizeof(LocalDate): 4
sizeof(LocalTime): 4
sizeof(LocalDateTime): 8
sizeof(TimeOffset): 4
sizeof(OffsetDateTime): 12
sizeof(TimeZone): 12
sizeof(TimeZoneData): 8
sizeof(ZonedDateTime): 24
sizeof(ZonedExtra): 28
sizeof(TimePeriod): 4
Basic:
  sizeof(basic::Info::ZoneContext): 28
  sizeof(basic::Info::ZoneEra): 16
  sizeof(basic::Info::ZoneInfo): 24
  sizeof(basic::Info::ZoneRule): 9
  sizeof(basic::Info::ZonePolicy): 8
  sizeof(basic::ZoneRegistrar): 8
  sizeof(BasicZoneProcessor): 208
  sizeof(BasicZoneProcessorCache<1>): 216
  sizeof(BasicZoneManager): 12
  sizeof(BasicZoneProcessor::Transition): 36
Extended:
  sizeof(extended::Info::ZoneContext): 28
  sizeof(extended::Info::ZoneEra): 16
  sizeof(extended::Info::ZoneInfo): 24
  sizeof(extended::Info::ZoneRule): 9
  sizeof(extended::Info::ZonePolicy): 8
  sizeof(extended::ZoneRegistrar): 8
  sizeof(ExtendedZoneProcessor): 752
  sizeof(ExtendedZoneProcessorCache<1>): 760
  sizeof(ExtendedZoneManager): 12
  sizeof(ExtendedZoneProcessor::Transition): 64
  sizeof(ExtendedZoneProcessor::TransitionStorage): 548
  sizeof(ExtendedZoneProcessor::MatchingEra): 44
Complete:
  sizeof(complete::Info::ZoneContext): 28
  sizeof(complete::Info::ZoneEra): 20
  sizeof(complete::Info::ZoneInfo): 24
  sizeof(complete::Info::ZoneRule): 12
  sizeof(complete::Info::ZonePolicy): 8
  sizeof(complete::ZoneRegistrar): 8
  sizeof(CompleteZoneProcessor): 752
  sizeof(CompleteZoneProcessorCache<1>): 760
  sizeof(CompleteZoneManager): 12
  sizeof(CompleteZoneProcessor::Transition): 64
  sizeof(CompleteZoneProcessor::TransitionStorage): 548
  sizeof(CompleteZoneProcessor::MatchingEra): 44

CPU:
+--------------------------------------------------+----------+
| Method                                           |   micros |
|--------------------------------------------------+----------|
| EmptyLoop                                        |    5.000 |
|--------------------------------------------------+----------|
| LocalDate::forEpochDays()                        |    7.000 |
| LocalDate::toEpochDays()                         |    3.500 |
| LocalDate::dayOfWeek()                           |    3.500 |
|--------------------------------------------------+----------|
| OffsetDateTime::forEpochSeconds()                |   12.500 |
| OffsetDateTime::toEpochSeconds()                 |    7.000 |
|--------------------------------------------------+----------|
| ZonedDateTime::toEpochSeconds()                  |    6.500 |
| ZonedDateTime::toEpochDays()                     |    5.000 |
| ZonedDateTime::forEpochSeconds(UTC)              |   14.000 |
|--------------------------------------------------+----------|
| ZonedDateTime::forEpochSeconds(Basic_nocache)    |  146.000 |
| ZonedDateTime::forEpochSeconds(Basic_cached)     |   21.500 |
| ZonedDateTime::forEpochSeconds(Extended_nocache) |  364.000 |
| ZonedDateTime::forEpochSeconds(Extended_cached)  |   28.000 |
| ZonedDateTime::forEpochSeconds(Complete_nocache) |  447.000 |
| ZonedDateTime::forEpochSeconds(Complete_cached)  |   28.000 |
|--------------------------------------------------+----------|
| ZonedDateTime::forComponents(Basic_nocache)      |  163.500 |
| ZonedDateTime::forComponents(Basic_cached)       |   45.500 |
| ZonedDateTime::forComponents(Extended_nocache)   |  273.000 |
| ZonedDateTime::forComponents(Extended_cached)    |    2.500 |
| ZonedDateTime::forComponents(Complete_nocache)   |  417.000 |
| ZonedDateTime::forComponents(Complete_cached)    |   49.500 |
|--------------------------------------------------+----------|
| ZonedExtra::forEpochSeconds(Basic_nocache)       |  139.500 |
| ZonedExtra::forEpochSeconds(Basic_cached)        |   11.000 |
| ZonedExtra::forEpochSeconds(Extended_nocache)    |  329.500 |
| ZonedExtra::forEpochSeconds(Extended_cached)     |   17.500 |
| ZonedExtra::forEpochSeconds(Complete_nocache)    |  436.500 |
| ZonedExtra::forEpochSeconds(Complete_cached)     |   17.500 |
|--------------------------------------------------+----------|
| ZonedExtra::forComponents(Basic_nocache)         |  166.500 |
| ZonedExtra::forComponents(Basic_cached)          |   48.000 |
| ZonedExtra::forComponents(Extended_nocache)      |  252.000 |
| ZonedExtra::forComponents(Extended_cached)       |    5.000 |
| ZonedExtra::forComponents(Complete_nocache)      |  348.500 |
| ZonedExtra::forComponents(Complete_cached)       |    5.000 |
|--------------------------------------------------+----------|
| BasicZoneRegistrar::findIndexForName(binary)     |   17.500 |
| BasicZoneRegistrar::findIndexForIdBinary()       |    7.000 |
| BasicZoneRegistrar::findIndexForIdLinear()       |   42.500 |
|--------------------------------------------------+----------|
| ExtendedZoneRegistrar::findIndexForName(binary)  |   18.500 |
| ExtendedZoneRegistrar::findIndexForIdBinary()    |    6.500 |
| ExtendedZoneRegistrar::findIndexForIdLinear()    |   42.500 |
|--------------------------------------------------+----------|
| CompleteZoneRegistrar::findIndexForName(binary)  |   19.000 |
| CompleteZoneRegistrar::findIndexForIdBinary()    |    7.000 |
| CompleteZoneRegistrar::findIndexForIdLinear()    |   42.500 |
+--------------------------------------------------+----------+
Iterations_per_run: 2000

```

## ESP32

* ESP32-01 Dev Board, 240 MHz Tensilica LX6
* Arduino IDE 1.8.19, Arduino CLI 1.1.1
* ESP32 Boards 2.0.9

```
Sizes of Objects:
sizeof(LocalDate): 4
sizeof(LocalTime): 4
sizeof(LocalDateTime): 8
sizeof(TimeOffset): 4
sizeof(OffsetDateTime): 12
sizeof(TimeZone): 12
sizeof(TimeZoneData): 8
sizeof(ZonedDateTime): 24
sizeof(ZonedExtra): 28
sizeof(TimePeriod): 4
Basic:
  sizeof(basic::Info::ZoneContext): 28
  sizeof(basic::Info::ZoneEra): 16
  sizeof(basic::Info::ZoneInfo): 24
  sizeof(basic::Info::ZoneRule): 9
  sizeof(basic::Info::ZonePolicy): 8
  sizeof(basic::ZoneRegistrar): 8
  sizeof(BasicZoneProcessor): 208
  sizeof(BasicZoneProcessorCache<1>): 216
  sizeof(BasicZoneManager): 12
  sizeof(BasicZoneProcessor::Transition): 36
Extended:
  sizeof(extended::Info::ZoneContext): 28
  sizeof(extended::Info::ZoneEra): 16
  sizeof(extended::Info::ZoneInfo): 24
  sizeof(extended::Info::ZoneRule): 9
  sizeof(extended::Info::ZonePolicy): 8
  sizeof(extended::ZoneRegistrar): 8
  sizeof(ExtendedZoneProcessor): 752
  sizeof(ExtendedZoneProcessorCache<1>): 760
  sizeof(ExtendedZoneManager): 12
  sizeof(ExtendedZoneProcessor::Transition): 64
  sizeof(ExtendedZoneProcessor::TransitionStorage): 548
  sizeof(ExtendedZoneProcessor::MatchingEra): 44
Complete:
  sizeof(complete::Info::ZoneContext): 28
  sizeof(complete::Info::ZoneEra): 20
  sizeof(complete::Info::ZoneInfo): 24
  sizeof(complete::Info::ZoneRule): 12
  sizeof(complete::Info::ZonePolicy): 8
  sizeof(complete::ZoneRegistrar): 8
  sizeof(CompleteZoneProcessor): 752
  sizeof(CompleteZoneProcessorCache<1>): 760
  sizeof(CompleteZoneManager): 12
  sizeof(CompleteZoneProcessor::Transition): 64
  sizeof(CompleteZoneProcessor::TransitionStorage): 548
  sizeof(CompleteZoneProcessor::MatchingEra): 44

CPU:
+--------------------------------------------------+----------+
| Method                                           |   micros |
|--------------------------------------------------+----------|
| EmptyLoop                                        |    1.200 |
|--------------------------------------------------+----------|
| LocalDate::forEpochDays()                        |    0.800 |
| LocalDate::toEpochDays()                         |    0.300 |
| LocalDate::dayOfWeek()                           |    0.400 |
|--------------------------------------------------+----------|
| OffsetDateTime::forEpochSeconds()                |    1.300 |
| OffsetDateTime::toEpochSeconds()                 |    1.500 |
|--------------------------------------------------+----------|
| ZonedDateTime::toEpochSeconds()                  |    1.300 |
| ZonedDateTime::toEpochDays()                     |    1.150 |
| ZonedDateTime::forEpochSeconds(UTC)              |    2.000 |
|--------------------------------------------------+----------|
| ZonedDateTime::forEpochSeconds(Basic_nocache)    |   23.200 |
| ZonedDateTime::forEpochSeconds(Basic_cached)     |    2.350 |
| ZonedDateTime::forEpochSeconds(Extended_nocache) |   69.250 |
| ZonedDateTime::forEpochSeconds(Extended_cached)  |    4.000 |
| ZonedDateTime::forEpochSeconds(Complete_nocache) |   75.000 |
| ZonedDateTime::forEpochSeconds(Complete_cached)  |    4.000 |
|--------------------------------------------------+----------|
| ZonedDateTime::forComponents(Basic_nocache)      |   29.800 |
| ZonedDateTime::forComponents(Basic_cached)       |    9.650 |
| ZonedDateTime::forComponents(Extended_nocache)   |   55.850 |
| ZonedDateTime::forComponents(Extended_cached)    |    1.000 |
| ZonedDateTime::forComponents(Complete_nocache)   |   61.850 |
| ZonedDateTime::forComponents(Complete_cached)    |    1.050 |
|--------------------------------------------------+----------|
| ZonedExtra::forEpochSeconds(Basic_nocache)       |   23.100 |
| ZonedExtra::forEpochSeconds(Basic_cached)        |    1.550 |
| ZonedExtra::forEpochSeconds(Extended_nocache)    |   68.250 |
| ZonedExtra::forEpochSeconds(Extended_cached)     |    3.000 |
| ZonedExtra::forEpochSeconds(Complete_nocache)    |   79.350 |
| ZonedExtra::forEpochSeconds(Complete_cached)     |    3.000 |
|--------------------------------------------------+----------|
| ZonedExtra::forComponents(Basic_nocache)         |   37.200 |
| ZonedExtra::forComponents(Basic_cached)          |    9.950 |
| ZonedExtra::forComponents(Extended_nocache)      |   56.100 |
| ZonedExtra::forComponents(Extended_cached)       |    1.250 |
| ZonedExtra::forComponents(Complete_nocache)      |   63.900 |
| ZonedExtra::forComponents(Complete_cached)       |    1.250 |
|--------------------------------------------------+----------|
| BasicZoneRegistrar::findIndexForName(binary)     |    3.050 |
| BasicZoneRegistrar::findIndexForIdBinary()       |    0.700 |
| BasicZoneRegistrar::findIndexForIdLinear()       |    2.700 |
|--------------------------------------------------+----------|
| ExtendedZoneRegistrar::findIndexForName(binary)  |    3.000 |
| ExtendedZoneRegistrar::findIndexForIdBinary()    |    0.700 |
| ExtendedZoneRegistrar::findIndexForIdLinear()    |    2.750 |
|--------------------------------------------------+----------|
| CompleteZoneRegistrar::findIndexForName(binary)  |    3.050 |
| CompleteZoneRegistrar::findIndexForIdBinary()    |    0.650 |
| CompleteZoneRegistrar::findIndexForIdLinear()    |    2.900 |
+--------------------------------------------------+----------+
Iterations_per_run: 20000

```

Note: Once the benchmark of the function under test becomes smaller than the
duration of an empty loop, the numbers become unreliable.

