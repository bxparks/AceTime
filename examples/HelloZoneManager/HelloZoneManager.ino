/*
 * A program to demonstrate using the ZoneManager with the entire
 * zoneinfo database loaded, creating 3 timezones in 3 different ways
 * (createForZoneInfo(), createForZoneName() and createForZoneId()),
 * then displaying the time in those zones. Should print the following:
 *
 * Time: 2019-03-10T03:00:00-07:00[America/Los_Angeles]
 * Time: 2019-03-10T10:00:00+00:00[Europe/London]
 * Time: 2019-03-10T21:00:00+11:00[Australia/Sydney]
 */

#include <Arduino.h>
#include <AceTime.h>

using namespace ace_time;

void setup() {
#if defined(ARDUINO)
  delay(1000);
#endif
  SERIAL_PORT_MONITOR.begin(115200);
  while (!SERIAL_PORT_MONITOR); // Wait Serial is ready - Leonardo/Micro

  // Create a BasicZoneManager with the entire TZ Database.
  const int CACHE_SIZE = 3;
  BasicZoneManager<CACHE_SIZE> manager(
      zonedb::kZoneRegistrySize, zonedb::kZoneRegistry);

  // Create Los Angeles by ZoneInfo
  auto pacificTz = manager.createForZoneInfo(&zonedb::kZoneAmerica_Los_Angeles);
  auto pacificTime = ZonedDateTime::forComponents(
      2019, 3, 10, 3, 0, 0, pacificTz);
  SERIAL_PORT_MONITOR.print(F("Time: "));
  pacificTime.printTo(SERIAL_PORT_MONITOR);
  SERIAL_PORT_MONITOR.println();

  // Create London by ZoneName
  auto londonTz = manager.createForZoneName("Europe/London");
  auto londonTime = pacificTime.convertToTimeZone(londonTz);
  SERIAL_PORT_MONITOR.print(F("Time: "));
  londonTime.printTo(SERIAL_PORT_MONITOR);
  SERIAL_PORT_MONITOR.println();

  // Create Sydney by ZoneId
  uint32_t syndeyId = BasicZone(&zonedb::kZoneAustralia_Sydney).zoneId();
  auto sydneyTz = manager.createForZoneId(syndeyId);
  auto sydneyTime = pacificTime.convertToTimeZone(sydneyTz);
  SERIAL_PORT_MONITOR.print(F("Time: "));
  sydneyTime.printTo(SERIAL_PORT_MONITOR);
  SERIAL_PORT_MONITOR.println();

#if ! defined(ARDUINO)
  exit(0);
#endif
}

void loop() {
}
