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

* AceTime 0.7
* Arduino IDE 1.8.9
* Arduino AVR Boards 1.6.23

```
+--------------------------------------------------------------+
| Functionality                   |    flash/ram |       Delta |
|---------------------------------+--------------+-------------|
| Baseline                        |    448/   10 |     0/    0 |
|---------------------------------+--------------+-------------|
| LocalDateTime                   |   1452/  147 |  1004/  137 |
| ZonedDateTime                   |   2294/  147 |  1846/  137 |
| Basic TimeZone                  |   7034/  211 |  6586/  201 |
| Basic TimeZone (2 zones)        |   7578/  217 |  7130/  207 |
| Basic ZoneManager (1 zone)      |   7170/  219 |  6722/  209 |
| Basic ZoneManager (all)         |  22236/  607 | 21788/  597 |
| Extended TimeZone               |   9800/  211 |  9352/  201 |
| Extended TimeZone (2 zones)     |  10296/  217 |  9848/  207 |
| Extended ZoneManager (1 zone)   |  10108/  219 |  9660/  209 |
| Extended ZoneManager (all)      |  34712/  709 | 34264/  699 |
|---------------------------------+--------------+-------------|
| SystemClock                     |   4860/  276 |  4412/  266 |
| SystemClock+Basic TimeZone      |   9480/  366 |  9032/  356 |
+--------------------------------------------------------------+
```

## Sparkfun Pro Micro

* AceTime 0.7
* Arduino IDE 1.8.9
* SparkFun AVR Boards 1.1.12

```
+--------------------------------------------------------------+
| Functionality                   |    flash/ram |       Delta |
|---------------------------------+--------------+-------------|
| Baseline                        |   3622/  150 |     0/    0 |
|---------------------------------+--------------+-------------|
| LocalDateTime                   |   4452/  287 |   830/  137 |
| ZonedDateTime                   |   5294/  287 |  1672/  137 |
| Basic TimeZone                  |  10034/  351 |  6412/  201 |
| Basic TimeZone (2 zones)        |  10576/  355 |  6954/  205 |
| Basic ZoneManager (1 zone)      |  10170/  359 |  6548/  209 |
| Basic ZoneManager (all)         |  25234/  745 | 21612/  595 |
| Extended TimeZone               |  12800/  351 |  9178/  201 |
| Extended TimeZone (2 zones)     |  13294/  355 |  9672/  205 |
| Extended ZoneManager (1 zone)   |  13108/  359 |  9486/  209 |
| Extended ZoneManager (all)      |  37710/  847 | 34088/  697 |
|---------------------------------+--------------+-------------|
| SystemClock                     |   7860/  416 |  4238/  266 |
| SystemClock+Basic TimeZone      |  12480/  506 |  8858/  356 |
+--------------------------------------------------------------+
```

## SAMD21 M0 Mini

