# MemoryBenchmark

The `MemoryBenchmark.ino` was compiled with each `FEATURE_*` and the flash
memory and static RAM sizes were recorded. The `FEATURE_BASELINE` selection is
the baseline, and its memory usage  numbers are subtracted from the subsequent
FEATURE memory usage.

## Arduino Nano

```
+------------------------------------------------------------------+
| Functionaltiy                       | flash/ram | Baseline Delta |
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
| SystemTimeKeeper                    |  4720/276 |     4272/266   |
| SystemTimeKeeper+BasicZoneSpecifier |  8406/444 |     7958/434   |
+------------------------------------------------------------------+
```

## ESP8266

```
+---------------------------------------------------------------------+
| Functionaltiy                       |    flash/ram | Baseline Delta |
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
| SystemTimeKeeper                    | 263772/26960 |     6668/ 420  |
| SystemTimeKeeper+BasicZoneSpecifier | 271232/31180 |    14128/4640  |
+---------------------------------------------------------------------+
```
