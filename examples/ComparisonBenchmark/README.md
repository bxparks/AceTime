# Comparison Benchmark

Here are the results from `ComparisonBenchmark.ino` for various boards.

**Version**: AceTime v1.11.4

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

## Arduino Nano

* 16MHz ATmega328P
* Arduino IDE 1.8.19, Arduino CLI 0.20.2
* Arduino AVR Boards 1.8.4

```
Sizes of Objects:

CPU:
+----------------------------------------+----------+
| Method                                 |   micros |
|----------------------------------------+----------|
| EmptyLoop                              |    5.000 |
|----------------------------------------+----------|
| LocalDateTime::forEpochSeconds()       |  270.000 |
| breakTime()                            |  594.500 |
|----------------------------------------+----------|
| LocalDateTime::toEpochSeconds()        |   66.500 |
| makeTime()                             |  344.500 |
+----------------------------------------+----------+
Iterations_per_run: 2000
Delta_seconds: 2000

```

## Sparkfun Pro Micro

* 16 MHz ATmega32U4
* Arduino IDE 1.8.19, Arduino CLI 0.20.2
* SparkFun AVR Boards 1.1.13

```
Sizes of Objects:

CPU:
+----------------------------------------+----------+
| Method                                 |   micros |
|----------------------------------------+----------|
| EmptyLoop                              |    5.000 |
|----------------------------------------+----------|
| LocalDateTime::forEpochSeconds()       |  271.000 |
| breakTime()                            |  598.500 |
|----------------------------------------+----------|
| LocalDateTime::toEpochSeconds()        |   66.500 |
| makeTime()                             |  345.500 |
+----------------------------------------+----------+
Iterations_per_run: 2000
Delta_seconds: 2000

```

## STM32 Blue Pill

* STM32F103C8, 72 MHz ARM Cortex-M3
* Arduino IDE 1.8.19, Arduino CLI 0.20.2
* STM32duino 2.2.0

```
Sizes of Objects:

CPU:
+----------------------------------------+----------+
| Method                                 |   micros |
|----------------------------------------+----------|
| EmptyLoop                              |    0.900 |
|----------------------------------------+----------|
| LocalDateTime::forEpochSeconds()       |    2.000 |
| breakTime()                            |   25.600 |
|----------------------------------------+----------|
| LocalDateTime::toEpochSeconds()        |    1.900 |
| makeTime()                             |   21.500 |
+----------------------------------------+----------+
Iterations_per_run: 10000
Delta_seconds: 10000

```

## ESP8266

* NodeMCU 1.0 clone, 80MHz ESP8266
* Arduino IDE 1.8.19, Arduino CLI 0.20.2
* ESP8266 Boards 3.0.2

```
Sizes of Objects:

CPU:
+----------------------------------------+----------+
| Method                                 |   micros |
|----------------------------------------+----------|
| EmptyLoop                              |    0.800 |
|----------------------------------------+----------|
| LocalDateTime::forEpochSeconds()       |   13.100 |
| breakTime()                            |   42.600 |
|----------------------------------------+----------|
| LocalDateTime::toEpochSeconds()        |    4.500 |
| makeTime()                             |   24.800 |
+----------------------------------------+----------+
Iterations_per_run: 10000
Delta_seconds: 10000

```

## ESP32

* ESP32-01 Dev Board, 240 MHz Tensilica LX6
* Arduino IDE 1.8.19, Arduino CLI 0.20.2
* ESP32 Boards 2.0.2

```
Sizes of Objects:

CPU:
+----------------------------------------+----------+
| Method                                 |   micros |
|----------------------------------------+----------|
| EmptyLoop                              |    0.390 |
|----------------------------------------+----------|
| LocalDateTime::forEpochSeconds()       |    0.610 |
| breakTime()                            |    5.410 |
|----------------------------------------+----------|
| LocalDateTime::toEpochSeconds()        |    0.550 |
| makeTime()                             |    4.870 |
+----------------------------------------+----------+
Iterations_per_run: 100000
Delta_seconds: 100000

```

Note: Once the benchmark of the function under test becomes smaller than the
duration of an empty loop, the numbers become unreliable.

## Teensy 3.2

* 96 MHz ARM Cortex-M4
* Arduino IDE 1.8.19, Arduino CLI 0.20.2
* Teensyduino 1.56
* Compiler options: "Faster"

```
Sizes of Objects:

CPU:
+----------------------------------------+----------+
| Method                                 |   micros |
|----------------------------------------+----------|
| EmptyLoop                              |    0.510 |
|----------------------------------------+----------|
| LocalDateTime::forEpochSeconds()       |    1.700 |
| breakTime()                            |   12.610 |
|----------------------------------------+----------|
| LocalDateTime::toEpochSeconds()        |    0.780 |
| makeTime()                             |    9.730 |
+----------------------------------------+----------+
Iterations_per_run: 100000
Delta_seconds: 100000

```