* AceTime 0.7
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
| Basic TimeZone                  |  16028/    0 |  5104/    0 |
| Basic TimeZone (2 zones)        |  16420/    0 |  5496/    0 |
| Basic ZoneManager (1 zone)      |  16164/    0 |  5240/    0 |
| Basic ZoneManager (all)         |  35336/    0 | 24412/    0 |
| Extended TimeZone               |  17956/    0 |  7032/    0 |
| Extended TimeZone (2 zones)     |  18360/    0 |  7436/    0 |
| Extended ZoneManager (1 zone)   |  18096/    0 |  7172/    0 |
| Extended ZoneManager (all)      |  49924/    0 | 39000/    0 |
|---------------------------------+--------------+-------------|
| SystemClock                     |  14116/    0 |  3192/    0 |
| SystemClock+Basic TimeZone      |  17608/    0 |  6684/    0 |
+--------------------------------------------------------------+
```

(SAMD compiler does not produce RAM usage numbers.)

## ESP8266

* AceTime 0.7
* Arduino IDE 1.8.9
* ESP8266 Boards 2.5.2

```
+--------------------------------------------------------------+
| Functionality                   |    flash/ram |       Delta |
|---------------------------------+--------------+-------------|
| Baseline                        | 257104/26540 |     0/    0 |
|---------------------------------+--------------+-------------|
| LocalDateTime                   | 259480/26884 |  2376/  344 |
| ZonedDateTime                   | 260136/26884 |  3032/  344 |
| Basic TimeZone                  | 265396/27312 |  8292/  772 |
| Basic TimeZone (2 zones)        | 265652/27312 |  8548/  772 |
| Basic ZoneManager (1 zone)      | 265636/27312 |  8532/  772 |
| Basic ZoneManager (all)         | 285316/27312 | 28212/  772 |
| Extended TimeZone               | 267544/27412 | 10440/  872 |
| Extended TimeZone (2 zones)     | 267848/27412 | 10744/  872 |
| Extended ZoneManager (1 zone)   | 267736/27412 | 10632/  872 |
| Extended ZoneManager (all)      | 300248/27412 | 43144/  872 |
|---------------------------------+--------------+-------------|
| SystemClock                     | 262932/26908 |  5828/  368 |
| SystemClock+Basic TimeZone      | 267828/27324 | 10724/  784 |
+--------------------------------------------------------------+
```

## ESP32

* AceTime 0.7
* Arduino IDE 1.8.9
* ESP32 Boards 1.0.2

```
+--------------------------------------------------------------+
| Functionality                   |    flash/ram |       Delta |
|---------------------------------+--------------+-------------|
| Baseline                        | 193200/12680 |     0/    0 |
|---------------------------------+--------------+-------------|
| LocalDateTime                   | 203844/14164 | 10644/ 1484 |
| ZonedDateTime                   | 204648/14164 | 11448/ 1484 |
| Basic TimeZone                  | 208436/14164 | 15236/ 1484 |
| Basic TimeZone (2 zones)        | 208704/14164 | 15504/ 1484 |
| Basic ZoneManager (1 zone)      | 208744/14164 | 15544/ 1484 |
| Basic ZoneManager (all)         | 228332/14164 | 35132/ 1484 |
| Extended TimeZone               | 210332/14164 | 17132/ 1484 |
| Extended TimeZone (2 zones)     | 210616/14164 | 17416/ 1484 |
| Extended ZoneManager (1 zone)   | 210588/14164 | 17388/ 1484 |
| Extended ZoneManager (all)      | 243028/14164 | 49828/ 1484 |
|---------------------------------+--------------+-------------|
| SystemClock                     | 211588/14268 | 18388/ 1588 |
| SystemClock+Basic TimeZone      | 215312/14268 | 22112/ 1588 |
+--------------------------------------------------------------+
```

RAM usage remains constant as more objects are created, which indicates that an
initial pool of a certain minimum size is created regardless of the actual RAM
usage by objects.

## Teensy 3.2

* AceTime 0.7
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
| Basic TimeZone                  |  22216/ 5204 | 13368/ 1768 |
| Basic TimeZone (2 zones)        |  22952/ 5204 | 14104/ 1768 |
| Basic ZoneManager (1 zone)      |  22492/ 5204 | 13644/ 1768 |
| Basic ZoneManager (all)         |  42140/ 5204 | 33292/ 1768 |
| Extended TimeZone               |  25552/ 5204 | 16704/ 1768 |
| Extended TimeZone (2 zones)     |  26224/ 5204 | 17376/ 1768 |
| Extended ZoneManager (1 zone)   |  25764/ 5204 | 16916/ 1768 |
| Extended ZoneManager (all)      |  58332/ 5204 | 49484/ 1768 |
|---------------------------------+--------------+-------------|
| SystemClock                     |  17380/ 5204 |  8532/ 1768 |
| SystemClock+Basic TimeZone      |  25260/ 5204 | 16412/ 1768 |
+--------------------------------------------------------------+
```
