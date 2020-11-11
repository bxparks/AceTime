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
samd_results = check_output(
    "./generate_table.awk < samd.txt", shell=True, text=True)
esp8266_results = check_output(
    "./generate_table.awk < esp8266.txt", shell=True, text=True)
esp32_results = check_output(
    "./generate_table.awk < esp32.txt", shell=True, text=True)
teensy_results = check_output(
    "./generate_table.awk < teensy.txt", shell=True, text=True)

print(f"""\
# MemoryBenchmark

The `MemoryBenchmark.ino` was compiled with each `FEATURE_*` and the flash
memory and static RAM sizes were recorded. The `FEATURE_BASELINE` selection is
the baseline, and its memory usage  numbers are subtracted from the subsequent
`FEATURE_*` memory usage.

The `collect.sh` script automates the collection of this data by using
[AUniter](https://github.com/bxparks/AUniter) to invoke the Arduino IDE
programmatically. It produces a `*.txt` file with the flash and ram usage
information (e.g. `nano.txt`). The `generate_table.awk` script takes the `*.txt`
file and produces the ASCII tables below.

**NOTE**: This file was auto-generated using `make README.md`. DO NOT EDIT.

## Arduino Nano

* AceTime 1.2
* Arduino IDE 1.8.13
* Arduino AVR Boards 1.8.3

```
{nano_results}
```

## Sparkfun Pro Micro

* AceTime 1.2
* Arduino IDE 1.8.13
* SparkFun AVR Boards 1.1.13

```
{micro_results}
```

## SAMD21 M0 Mini

* AceTime 1.2
* Arduino IDE 1.8.13
* Arduino SAMD Boards 1.8.9

```
{samd_results}
```

(SAMD compiler does not produce RAM usage numbers.)

## ESP8266

* AceTime 1.2
* Arduino IDE 1.8.13
* ESP8266 Boards 2.7.4

```
{esp8266_results}
```

## ESP32

* AceTime 1.2
* Arduino IDE 1.8.13
* ESP32 Boards 1.0.4

```
{esp32_results}
```

RAM usage remains constant as more objects are created, which indicates that an
initial pool of a certain minimum size is created regardless of the actual RAM
usage by objects.

## Teensy 3.2

* AceTime 1.2
* Arduino IDE 1.8.13
* Teensyduino 1.53

```
{teensy_results}
```
""")
