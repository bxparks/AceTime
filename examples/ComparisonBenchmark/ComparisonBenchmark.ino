/*
 * Determine the execution time of functions which convert human-readable
 * date/time components to and from epochSeconds. We compare the equivalent
 * methods from the AceTime library to the Arduino Time library
 * (https://github.com/PaulStoffregen/Time).
 *
 * The Arduino Time library does not compile on EpoxyDuino.
 */

#include "Benchmark.h"

// ESP32 does not define SERIAL_PORT_MONITOR
#if !defined(SERIAL_PORT_MONITOR)
#define SERIAL_PORT_MONITOR Serial
#endif

void setup() {
  delay(1000);

  SERIAL_PORT_MONITOR.begin(115200);
  while (!SERIAL_PORT_MONITOR); // Wait until ready - Leonardo/Micro

  SERIAL_PORT_MONITOR.println(F("BENCHMARKS"));
  runBenchmarks();
  SERIAL_PORT_MONITOR.println(F("END"));
}

void loop() {}
