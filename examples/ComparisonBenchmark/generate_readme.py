#!/usr/bin/python3
#
# Python script that regenerates the README.md from the embedded template. Uses
# ./generate_table.awk to regenerate the ASCII tables from the various *.txt
# files.

from subprocess import check_output

nano_results = check_output(
    "./generate_table.awk < nano.txt", shell=True, text=True)
micro_results = check_output(
    "./generate_table.awk < micro.txt", shell=True, text=True)
stm32_results = check_output(
    "./generate_table.awk < stm32.txt", shell=True, text=True)
esp8266_results = check_output(
    "./generate_table.awk < esp8266.txt", shell=True, text=True)
esp32_results = check_output(
    "./generate_table.awk < esp32.txt", shell=True, text=True)
teensy32_results = check_output(
    "./generate_table.awk < teensy32.txt", shell=True, text=True)

print(f"""\
# Comparison Benchmark

Here are the results from `ComparisonBenchmark.ino` for various boards.

**Version**: AceTime v1.11.3

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
{nano_results}
```

## Sparkfun Pro Micro

* 16 MHz ATmega32U4
* Arduino IDE 1.8.19, Arduino CLI 0.20.2
* SparkFun AVR Boards 1.1.13

```
{micro_results}
```

## STM32 Blue Pill

* STM32F103C8, 72 MHz ARM Cortex-M3
* Arduino IDE 1.8.19, Arduino CLI 0.20.2
* STM32duino 2.2.0

```
{stm32_results}
```

## ESP8266

* NodeMCU 1.0 clone, 80MHz ESP8266
* Arduino IDE 1.8.19, Arduino CLI 0.20.2
* ESP8266 Boards 3.0.2

```
{esp8266_results}
```

## ESP32

* ESP32-01 Dev Board, 240 MHz Tensilica LX6
* Arduino IDE 1.8.19, Arduino CLI 0.20.2
* ESP32 Boards 2.0.2

```
{esp32_results}
```

Note: Once the benchmark of the function under test becomes smaller than the
duration of an empty loop, the numbers become unreliable.

## Teensy 3.2

* 96 MHz ARM Cortex-M4
* Arduino IDE 1.8.19, Arduino CLI 0.20.2
* Teensyduino 1.56
* Compiler options: "Faster"

```
{teensy32_results}
```
""")
