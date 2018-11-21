# Auto Benchmark

Here are the results from `AutoBenchmark.ino` for various boards.
These results show that integer division and modulus operations are incredibly
slow on 8-bit AVR processors.

## Arduino Nano

IDE: Arduino 1.8.7 (AVR Core 1.6.23)

Memory:

```
sizeof(ZoneEntry): 6
sizeof(ZoneInfo): 5
sizeof(ZoneRule): 11
sizeof(ZonePolicy): 11
sizeof(LocalDate): 3
sizeof(LocalTime): 3
sizeof(ZoneOffset): 1
sizeof(ZoneMatch): 15
sizeof(ZoneManager): 80
sizeof(TimeZone): 87
sizeof(OffsetDateTime): 7
sizeof(DateTime): 94
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
----------------------------+---------+
Method                      |  micros |
----------------------------|---------|
Empty loop                  |   2.800 |
----------------------------|---------|
DateTime::forEpochSeconds() | 370.100 |
DateTime::toEpochDays()     |  64.100 |
Datetime::toEpochSeconds()  |  70.000 |
LocalDate::forEpochDays()   | 215.900 |
LocalDate::toEpochDays()    |  52.000 |
LocalDate::dayOfWeek()      |  41.300 |
----------------------------+---------+
Number of iterations per run: 10000
```

## ESP8266

IDE: Arduino 1.8.7 (ESP Core 2.4.2)

Memory:
```
sizeof(LocalDate): 3
sizeof(ZoneOffset): 1
sizeof(OffsetDateTime): 8
sizeof(DateTime): 10
sizeof(TimeZone): 2
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
----------------------------+---------+
Method                      |  micros |
----------------------------|---------|
Empty loop                  |   4.720 |
----------------------------|---------|
DateTime::forEpochSeconds() |  13.160 |
DateTime::toEpochDays()     |   4.420 |
DateTime::toEpochSeconds()  |   4.620 |
LocalDate::forEpochDays()   |   7.880 |
LocalDate::toEpochDays()    |   3.600 |
LocalDate::dayOfWeek()      |   3.040 |
----------------------------+---------+
Number of iterations per run: 50000
```

## ESP32

IDE: Arduino 1.8.7 (ESP32 Core 1.0.0)

Memory:

```
sizeof(LocalDate): 3
sizeof(ZoneOffset): 1
sizeof(OffsetDateTime): 8
sizeof(DateTime): 10
sizeof(TimeZone): 2
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
----------------------------+---------+
Method                      |  micros |
----------------------------|---------|
Empty loop                  |   1.310 |
----------------------------|---------|
DateTime::forEpochSeconds() |   1.115 |
DateTime::toEpochDays()     |   0.445 |
DateTime::toEpochSeconds()  |   0.410 |
LocalDate::forEpochDays()   |   0.450 |
LocalDate::toEpochDays()    |   0.225 |
LocalDate::dayOfWeek()      |   0.180 |
----------------------------+---------+
Number of iterations per run: 200000
```

Note: Once the benchmark of the function under test becomes smaller than the
duration of an empty loop, the numbers become suspect.

## Teensy 3.2

IDE: Arduino 1.8.5 (Teensyduino 1.42)

Memory:

```
sizeof(LocalDate): 3
sizeof(ZoneOffset): 1
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
