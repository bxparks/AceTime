/*
 * A program to demonstrate the use of AceTime classes. It should print the
 * following on the Serial port:
 *
 *   America/Los_Angeles: 2019-06-01T11:38:00-07:00 Saturday [America/Los_Angeles]
 *   Epoch Seconds: 612729480
 *   Unix Seconds: 1559414280
 *   America/New_York: 2019-06-01T14:38:00-04:00 Saturday [America/New_York]
 *   Europe/Istanbul: 2019-06-01T21:38:00+03:00 Saturday [Europe/Istanbul]
 *   pacific.compareTo(turkey): 0
 *   pacific == turkey: false
 */

#include <AceTime.h>

using namespace ace_time;

// ZoneSpecifier instances should be created statically at initialization time.
static BasicZoneSpecifier pacificSpec(&zonedb::kZoneAmerica_Los_Angeles);
static BasicZoneSpecifier easternSpec(&zonedb::kZoneAmerica_New_York);
static ExtendedZoneSpecifier turkeySpec(&zonedbx::kZoneEurope_Istanbul);

void setup() {
#if defined(ARDUINO)
  delay(1000);
#endif
  Serial.begin(115200); // ESP8266 default of 74880 not supported on Linux
  while (!Serial); // Wait until Serial is ready - Leonardo/Micro

  auto pacificTz = TimeZone::forZoneSpecifier(&pacificSpec);
  auto easternTz = TimeZone::forZoneSpecifier(&easternSpec);
  auto turkeyTz = TimeZone::forZoneSpecifier(&turkeySpec);

  // Create from components
  auto pacificTime = ZonedDateTime::forComponents(
      2019, 6, 1, 11, 38, 0, pacificTz);

  Serial.print(F("America/Los_Angeles: "));
  pacificTime.printTo(Serial);
  Serial.println();

  Serial.print(F("Epoch Seconds: "));
  acetime_t epochSeconds = pacificTime.toEpochSeconds();
  Serial.println(epochSeconds);

  Serial.print(F("Unix Seconds: "));
  acetime_t unixSeconds = pacificTime.toUnixSeconds();
  Serial.println(unixSeconds);

  // Create from epoch seconds
  auto easternTime = ZonedDateTime::forEpochSeconds(epochSeconds, easternTz);

  Serial.print(F("America/New_York: "));
  easternTime.printTo(Serial);
  Serial.println();

  // Create by conversion to time zone
  auto turkeyTime = easternTime.convertToTimeZone(turkeyTz);

  Serial.print(F("Europe/Istanbul: "));
  turkeyTime.printTo(Serial);
  Serial.println();

  Serial.print(F("pacificTime.compareTo(turkeyTime): "));
  Serial.println(pacificTime.compareTo(turkeyTime));

  Serial.print(F("pacificTime == turkeyTime: "));
  Serial.println((pacificTime == turkeyTime) ? "true" : "false");

#ifndef ARDUINO
  exit(0);
#endif
}

void loop() {
}
