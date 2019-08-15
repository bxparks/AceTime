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

* AceTime 0.7.2
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
| Basic TimeZone                  |   7020/  211 |  6572/  201 |
| Basic TimeZone (2 zones)        |   7564/  217 |  7116/  207 |
| Basic ZoneManager (1 zone)      |   7156/  219 |  6708/  209 |
| Basic ZoneManager (all)         |  22124/  607 | 21676/  597 |
| Extended TimeZone               |   9800/  211 |  9352/  201 |
| Extended TimeZone (2 zones)     |  10296/  217 |  9848/  207 |
| Extended ZoneManager (1 zone)   |  10108/  219 |  9660/  209 |
| Extended ZoneManager (all)      |  34864/  709 | 34416/  699 |
|---------------------------------+--------------+-------------|
| SystemClock                     |   4860/  276 |  4412/  266 |
| SystemClock+Basic TimeZone      |   9466/  366 |  9018/  356 |
+--------------------------------------------------------------+
```

## Sparkfun Pro Micro

* AceTime 0.7.2
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
| Basic TimeZone                  |  10020/  351 |  6398/  201 |
| Basic TimeZone (2 zones)        |  10562/  355 |  6940/  205 |
| Basic ZoneManager (1 zone)      |  10156/  359 |  6534/  209 |
| Basic ZoneManager (all)         |  25122/  745 | 21500/  595 |
| Extended TimeZone               |  12800/  351 |  9178/  201 |
| Extended TimeZone (2 zones)     |  13294/  355 |  9672/  205 |
| Extended ZoneManager (1 zone)   |  13108/  359 |  9486/  209 |
| Extended ZoneManager (all)      |  37862/  847 | 34240/  697 |
|---------------------------------+--------------+-------------|
| SystemClock                     |   7860/  416 |  4238/  266 |
| SystemClock+Basic TimeZone      |  12466/  506 |  8844/  356 |
+--------------------------------------------------------------+
```

## SAMD21 M0 Mini

* AceTime 0.7.2
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
| Basic ZoneManager (all)         |  35236/    0 | 24312/    0 |
| Extended TimeZone               |  17940/    0 |  7016/    0 |
| Extended TimeZone (2 zones)     |  18344/    0 |  7420/    0 |
| Extended ZoneManager (1 zone)   |  18080/    0 |  7156/    0 |
| Extended ZoneManager (all)      |  50064/    0 | 39140/    0 |
|---------------------------------+--------------+-------------|
| SystemClock                     |  14116/    0 |  3192/    0 |
| SystemClock+Basic TimeZone      |  17608/    0 |  6684/    0 |
+--------------------------------------------------------------+
```

(SAMD compiler does not produce RAM usage numbers.)

## ESP8266

* AceTime 0.7.2
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
| Basic TimeZone                  | 265364/27312 |  8260/  772 |
| Basic TimeZone (2 zones)        | 265620/27312 |  8516/  772 |
| Basic ZoneManager (1 zone)      | 265604/27312 |  8500/  772 |
| Basic ZoneManager (all)         | 285188/27312 | 28084/  772 |
| Extended TimeZone               | 267560/27412 | 10456/  872 |
| Extended TimeZone (2 zones)     | 267864/27412 | 10760/  872 |
| Extended ZoneManager (1 zone)   | 267752/27412 | 10648/  872 |
| Extended ZoneManager (all)      | 300408/27412 | 43304/  872 |
|---------------------------------+--------------+-------------|
| SystemClock                     | 262932/26908 |  5828/  368 |
| SystemClock+Basic TimeZone      | 267796/27324 | 10692/  784 |
+--------------------------------------------------------------+
```

## ESP32

* AceTime 0.7.2
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
| Basic TimeZone                  | 208392/14164 | 15192/ 1484 |
| Basic TimeZone (2 zones)        | 208660/14164 | 15460/ 1484 |
| Basic ZoneManager (1 zone)      | 208700/14164 | 15500/ 1484 |
| Basic ZoneManager (all)         | 228188/14164 | 34988/ 1484 |
| Extended TimeZone               | 210308/14164 | 17108/ 1484 |
| Extended TimeZone (2 zones)     | 210592/14164 | 17392/ 1484 |
| Extended ZoneManager (1 zone)   | 210564/14164 | 17364/ 1484 |
| Extended ZoneManager (all)      | 243160/14164 | 49960/ 1484 |
|---------------------------------+--------------+-------------|
| SystemClock                     | 211564/14268 | 18364/ 1588 |
| SystemClock+Basic TimeZone      | 215268/14268 | 22068/ 1588 |
+--------------------------------------------------------------+
```

RAM usage remains constant as more objects are created, which indicates that an
initial pool of a certain minimum size is created regardless of the actual RAM
usage by objects.

## Teensy 3.2

* AceTime 0.7.2
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
| Basic ZoneManager (all)         |  42040/ 5204 | 33192/ 1768 |
| Extended TimeZone               |  25552/ 5204 | 16704/ 1768 |
| Extended TimeZone (2 zones)     |  26224/ 5204 | 17376/ 1768 |
| Extended ZoneManager (1 zone)   |  25764/ 5204 | 16916/ 1768 |
| Extended ZoneManager (all)      |  58488/ 5204 | 49640/ 1768 |
|---------------------------------+--------------+-------------|
| SystemClock                     |  17380/ 5204 |  8532/ 1768 |
| SystemClock+Basic TimeZone      |  25324/ 5204 | 16476/ 1768 |
+--------------------------------------------------------------+
```
