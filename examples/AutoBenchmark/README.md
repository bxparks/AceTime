# Auto Benchmark

Here are the results from `AutoBenchmark.ino` for various boards.
These results show that integer division and modulus operations are incredibly
slow on 8-bit AVR processors.

**Version**: AceTime v1.7.4

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

In v1.7.4+:
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
sizeof(BasicZoneProcessor): 116
sizeof(ExtendedZoneProcessor): 432
sizeof(BasicZoneManager<1>): 129
sizeof(ExtendedZoneManager<1>): 445
sizeof(TimeZoneData): 5
sizeof(TimeZone): 5
sizeof(ZonedDateTime): 13
sizeof(TimePeriod): 4
sizeof(clock::DS3231Clock): 3
sizeof(clock::SystemClock): 28
sizeof(clock::SystemClockLoop): 41
sizeof(clock::SystemClockCoroutine): 52
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
| EmptyLoop                                        |      4.0 |
|--------------------------------------------------+----------|
| LocalDate::forEpochDays()                        |    218.0 |
| LocalDate::toEpochDays()                         |     56.0 |
| LocalDate::dayOfWeek()                           |     49.0 |
| OffsetDateTime::forEpochSeconds()                |    323.0 |
| OffsetDateTime::toEpochSeconds()                 |     86.0 |
| ZonedDateTime::toEpochSeconds()                  |     83.0 |
| ZonedDateTime::toEpochDays()                     |     72.0 |
| ZonedDateTime::forEpochSeconds(UTC)              |    338.0 |
| ZonedDateTime::forEpochSeconds(Basic_nocache)    |   1188.0 |
| ZonedDateTime::forEpochSeconds(Basic_cached)     |    617.0 |
| ZonedDateTime::forEpochSeconds(Extended_nocache) |   2125.0 |
| ZonedDateTime::forEpochSeconds(Extended_cached)  |    616.0 |
| BasicZoneManager::createForZoneName(binary)      |    123.0 |
| BasicZoneManager::createForZoneId(binary)        |     48.0 |
| BasicZoneManager::createForZoneId(linear)        |    307.0 |
| BasicZoneManager::createForZoneId(link)          |     82.0 |
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
sizeof(BasicZoneProcessor): 116
sizeof(ExtendedZoneProcessor): 432
sizeof(BasicZoneManager<1>): 129
sizeof(ExtendedZoneManager<1>): 445
sizeof(TimeZoneData): 5
sizeof(TimeZone): 5
sizeof(ZonedDateTime): 13
sizeof(TimePeriod): 4
sizeof(clock::DS3231Clock): 3
sizeof(clock::SystemClock): 28
sizeof(clock::SystemClockLoop): 41
sizeof(clock::SystemClockCoroutine): 52
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
| EmptyLoop                                        |      3.0 |
|--------------------------------------------------+----------|
| LocalDate::forEpochDays()                        |    219.0 |
| LocalDate::toEpochDays()                         |     56.0 |
| LocalDate::dayOfWeek()                           |     48.0 |
| OffsetDateTime::forEpochSeconds()                |    324.0 |
| OffsetDateTime::toEpochSeconds()                 |     81.0 |
| ZonedDateTime::toEpochSeconds()                  |     80.0 |
| ZonedDateTime::toEpochDays()                     |     68.0 |
| ZonedDateTime::forEpochSeconds(UTC)              |    338.0 |
| ZonedDateTime::forEpochSeconds(Basic_nocache)    |   1174.0 |
| ZonedDateTime::forEpochSeconds(Basic_cached)     |    621.0 |
| ZonedDateTime::forEpochSeconds(Extended_nocache) |     -1.0 |
| ZonedDateTime::forEpochSeconds(Extended_cache)   |     -1.0 |
| BasicZoneManager::createForZoneName(binary)      |    118.0 |
| BasicZoneManager::createForZoneId(binary)        |     47.0 |
| BasicZoneManager::createForZoneId(linear)        |    306.0 |
| BasicZoneManager::createForZoneId(link)          |     85.0 |
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
sizeof(BasicZoneProcessor): 164
sizeof(ExtendedZoneProcessor): 540
sizeof(BasicZoneManager<1>): 188
sizeof(ExtendedZoneManager<1>): 564
sizeof(TimeZoneData): 8
sizeof(TimeZone): 12
sizeof(ZonedDateTime): 20
sizeof(TimePeriod): 4
sizeof(clock::DS3231Clock): 8
sizeof(clock::SystemClock): 36
sizeof(clock::SystemClockLoop): 52
sizeof(clock::SystemClockCoroutine): 72
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
| EmptyLoop                                        |      1.4 |
|--------------------------------------------------+----------|
| LocalDate::forEpochDays()                        |     24.2 |
| LocalDate::toEpochDays()                         |      8.6 |
| LocalDate::dayOfWeek()                           |     11.6 |
| OffsetDateTime::forEpochSeconds()                |     35.0 |
| OffsetDateTime::toEpochSeconds()                 |     18.6 |
| ZonedDateTime::toEpochSeconds()                  |     18.4 |
| ZonedDateTime::toEpochDays()                     |     14.8 |
| ZonedDateTime::forEpochSeconds(UTC)              |     40.2 |
| ZonedDateTime::forEpochSeconds(Basic_nocache)    |    235.0 |
| ZonedDateTime::forEpochSeconds(Basic_cached)     |     71.8 |
| ZonedDateTime::forEpochSeconds(Extended_nocache) |    473.8 |
| ZonedDateTime::forEpochSeconds(Extended_cached)  |     71.8 |
| BasicZoneManager::createForZoneName(binary)      |     15.8 |
| BasicZoneManager::createForZoneId(binary)        |      4.4 |
| BasicZoneManager::createForZoneId(linear)        |     14.6 |
| BasicZoneManager::createForZoneId(link)          |      8.8 |
+--------------------------------------------------+----------+
Iterations_per_run: 5000

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
sizeof(BasicZoneProcessor): 164
sizeof(ExtendedZoneProcessor): 540
sizeof(BasicZoneManager<1>): 188
sizeof(ExtendedZoneManager<1>): 564
sizeof(TimeZoneData): 8
sizeof(TimeZone): 12
sizeof(ZonedDateTime): 20
sizeof(TimePeriod): 4
sizeof(clock::DS3231Clock): 8
sizeof(clock::SystemClock): 36
sizeof(clock::SystemClockLoop): 52
sizeof(clock::SystemClockCoroutine): 72
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
| EmptyLoop                                        |      1.2 |
|--------------------------------------------------+----------|
| LocalDate::forEpochDays()                        |      2.3 |
| LocalDate::toEpochDays()                         |      1.1 |
| LocalDate::dayOfWeek()                           |      1.2 |
| OffsetDateTime::forEpochSeconds()                |      3.7 |
| OffsetDateTime::toEpochSeconds()                 |      4.8 |
| ZonedDateTime::toEpochSeconds()                  |      4.9 |
| ZonedDateTime::toEpochDays()                     |      3.6 |
| ZonedDateTime::forEpochSeconds(UTC)              |      4.7 |
| ZonedDateTime::forEpochSeconds(Basic_nocache)    |     70.5 |
| ZonedDateTime::forEpochSeconds(Basic_cached)     |     10.5 |
| ZonedDateTime::forEpochSeconds(Extended_nocache) |    145.9 |
| ZonedDateTime::forEpochSeconds(Extended_cached)  |     10.2 |
| BasicZoneManager::createForZoneName(binary)      |     11.8 |
| BasicZoneManager::createForZoneId(binary)        |      3.1 |
| BasicZoneManager::createForZoneId(linear)        |     17.7 |
| BasicZoneManager::createForZoneId(link)          |      6.4 |
+--------------------------------------------------+----------+
Iterations_per_run: 10000

