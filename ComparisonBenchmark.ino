/*
 * Determine the execution time of a round trip conversion from human-readable
 * date/time components, to secondsSinceEpoch, then back to human-readable
 * components. We compare this round trip for the AceTime library compared to
 * the Arduino Time library (https://github.com/PaulStoffregen/Time).
 */

#include "Benchmark.h"

void setup() {
  delay(1000);
  Serial.begin(115200); // ESP8266 default of 74880 not supported on Linux
  while (!Serial); // Wait until Serial is ready - Leonardo/Micro

  // Used to disable compiler optimization. The compiler is extremely good at
  // detecting code that does nothing and simply removing it. This will cause
  // the benchmark program to have an external side-effect, which prevents
  // optimization.
  pinMode(LED_BENCHMARK, OUTPUT);

  runBenchmarks();
}

void loop() {
}
