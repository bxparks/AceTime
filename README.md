# Compare Time Libraries

This program compares the runtime performance of
AceTime library versus the
[Arduino Time library](https://github.com/PaulStoffregen/Time).

## Arduino Nano

```
----------------------------+---------+
Method                      |  micros |
----------------------------|---------|
Empty loop                  |   2.500 |
AceTime library             | 364.000 |
Arduino Time library        | 879.000 |
----------------------------+---------+
Number of iterations per run: 2000
Delta seconds: 236682
```

## ESP8266

```
----------------------------+---------+
Method                      |  micros |
----------------------------|---------|
Empty loop                  |   0.400 |
AceTime library             |  20.900 |
Arduino Time library        |  68.500 |
----------------------------+---------+
Number of iterations per run: 10000
Delta seconds: 47336
```

## ESP32

```
----------------------------+---------+
Method                      |  micros |
----------------------------|---------|
Empty loop                  |   0.130 |
AceTime library             |   0.570 |
Arduino Time library        |   9.330 |
----------------------------+---------+
Number of iterations per run: 100000
Delta seconds: 4733
```

## Teensy 3.2

```
----------------------------+---------+
Method                      |  micros |
----------------------------|---------|
Empty loop                  |   0.110 |
AceTime library             |   1.980 |
Arduino Time library        |  22.570 |
----------------------------+---------+
Number of iterations per run: 100000
Delta seconds: 4733

```
