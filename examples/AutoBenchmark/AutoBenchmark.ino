/*
 * A program to determine how long it takes to execute some of the more complex
 * methods of DateTime:
 *
 *  - DateTime(seconds) constructor
 *  - DateTime::toDaysSinceEpoch()
 *  - DateTime::toSecondsSinceEpoch()
 *
 * This should run on all microcontrollers supported by the Arduino IDE.
 */

#include "Benchmark.h"

void setup() {
  delay(1000);
  Serial.begin(115200); // ESP8266 default of 74880 not supported on Linux
  while (!Serial); // Wait until Serial is ready - Leonardo/Micro
  pinMode(LED_BENCHMARK, OUTPUT);

  runBenchmarks();
}

void loop() {
}
