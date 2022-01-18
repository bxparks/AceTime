/*
 * Determine the execution time of a round trip conversion from human-readable
 * date/time components, to epochSeconds, then back to human-readable
 * components. We compare this round trip for the AceTime library compared to
 * the Arduino Time library (https://github.com/PaulStoffregen/Time).
 *
 * The Arduino Time library does not compile on EpoxyDuino.
 */

#include "Benchmark.h"

void setup() {
  delay(1000);

  SERIAL_PORT_MONITOR.begin(115200);
  while (!SERIAL_PORT_MONITOR); // Wait until ready - Leonardo/Micro

  runBenchmarks();
}

void loop() {}
