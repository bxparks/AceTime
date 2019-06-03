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

IDE: Arduino 1.8.8 (ESP Core 2.4.2)

Memory:
```
sizeof(LocalDate): 3
sizeof(LocalTime): 3
sizeof(TimeOffset): 1
sizeof(ZoneEra): 20
sizeof(ZoneInfo): 12
sizeof(ZoneRule): 9
sizeof(ZonePolicy): 8
sizeof(internal::Transition): 20
sizeof(ZoneSpecifier): 108
sizeof(TimeZone): 16
sizeof(OffsetDateTime): 7
sizeof(ZonedDateTime): 24
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
---------------------------------------------+----------+
Method                                       |   micros |
---------------------------------------------|----------|
Empty loop                                   |    4.720 |
---------------------------------------------|----------|
LocalDate::forEpochDays()                    |    8.160 |
LocalDate::toEpochDays()                     |    8.560 |
LocalDate::dayOfWeek()                       |    8.080 |
ZonedDateTime::forEpochSeconds(UTC)          |   13.880 |
ZonedDateTime::forEpochSeconds(Los_Angeles)  |   61.160 |
ZonedDateTime::forEpochSeconds(Cached)       |   25.400 |
ZonedDateTime::toEpochDays()                 |   10.520 |
ZonedDateTime::toEpochSeconds()              |   10.680 |
---------------------------------------------+----------+
Number of iterations per run: 25000
```

## ESP32

IDE: Arduino 1.8.8 (ESP32 Core 1.0.0)

Memory:

```
sizeof(LocalDate): 3
sizeof(LocalTime): 3
sizeof(TimeOffset): 1
sizeof(ZoneEra): 20
sizeof(ZoneInfo): 12
sizeof(ZoneRule): 9
sizeof(ZonePolicy): 8
sizeof(internal::Transition): 20
sizeof(ZoneSpecifier): 108
sizeof(TimeZone): 16
sizeof(OffsetDateTime): 7
sizeof(ZonedDateTime): 24
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
---------------------------------------------+----------+
Method                                       |   micros |
---------------------------------------------|----------|
Empty loop                                   |    1.315 |
---------------------------------------------|----------|
LocalDate::forEpochDays()                    |    0.555 |
LocalDate::toEpochDays()                     |    1.675 |
LocalDate::dayOfWeek()                       |    1.555 |
ZonedDateTime::forEpochSeconds(UTC)          |    1.280 |
ZonedDateTime::forEpochSeconds(Los_Angeles)  |   10.935 |
ZonedDateTime::forEpochSeconds(Cached)       |    2.085 |
ZonedDateTime::toEpochDays()                 |    2.125 |
ZonedDateTime::toEpochSeconds()              |    2.150 |
---------------------------------------------+----------+
Number of iterations per run: 200000
```

Note: Once the benchmark of the function under test becomes smaller than the
duration of an empty loop, the numbers become suspect.

## Teensy 3.2

IDE: Arduino 1.8.5 (Teensyduino 1.42)

Memory:

```
sizeof(LocalDate): 3
sizeof(TimeOffset): 1
sizeof(OffsetDateTime): 8
sizeof(DateTime): 10
sizeof(TimeZone): 2
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
----------------------------+---------+
Method                      |  micros |
----------------------------|---------|
Empty loop                  |   0.175 |
----------------------------|---------|
DateTime::forEpochSeconds() |   2.500 |
DateTime::toEpochDays()     |   0.585 |
DateTime::toEpochSeconds()  |   0.750 |
LocalDate::forEpochDays()   |   0.525 |
LocalDate::toEpochDays()    |   1.140 |
LocalDate::dayOfWeek()      |   0.595 |
----------------------------+---------+
Number of iterations per run: 200000
```

Note: Once the benchmark of the function under test becomes smaller than the
duration of an empty loop, the numbers become suspect.
