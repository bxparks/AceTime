/*
 * A program to demonstrate the use of the disambiguate parameter in
 * ZonedDateTime::forComponents(). It should print the following on the
 * SERIAL_PORT_MONITOR port:
 *
 * 2022-03-13T03:29:00-07:00[America/Los_Angeles]
 * resolved=4
 * 2022-03-13T01:29:00-08:00[America/Los_Angeles]
 * resolved=3
 */

#include <Arduino.h>
#include <AceTime.h>

using namespace ace_time;

// ESP32 does not define SERIAL_PORT_MONITOR
#ifndef SERIAL_PORT_MONITOR
#define SERIAL_PORT_MONITOR Serial
#endif

// ZoneProcessor instances should be created statically at initialization time.
static ExtendedZoneProcessor losAngelesProcessor;
static ExtendedZoneProcessor londonProcessor;

void setup() {
#if ! defined(EPOXY_DUINO)
  delay(1000);
#endif

  SERIAL_PORT_MONITOR.begin(115200);
  while (!SERIAL_PORT_MONITOR); // Wait until ready - Leonardo/Micro
#if defined(EPOXY_DUINO)
  SERIAL_PORT_MONITOR.setLineModeUnix();
#endif

  TimeZone tz = TimeZone::forZoneInfo(
      &zonedbx::kZoneAmerica_Los_Angeles,
      &losAngelesProcessor);

  // In the gap, kCompatible selects the later time, so should print:
  // 2022-03-13T03:29:00-07:00[America/Los_Angeles]
  // resolved=4 (kGapLater)
  auto dt = ZonedDateTime::forComponents(2022, 3, 13, 2, 29, 0, tz,
      Disambiguate::kCompatible);
  dt.printTo(SERIAL_PORT_MONITOR);

  SERIAL_PORT_MONITOR.println();
  SERIAL_PORT_MONITOR.print("resolved=");
  SERIAL_PORT_MONITOR.println((uint8_t) dt.resolved());

  // In the gap, kEarlier selects the earlier time, so should print:
  // 2022-03-13T01:29:00-08:00[America/Los_Angeles]
  // resolved=3 (kGapEarlier)
  dt = ZonedDateTime::forComponents(2022, 3, 13, 2, 29, 0, tz,
      Disambiguate::kEarlier);
  dt.printTo(SERIAL_PORT_MONITOR);
  SERIAL_PORT_MONITOR.println();
  SERIAL_PORT_MONITOR.print("resolved=");
  SERIAL_PORT_MONITOR.println((uint8_t) dt.resolved());

#if defined(EPOXY_DUINO)
  exit(0);
#endif
}

void loop() {
}
