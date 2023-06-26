/*
 * Similar to examples/HelloZoneManager, but the ExtendedZoneManager is
 * configured with a custom zoneinfo database containing the major timezones of
 * the United States, instead of all ~600 zones and links in the TZDB. A sample
 * date-time of 2023-03-12 03:00 in Los Angeles is converted to each of those
 * timezones and the local time is printed.
 *
 * The output should look like:
 *
 * 2023-03-12T00:00:00-10:00[Pacific/Honolulu]
 * 2023-03-12T01:00:00-09:00[America/Anchorage]
 * 2023-03-12T03:00:00-07:00[America/Los_Angeles]
 * 2023-03-12T03:00:00-07:00[America/Phoenix]
 * 2023-03-12T04:00:00-06:00[America/Denver]
 * 2023-03-12T05:00:00-05:00[America/Chicago]
 * 2023-03-12T06:00:00-04:00[America/New_York]
 * 2023-03-12T10:00:00+00:00[UTC]
 *
 * Many places have already switched to DST summer time. Arizona (Phoenix) and
 * Hawaii (Honolulu) don't use DST. Alaska (Anchorage) is still on standard time
 * because it is only 1am there.
 */

#include <Arduino.h>
#include <AceTime.h>

using namespace ace_time;

// ESP32 does not define SERIAL_PORT_MONITOR
#ifndef SERIAL_PORT_MONITOR
#define SERIAL_PORT_MONITOR Serial
#endif

// Create a custom zone registry of the major timezones in the United States,
// plus UTC.
static const extended::ZoneInfo* const ZONE_REGISTRY[] ACE_TIME_PROGMEM = {
  &zonedbx::kZonePacific_Honolulu,
  &zonedbx::kZoneAmerica_Anchorage,
  &zonedbx::kZoneAmerica_Los_Angeles,
  &zonedbx::kZoneAmerica_Phoenix,
  &zonedbx::kZoneAmerica_Denver,
  &zonedbx::kZoneAmerica_Chicago,
  &zonedbx::kZoneAmerica_New_York,
  &zonedbx::kZoneUTC,
};

static const uint16_t ZONE_REGISTRY_SIZE =
    sizeof(ZONE_REGISTRY) / sizeof(basic::ZoneInfo*);

// Create an ExtendedZoneManager with just the major timezones in the United
// States. A CACHE_SIZE=2 means that 2 timezones can be concurrently active
// without suffering performance penalties.
static const int CACHE_SIZE = 2;
static ExtendedZoneProcessorCache<CACHE_SIZE> zoneProcessorCache;
static ExtendedZoneManager manager(
    ZONE_REGISTRY_SIZE, ZONE_REGISTRY, zoneProcessorCache);

void setup() {
#if ! defined(EPOXY_DUINO)
  delay(1000);
#endif

  SERIAL_PORT_MONITOR.begin(115200);
  while (!SERIAL_PORT_MONITOR); // Wait until ready - Leonardo/Micro
#if defined(EPOXY_DUINO)
  SERIAL_PORT_MONITOR.setLineModeUnix();
#endif

  // Create the base timezone, America/Los_Angeles. Select the time as the
  // moment just after the "spring forward" DST shift on Mar 12, 2023, when
  // 02:00 became 03:00.
  TimeZone baseTz = manager.createForZoneId(
      zonedbx::kZoneIdAmerica_Los_Angeles);
  ZonedDateTime baseTime = ZonedDateTime::forComponents(
      2023, 3, 12, 3, 0, 0, baseTz);

  // Loop over each zone in the custom registry
  for (uint16_t i = 0; i < manager.zoneRegistrySize(); i++) {
    TimeZone tz = manager.createForZoneIndex(i);
    if (tz.isError()) {
      SERIAL_PORT_MONITOR.print(
        F("Unable to create timezone for index "));
      SERIAL_PORT_MONITOR.println(i);
      continue;
    }

    // Convert to the target zone time. (This requires a cache of size 2 for
    // maximum performance.)
    ZonedDateTime zdt = baseTime.convertToTimeZone(tz);
    if (zdt.isError()) {
      SERIAL_PORT_MONITOR.print(F("Unable to convert to timezone "));
      tz.printTo(SERIAL_PORT_MONITOR);
      SERIAL_PORT_MONITOR.println();
      continue;
    }
      
    // Print the target date time.
    zdt.printTo(SERIAL_PORT_MONITOR);
    SERIAL_PORT_MONITOR.println();
  }

#if defined(EPOXY_DUINO)
  exit(0);
#endif
}

void loop() {
}
