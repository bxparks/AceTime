# EpochBenchmark

Here are the results from `EpochBenchmark.ino` for various boards.

**Version**: AceTime v2.0

**NOTE**: This file was auto-generated using `make README.md`. DO NOT EDIT.

## Dependencies

This program depends on the following libraries:

* [AceTime](https://github.com/bxparks/AceTime)
* [AceSorting](https://github.com/bxparks/AceSorting)
* [AceCommon](https://github.com/bxparks/AceCommon)

## How to Generate

This requires the [AUniter](https://github.com/bxparks/AUniter) script
to execute the Arduino IDE programmatically.

The `Makefile` has rules to generate the `*.txt` results file for several
microcontrollers that I usually support, but the `$ make benchmarks` command
does not work very well because the USB port of the microcontroller is a
dynamically changing parameter. I created a semi-automated way of collect the
`*.txt` files:

1. Connect the microcontroller to the serial port. I usually do this through a
USB hub with individually controlled switch.
2. Type `$ auniter ports` to determine its `/dev/ttyXXX` port number (e.g.
`/dev/ttyUSB0` or `/dev/ttyACM0`).
3. If the port is `USB0` or `ACM0`, type `$ make nano.txt`, etc.
4. Switch off the old microontroller.
5. Go to Step 1 and repeat for each microcontroller.

The `generate_table.awk` program reads one of `*.txt` files and prints out an
ASCII table that can be directly embedded into this README.md file. For example
the following command produces the table in the Nano section below:

```
$ ./generate_table.awk < nano.txt
```

Fortunately, we no longer need to run `generate_table.awk` for each `*.txt`
file. The process has been automated using the `generate_readme.py` script which
will be invoked by the following command:
```
$ make README.md
```

The CPU times below are given in microseconds per iteration.

## CPU Time Changes

## Arduino Nano

* 16MHz ATmega328P
* Arduino IDE 1.8.19, Arduino CLI 0.20.2
* Arduino AVR Boards 1.8.4

```
+----------------------------------------+-------------+----------+
| Algorithm                              | micros/iter |     diff |
|----------------------------------------+-------------+----------|
| EmptyLoop                              |       4.425 |    0.000 |
|----------------------------------------+-------------+----------|
| EpochConverterJulian                   |     217.808 |  213.383 |
| EpochConverterHinnant                  |     241.938 |  237.513 |
+----------------------------------------+-------------+----------+

```

## Sparkfun Pro Micro

* 16 MHz ATmega32U4
* Arduino IDE 1.8.19, Arduino CLI 0.20.2
* SparkFun AVR Boards 1.1.13

```
+----------------------------------------+-------------+----------+
| Algorithm                              | micros/iter |     diff |
|----------------------------------------+-------------+----------|
| EmptyLoop                              |       4.425 |    0.000 |
|----------------------------------------+-------------+----------|
| EpochConverterJulian                   |     218.861 |  214.436 |
| EpochConverterHinnant                  |     243.203 |  238.778 |
+----------------------------------------+-------------+----------+

```

## STM32 Blue Pill

* STM32F103C8, 72 MHz ARM Cortex-M3
* Arduino IDE 1.8.19, Arduino CLI 0.20.2
* STM32duino 2.2.0

```
+----------------------------------------+-------------+----------+
| Algorithm                              | micros/iter |     diff |
|----------------------------------------+-------------+----------|
| EmptyLoop                              |       0.408 |    0.000 |
|----------------------------------------+-------------+----------|
| EpochConverterJulian                   |       1.406 |    0.998 |
| EpochConverterHinnant                  |       1.782 |    1.374 |
+----------------------------------------+-------------+----------+

```

## ESP8266

* NodeMCU 1.0 clone, 80MHz ESP8266
* Arduino IDE 1.8.19, Arduino CLI 0.20.2
* ESP8266 Boards 3.0.2

```
+----------------------------------------+-------------+----------+
| Algorithm                              | micros/iter |     diff |
|----------------------------------------+-------------+----------|
| EmptyLoop                              |       0.408 |    0.000 |
|----------------------------------------+-------------+----------|
| EpochConverterJulian                   |       8.095 |    7.687 |
| EpochConverterHinnant                  |       7.097 |    6.689 |
+----------------------------------------+-------------+----------+

```

## ESP32

* ESP32-01 Dev Board, 240 MHz Tensilica LX6
* Arduino IDE 1.8.19, Arduino CLI 0.20.2
* ESP32 Boards 2.0.2

```
+----------------------------------------+-------------+----------+
| Algorithm                              | micros/iter |     diff |
|----------------------------------------+-------------+----------|
| EmptyLoop                              |       0.161 |    0.000 |
|----------------------------------------+-------------+----------|
| EpochConverterJulian                   |       0.483 |    0.322 |
| EpochConverterHinnant                  |       0.579 |    0.418 |
+----------------------------------------+-------------+----------+

```

Note: Once the benchmark of the function under test becomes smaller than the
duration of an empty loop, the numbers become unreliable.

## Teensy 3.2

* 96 MHz ARM Cortex-M4
* Arduino IDE 1.8.19, Arduino CLI 0.20.2
* Teensyduino 1.56
* Compiler options: "Faster"

```
+----------------------------------------+-------------+----------+
| Algorithm                              | micros/iter |     diff |
|----------------------------------------+-------------+----------|
| EmptyLoop                              |       0.214 |    0.000 |
|----------------------------------------+-------------+----------|
| EpochConverterJulian                   |       0.805 |    0.591 |
| EpochConverterHinnant                  |       1.997 |    1.783 |
+----------------------------------------+-------------+----------+

```

## EpoxyDuino

* Intel(R) Core(TM) i5-6300U CPU @ 2.40GHz
* Ubuntu 20.04.4 LTS

```
+----------------------------------------+-------------+----------+
| Algorithm                              | micros/iter |     diff |
|----------------------------------------+-------------+----------|
| EmptyLoop                              |       0.010 |    0.000 |
|----------------------------------------+-------------+----------|
| EpochConverterJulian                   |       0.085 |    0.075 |
| EpochConverterHinnant                  |       0.085 |    0.075 |
+----------------------------------------+-------------+----------+

```

