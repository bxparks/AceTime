# Auto Benchmark

Here are the results from `AutoBenchmark.ino` for various boards.
These results show that integer division and modulus operations are incredibly
slow on 8-bit AVR processors.

## Dependencies

This program depends on the following libraries:

* [AceTime](https://github.com/bxparks/AceTime)
* [AceRoutine](https://github.com/bxparks/AceRoutine)

## Arduino Nano

* AceTime 0.7
* IDE: Arduino 1.8.9
* AVR Boards 1.6.23

Memory:

```
sizeof(LocalDate): 3
sizeof(LocalTime): 3
sizeof(LocalDateTime): 6
sizeof(TimeOffset): 2
sizeof(OffsetDateTime): 8
sizeof(BasicZoneProcessor): 99
sizeof(ExtendedZoneProcessor): 437
sizeof(BasicZoneRegistrar): 5
sizeof(ExtendedZoneRegistrar): 5
sizeof(BasicZoneManager<1>): 107
sizeof(ExtendedZoneManager<1>): 445
sizeof(TimeZoneData): 5
sizeof(TimeZone): 5
sizeof(ZonedDateTime): 13
sizeof(TimePeriod): 4
sizeof(clock::DS3231Clock): 3
sizeof(clock::SystemClock): 17
sizeof(clock::SystemClockLoop): 34
sizeof(clock::SystemClockCoroutine): 44
sizeof(basic::ZoneContext): 6
sizeof(basic::ZoneEra): 11
sizeof(basic::ZoneInfo): 12
sizeof(basic::ZoneRule): 9
sizeof(basic::ZonePolicy): 6
sizeof(basic::Transition): 18
sizeof(extended::Transition): 44
sizeof(extended::ZoneMatch): 14
```

CPU:

```
+--------------------------------------------------+----------+
| Method                                           |   micros |
|--------------------------------------------------|----------|
| Empty loop                                       |    5.200 |
|--------------------------------------------------|----------|
| LocalDate::forEpochDays()                        |  216.400 |
| LocalDate::toEpochDays()                         |   57.200 |
| LocalDate::dayOfWeek()                           |   50.400 |
| OffsetDateTime::forEpochSeconds()                |  320.800 |
| OffsetDateTime::toEpochSeconds()                 |   86.400 |
| ZonedDateTime::toEpochSeconds()                  |   87.600 |
| ZonedDateTime::toEpochDays()                     |   73.600 |
| ZonedDateTime::forEpochSeconds(UTC)              |  334.800 |
| ZonedDateTime::forEpochSeconds(Basic nocache)    | 1174.800 |
| ZonedDateTime::forEpochSeconds(Basic cached)     |  620.000 |
| ZonedDateTime::forEpochSeconds(Extended nocache) | 2039.200 |
| ZonedDateTime::forEpochSeconds(Extended cached)  |  617.600 |
+--------------------------------------------------+----------+
Number of iterations per run: 2500
```

## SAMD21 M0 Mini (Arduino MKR Zero Compatible)

* AceTime 0.7
* IDE: Arduino 1.8.9
* SparkFun SAMD Boards 1.6.2

Memory:
```
sizeof(LocalDate): 3
sizeof(LocalTime): 3
sizeof(LocalDateTime): 6
sizeof(TimeOffset): 2
sizeof(OffsetDateTime): 8
sizeof(BasicZoneProcessor): 136
sizeof(ExtendedZoneProcessor): 500
sizeof(BasicZoneRegistrar): 12
sizeof(ExtendedZoneRegistrar): 12
sizeof(BasicZoneManager<1>): 156
sizeof(ExtendedZoneManager<1>): 520
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
sizeof(basic::Transition): 24
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
| LocalDate::forEpochDays()                        |   23.200 |
| LocalDate::toEpochDays()                         |    9.600 |
| LocalDate::dayOfWeek()                           |   12.600 |
| OffsetDateTime::forEpochSeconds()                |   34.900 |
| OffsetDateTime::toEpochSeconds()                 |   17.200 |
| ZonedDateTime::toEpochSeconds()                  |   18.500 |
| ZonedDateTime::toEpochDays()                     |   16.400 |
| ZonedDateTime::forEpochSeconds(UTC)              |   41.200 |
| ZonedDateTime::forEpochSeconds(Basic nocache)    |  226.100 |
| ZonedDateTime::forEpochSeconds(Basic cached)     |   73.500 |
| ZonedDateTime::forEpochSeconds(Extended nocache) |  437.400 |
| ZonedDateTime::forEpochSeconds(Extended cached)  |   74.300 |
+--------------------------------------------------+----------+
Number of iterations per run: 10000
```

## ESP8266

* AceTime 0.7
* IDE: Arduino 1.8.9
* ESP8266 Boards 2.5.2

Memory:

```
sizeof(LocalDate): 3
sizeof(LocalTime): 3
sizeof(LocalDateTime): 6
sizeof(TimeOffset): 2
sizeof(OffsetDateTime): 8
sizeof(BasicZoneProcessor): 136
sizeof(ExtendedZoneProcessor): 500
sizeof(BasicZoneRegistrar): 12
sizeof(ExtendedZoneRegistrar): 12
sizeof(BasicZoneManager<1>): 156
sizeof(ExtendedZoneManager<1>): 520
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
sizeof(basic::Transition): 24
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
| LocalDate::forEpochDays()                        |    7.800 |
| LocalDate::toEpochDays()                         |    4.000 |
| LocalDate::dayOfWeek()                           |    4.100 |
| OffsetDateTime::forEpochSeconds()                |   12.100 |
| OffsetDateTime::toEpochSeconds()                 |    7.100 |
| ZonedDateTime::toEpochSeconds()                  |    7.000 |
| ZonedDateTime::toEpochDays()                     |    6.100 |
| ZonedDateTime::forEpochSeconds(UTC)              |   13.200 |
| ZonedDateTime::forEpochSeconds(Basic nocache)    |   95.000 |
| ZonedDateTime::forEpochSeconds(Basic cached)     |   26.900 |
| ZonedDateTime::forEpochSeconds(Extended nocache) |  174.800 |
| ZonedDateTime::forEpochSeconds(Extended cached)  |   27.100 |
+--------------------------------------------------+----------+
Number of iterations per run: 10000
```

## ESP32

* AceTime 0.7
* IDE: Arduino 1.8.9
* ESP32 Boards 1.0.2

Memory:

```
sizeof(LocalDate): 3
sizeof(LocalTime): 3
sizeof(LocalDateTime): 6
sizeof(TimeOffset): 2
sizeof(OffsetDateTime): 8
sizeof(BasicZoneProcessor): 136
sizeof(ExtendedZoneProcessor): 500
sizeof(BasicZoneRegistrar): 12
sizeof(ExtendedZoneRegistrar): 12
sizeof(BasicZoneManager<1>): 156
sizeof(ExtendedZoneManager<1>): 520
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
sizeof(basic::Transition): 24
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
| LocalDate::forEpochDays()                        |    0.460 |
| LocalDate::toEpochDays()                         |    0.390 |
| LocalDate::dayOfWeek()                           |    0.410 |
| OffsetDateTime::forEpochSeconds()                |    0.940 |
| OffsetDateTime::toEpochSeconds()                 |    1.410 |
| ZonedDateTime::toEpochSeconds()                  |    1.410 |
| ZonedDateTime::toEpochDays()                     |    1.070 |
| ZonedDateTime::forEpochSeconds(UTC)              |    1.220 |
| ZonedDateTime::forEpochSeconds(Basic nocache)    |   14.730 |
| ZonedDateTime::forEpochSeconds(Basic cached)     |    2.520 |
| ZonedDateTime::forEpochSeconds(Extended nocache) |   29.710 |
| ZonedDateTime::forEpochSeconds(Extended cached)  |    2.450 |
+--------------------------------------------------+----------+
Number of iterations per run: 100000
```

Note: Once the benchmark of the function under test becomes smaller than the
duration of an empty loop, the numbers become suspect.

## Teensy 3.2

* AceTime 0.7
* IDE: Arduino 1.8.9
* Teensyduino 1.46
* Compiler Optimization: "Faster"

Memory:

```
sizeof(LocalDate): 3
sizeof(LocalTime): 3
sizeof(LocalDateTime): 6
sizeof(TimeOffset): 2
sizeof(OffsetDateTime): 8
sizeof(BasicZoneProcessor): 136
sizeof(ExtendedZoneProcessor): 500
sizeof(BasicZoneRegistrar): 12
sizeof(ExtendedZoneRegistrar): 12
sizeof(BasicZoneManager<1>): 156
sizeof(ExtendedZoneManager<1>): 520
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
sizeof(basic::Transition): 24
sizeof(extended::Transition): 48
sizeof(extended::ZoneMatch): 16
```
CPU:
```
+--------------------------------------------------+----------+
| Method                                           |   micros |
|--------------------------------------------------|----------|
| Empty loop                                       |    0.570 |
|--------------------------------------------------|----------|
| LocalDate::forEpochDays()                        |    1.140 |
| LocalDate::toEpochDays()                         |    0.910 |
| LocalDate::dayOfWeek()                           |    1.700 |
| OffsetDateTime::forEpochSeconds()                |    1.880 |
| OffsetDateTime::toEpochSeconds()                 |    0.450 |
| ZonedDateTime::toEpochSeconds()                  |    0.410 |
| ZonedDateTime::toEpochDays()                     |    0.260 |
| ZonedDateTime::forEpochSeconds(UTC)              |    2.270 |
| ZonedDateTime::forEpochSeconds(Basic nocache)    |   28.800 |
| ZonedDateTime::forEpochSeconds(Basic cached)     |    6.060 |
| ZonedDateTime::forEpochSeconds(Extended nocache) |   66.900 |
| ZonedDateTime::forEpochSeconds(Extended cached)  |    6.030 |
+--------------------------------------------------+----------+
Number of iterations per run: 100000
```

Note: Once the benchmark of the function under test becomes smaller than the
duration of an empty loop, the numbers become suspect.
