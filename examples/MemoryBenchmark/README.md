# MemoryBenchmark

The `MemoryBenchmark.ino` was compiled with each `FEATURE_*` and the flash
memory and static RAM sizes were recorded. The `FEATURE_BASELINE` selection is
the baseline, and its memory usage  numbers are subtracted from the subsequent
`FEATURE_*` memory usage.

The `collect.sh` script automates the collection of this data by using
[AUniter](https://github.com/bxparks/AUniter) to invoke the Arduino IDE
programmatically. It produces a `*.txt` file with the flash and ram usage
information (e.g. `nano.txt`). The `generate_table.awk` script takes the `*.txt`
file and produces the ASCII tables below. (I had to generate `teensy.txt` by
hand since Teensyduino does not seem to allow headless operation.)

## Arduino Nano

* AceTime 0.5
* Arduino IDE 1.8.9
* AVR Core 1.6.23

```
+--------------------------------------------------------------+
| Functionality                   |    flash/ram |       Delta |
|---------------------------------+--------------+-------------|
| Baseline                        |    448/   10 |     0/    0 |
|---------------------------------+--------------+-------------|
| LocalDateTime                   |   1452/  147 |  1004/  137 |
| ZonedDateTime                   |   2282/  147 |  1834/  137 |
| Basic TimeZone                  |   6854/  211 |  6406/  201 |
| Basic TimeZone (2 zones)        |   7372/  217 |  6924/  207 |
| Basic TimeZone (all zones)      |  22056/  607 | 21608/  597 |
| Extended TimeZone               |   9230/  211 |  8782/  201 |
| Extended TimeZone (2 zones)     |   9700/  217 |  9252/  207 |
| Extended TimeZone (all zones)   |  34112/  709 | 33664/  699 |
|---------------------------------+--------------+-------------|
| SystemClock                     |   4750/  276 |  4302/  266 |
| SystemClock+Basic TimeZone      |   9124/  362 |  8676/  352 |
+--------------------------------------------------------------+
```

## Sparkfun Pro Micro

* AceTime 0.5
* Arduino IDE 1.8.9
* AVR Core 1.6.23

```
+--------------------------------------------------------------+
| Functionality                   |    flash/ram |       Delta |
|---------------------------------+--------------+-------------|
| Baseline                        |   3622/  150 |     0/    0 |
|---------------------------------+--------------+-------------|
| LocalDateTime                   |   4452/  287 |   830/  137 |
| ZonedDateTime                   |   5282/  287 |  1660/  137 |
| Basic TimeZone                  |   9854/  351 |  6232/  201 |
| Basic TimeZone (2 zones)        |  10370/  355 |  6748/  205 |
| Basic TimeZone (all zones)      |  25054/  745 | 21432/  595 |
| Extended TimeZone               |  12230/  351 |  8608/  201 |
| Extended TimeZone (2 zones)     |  12698/  355 |  9076/  205 |
| Extended TimeZone (all zones)   |  37110/  847 | 33488/  697 |
|---------------------------------+--------------+-------------|
| SystemClock                     |   7750/  416 |  4128/  266 |
| SystemClock+Basic TimeZone      |  12124/  502 |  8502/  352 |
+--------------------------------------------------------------+
```

## ESP8266

* AceTime 0.5
* Arduino IDE 1.8.9
* ESP8266 Core 2.5.2
* ESP32 Core 1.0.2

```
+--------------------------------------------------------------+
| Functionality                   |    flash/ram |       Delta |
|---------------------------------+--------------+-------------|
| Baseline                        | 257104/26540 |     0/    0 |
|---------------------------------+--------------+-------------|
| LocalDateTime                   | 260448/26944 |  3344/  404 |
| ZonedDateTime                   | 261168/26944 |  4064/  404 |
| Basic TimeZone                  | 266300/27364 |  9196/  824 |
| Basic TimeZone (2 zones)        | 266604/27364 |  9500/  824 |
| Basic TimeZone (all zones)      | 288716/27364 | 31612/  824 |
| Extended TimeZone               | 268272/27472 | 11168/  932 |
| Extended TimeZone (2 zones)     | 268640/27472 | 11536/  932 |
| Extended TimeZone (all zones)   | 306624/27472 | 49520/  932 |
|---------------------------------+--------------+-------------|
| SystemClock                     | 263820/26960 |  6716/  420 |
| SystemClock+Basic TimeZone      | 268620/27376 | 11516/  836 |
+--------------------------------------------------------------+
```

## ESP32

* AceTime 0.5
* Arduino IDE 1.8.9
* ESP32 Core 1.0.2

```
+--------------------------------------------------------------+
| Functionality                   |    flash/ram |       Delta |
|---------------------------------+--------------+-------------|
| Baseline                        | 193200/12680 |     0/    0 |
|---------------------------------+--------------+-------------|
| LocalDateTime                   | 203884/14164 | 10684/ 1484 |
| ZonedDateTime                   | 204724/14164 | 11524/ 1484 |
| Basic TimeZone                  | 208640/14164 | 15440/ 1484 |
| Basic TimeZone (2 zones)        | 208932/14164 | 15732/ 1484 |
| Basic TimeZone (all zones)      | 231004/14164 | 37804/ 1484 |
| Extended TimeZone               | 210304/14164 | 17104/ 1484 |
| Extended TimeZone (2 zones)     | 210636/14164 | 17436/ 1484 |
| Extended TimeZone (all zones)   | 248632/14164 | 55432/ 1484 |
|---------------------------------+--------------+-------------|
| SystemClock                     | 211568/14268 | 18368/ 1588 |
| SystemClock+Basic TimeZone      | 215428/14268 | 22228/ 1588 |
+--------------------------------------------------------------+
```

RAM usage remains constant as more objects are created, which indicates that an
initial pool of a certain minimum size is created regardless of the actual RAM
usage by objects.

## Teensy 3.2

* AceTime 0.5
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
| Basic TimeZone                  |  22100/ 5204 | 13252/ 1768 |
| Basic TimeZone (2 zones)        |  22848/ 5204 | 14000/ 1768 |
| Basic TimeZone (all zones)      |  44472/ 5204 | 35624/ 1768 |
| Extended TimeZone               |  25380/ 5204 | 16532/ 1768 |
| Extended TimeZone (2 zones)     |  26128/ 5204 | 17280/ 1768 |
| Extended TimeZone (all zones)   |  63804/ 5204 | 54956/ 1768 |
|---------------------------------+--------------+-------------|
| SystemClock                     |  17368/ 5204 |  8520/ 1768 |
| SystemClock+Basic TimeZone      |  25144/ 5204 | 16296/ 1768 |
+--------------------------------------------------------------+
```
