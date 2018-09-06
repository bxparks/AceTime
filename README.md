# Auto Benchmark

Here are the results from `AutoBenchmark.ino` for various boards.
Integer division and modulus operations are incredibly slow
on 8-bit AVR processors.

## Arduino Nano

```
----------------------------+---------+
Method                      |  micros |
----------------------------|---------|
Empty loop                  |   6.400 |
----------------------------|---------|
DateTime(seconds)           | 402.000 |
toDaysSinceEpochMillis()    |  65.400 |
toSecondsSinceEpochMillis() |  70.100 |
----------------------------+---------+
Number of iterations per run: 10000
```

## ESP8266

```
----------------------------+---------+
Method                      |  micros |
----------------------------|---------|
Empty loop                  |   6.000 |
----------------------------|---------|
DateTime(seconds)           |  12.200 |
toDaysSinceEpochMillis()    |   4.460 |
toSecondsSinceEpochMillis() |   4.520 |
----------------------------+---------+
Number of iterations per run: 50000
```

## ESP32

```
----------------------------+---------+
Method                      |  micros |
----------------------------|---------|
Empty loop                  |   1.325 |
----------------------------|---------|
DateTime(seconds)           |   0.750 |
toDaysSinceEpochMillis()    |   0.445 |
toSecondsSinceEpochMillis() |   0.445 |
----------------------------+---------+
Number of iterations per run: 200000
```

## Teensy 3.2

```
----------------------------+---------+
Method                      |  micros |
----------------------------|---------|
Empty loop                  |   0.460 |
----------------------------|---------|
DateTime(seconds)           |   2.755 |
toDaysSinceEpochMillis()    |   1.090 |
toSecondsSinceEpochMillis() |   1.045 |
----------------------------+---------+
Number of iterations per run: 200000
```
