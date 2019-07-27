# MemoryBenchmark

The `MemoryBenchmark.ino` was compiled with each `FEATURE_*` and the flash
memory and static RAM sizes were recorded. The `FEATURE_BASELINE` selection is
the baseline, and its memory usage  numbers are subtracted from the subsequent
`FEATURE_*` memory usage.

The `collect.sh` script automates the collection of this data by using
[AUniter](https://github.com/bxparks/AUniter) to invoke the Arduino IDE
programmatically. It produces a `*.txt` file with the flash and ram usage
information (e.g. `nano.txt`). The `generate_table.awk` script takes the `*.txt`
file and produces the ASCII tables below. (I generate `teensy.txt` by hand since
Teensyduino does not seem to allow headless operation.)

## Arduino Nano

* AceTime 0.5.2
* Arduino IDE 1.8.9
* Arduino AVR Boards 1.6.23

```
+--------------------------------------------------------------+
| Functionality                   |    flash/ram |       Delta |
|---------------------------------+--------------+-------------|
| Baseline                        |    448/   10 |     0/    0 |
|---------------------------------+--------------+-------------|
| LocalDateTime                   |   1452/  147 |  1004/  137 |
| ZonedDateTime                   |   2282/  147 |  1834/  137 |
| Basic TimeZone                  |   6860/  211 |  6412/  201 |
| Basic TimeZone (2 zones)        |   7378/  217 |  6930/  207 |
| Basic TimeZone (all zones)      |  22062/  607 | 21614/  597 |
| Extended TimeZone               |   9232/  211 |  8784/  201 |
| Extended TimeZone (2 zones)     |   9702/  217 |  9254/  207 |
| Extended TimeZone (all zones)   |  34114/  709 | 33666/  699 |
|---------------------------------+--------------+-------------|
| SystemClock                     |   4758/  276 |  4310/  266 |
| SystemClock+Basic TimeZone      |   9138/  362 |  8690/  352 |
+--------------------------------------------------------------+
```

## Sparkfun Pro Micro

* AceTime 0.5.2
* Arduino IDE 1.8.9
* SparkFun AVR Boards 1.1.12

```
+--------------------------------------------------------------+
| Functionality                   |    flash/ram |       Delta |
|---------------------------------+--------------+-------------|
| Baseline                        |   3622/  150 |     0/    0 |
|---------------------------------+--------------+-------------|
| LocalDateTime                   |   4452/  287 |   830/  137 |
| ZonedDateTime                   |   5282/  287 |  1660/  137 |
| Basic TimeZone                  |   9860/  351 |  6238/  201 |
| Basic TimeZone (2 zones)        |  10376/  355 |  6754/  205 |
| Basic TimeZone (all zones)      |  25060/  745 | 21438/  595 |
| Extended TimeZone               |  12232/  351 |  8610/  201 |
| Extended TimeZone (2 zones)     |  12700/  355 |  9078/  205 |
| Extended TimeZone (all zones)   |  37112/  847 | 33490/  697 |
|---------------------------------+--------------+-------------|
| SystemClock                     |   7758/  416 |  4136/  266 |
| SystemClock+Basic TimeZone      |  12138/  502 |  8516/  352 |
+--------------------------------------------------------------+
```

## SAMD21 M0 Mini

* AceTime 0.5.2
* Arduino IDE 1.8.9
* SparkFun SAMD Boards 1.6.2

```
+--------------------------------------------------------------+
| Functionality                   |    flash/ram |       Delta |
|---------------------------------+--------------+-------------|
| Baseline                        |  10072/    0 |     0/    0 |
|---------------------------------+--------------+-------------|
| LocalDateTime                   |  10928/    0 |   856/    0 |
| ZonedDateTime                   |  11168/    0 |  1096/    0 |
| Basic TimeZone                  |  15052/    0 |  4980/    0 |
| Basic TimeZone (2 zones)        |  15436/    0 |  5364/    0 |
| Basic TimeZone (all zones)      |  34732/    0 | 24660/    0 |
| Extended TimeZone               |  16740/    0 |  6668/    0 |
| Extended TimeZone (2 zones)     |  17180/    0 |  7108/    0 |
| Extended TimeZone (all zones)   |  49188/    0 | 39116/    0 |
|---------------------------------+--------------+-------------|
| SystemClock                     |  13208/    0 |  3136/    0 |
| SystemClock+Basic TimeZone      |  16628/    0 |  6556/    0 |
+--------------------------------------------------------------+
```

(SAMD compiler does not produce RAM usage numbers.)

## ESP8266

* AceTime 0.5.2
* Arduino IDE 1.8.9
* ESP8266 Boards 2.5.2

```
+--------------------------------------------------------------+
| Functionality                   |    flash/ram |       Delta |
|---------------------------------+--------------+-------------|
| Baseline                        | 257104/26540 |     0/    0 |
|---------------------------------+--------------+-------------|
| LocalDateTime                   | 259480/26884 |  2376/  344 |
| ZonedDateTime                   | 260184/26884 |  3080/  344 |
| Basic TimeZone                  | 265428/27312 |  8324/  772 |
| Basic TimeZone (2 zones)        | 265732/27312 |  8628/  772 |
| Basic TimeZone (all zones)      | 285348/27312 | 28244/  772 |
| Extended TimeZone               | 267352/27412 | 10248/  872 |
| Extended TimeZone (2 zones)     | 267720/27412 | 10616/  872 |
| Extended TimeZone (all zones)   | 300072/27412 | 42968/  872 |
|---------------------------------+--------------+-------------|
| SystemClock                     | 262852/26908 |  5748/  368 |
| SystemClock+Basic TimeZone      | 267748/27324 | 10644/  784 |
+--------------------------------------------------------------+
```

## ESP32

* AceTime 0.5.2
* Arduino IDE 1.8.9
* ESP32 Boards 1.0.2

```
+--------------------------------------------------------------+
| Functionality                   |    flash/ram |       Delta |
|---------------------------------+--------------+-------------|
| Baseline                        | 193200/12680 |     0/    0 |
|---------------------------------+--------------+-------------|
| LocalDateTime                   | 203844/14164 | 10644/ 1484 |
| ZonedDateTime                   | 204684/14164 | 11484/ 1484 |
| Basic TimeZone                  | 208588/14164 | 15388/ 1484 |
| Basic TimeZone (2 zones)        | 208868/14164 | 15668/ 1484 |
| Basic TimeZone (all zones)      | 228464/14164 | 35264/ 1484 |
| Extended TimeZone               | 210244/14164 | 17044/ 1484 |
| Extended TimeZone (2 zones)     | 210564/14164 | 17364/ 1484 |
| Extended TimeZone (all zones)   | 242956/14164 | 49756/ 1484 |
|---------------------------------+--------------+-------------|
| SystemClock                     | 211532/14268 | 18332/ 1588 |
| SystemClock+Basic TimeZone      | 215380/14268 | 22180/ 1588 |
+--------------------------------------------------------------+
```

RAM usage remains constant as more objects are created, which indicates that an
initial pool of a certain minimum size is created regardless of the actual RAM
usage by objects.

## Teensy 3.2

* AceTime 0.5.2
* Arduino IDE 1.8.9
* Teensyduino 1.46

```
+--------------------------------------------------------------+
| Functionality                   |    flash/ram |       Delta |
|---------------------------------+--------------+-------------|
| Baseline                        |   8848/ 3436 |     0/    0 |
|---------------------------------+--------------+-------------|
| LocalDateTime                   |  14492/ 5204 |  5644/ 1768 |
| ZonedDateTime                   |  14492/ 5204 |  5644/ 1768 |
| Basic TimeZone                  |  22024/ 5204 | 13176/ 1768 |
| Basic TimeZone (2 zones)        |  22760/ 5204 | 13912/ 1768 |
| Basic TimeZone (all zones)      |  41948/ 5204 | 33100/ 1768 |
| Extended TimeZone               |  25360/ 5204 | 16512/ 1768 |
| Extended TimeZone (2 zones)     |  26096/ 5204 | 17248/ 1768 |
| Extended TimeZone (all zones)   |  58140/ 5204 | 49292/ 1768 |
|---------------------------------+--------------+-------------|
| SystemClock                     |  17368/ 5204 |  8520/ 1768 |
| SystemClock+Basic TimeZone      |  25068/ 5204 | 16220/ 1768 |
+--------------------------------------------------------------+
```
