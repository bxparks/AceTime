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
| Basic ZoneManager (1 zone)      |   6996/  219 |  6548/  209 |
| Basic ZoneManager (all)         |  22062/  607 | 21614/  597 |
| Extended TimeZone               |   9232/  211 |  8784/  201 |
| Extended TimeZone (2 zones)     |   9702/  217 |  9254/  207 |
| Extended ZoneManager (1 zone)   |   9510/  219 |  9062/  209 |
| Extended ZoneManager (all)      |  34114/  709 | 33666/  699 |
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
| Basic ZoneManager (1 zone)      |   9996/  359 |  6374/  209 |
| Basic ZoneManager (all)         |  25060/  745 | 21438/  595 |
| Extended TimeZone               |  12232/  351 |  8610/  201 |
| Extended TimeZone (2 zones)     |  12700/  355 |  9078/  205 |
| Extended ZoneManager (1 zone)   |  12510/  359 |  8888/  209 |
| Extended ZoneManager (all)      |  37112/  847 | 33490/  697 |
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
| Baseline                        |  10924/    0 |     0/    0 |
|---------------------------------+--------------+-------------|
| LocalDateTime                   |  11788/    0 |   864/    0 |
| ZonedDateTime                   |  11944/    0 |  1020/    0 |
| Basic TimeZone                  |  15924/    0 |  5000/    0 |
| Basic TimeZone (2 zones)        |  16316/    0 |  5392/    0 |
| Basic ZoneManager (1 zone)      |  16052/    0 |  5128/    0 |
| Basic ZoneManager (all)         |  35224/    0 | 24300/    0 |
| Extended TimeZone               |  17764/    0 |  6840/    0 |
| Extended TimeZone (2 zones)     |  18184/    0 |  7260/    0 |
| Extended ZoneManager (1 zone)   |  17904/    0 |  6980/    0 |
| Extended ZoneManager (all)      |  49732/    0 | 38808/    0 |
|---------------------------------+--------------+-------------|
| SystemClock                     |  14068/    0 |  3144/    0 |
| SystemClock+Basic TimeZone      |  17448/    0 |  6524/    0 |
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
| Basic TimeZone                  | 265412/27312 |  8308/  772 |
| Basic TimeZone (2 zones)        | 265700/27312 |  8596/  772 |
| Basic ZoneManager (1 zone)      | 265636/27312 |  8532/  772 |
| Basic ZoneManager (all)         | 285316/27312 | 28212/  772 |
| Extended TimeZone               | 267352/27412 | 10248/  872 |
| Extended TimeZone (2 zones)     | 267720/27412 | 10616/  872 |
| Extended ZoneManager (1 zone)   | 267576/27412 | 10472/  872 |
| Extended ZoneManager (all)      | 300088/27412 | 42984/  872 |
|---------------------------------+--------------+-------------|
| SystemClock                     | 262852/26908 |  5748/  368 |
| SystemClock+Basic TimeZone      | 267732/27324 | 10628/  784 |
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
| Basic TimeZone                  | 208568/14164 | 15368/ 1484 |
| Basic TimeZone (2 zones)        | 208840/14164 | 15640/ 1484 |
| Basic ZoneManager (1 zone)      | 208856/14164 | 15656/ 1484 |
| Basic ZoneManager (all)         | 228440/14164 | 35240/ 1484 |
| Extended TimeZone               | 210260/14164 | 17060/ 1484 |
| Extended TimeZone (2 zones)     | 210576/14164 | 17376/ 1484 |
| Extended ZoneManager (1 zone)   | 210516/14164 | 17316/ 1484 |
| Extended ZoneManager (all)      | 242956/14164 | 49756/ 1484 |
|---------------------------------+--------------+-------------|
| SystemClock                     | 211532/14268 | 18332/ 1588 |
| SystemClock+Basic TimeZone      | 215356/14268 | 22156/ 1588 |
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
| Basic ZoneManager (1 zone)      |  22236/ 5204 | 13388/ 1768 |
| Basic ZoneManager (all)         |  41948/ 5204 | 33100/ 1768 |
| Extended TimeZone               |  25360/ 5204 | 16512/ 1768 |
| Extended TimeZone (2 zones)     |  26096/ 5204 | 17248/ 1768 |
| Extended ZoneManager (1 zone)   |  25572/ 5204 | 16724/ 1768 |
| Extended ZoneManager (all)      |  58140/ 5204 | 49292/ 1768 |
|---------------------------------+--------------+-------------|
| SystemClock                     |  17368/ 5204 |  8520/ 1768 |
| SystemClock+Basic TimeZone      |  25068/ 5204 | 16220/ 1768 |
+--------------------------------------------------------------+
```
