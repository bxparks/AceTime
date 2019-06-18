/*
 * A program to demonstrate the use of AceTime SystemClock. It should print the
 * following on the Serial port every 2 seconds:
 *
 *   Pacific Time: 2019-06-17T19:50:00-07:00 Monday [America/Los_Angeles]
 *   Eastern Time: 2019-06-17T22:50:00-04:00 Monday [America/New_York]
 *
 *   Pacific Time: 2019-06-17T19:50:02-07:00 Monday [America/Los_Angeles]
 *   Eastern Time: 2019-06-17T22:50:02-04:00 Monday [America/New_York]
 *
 *   Pacific Time: 2019-06-17T19:50:04-07:00 Monday [America/Los_Angeles]
 *   Eastern Time: 2019-06-17T22:50:04-04:00 Monday [America/New_York]
 *   ...
 */

#include <AceTime.h>

using namespace ace_time;
using namespace ace_time::clock;

// ZoneSpecifier instances should be created statically at initialization time.
static BasicZoneSpecifier pacificSpec(&zonedb::kZoneAmerica_Los_Angeles);
static BasicZoneSpecifier easternSpec(&zonedb::kZoneAmerica_New_York);

SystemClock systemClock(nullptr /*sync*/, nullptr /*backup*/);
SystemClockHeartbeatLoop systemClockHeartbeat(systemClock);

//------------------------------------------------------------------

void setup() {
  delay(1000);
  Serial.begin(115200); // ESP8266 default of 74880 not supported on Linux
  while (!Serial); // Wait until Serial is ready - Leonardo/Micro

  systemClock.setup();

  // Creating timezones is cheap, so we can create them on the fly as needed.
  auto pacificTz = TimeZone::forZoneSpecifier(&pacificSpec);

  // Set the SystemClock using these components.
  auto pacificTime = ZonedDateTime::forComponents(
      2019, 6, 17, 19, 50, 0, pacificTz);
  systemClock.setNow(pacificTime.toEpochSeconds());
}

//------------------------------------------------------------------

void printCurrentTime() {
  acetime_t now = systemClock.getNow();

  // Create Pacific Time and print.
  auto pacificTz = TimeZone::forZoneSpecifier(&pacificSpec);
  auto pacificTime = ZonedDateTime::forEpochSeconds(now, pacificTz);
  Serial.print(F("Pacific Time: "));
  pacificTime.printTo(Serial);
  Serial.println();

  // Convert to Eastern Time and print.
  auto easternTz = TimeZone::forZoneSpecifier(&easternSpec);
  auto easternTime = pacificTime.convertToTimeZone(easternTz);
  Serial.print(F("Eastern Time: "));
  easternTime.printTo(Serial);
  Serial.println();

  Serial.println();
}

void loop() {
  printCurrentTime();
  systemClockHeartbeat.loop(); // must call every 65s or less
  delay(2000);
}
