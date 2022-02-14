/*
 * Determine the execution time of functions which convert human-readable
 * date/time components to and from epochSeconds. We compare the equivalent
 * methods from the AceTime library to the Arduino Time library
 * (https://github.com/PaulStoffregen/Time).
 *
 * The Arduino Time library does not compile on EpoxyDuino.
 */

#include <Arduino.h>
#include "Benchmark.h"

// ESP32 does not define SERIAL_PORT_MONITOR
#if !defined(SERIAL_PORT_MONITOR)
#define SERIAL_PORT_MONITOR Serial
#endif

void setup() {
#if ! defined(EPOXY_DUINO)
  delay(1000);
#endif

  SERIAL_PORT_MONITOR.begin(115200);
  while (!SERIAL_PORT_MONITOR); // Wait until ready - Leonardo/Micro
#if defined(EPOXY_DUINO)
  SERIAL_PORT_MONITOR.setLineModeUnix();
#endif

  SERIAL_PORT_MONITOR.println(F("BENCHMARKS"));
  runBenchmarks();
  SERIAL_PORT_MONITOR.println(F("END"));

#if defined(EPOXY_DUINO)
  exit(0);
#endif
}

void loop() {}
