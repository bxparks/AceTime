# Auto Benchmark

Here are the results from `AutoBenchmark.ino` for various boards.
These results show that integer division and modulus operations are incredibly
slow on 8-bit AVR processors.

## Dependencies

This program depends on the following libraries:

* [AceTime](https://github.com/bxparks/AceTime)
* [AceRoutine](https://github.com/bxparks/AceRoutine)

## Arduino Nano

IDE: Arduino 1.8.9 (AVR Core 1.6.23)

Memory:

```
sizeof(LocalDate): 3
sizeof(LocalTime): 3
sizeof(LocalDateTime): 6
sizeof(TimeOffset): 1
sizeof(OffsetDateTime): 7
sizeof(BasicZoneProcessor): 99
sizeof(ExtendedZoneProcessor): 397
sizeof(BasicZoneRegistrar): 5
sizeof(ExtendedZoneRegistrar): 5
sizeof(BasicZoneManager<1>): 107
sizeof(ExtendedZoneManager<1>): 405
sizeof(TimeZoneData): 5
sizeof(TimeZone): 5
sizeof(ZonedDateTime): 12
sizeof(TimePeriod): 4
sizeof(clock::SystemClock): 17
sizeof(clock::DS3231TimeKeeper): 3
sizeof(clock::SystemClockSyncLoop): 10
sizeof(clock::SystemClockSyncCoroutine): 29
sizeof(basic::ZoneContext): 6
sizeof(basic::ZoneEra): 11
sizeof(basic::ZoneInfo): 12
sizeof(basic::ZoneRule): 9
sizeof(basic::ZonePolicy): 6
sizeof(basic::Transition): 18
sizeof(extended::Transition): 40
sizeof(extended::ZoneMatch): 12
```

CPU:

```
+--------------------------------------------------+----------+
| Method                                           |   micros |
|--------------------------------------------------|----------|
| Empty loop                                       |    5.200 |
|--------------------------------------------------|----------|
| LocalDate::forEpochDays()                        |  216.800 |
| LocalDate::toEpochDays()                         |   57.200 |
| LocalDate::dayOfWeek()                           |   50.400 |
| OffsetDateTime::forEpochSeconds()                |  320.800 |
| OffsetDateTime::toEpochSeconds()                 |   86.400 |
| ZonedDateTime::toEpochSeconds()                  |   87.600 |
| ZonedDateTime::toEpochDays()                     |   73.200 |
| ZonedDateTime::forEpochSeconds(UTC)              |  334.000 |
| ZonedDateTime::forEpochSeconds(Basic nocache)    | 1139.200 |
| ZonedDateTime::forEpochSeconds(Basic cached)     |  618.000 |
| ZonedDateTime::forEpochSeconds(Extended nocache) | 1962.000 |
| ZonedDateTime::forEpochSeconds(Extended cached)  |  615.600 |
+--------------------------------------------------+----------+
Number of iterations per run: 2500
```

## SAMD21 M0 Mini (Arduino MKR Zero Compatible)

IDE: Arduino 1.8.9 (SAMD Core 1.8.3)

Memory:
```
sizeof(LocalDate): 3
sizeof(LocalTime): 3
sizeof(LocalDateTime): 6
sizeof(TimeOffset): 1
sizeof(OffsetDateTime): 7
sizeof(BasicZoneProcessor): 156
sizeof(ExtendedZoneProcessor): 500
sizeof(BasicZoneRegistrar): 12
sizeof(ExtendedZoneRegistrar): 12
sizeof(BasicZoneManager<1>): 176
sizeof(ExtendedZoneManager<1>): 520
sizeof(TimeZoneData): 8
sizeof(TimeZone): 12
sizeof(ZonedDateTime): 20
sizeof(TimePeriod): 4
sizeof(clock::SystemClock): 24
sizeof(clock::DS3231TimeKeeper): 8
sizeof(clock::SystemClockSyncLoop): 16
sizeof(clock::SystemClockSyncCoroutine): 48
sizeof(basic::ZoneContext): 8
sizeof(basic::ZoneEra): 16
sizeof(basic::ZoneInfo): 20
sizeof(basic::ZoneRule): 9
sizeof(basic::ZonePolicy): 12
sizeof(basic::Transition): 28
sizeof(extended::Transition): 48
sizeof(extended::ZoneMatch): 16
```

CPU:
```
+--------------------------------------------------+----------+
| Method                                           |   micros |
|--------------------------------------------------|----------|
| Empty loop                                       |    1.700 |
|--------------------------------------------------|----------|
| LocalDate::forEpochDays()                        |   14.900 |
| LocalDate::toEpochDays()                         |    7.600 |
| LocalDate::dayOfWeek()                           |    7.500 |
| OffsetDateTime::forEpochSeconds()                |   21.500 |
| OffsetDateTime::toEpochSeconds()                 |   13.800 |
| ZonedDateTime::toEpochSeconds()                  |   14.000 |
| ZonedDateTime::toEpochDays()                     |   12.300 |
| ZonedDateTime::forEpochSeconds(UTC)              |   24.300 |
| ZonedDateTime::forEpochSeconds(Basic nocache)    |  177.400 |
| ZonedDateTime::forEpochSeconds(Basic cached)     |   49.900 |
| ZonedDateTime::forEpochSeconds(Extended nocache) |  333.200 |
| ZonedDateTime::forEpochSeconds(Extended cached)  |   50.600 |
+--------------------------------------------------+----------+
Number of iterations per run: 10000
```

## ESP8266

IDE: Arduino 1.8.9 (ESP Core 2.5.2)

Memory:

```
sizeof(LocalDate): 3
sizeof(LocalTime): 3
sizeof(LocalDateTime): 6
sizeof(TimeOffset): 1
sizeof(OffsetDateTime): 7
sizeof(BasicZoneProcessor): 156
sizeof(ExtendedZoneProcessor): 500
sizeof(BasicZoneRegistrar): 12
sizeof(ExtendedZoneRegistrar): 12
sizeof(BasicZoneManager<1>): 176
sizeof(ExtendedZoneManager<1>): 520
sizeof(TimeZoneData): 8
sizeof(TimeZone): 12
sizeof(ZonedDateTime): 20
sizeof(TimePeriod): 4
sizeof(clock::SystemClock): 24
sizeof(clock::DS3231TimeKeeper): 8
sizeof(clock::NtpTimeProvider): 88
sizeof(clock::SystemClockSyncLoop): 16
sizeof(clock::SystemClockSyncCoroutine): 48
sizeof(basic::ZoneContext): 8
sizeof(basic::ZoneEra): 16
sizeof(basic::ZoneInfo): 20
sizeof(basic::ZoneRule): 9
sizeof(basic::ZonePolicy): 12
sizeof(basic::Transition): 28
sizeof(extended::Transition): 48
sizeof(extended::ZoneMatch): 16
```

CPU:

```
+--------------------------------------------------+----------+
| Method                                           |   micros |
|--------------------------------------------------|----------|
| Empty loop                                       |    5.100 |
|--------------------------------------------------|----------|
| LocalDate::forEpochDays()                        |    7.600 |
| LocalDate::toEpochDays()                         |    4.200 |
| LocalDate::dayOfWeek()                           |    4.100 |
| OffsetDateTime::forEpochSeconds()                |   12.500 |
| OffsetDateTime::toEpochSeconds()                 |    7.000 |
| ZonedDateTime::toEpochSeconds()                  |    7.100 |
| ZonedDateTime::toEpochDays()                     |    6.000 |
| ZonedDateTime::forEpochSeconds(UTC)              |   13.500 |
| ZonedDateTime::forEpochSeconds(Basic nocache)    |   93.500 |
| ZonedDateTime::forEpochSeconds(Basic cached)     |   27.600 |
| ZonedDateTime::forEpochSeconds(Extended nocache) |  178.200 |
| ZonedDateTime::forEpochSeconds(Extended cached)  |   27.300 |
+--------------------------------------------------+----------+
Number of iterations per run: 10000
```

## ESP32

IDE: Arduino 1.8.9 (ESP32 Core 1.0.2)

Memory:

```
sizeof(LocalDate): 3
sizeof(LocalTime): 3
sizeof(LocalDateTime): 6
sizeof(TimeOffset): 1
sizeof(OffsetDateTime): 7
sizeof(BasicZoneProcessor): 156
sizeof(ExtendedZoneProcessor): 500
sizeof(BasicZoneRegistrar): 12
sizeof(ExtendedZoneRegistrar): 12
sizeof(BasicZoneManager<1>): 176
sizeof(ExtendedZoneManager<1>): 520
sizeof(TimeZoneData): 8
sizeof(TimeZone): 12
sizeof(ZonedDateTime): 20
sizeof(TimePeriod): 4
sizeof(clock::SystemClock): 24
sizeof(clock::DS3231TimeKeeper): 8
sizeof(clock::NtpTimeProvider): 116
sizeof(clock::SystemClockSyncLoop): 16
sizeof(clock::SystemClockSyncCoroutine): 48
sizeof(basic::ZoneContext): 8
sizeof(basic::ZoneEra): 16
sizeof(basic::ZoneInfo): 20
sizeof(basic::ZoneRule): 9
sizeof(basic::ZonePolicy): 12
sizeof(basic::Transition): 28
sizeof(extended::Transition): 48
sizeof(extended::ZoneMatch): 16
```

CPU:

```
+--------------------------------------------------+----------+
| Method                                           |   micros |
|--------------------------------------------------|----------|
| Empty loop                                       |    1.400 |
|--------------------------------------------------|----------|
| LocalDate::forEpochDays()                        |    0.470 |
| LocalDate::toEpochDays()                         |    0.390 |
| LocalDate::dayOfWeek()                           |    0.410 |
| OffsetDateTime::forEpochSeconds()                |    1.020 |
| OffsetDateTime::toEpochSeconds()                 |    1.400 |
| ZonedDateTime::toEpochSeconds()                  |    1.410 |
| ZonedDateTime::toEpochDays()                     |    1.070 |
| ZonedDateTime::forEpochSeconds(UTC)              |    1.550 |
| ZonedDateTime::forEpochSeconds(Basic nocache)    |   14.940 |
| ZonedDateTime::forEpochSeconds(Basic cached)     |    2.820 |
| ZonedDateTime::forEpochSeconds(Extended nocache) |   30.990 |
| ZonedDateTime::forEpochSeconds(Extended cached)  |    2.760 |
+--------------------------------------------------+----------+
Number of iterations per run: 100000
```

Note: Once the benchmark of the function under test becomes smaller than the
duration of an empty loop, the numbers become suspect.

## Teensy 3.2

IDE: Arduino 1.8.9 (Teensyduino 1.46)
Compiler Optimization: "Faster"

Memory:

```
sizeof(LocalDate): 3
sizeof(LocalTime): 3
sizeof(LocalDateTime): 6
sizeof(TimeOffset): 1
sizeof(OffsetDateTime): 7
sizeof(BasicZoneProcessor): 156
sizeof(ExtendedZoneProcessor): 500
sizeof(BasicZoneRegistrar): 12
sizeof(ExtendedZoneRegistrar): 12
sizeof(BasicZoneManager<1>): 176
sizeof(ExtendedZoneManager<1>): 520
sizeof(TimeZoneData): 8
sizeof(TimeZone): 12
sizeof(ZonedDateTime): 20
sizeof(TimePeriod): 4
sizeof(clock::SystemClock): 24
sizeof(clock::DS3231TimeKeeper): 8
sizeof(clock::SystemClockSyncLoop): 16
sizeof(clock::SystemClockSyncCoroutine): 48
sizeof(basic::ZoneContext): 8
sizeof(basic::ZoneEra): 16
sizeof(basic::ZoneInfo): 20
sizeof(basic::ZoneRule): 9
sizeof(basic::ZonePolicy): 12
sizeof(basic::Transition): 28
sizeof(extended::Transition): 48
sizeof(extended::ZoneMatch): 16
```
CPU:
```
+--------------------------------------------------+----------+
| Method                                           |   micros |
|--------------------------------------------------|----------|
| Empty loop                                       |    0.580 |
|--------------------------------------------------|----------|
| LocalDate::forEpochDays()                        |    1.390 |
| LocalDate::toEpochDays()                         |    0.720 |
| LocalDate::dayOfWeek()                           |    1.340 |
| OffsetDateTime::forEpochSeconds()                |    1.970 |
| OffsetDateTime::toEpochSeconds()                 |    0.450 |
| ZonedDateTime::toEpochSeconds()                  |    0.590 |
| ZonedDateTime::toEpochDays()                     |    0.210 |
| ZonedDateTime::forEpochSeconds(UTC)              |    2.270 |
| ZonedDateTime::forEpochSeconds(Basic nocache)    |   29.200 |
| ZonedDateTime::forEpochSeconds(Basic cached)     |    6.060 |
| ZonedDateTime::forEpochSeconds(Extended nocache) |   65.930 |
| ZonedDateTime::forEpochSeconds(Extended cached)  |    6.160 |
+--------------------------------------------------+----------+
Number of iterations per run: 100000
```

Note: Once the benchmark of the function under test becomes smaller than the
duration of an empty loop, the numbers become suspect.