```

## ESP8266

* NodeMCU 1.0 clone, 80MHz ESP8266
* Arduino IDE 1.8.13
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
sizeof(clock::DS3231Clock): 8
sizeof(clock::NtpClock): 92
sizeof(clock::SystemClock): 36
sizeof(clock::SystemClockLoop): 52
sizeof(clock::SystemClockCoroutine): 72
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
| EmptyLoop                                        |      4.9 |
|--------------------------------------------------+----------|
| LocalDate::forEpochDays()                        |      8.0 |
| LocalDate::toEpochDays()                         |      3.8 |
| LocalDate::dayOfWeek()                           |      3.8 |
| OffsetDateTime::forEpochSeconds()                |     12.3 |
| OffsetDateTime::toEpochSeconds()                 |      6.8 |
| ZonedDateTime::toEpochSeconds()                  |      6.9 |
| ZonedDateTime::toEpochDays()                     |      5.7 |
| ZonedDateTime::forEpochSeconds(UTC)              |     12.9 |
| ZonedDateTime::forEpochSeconds(Basic_nocache)    |     95.4 |
| ZonedDateTime::forEpochSeconds(Basic_cached)     |     25.9 |
| ZonedDateTime::forEpochSeconds(Extended_nocache) |    189.6 |
| ZonedDateTime::forEpochSeconds(Extended_cached)  |     25.8 |
| BasicZoneManager::createForZoneName(binary)      |     14.8 |
| BasicZoneManager::createForZoneId(binary)        |      6.6 |
| BasicZoneManager::createForZoneId(linear)        |     44.2 |
| BasicZoneManager::createForZoneId(link)          |     11.5 |
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
sizeof(BasicZoneProcessor): 164
sizeof(ExtendedZoneProcessor): 540
sizeof(BasicZoneManager<1>): 188
sizeof(ExtendedZoneManager<1>): 564
sizeof(TimeZoneData): 8
sizeof(TimeZone): 12
sizeof(ZonedDateTime): 20
sizeof(TimePeriod): 4
sizeof(clock::DS3231Clock): 8
sizeof(clock::NtpClock): 116
sizeof(clock::SystemClock): 36
sizeof(clock::SystemClockLoop): 52
sizeof(clock::SystemClockCoroutine): 72
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
| EmptyLoop                                        |      1.4 |
|--------------------------------------------------+----------|
| LocalDate::forEpochDays()                        |      0.7 |
| LocalDate::toEpochDays()                         |      0.2 |
| LocalDate::dayOfWeek()                           |      0.4 |
| OffsetDateTime::forEpochSeconds()                |      1.1 |
| OffsetDateTime::toEpochSeconds()                 |      1.3 |
| ZonedDateTime::toEpochSeconds()                  |      1.4 |
| ZonedDateTime::toEpochDays()                     |      0.9 |
| ZonedDateTime::forEpochSeconds(UTC)              |      1.3 |
| ZonedDateTime::forEpochSeconds(Basic_nocache)    |     16.1 |
| ZonedDateTime::forEpochSeconds(Basic_cached)     |      2.6 |
| ZonedDateTime::forEpochSeconds(Extended_nocache) |     34.0 |
| ZonedDateTime::forEpochSeconds(Extended_cached)  |      2.5 |
| BasicZoneManager::createForZoneName(binary)      |      3.0 |
| BasicZoneManager::createForZoneId(binary)        |      0.7 |
| BasicZoneManager::createForZoneId(linear)        |      2.6 |
| BasicZoneManager::createForZoneId(link)          |      1.4 |
+--------------------------------------------------+----------+
Iterations_per_run: 20000

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
sizeof(BasicZoneProcessor): 164
sizeof(ExtendedZoneProcessor): 540
sizeof(BasicZoneManager<1>): 188
sizeof(ExtendedZoneManager<1>): 564
sizeof(TimeZoneData): 8
sizeof(TimeZone): 12
sizeof(ZonedDateTime): 20
sizeof(TimePeriod): 4
sizeof(clock::DS3231Clock): 8
sizeof(clock::SystemClock): 36
sizeof(clock::SystemClockLoop): 52
sizeof(clock::SystemClockCoroutine): 72
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
| EmptyLoop                                        |      0.5 |
|--------------------------------------------------+----------|
| LocalDate::forEpochDays()                        |      1.9 |
| LocalDate::toEpochDays()                         |      0.1 |
| LocalDate::dayOfWeek()                           |      0.9 |
| OffsetDateTime::forEpochSeconds()                |      2.4 |
| OffsetDateTime::toEpochSeconds()                 |      0.5 |
| ZonedDateTime::toEpochSeconds()                  |      0.2 |
| ZonedDateTime::toEpochDays()                     |      0.5 |
| ZonedDateTime::forEpochSeconds(UTC)              |      2.6 |
| ZonedDateTime::forEpochSeconds(Basic_nocache)    |     36.0 |
| ZonedDateTime::forEpochSeconds(Basic_cached)     |      6.5 |
| ZonedDateTime::forEpochSeconds(Extended_nocache) |     82.2 |
| ZonedDateTime::forEpochSeconds(Extended_cached)  |      5.7 |
| BasicZoneManager::createForZoneName(binary)      |      6.0 |
| BasicZoneManager::createForZoneId(binary)        |      2.0 |
| BasicZoneManager::createForZoneId(linear)        |     10.7 |
| BasicZoneManager::createForZoneId(link)          |      4.7 |
+--------------------------------------------------+----------+
Iterations_per_run: 20000

```

