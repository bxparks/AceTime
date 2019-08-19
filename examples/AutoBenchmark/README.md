# Auto Benchmark

Here are the results from `AutoBenchmark.ino` for various boards.
These results show that integer division and modulus operations are incredibly
slow on 8-bit AVR processors.

## Dependencies

This program depends on the following libraries:

* [AceTime](https://github.com/bxparks/AceTime)
* [AceRoutine](https://github.com/bxparks/AceRoutine)

## Arduino Nano

* AceTime 0.8
* IDE: Arduino 1.8.9
* AVR Boards 1.6.23

Memory:

```
sizeof(LocalDate): 3
sizeof(LocalTime): 3
sizeof(LocalDateTime): 6
sizeof(TimeOffset): 2
sizeof(OffsetDateTime): 8
sizeof(BasicZoneProcessor): 113
sizeof(ExtendedZoneProcessor): 453
sizeof(BasicZoneRegistrar): 5
sizeof(ExtendedZoneRegistrar): 5
sizeof(BasicZoneManager<1>): 121
sizeof(ExtendedZoneManager<1>): 461
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
sizeof(basic::Transition): 21
sizeof(extended::Transition): 46
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
| ZonedDateTime::forEpochSeconds(Basic nocache)    | 1186.800 |
| ZonedDateTime::forEpochSeconds(Basic cached)     |  617.200 |
| ZonedDateTime::forEpochSeconds(Extended nocache) | 2056.000 |
| ZonedDateTime::forEpochSeconds(Extended cached)  |  617.600 |
+--------------------------------------------------+----------+
Number of iterations per run: 2500
```

## SAMD21 M0 Mini (Arduino MKR Zero Compatible)

* AceTime 0.8
* IDE: Arduino 1.8.9
* SparkFun SAMD Boards 1.6.2

Memory:
```
sizeof(LocalDate): 3
sizeof(LocalTime): 3
sizeof(LocalDateTime): 6
sizeof(TimeOffset): 2
sizeof(OffsetDateTime): 8
sizeof(BasicZoneProcessor): 156
sizeof(ExtendedZoneProcessor): 532
sizeof(BasicZoneRegistrar): 12
sizeof(ExtendedZoneRegistrar): 12
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
sizeof(basic::ZoneContext): 8
sizeof(basic::ZoneEra): 16
sizeof(basic::ZoneInfo): 20
sizeof(basic::ZoneRule): 9
sizeof(basic::ZonePolicy): 12
sizeof(basic::Transition): 28
sizeof(extended::Transition): 52
sizeof(extended::ZoneMatch): 16
```

CPU:
```
+--------------------------------------------------+----------+
| Method                                           |   micros |
|--------------------------------------------------|----------|
| Empty loop                                       |    1.400 |
|--------------------------------------------------|----------|
| LocalDate::forEpochDays()                        |   23.300 |
| LocalDate::toEpochDays()                         |    9.600 |
| LocalDate::dayOfWeek()                           |   12.700 |
| OffsetDateTime::forEpochSeconds()                |   34.900 |
| OffsetDateTime::toEpochSeconds()                 |   17.000 |
| ZonedDateTime::toEpochSeconds()                  |   18.200 |
| ZonedDateTime::toEpochDays()                     |   16.100 |
| ZonedDateTime::forEpochSeconds(UTC)              |   41.100 |
| ZonedDateTime::forEpochSeconds(Basic nocache)    |  239.800 |
| ZonedDateTime::forEpochSeconds(Basic cached)     |   73.900 |
| ZonedDateTime::forEpochSeconds(Extended nocache) |  435.400 |
| ZonedDateTime::forEpochSeconds(Extended cached)  |   74.100 |
+--------------------------------------------------+----------+
Number of iterations per run: 10000
```

## ESP8266

* AceTime 0.8
* IDE: Arduino 1.8.9
* ESP8266 Boards 2.5.2

Memory:

```
sizeof(LocalDate): 3
sizeof(LocalTime): 3
sizeof(LocalDateTime): 6
sizeof(TimeOffset): 2
sizeof(OffsetDateTime): 8
sizeof(BasicZoneProcessor): 156
sizeof(ExtendedZoneProcessor): 532
sizeof(BasicZoneRegistrar): 12
sizeof(ExtendedZoneRegistrar): 12
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
sizeof(basic::ZoneContext): 8
sizeof(basic::ZoneEra): 16
sizeof(basic::ZoneInfo): 20
sizeof(basic::ZoneRule): 9
sizeof(basic::ZonePolicy): 12
sizeof(basic::Transition): 28
sizeof(extended::Transition): 52
sizeof(extended::ZoneMatch): 16
```

CPU:

```
+--------------------------------------------------+----------+
| Method                                           |   micros |
|--------------------------------------------------|----------|
| Empty loop                                       |    5.100 |
|--------------------------------------------------|----------|
| LocalDate::forEpochDays()                        |    7.700 |
| LocalDate::toEpochDays()                         |    4.000 |
| LocalDate::dayOfWeek()                           |    4.200 |
| OffsetDateTime::forEpochSeconds()                |   12.100 |
| OffsetDateTime::toEpochSeconds()                 |    7.200 |
| ZonedDateTime::toEpochSeconds()                  |    7.100 |
| ZonedDateTime::toEpochDays()                     |    6.100 |
| ZonedDateTime::forEpochSeconds(UTC)              |   13.200 |
| ZonedDateTime::forEpochSeconds(Basic nocache)    |   95.000 |
| ZonedDateTime::forEpochSeconds(Basic cached)     |   27.000 |
| ZonedDateTime::forEpochSeconds(Extended nocache) |  177.300 |
| ZonedDateTime::forEpochSeconds(Extended cached)  |   27.000 |
+--------------------------------------------------+----------+
Number of iterations per run: 10000
```

## ESP32

* AceTime 0.8
* IDE: Arduino 1.8.9
* ESP32 Boards 1.0.2

Memory:

```
sizeof(LocalDate): 3
sizeof(LocalTime): 3
sizeof(LocalDateTime): 6
sizeof(TimeOffset): 2
sizeof(OffsetDateTime): 8
sizeof(BasicZoneProcessor): 156
sizeof(ExtendedZoneProcessor): 532
sizeof(BasicZoneRegistrar): 12
sizeof(ExtendedZoneRegistrar): 12
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
sizeof(basic::ZoneContext): 8
sizeof(basic::ZoneEra): 16
sizeof(basic::ZoneInfo): 20
sizeof(basic::ZoneRule): 9
sizeof(basic::ZonePolicy): 12
sizeof(basic::Transition): 28
sizeof(extended::Transition): 52
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
| LocalDate::toEpochDays()                         |    0.380 |
| LocalDate::dayOfWeek()                           |    0.420 |
| OffsetDateTime::forEpochSeconds()                |    0.930 |
| OffsetDateTime::toEpochSeconds()                 |    1.430 |
| ZonedDateTime::toEpochSeconds()                  |    1.410 |
| ZonedDateTime::toEpochDays()                     |    1.070 |
| ZonedDateTime::forEpochSeconds(UTC)              |    1.220 |
| ZonedDateTime::forEpochSeconds(Basic nocache)    |   15.210 |
| ZonedDateTime::forEpochSeconds(Basic cached)     |    2.520 |
| ZonedDateTime::forEpochSeconds(Extended nocache) |   29.850 |
| ZonedDateTime::forEpochSeconds(Extended cached)  |    2.470 |
+--------------------------------------------------+----------+
Number of iterations per run: 100000
```

Note: Once the benchmark of the function under test becomes smaller than the
duration of an empty loop, the numbers become suspect.

## Teensy 3.2

* AceTime 0.8
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
sizeof(BasicZoneProcessor): 156
sizeof(ExtendedZoneProcessor): 532
sizeof(BasicZoneRegistrar): 12
sizeof(ExtendedZoneRegistrar): 12
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
sizeof(basic::ZoneContext): 8
sizeof(basic::ZoneEra): 16
sizeof(basic::ZoneInfo): 20
sizeof(basic::ZoneRule): 9
sizeof(basic::ZonePolicy): 12
sizeof(basic::Transition): 28
sizeof(extended::Transition): 52
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
| LocalDate::toEpochDays()                         |    0.970 |
| LocalDate::dayOfWeek()                           |    1.580 |
| OffsetDateTime::forEpochSeconds()                |    1.880 |
| OffsetDateTime::toEpochSeconds()                 |    0.590 |
| ZonedDateTime::toEpochSeconds()                  |    0.540 |
| ZonedDateTime::toEpochDays()                     |    0.190 |
| ZonedDateTime::forEpochSeconds(UTC)              |    2.270 |
| ZonedDateTime::forEpochSeconds(Basic nocache)    |   30.540 |
| ZonedDateTime::forEpochSeconds(Basic cached)     |    6.060 |
| ZonedDateTime::forEpochSeconds(Extended nocache) |   68.070 |
| ZonedDateTime::forEpochSeconds(Extended cached)  |    5.980 |
+--------------------------------------------------+----------+
Number of iterations per run: 100000
```

Note: Once the benchmark of the function under test becomes smaller than the
duration of an empty loop, the numbers become suspect.
