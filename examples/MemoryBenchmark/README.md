# Memory Benchmark

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

**Version**: AceTime v1.2

## Arduino Nano

* Arduino IDE 1.8.13
* Arduino AVR Boards 1.8.3

```
+--------------------------------------------------------------+
| Functionality                   |    flash/ram |       Delta |
|---------------------------------+--------------+-------------|
| Baseline                        |    448/   10 |     0/    0 |
|---------------------------------+--------------+-------------|
| LocalDateTime                   |   1648/  123 |  1200/  113 |
| ZonedDateTime                   |   2440/  123 |  1992/  113 |
| Basic TimeZone (1 zone)         |   6910/  187 |  6462/  177 |
| Basic TimeZone (2 zones)        |   7488/  193 |  7040/  183 |
| Basic ZoneManager (1 zone)      |   7084/  195 |  6636/  185 |
| Basic ZoneManager (all)         |  21988/  583 | 21540/  573 |
| Extended TimeZone (1 zone)      |   9690/  187 |  9242/  177 |
| Extended TimeZone (2 zones)     |  10166/  193 |  9718/  183 |
| Extended ZoneManager (1 zone)   |   9956/  195 |  9508/  185 |
| Extended ZoneManager (all)      |  34872/  689 | 34424/  679 |
|---------------------------------+--------------+-------------|
| SystemClock                     |   5662/  282 |  5214/  272 |
| SystemClock+Basic TimeZone      |   9990/  334 |  9542/  324 |
| SystemClock+Extended TimeZone   |  12996/  334 | 12548/  324 |
+--------------------------------------------------------------+

```

## Sparkfun Pro Micro

* Arduino IDE 1.8.13
* SparkFun AVR Boards 1.1.13

```
+--------------------------------------------------------------+
| Functionality                   |    flash/ram |       Delta |
|---------------------------------+--------------+-------------|
| Baseline                        |   3464/  150 |     0/    0 |
|---------------------------------+--------------+-------------|
| LocalDateTime                   |   4772/  263 |  1308/  113 |
| ZonedDateTime                   |   5564/  263 |  2100/  113 |
| Basic TimeZone (1 zone)         |  10012/  327 |  6548/  177 |
| Basic TimeZone (2 zones)        |  10588/  331 |  7124/  181 |
| Basic ZoneManager (1 zone)      |  10186/  335 |  6722/  185 |
| Basic ZoneManager (all)         |  25088/  721 | 21624/  571 |
| Extended TimeZone (1 zone)      |  12792/  327 |  9328/  177 |
| Extended TimeZone (2 zones)     |  13266/  331 |  9802/  181 |
| Extended ZoneManager (1 zone)   |  13058/  335 |  9594/  185 |
| Extended ZoneManager (all)      |  37974/  829 | 34510/  679 |
|---------------------------------+--------------+-------------|
| SystemClock                     |   8646/  422 |  5182/  272 |
| SystemClock+Basic TimeZone      |  12974/  474 |  9510/  324 |
| SystemClock+Extended TimeZone   |  15980/  474 | 12516/  324 |
+--------------------------------------------------------------+

```

## SAMD21 M0 Mini

* Arduino IDE 1.8.13
* Arduino SAMD Boards 1.8.9

```
+--------------------------------------------------------------+
| Functionality                   |    flash/ram |       Delta |
|---------------------------------+--------------+-------------|
| Baseline                        |  11096/ 2368 |     0/    0 |
|---------------------------------+--------------+-------------|
| LocalDateTime                   |  11976/ 2948 |   880/  580 |
| ZonedDateTime                   |  12132/ 2948 |  1036/  580 |
| Basic TimeZone (1 zone)         |  16144/ 2948 |  5048/  580 |
| Basic TimeZone (2 zones)        |  16532/ 2948 |  5436/  580 |
| Basic ZoneManager (1 zone)      |  16276/ 2948 |  5180/  580 |
| Basic ZoneManager (all)         |  35260/ 2948 | 24164/  580 |
| Extended TimeZone (1 zone)      |  18028/ 2948 |  6932/  580 |
| Extended TimeZone (2 zones)     |  18428/ 2948 |  7332/  580 |
| Extended ZoneManager (1 zone)   |  18164/ 2948 |  7068/  580 |
| Extended ZoneManager (all)      |  50348/ 2948 | 39252/  580 |
|---------------------------------+--------------+-------------|
| SystemClock                     |  14300/ 2948 |  3204/  580 |
| SystemClock+Basic TimeZone      |  17716/ 2948 |  6620/  580 |
| SystemClock+Extended TimeZone   |  19664/ 2948 |  8568/  580 |
+--------------------------------------------------------------+

```

(SAMD compiler does not produce RAM usage numbers.)

## ESP8266

