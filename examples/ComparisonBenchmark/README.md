# Comparison Benchmark

Here are the results from `ComparisonBenchmark.ino` for various boards.

**Version**: AceTime v2.2.3

**NOTE**: This file was auto-generated using `make README.md`. DO NOT EDIT.

## Dependencies

This program depends on the following libraries:

* [AceTime](https://github.com/bxparks/AceTime)
* [AceCommon](https://github.com/bxparks/AceCommon)
* [Arduino Time Library](https://github.com/PaulStoffregen/Time)

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

The CPU times below are given in microseconds.

## CPU Time Changes

**v1.11**
* Generate `*.txt` files from the `*.ino` program.
* Generate the `README.md` file using `generate_readme.py` and
  `generate_table.awk`.

**v2.2.3**
* Add SAMD21, SAMD51.
* Remove Teensy 3.2.

## Arduino Nano

* 16MHz ATmega328P
* Arduino IDE 1.8.19, Arduino CLI 0.31.0
* Arduino AVR Boards 1.8.6

```
Sizes of Objects:

CPU:
+----------------------------------------+----------+
| Method                                 |   micros |
|----------------------------------------+----------|
| EmptyLoop                              |    5.000 |
|----------------------------------------+----------|
| LocalDateTime::forEpochSeconds()       |  338.500 |
| breakTime()                            |  595.000 |
|----------------------------------------+----------|
| LocalDateTime::toEpochSeconds()        |   61.500 |
| makeTime()                             |  344.000 |
+----------------------------------------+----------+
Iterations_per_run: 2000
Delta_seconds: 2000

```

## Sparkfun Pro Micro

* 16 MHz ATmega32U4
* Arduino IDE 1.8.19, Arduino CLI 0.31.0
* SparkFun AVR Boards 1.1.13

```
Sizes of Objects:

CPU:
+----------------------------------------+----------+
| Method                                 |   micros |
|----------------------------------------+----------|
| EmptyLoop                              |    4.500 |
|----------------------------------------+----------|
| LocalDateTime::forEpochSeconds()       |  341.000 |
| breakTime()                            |  597.000 |
|----------------------------------------+----------|
| LocalDateTime::toEpochSeconds()        |   62.000 |
| makeTime()                             |  346.500 |
+----------------------------------------+----------+
Iterations_per_run: 2000
Delta_seconds: 2000

```

## Seeed Studio XIAO SAMD21

* SAMD21, 48 MHz ARM Cortex-M0+
* Arduino IDE 1.8.19, Arduino CLI 0.31.0
* Seeeduino SAMD Boards 1.8.4

```
Sizes of Objects:

CPU:
+----------------------------------------+----------+
| Method                                 |   micros |
|----------------------------------------+----------|
| EmptyLoop                              |    1.300 |
|----------------------------------------+----------|
| LocalDateTime::forEpochSeconds()       |   23.100 |
| breakTime()                            |   93.400 |
|----------------------------------------+----------|
| LocalDateTime::toEpochSeconds()        |    6.500 |
| makeTime()                             |   56.000 |
+----------------------------------------+----------+
Iterations_per_run: 10000
Delta_seconds: 10000

```

## STM32 Blue Pill

* STM32F103C8, 72 MHz ARM Cortex-M3
* Arduino IDE 1.8.19, Arduino CLI 0.31.0
* STM32duino 2.5.0

```
Sizes of Objects:

CPU:
+----------------------------------------+----------+
| Method                                 |   micros |
|----------------------------------------+----------|
| EmptyLoop                              |    0.800 |
|----------------------------------------+----------|
| LocalDateTime::forEpochSeconds()       |    3.000 |
| breakTime()                            |   31.200 |
|----------------------------------------+----------|
| LocalDateTime::toEpochSeconds()        |    1.900 |
| makeTime()                             |   22.600 |
+----------------------------------------+----------+
Iterations_per_run: 10000
Delta_seconds: 10000

```

## Adafruit ItsyBitsy M4 SAMD51

* SAMD51, 120 MHz ARM Cortex-M4
* Arduino IDE 1.8.19, Arduino CLI 0.31.0
* Adafruit SAMD Boards 1.7.11

```
Sizes of Objects:

CPU:
+----------------------------------------+----------+
| Method                                 |   micros |
|----------------------------------------+----------|
| EmptyLoop                              |    0.400 |
|----------------------------------------+----------|
| LocalDateTime::forEpochSeconds()       |    1.400 |
| breakTime()                            |    9.000 |
|----------------------------------------+----------|
| LocalDateTime::toEpochSeconds()        |    0.900 |
| makeTime()                             |    7.700 |
+----------------------------------------+----------+
Iterations_per_run: 10000
Delta_seconds: 10000

```

## ESP8266

* NodeMCU 1.0 clone, 80MHz ESP8266
* Arduino IDE 1.8.19, Arduino CLI 0.31.0
* ESP8266 Boards 3.0.2

```
Sizes of Objects:

CPU:
+----------------------------------------+----------+
| Method                                 |   micros |
|----------------------------------------+----------|
| EmptyLoop                              |    0.700 |
|----------------------------------------+----------|
| LocalDateTime::forEpochSeconds()       |   13.300 |
| breakTime()                            |   42.400 |
|----------------------------------------+----------|
| LocalDateTime::toEpochSeconds()        |    4.200 |
| makeTime()                             |   24.800 |
+----------------------------------------+----------+
Iterations_per_run: 10000
Delta_seconds: 10000

```

## ESP32

* ESP32-01 Dev Board, 240 MHz Tensilica LX6
* Arduino IDE 1.8.19, Arduino CLI 0.31.0
* ESP32 Boards 2.0.9

```
Sizes of Objects:

CPU:
+----------------------------------------+----------+
| Method                                 |   micros |
|----------------------------------------+----------|
| EmptyLoop                              |    0.380 |
|----------------------------------------+----------|
| LocalDateTime::forEpochSeconds()       |    0.880 |
| breakTime()                            |    5.430 |
|----------------------------------------+----------|
| LocalDateTime::toEpochSeconds()        |    0.600 |
| makeTime()                             |    4.880 |
+----------------------------------------+----------+
Iterations_per_run: 100000
Delta_seconds: 100000

```

Note: Once the benchmark of the function under test becomes smaller than the
duration of an empty loop, the numbers become unreliable.

