/*
 * Determine the execution time of a round trip conversion from human-readable
 * date/time components, to epochSeconds, then back to human-readable
 * components. We compare this round trip for the AceTime library compared to
 * the Arduino Time library (https://github.com/PaulStoffregen/Time).
 */

#include "Benchmark.h"

void setup() {
  delay(1000);
  Serial.begin(115200); // ESP8266 default of 74880 not supported on Linux
  while (!Serial); // Wait until Serial is ready - Leonardo/Micro

  runBenchmarks();
}

void loop() {
}