* Arduino IDE 1.8.13
* ESP8266 Boards 2.7.4

```
+--------------------------------------------------------------+
| Functionality                   |    flash/ram |       Delta |
|---------------------------------+--------------+-------------|
| Baseline                        | 256700/26776 |     0/    0 |
|---------------------------------+--------------+-------------|
| LocalDateTime                   | 258820/27264 |  2120/  488 |
| ZonedDateTime                   | 259508/27264 |  2808/  488 |
| Basic TimeZone (1 zone)         | 264800/27692 |  8100/  916 |
| Basic TimeZone (2 zones)        | 265040/27692 |  8340/  916 |
| Basic ZoneManager (1 zone)      | 264944/27692 |  8244/  916 |
| Basic ZoneManager (all)         | 284400/27692 | 27700/  916 |
| Extended TimeZone (1 zone)      | 266824/27788 | 10124/ 1012 |
| Extended TimeZone (2 zones)     | 267128/27788 | 10428/ 1012 |
| Extended ZoneManager (1 zone)   | 267016/27788 | 10316/ 1012 |
| Extended ZoneManager (all)      | 299852/27792 | 43152/ 1016 |
|---------------------------------+--------------+-------------|
| SystemClock                     | 262284/27276 |  5584/  500 |
| SystemClock+Basic TimeZone      | 267212/27692 | 10512/  916 |
| SystemClock+Extended TimeZone   | 269476/27788 | 12776/ 1012 |
+--------------------------------------------------------------+

```

## ESP32

* Arduino IDE 1.8.13
* ESP32 Boards 1.0.4

```
+--------------------------------------------------------------+
| Functionality                   |    flash/ram |       Delta |
|---------------------------------+--------------+-------------|
| Baseline                        | 206435/14564 |     0/    0 |
|---------------------------------+--------------+-------------|
| LocalDateTime                   | 218577/16196 | 12142/ 1632 |
| ZonedDateTime                   | 219381/16196 | 12946/ 1632 |
| Basic TimeZone (1 zone)         | 223081/16196 | 16646/ 1632 |
| Basic TimeZone (2 zones)        | 223333/16196 | 16898/ 1632 |
| Basic ZoneManager (1 zone)      | 223353/16196 | 16918/ 1632 |
| Basic ZoneManager (all)         | 242753/16196 | 36318/ 1632 |
| Extended TimeZone (1 zone)      | 224969/16196 | 18534/ 1632 |
| Extended TimeZone (2 zones)     | 225245/16196 | 18810/ 1632 |
| Extended ZoneManager (1 zone)   | 225221/16196 | 18786/ 1632 |
| Extended ZoneManager (all)      | 258025/16196 | 51590/ 1632 |
|---------------------------------+--------------+-------------|
| SystemClock                     | 226165/16300 | 19730/ 1736 |
| SystemClock+Basic TimeZone      | 229765/16300 | 23330/ 1736 |
| SystemClock+Extended TimeZone   | 231717/16300 | 25282/ 1736 |
+--------------------------------------------------------------+

```

RAM usage remains constant as more objects are created, which indicates that an
initial pool of a certain minimum size is created regardless of the actual RAM
usage by objects.

## Teensy 3.2

* Arduino IDE 1.8.13
* Teensyduino 1.53

```
+--------------------------------------------------------------+
| Functionality                   |    flash/ram |       Delta |
|---------------------------------+--------------+-------------|
| Baseline                        |   7624/ 3048 |     0/    0 |
|---------------------------------+--------------+-------------|
| LocalDateTime                   |  13268/ 4812 |  5644/ 1764 |
| ZonedDateTime                   |  13268/ 4812 |  5644/ 1764 |
| Basic TimeZone (1 zone)         |  22024/ 4812 | 14400/ 1764 |
| Basic TimeZone (2 zones)        |  22696/ 4812 | 15072/ 1764 |
| Basic ZoneManager (1 zone)      |  22236/ 4812 | 14612/ 1764 |
| Basic ZoneManager (all)         |  41760/ 4812 | 34136/ 1764 |
| Extended TimeZone (1 zone)      |  24216/ 4812 | 16592/ 1764 |
| Extended TimeZone (2 zones)     |  24952/ 4812 | 17328/ 1764 |
| Extended ZoneManager (1 zone)   |  24492/ 4812 | 16868/ 1764 |
| Extended ZoneManager (all)      |  57360/ 4812 | 49736/ 1764 |
|---------------------------------+--------------+-------------|
| SystemClock                     |  16192/ 4812 |  8568/ 1764 |
| SystemClock+Basic TimeZone      |  25132/ 4812 | 17508/ 1764 |
| SystemClock+Extended TimeZone   |  27324/ 4812 | 19700/ 1764 |
+--------------------------------------------------------------+

```

