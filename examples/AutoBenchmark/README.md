# Auto Benchmark

Here are the results from `AutoBenchmark.ino` for various boards.
These results show that integer division and modulus operations are incredibly
slow on 8-bit AVR processors.

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
sizeof(basic::Transition): 17
sizeof(extended::Transition): 36
sizeof(extended::ZoneMatch): 12
sizeof(ZoneSpecifier): 3
sizeof(ManualZoneSpecifier): 10
sizeof(BasicZoneSpecifier): 95
sizeof(ExtendedZoneSpecifier): 366
sizeof(TimeZone): 3
sizeof(ZonedDateTime): 10
sizeof(TimePeriod): 4
sizeof(SystemTimeKeeper): 17
sizeof(DS3231TimeKeeper): 3
sizeof(SystemTimeSyncLoop): 14
sizeof(SystemTimeHeartbeatLoop): 8
sizeof(SystemTimeSyncCoroutine): 31
sizeof(SystemTimeHeartbeatCoroutine): 18
```

CPU:

```
-------------------------------------------------+----------+
Method                                           |   micros |
-------------------------------------------------|----------|
Empty loop                                       |    3.600 |
-------------------------------------------------|----------|
LocalDate::forEpochDays()                        |  216.000 |
LocalDate::toEpochDays()                         |   56.000 |
LocalDate::dayOfWeek()                           |   48.800 |
OffsetDateTime::forEpochSeconds()                |  323.600 |
OffsetDateTime::toEpochSeconds()                 |   75.600 |
ZonedDateTime::forEpochSeconds(UTC)              |  336.000 |
ZonedDateTime::forEpochSeconds(BasicZoneSpec)    | 1041.200 |
ZonedDateTime::forEpochSeconds(BasicZone cached) |  613.600 |
ZonedDateTime::toEpochSeconds()                  |   75.600 |
ZonedDateTime::toEpochDays()                     |   68.400 |
-------------------------------------------------+----------+
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
sizeof(basic::Transition): 24
sizeof(extended::Transition): 44
sizeof(extended::ZoneMatch): 16
sizeof(ZoneSpecifier): 8
sizeof(ManualZoneSpecifier): 20
sizeof(BasicZoneSpecifier): 140
sizeof(ExtendedZoneSpecifier): 472
sizeof(TimeZone): 8
sizeof(ZonedDateTime): 16
sizeof(TimePeriod): 4
sizeof(SystemTimeKeeper): 24
sizeof(DS3231TimeKeeper): 8
sizeof(NtpTimeProvider): 88
sizeof(SystemTimeSyncLoop): 20
sizeof(SystemTimeHeartbeatLoop): 12
sizeof(SystemTimeSyncCoroutine): 52
sizeof(SystemTimeHeartbeatCoroutine): 36
```

CPU:

```
+--------------------------------------------------+----------+
| Method                                           |   micros |
|--------------------------------------------------|----------|
| Empty loop                                       |    5.080 |
|--------------------------------------------------|----------|
| LocalDate::forEpochDays()                        |    7.800 |
| LocalDate::toEpochDays()                         |    3.680 |
| LocalDate::dayOfWeek()                           |    3.880 |
| OffsetDateTime::forEpochSeconds()                |   13.480 |
| OffsetDateTime::toEpochSeconds()                 |    6.240 |
| ZonedDateTime::forEpochSeconds(UTC)              |   14.560 |
| ZonedDateTime::forEpochSeconds(BasicZoneSpec)    |   64.960 |
| ZonedDateTime::forEpochSeconds(BasicZone cached) |   26.760 |
| ZonedDateTime::toEpochSeconds()                  |    6.280 |
| ZonedDateTime::toEpochDays()                     |    5.480 |
+--------------------------------------------------+----------+
Number of iterations per run: 25000

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
sizeof(basic::Transition): 24
sizeof(extended::Transition): 44
sizeof(extended::ZoneMatch): 16
sizeof(ZoneSpecifier): 8
sizeof(ManualZoneSpecifier): 20
sizeof(BasicZoneSpecifier): 140
sizeof(ExtendedZoneSpecifier): 472
sizeof(TimeZone): 8
sizeof(ZonedDateTime): 16
sizeof(TimePeriod): 4
sizeof(SystemTimeKeeper): 24
sizeof(DS3231TimeKeeper): 8
sizeof(NtpTimeProvider): 116
sizeof(SystemTimeSyncLoop): 20
sizeof(SystemTimeHeartbeatLoop): 12
sizeof(SystemTimeSyncCoroutine): 52
sizeof(SystemTimeHeartbeatCoroutine): 36
```

CPU:

```
+--------------------------------------------------+----------+
| Method                                           |   micros |
|--------------------------------------------------|----------|
| Empty loop                                       |    1.400 |
|--------------------------------------------------|----------|
| LocalDate::forEpochDays()                        |    0.440 |
| LocalDate::toEpochDays()                         |    0.332 |
| LocalDate::dayOfWeek()                           |    0.432 |
| OffsetDateTime::forEpochSeconds()                |    1.464 |
| OffsetDateTime::toEpochSeconds()                 |    1.256 |
| ZonedDateTime::forEpochSeconds(UTC)              |    1.916 |
| ZonedDateTime::forEpochSeconds(BasicZoneSpec)    |   13.176 |
| ZonedDateTime::forEpochSeconds(BasicZone cached) |    2.988 |
| ZonedDateTime::toEpochSeconds()                  |    1.292 |
| ZonedDateTime::toEpochDays()                     |    0.992 |
+--------------------------------------------------+----------+
Number of iterations per run: 250000
```

Note: Once the benchmark of the function under test becomes smaller than the
duration of an empty loop, the numbers become suspect.

## Teensy 3.2

IDE: Arduino 1.8.9 (Teensyduino 1.46)

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
sizeof(basic::Transition): 24
sizeof(extended::Transition): 44
sizeof(extended::ZoneMatch): 16
sizeof(ZoneSpecifier): 8
sizeof(ManualZoneSpecifier): 20
sizeof(BasicZoneSpecifier): 140
sizeof(ExtendedZoneSpecifier): 472
sizeof(TimeZone): 8
sizeof(ZonedDateTime): 16
sizeof(TimePeriod): 4
sizeof(SystemTimeKeeper): 24
sizeof(DS3231TimeKeeper): 8
sizeof(SystemTimeSyncLoop): 20
sizeof(SystemTimeHeartbeatLoop): 12
sizeof(SystemTimeSyncCoroutine): 52
sizeof(SystemTimeHeartbeatCoroutine): 36
```

CPU:

```
+--------------------------------------------------+----------+
| Method                                           |   micros |
|--------------------------------------------------|----------|
| Empty loop                                       |    0.576 |
|--------------------------------------------------|----------|
| LocalDate::forEpochDays()                        |    0.928 |
| LocalDate::toEpochDays()                         |    0.960 |
| LocalDate::dayOfWeek()                           |    2.000 |
| OffsetDateTime::forEpochSeconds()                |    2.136 |
| OffsetDateTime::toEpochSeconds()                 |    0.256 |
| ZonedDateTime::forEpochSeconds(UTC)              |    2.056 |
| ZonedDateTime::forEpochSeconds(BasicZoneSpec)    |   26.292 |
| ZonedDateTime::forEpochSeconds(BasicZone cached) |    5.524 |
| ZonedDateTime::toEpochSeconds()                  |    0.584 |
| ZonedDateTime::toEpochDays()                     |    0.668 |
+--------------------------------------------------+----------+
Number of iterations per run: 250000
```

Note: Once the benchmark of the function under test becomes smaller than the
duration of an empty loop, the numbers become suspect.
