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

* AceTime 0.8
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
| Basic TimeZone (1 zone)         |   6862/  245 |  6414/  235 |
| Basic TimeZone (2 zones)        |   7406/  249 |  6958/  239 |
| Basic ZoneManager (1 zone)      |   7036/  219 |  6588/  209 |
| Basic ZoneManager (all)         |  22004/  607 | 21556/  597 |
| Extended TimeZone (1 zone)      |   9628/  211 |  9180/  201 |
| Extended TimeZone (2 zones)     |  10118/  217 |  9670/  207 |
| Extended ZoneManager (1 zone)   |   9932/  219 |  9484/  209 |
| Extended ZoneManager (all)      |  34688/  709 | 34240/  699 |
|---------------------------------+--------------+-------------|
| SystemClock                     |   4860/  276 |  4412/  266 |
| SystemClock+Basic TimeZone      |   9320/  366 |  8872/  356 |
| SystemClock+Extended TimeZone   |  12290/  328 | 11842/  318 |
+--------------------------------------------------------------+
```

## Sparkfun Pro Micro

* AceTime 0.8
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
| Basic TimeZone (1 zone)         |   9860/  351 |  6238/  201 |
| Basic TimeZone (2 zones)        |  10404/  355 |  6782/  205 |
| Basic ZoneManager (1 zone)      |  10036/  359 |  6414/  209 |
| Basic ZoneManager (all)         |  25002/  745 | 21380/  595 |
| Extended TimeZone (1 zone)      |  12628/  351 |  9006/  201 |
| Extended TimeZone (2 zones)     |  13116/  355 |  9494/  205 |
| Extended ZoneManager (1 zone)   |  12932/  359 |  9310/  209 |
| Extended ZoneManager (all)      |  37686/  847 | 34064/  697 |
|---------------------------------+--------------+-------------|
| SystemClock                     |   7860/  416 |  4238/  266 |
| SystemClock+Basic TimeZone      |  12320/  506 |  8698/  356 |
| SystemClock+Extended TimeZone   |  15290/  468 | 11668/  318 |
+--------------------------------------------------------------+
```

## SAMD21 M0 Mini

* AceTime 0.8
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
| Basic TimeZone (1 zone)         |  15956/    0 |  5032/    0 |
| Basic TimeZone (2 zones)        |  16344/    0 |  5420/    0 |
| Basic ZoneManager (1 zone)      |  16088/    0 |  5164/    0 |
| Basic ZoneManager (all)         |  35160/    0 | 24236/    0 |
| Extended TimeZone (1 zone)      |  17832/    0 |  6908/    0 |
| Extended TimeZone (2 zones)     |  18232/    0 |  7308/    0 |
| Extended ZoneManager (1 zone)   |  17968/    0 |  7044/    0 |
| Extended ZoneManager (all)      |  49952/    0 | 39028/    0 |
|---------------------------------+--------------+-------------|
| SystemClock                     |  14116/    0 |  3192/    0 |
| SystemClock+Basic TimeZone      |  17532/    0 |  6608/    0 |
| SystemClock+Extended TimeZone   |  19472/    0 |  8548/    0 |
+--------------------------------------------------------------+
```

(SAMD compiler does not produce RAM usage numbers.)

## ESP8266

* AceTime 0.8
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
| Basic TimeZone (1 zone)         | 265220/27312 |  8116/  772 |
| Basic TimeZone (2 zones)        | 265460/27312 |  8356/  772 |
| Basic ZoneManager (1 zone)      | 265364/27312 |  8260/  772 |
| Basic ZoneManager (all)         | 284948/27312 | 27844/  772 |
| Extended TimeZone (1 zone)      | 267400/27412 | 10296/  872 |
| Extended TimeZone (2 zones)     | 267720/27412 | 10616/  872 |
| Extended ZoneManager (1 zone)   | 267624/27412 | 10520/  872 |
| Extended ZoneManager (all)      | 300280/27412 | 43176/  872 |
|---------------------------------+--------------+-------------|
| SystemClock                     | 262932/26908 |  5828/  368 |
| SystemClock+Basic TimeZone      | 267620/27324 | 10516/  784 |
| SystemClock+Extended TimeZone   | 270056/27424 | 12952/  884 |
+--------------------------------------------------------------+
```

## ESP32

* AceTime 0.8
* Arduino IDE 1.8.9
* ESP32 Boards 1.0.2

```
+--------------------------------------------------------------+
| Functionality                   |    flash/ram |       Delta |
|---------------------------------+--------------+-------------|
| Baseline                        | 193200/12680 |     0/    0 |
|---------------------------------+--------------+-------------|
| LocalDateTime                   | 203820/14164 | 10620/ 1484 |
| ZonedDateTime                   | 204624/14164 | 11424/ 1484 |
| Basic TimeZone (1 zone)         | 208348/14164 | 15148/ 1484 |
| Basic TimeZone (2 zones)        | 208600/14164 | 15400/ 1484 |
| Basic ZoneManager (1 zone)      | 208620/14164 | 15420/ 1484 |
| Basic ZoneManager (all)         | 228108/14164 | 34908/ 1484 |
| Extended TimeZone (1 zone)      | 210212/14164 | 17012/ 1484 |
| Extended TimeZone (2 zones)     | 210488/14164 | 17288/ 1484 |
| Extended ZoneManager (1 zone)   | 210464/14164 | 17264/ 1484 |
| Extended ZoneManager (all)      | 243060/14164 | 49860/ 1484 |
|---------------------------------+--------------+-------------|
| SystemClock                     | 211564/14268 | 18364/ 1588 |
| SystemClock+Basic TimeZone      | 215188/14268 | 21988/ 1588 |
| SystemClock+Extended TimeZone   | 217116/14268 | 23916/ 1588 |
+--------------------------------------------------------------+
```

RAM usage remains constant as more objects are created, which indicates that an
initial pool of a certain minimum size is created regardless of the actual RAM
usage by objects.

## Teensy 3.2

* AceTime 0.8
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
| Basic TimeZone (1 zone)         |  23240/ 5204 | 14392/ 1768 |
| Basic TimeZone (2 zones)        |  23912/ 5204 | 15064/ 1768 |
| Basic ZoneManager (1 zone)      |  23452/ 5204 | 14604/ 1768 |
| Basic ZoneManager (all)         |  43064/ 5204 | 34216/ 1768 |
| Extended TimeZone (1 zone)      |  25424/ 5204 | 16576/ 1768 |
| Extended TimeZone (2 zones)     |  26160/ 5204 | 17312/ 1768 |
| Extended ZoneManager (1 zone)   |  25700/ 5204 | 16852/ 1768 |
| Extended ZoneManager (all)      |  58360/ 5204 | 49512/ 1768 |
|---------------------------------+--------------+-------------|
| SystemClock                     |  17380/ 5204 |  8532/ 1768 |
| SystemClock+Basic TimeZone      |  26284/ 5204 | 17436/ 1768 |
| SystemClock+Extended TimeZone   |  28468/ 5204 | 19620/ 1768 |
+--------------------------------------------------------------+
```
