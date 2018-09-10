# Auto Benchmark

Here are the results from `AutoBenchmark.ino` for various boards.
Integer division and modulus operations are incredibly slow
on 8-bit AVR processors.

## Arduino Nano

IDE: Arduino 1.8.6 (AVR Core 1.6.22)

```
----------------------------+---------+
Method                      |  micros |
----------------------------|---------|
Empty loop                  |   2.800 |
----------------------------|---------|
DateTime(seconds)           | 401.800 |
toDaysSinceEpochMillis()    |  63.800 |
toSecondsSinceEpochMillis() |  68.600 |
----------------------------+---------+
Number of iterations per run: 10000
```

## ESP8266

IDE: Arduino 1.8.6 (ESP Core 2.4.2)

```
----------------------------+---------+
Method                      |  micros |
----------------------------|---------|
Empty loop                  |   4.720 |
----------------------------|---------|
DateTime(seconds)           |  12.080 |
toDaysSinceEpochMillis()    |   4.280 |
toSecondsSinceEpochMillis() |   4.540 |
----------------------------+---------+
Number of iterations per run: 50000
```

## ESP32

IDE: Arduino 1.8.6 (ESP32 Core 2018-09-09)

```
----------------------------+---------+
Method                      |  micros |
----------------------------|---------|
Empty loop                  |   1.310 |
----------------------------|---------|
DateTime(seconds)           |   0.705 |
toDaysSinceEpochMillis()    |   0.445 |
toSecondsSinceEpochMillis() |   0.405 |
----------------------------+---------+
Number of iterations per run: 200000
```

## Teensy 3.2

IDE: Arduino 1.8.5 (Teensyduino 1.42)

```
----------------------------+---------+
Method                      |  micros |
----------------------------|---------|
Empty loop                  |   0.175 |
----------------------------|---------|
DateTime(seconds)           |   2.750 |
toDaysSinceEpochMillis()    |   0.710 |
toSecondsSinceEpochMillis() |   0.915 |
----------------------------+---------+
Number of iterations per run: 200000
```
