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
sizeof(basic::ZoneEra): 11
sizeof(basic::ZoneInfo): 8
sizeof(basic::ZoneRule): 9
sizeof(basic::ZonePolicy): 6
sizeof(basic::Transition): 18
sizeof(extended::Transition): 40
sizeof(extended::ZoneMatch): 12
sizeof(ZoneSpecifier): 3
sizeof(ManualZoneSpecifier): 10
sizeof(BasicZoneSpecifier): 99
sizeof(ExtendedZoneSpecifier): 397
sizeof(TimeZone): 3
sizeof(ZonedDateTime): 10
sizeof(TimePeriod): 4
sizeof(SystemClock): 17
sizeof(DS3231TimeKeeper): 3
sizeof(SystemClockSyncLoop): 14
sizeof(SystemClockSyncCoroutine): 31
```

CPU:

```
+--------------------------------------------------+----------+
| Method                                           |   micros |
|--------------------------------------------------|----------|
| Empty loop                                       |    5.200 |
|--------------------------------------------------|----------|
| LocalDate::forEpochDays()                        |  216.400 |
| LocalDate::toEpochDays()                         |   57.600 |
| LocalDate::dayOfWeek()                           |   50.000 |
| OffsetDateTime::forEpochSeconds()                |  320.800 |
| OffsetDateTime::toEpochSeconds()                 |   85.600 |
| ZonedDateTime::toEpochSeconds()                  |   87.200 |
| ZonedDateTime::toEpochDays()                     |   72.800 |
| ZonedDateTime::forEpochSeconds(UTC)              |  332.000 |
| ZonedDateTime::forEpochSeconds(Basic nocache)    | 1134.800 |
| ZonedDateTime::forEpochSeconds(Basic cached)     |  610.800 |
| ZonedDateTime::forEpochSeconds(Extended nocache) | 1966.000 |
| ZonedDateTime::forEpochSeconds(Extended cached)  |  608.800 |
+--------------------------------------------------+----------+
Number of iterations per run: 2500
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
sizeof(basic::ZoneEra): 24
sizeof(basic::ZoneInfo): 16
sizeof(basic::ZoneRule): 9
sizeof(basic::ZonePolicy): 16
sizeof(basic::Transition): 28
sizeof(extended::Transition): 48
sizeof(extended::ZoneMatch): 16
sizeof(ZoneSpecifier): 8
sizeof(ManualZoneSpecifier): 20
sizeof(BasicZoneSpecifier): 156
sizeof(ExtendedZoneSpecifier): 500
sizeof(TimeZone): 8
sizeof(ZonedDateTime): 16
sizeof(TimePeriod): 4
sizeof(SystemClock): 24
sizeof(DS3231TimeKeeper): 8
sizeof(NtpTimeProvider): 88
sizeof(SystemClockSyncLoop): 20
sizeof(SystemClockSyncCoroutine): 52
```

CPU:

```
+--------------------------------------------------+----------+
| Method                                           |   micros |
|--------------------------------------------------|----------|
| Empty loop                                       |    5.100 |
|--------------------------------------------------|----------|
| LocalDate::forEpochDays()                        |    7.600 |
| LocalDate::toEpochDays()                         |    4.300 |
| LocalDate::dayOfWeek()                           |    4.000 |
| OffsetDateTime::forEpochSeconds()                |   12.600 |
| OffsetDateTime::toEpochSeconds()                 |    7.100 |
| ZonedDateTime::toEpochSeconds()                  |    6.900 |
| ZonedDateTime::toEpochDays()                     |    6.100 |
| ZonedDateTime::forEpochSeconds(UTC)              |   13.300 |
| ZonedDateTime::forEpochSeconds(Basic nocache)    |   91.600 |
| ZonedDateTime::forEpochSeconds(Basic cached)     |   25.900 |
| ZonedDateTime::forEpochSeconds(Extended nocache) |  175.900 |
| ZonedDateTime::forEpochSeconds(Extended cached)  |   25.800 |
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
sizeof(basic::ZoneEra): 24
sizeof(basic::ZoneInfo): 16
sizeof(basic::ZoneRule): 9
sizeof(basic::ZonePolicy): 16
sizeof(basic::Transition): 28
sizeof(extended::Transition): 48
sizeof(extended::ZoneMatch): 16
sizeof(ZoneSpecifier): 8
sizeof(ManualZoneSpecifier): 20
sizeof(BasicZoneSpecifier): 156
sizeof(ExtendedZoneSpecifier): 500
sizeof(TimeZone): 8
sizeof(ZonedDateTime): 16
sizeof(TimePeriod): 4
sizeof(SystemClock): 24
sizeof(DS3231TimeKeeper): 8
sizeof(NtpTimeProvider): 116
sizeof(SystemClockSyncLoop): 20
sizeof(SystemClockSyncCoroutine): 52
```

CPU:

```
+--------------------------------------------------+----------+
| Method                                           |   micros |
|--------------------------------------------------|----------|
| Empty loop                                       |    1.400 |
|--------------------------------------------------|----------|
| LocalDate::forEpochDays()                        |    0.460 |
| LocalDate::toEpochDays()                         |    0.390 |
| LocalDate::dayOfWeek()                           |    0.440 |
| OffsetDateTime::forEpochSeconds()                |    1.010 |
| OffsetDateTime::toEpochSeconds()                 |    1.390 |
| ZonedDateTime::toEpochSeconds()                  |    1.420 |
| ZonedDateTime::toEpochDays()                     |    1.110 |
| ZonedDateTime::forEpochSeconds(UTC)              |    1.450 |
| ZonedDateTime::forEpochSeconds(Basic nocache)    |   14.840 |
| ZonedDateTime::forEpochSeconds(Basic cached)     |    2.550 |
| ZonedDateTime::forEpochSeconds(Extended nocache) |   31.160 |
| ZonedDateTime::forEpochSeconds(Extended cached)  |    2.520 |
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
sizeof(basic::ZoneEra): 24
sizeof(basic::ZoneInfo): 16
sizeof(basic::ZoneRule): 9
sizeof(basic::ZonePolicy): 16
sizeof(basic::Transition): 28
sizeof(extended::Transition): 48
sizeof(extended::ZoneMatch): 16
sizeof(ZoneSpecifier): 8
sizeof(ManualZoneSpecifier): 20
sizeof(BasicZoneSpecifier): 156
sizeof(ExtendedZoneSpecifier): 500
sizeof(TimeZone): 8
sizeof(ZonedDateTime): 16
sizeof(TimePeriod): 4
sizeof(SystemClock): 24
sizeof(DS3231TimeKeeper): 8
sizeof(SystemClockSyncLoop): 20
sizeof(SystemClockSyncCoroutine): 52
```

CPU:

```
+--------------------------------------------------+----------+
| Method                                           |   micros |
|--------------------------------------------------|----------|
| Empty loop                                       |    0.570 |
|--------------------------------------------------|----------|
| LocalDate::forEpochDays()                        |    1.150 |
| LocalDate::toEpochDays()                         |    0.910 |
| LocalDate::dayOfWeek()                           |    1.590 |
| OffsetDateTime::forEpochSeconds()                |    1.900 |
| OffsetDateTime::toEpochSeconds()                 |    0.680 |
| ZonedDateTime::toEpochSeconds()                  |    0.500 |
| ZonedDateTime::toEpochDays()                     |    0.500 |
| ZonedDateTime::forEpochSeconds(UTC)              |    2.230 |
| ZonedDateTime::forEpochSeconds(Basic nocache)    |   28.290 |
| ZonedDateTime::forEpochSeconds(Basic cached)     |    5.530 |
| ZonedDateTime::forEpochSeconds(Extended nocache) |   64.400 |
| ZonedDateTime::forEpochSeconds(Extended cached)  |    5.450 |
+--------------------------------------------------+----------+
Number of iterations per run: 100000
```

Note: Once the benchmark of the function under test becomes smaller than the
duration of an empty loop, the numbers become suspect.
