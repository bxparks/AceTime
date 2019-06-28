# MemoryBenchmark

The `MemoryBenchmark.ino` was compiled with each `FEATURE_*` and the flash
memory and static RAM sizes were recorded. The `FEATURE_BASELINE` selection is
the baseline, and its memory usage  numbers are subtracted from the subsequent
`FEATURE_*` memory usage.

## Arduino Nano

* AceTime 0.3
* Arduino IDE 1.8.9
* AVR Core 1.6.23

```
+------------------------------------------------------------------+
| Functionality                       | flash/ram | Baseline Delta |
|-------------------------------------+-----------+----------------|
| Baseline                            |   448/ 10 |        0/  0   |
|-------------------------------------+-----------+----------------|
| LocalDateTime                       |  1452/147 |     1003/137   |
| ZonedDateTime                       |  1634/147 |     1186/137   |
| ManualZoneSpecifier                 |  3014/171 |     2566/161   |
| BasicZoneSpecifier                  |  5372/293 |     4924/283   |
| BasicZoneSpecifier (2 zones)        |  6300/367 |     5852/357   |
| ExtendedZoneSpecifier               |  7738/307 |     7290/297   |
| ExtendedZoneSpecifier (2 zones)     |  8634/381 |     8186/371   |
|-------------------------------------+-----------+----------------|
| SystemClock                         |  4720/276 |     4272/266   |
| SystemClock+BasicZoneSpecifier      |  8406/444 |     7958/434   |
+------------------------------------------------------------------+
```

## ESP8266

* AceTime 0.3
* Arduino IDE 1.8.9
* ESP8266 Core 2.5.2
* ESP32 Core 1.0.2

```
+---------------------------------------------------------------------+
| Functionality                       |    flash/ram | Baseline Delta |
|-------------------------------------+--------------+----------------|
| Baseline                            | 257104/26540 |        0/   0  |
|-------------------------------------+--------------+----------------|
| LocalDateTime                       | 260272/26944 |     3168/ 404  |
| ZonedDateTime                       | 260720/26944 |     3616/ 404  |
| ManualZoneSpecifier                 | 261384/26944 |     4280/ 404  |
| BasicZoneSpecifier                  | 267872/31168 |    10768/4628  |
| BasicZoneSpecifier (2 zones)        | 268676/31252 |    11572/4712  |
| ExtendedZoneSpecifier               | 272056/33344 |    14952/6804  |
| ExtendedZoneSpecifier (2 zones)     | 272988/33428 |    15884/6888  |
|-------------------------------------+--------------+----------------|
| SystemClock                         | 263772/26960 |     6668/ 420  |
| SystemClock+BasicZoneSpecifier      | 271232/31180 |    14128/4640  |
+---------------------------------------------------------------------+
```

The static RAM usage jump from 404 bytes to 4628 bytes when `BasicZoneSpecifier`
is added is a bit surprising since `sizeof(BasicZoneSpecifier)` is only 140
bytes. Fortunately, the ESP8266 has at least 80kB of RAM so this should not be
an issue.

## ESP32

* AceTime 0.3
* Arduino IDE 1.8.9
* ESP32 Core 1.0.2

```
+---------------------------------------------------------------------+
| Functionality                       |    flash/ram | Baseline Delta |
|-------------------------------------+--------------+----------------|
| Baseline                            | 193200/12680 |        0/   0  |
|-------------------------------------+--------------+----------------|
| LocalDateTime                       | 203796/14156 |    10596/1476  |
| ZonedDateTime                       | 204240/14156 |    11040/1476  |
| ManualZoneSpecifier                 | 204900/14156 |    11700/1476  |
| BasicZoneSpecifier                  | 211780/14156 |    18580/1476  |
| BasicZoneSpecifier (2 zones)        | 212152/14156 |    18952/1476  |
| ExtendedZoneSpecifier               | 215696/14156 |    22496/1476  |
| ExtendedZoneSpecifier (2 zones)     | 216120/14156 |    22920/1476  |
|-------------------------------------+--------------+----------------|
| SystemClock                         | 211480/14260 |    18280/1580  |
| SystemClock+BasicZoneSpecifier      | 218872/14260 |    42028/1580  |
+---------------------------------------------------------------------+
```

RAM usage remains constant as more objects are created, which indicates that an
initial pool of a certain minimum size is created regardless of the actual RAM
usage by objects.
