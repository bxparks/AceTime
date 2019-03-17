/*
 * A program to determine how long it takes to execute some of the more complex
 * methods of ZonedDateTime and LocalDate.
 *
 * This should compile on all microcontrollers supported by the Arduino IDE.
 */

#ifdef ARDUINO
#include <AceRoutine.h> // activate SystemTime coroutines
#endif
#include <AceTime.h>
#include "Benchmark.h"

using namespace ace_time;
using namespace ace_time::provider;

void setup() {
#if defined(ARDUINO)
  delay(1000);
#endif

  Serial.begin(115200); // ESP8266 default of 74880 not supported on Linux
  while (!Serial); // Wait until Serial is ready - Leonardo/Micro
  pinMode(LED_BENCHMARK, OUTPUT);

  // ace_time primitives

  Serial.print(F("sizeof(LocalDate): "));
  Serial.println(sizeof(LocalDate));

  Serial.print(F("sizeof(LocalTime): "));
  Serial.println(sizeof(LocalTime));

  Serial.print(F("sizeof(LocalDateTime): "));
  Serial.println(sizeof(LocalDateTime));

  Serial.print(F("sizeof(UtcOffset): "));
  Serial.println(sizeof(UtcOffset));

  Serial.print(F("sizeof(OffsetDateTime): "));
  Serial.println(sizeof(OffsetDateTime));

  Serial.print(F("sizeof(zonedb::ZoneEra): "));
  Serial.println(sizeof(zonedb::ZoneEra));

  Serial.print(F("sizeof(zonedb::ZoneInfo): "));
  Serial.println(sizeof(zonedb::ZoneInfo));

  Serial.print(F("sizeof(zonedb::ZoneRule): "));
  Serial.println(sizeof(zonedb::ZoneRule));

  Serial.print(F("sizeof(zonedb::ZonePolicy): "));
  Serial.println(sizeof(zonedb::ZonePolicy));

  Serial.print(F("sizeof(zonedb::Transition): "));
  Serial.println(sizeof(zonedb::Transition));

  Serial.print(F("sizeof(zonedbx::Transition): "));
  Serial.println(sizeof(zonedbx::Transition));

  Serial.print(F("sizeof(zonedbx::ZoneMatch): "));
  Serial.println(sizeof(zonedbx::ZoneMatch));

  Serial.print(F("sizeof(ZoneSpecifier): "));
  Serial.println(sizeof(ZoneSpecifier));

  Serial.print(F("sizeof(ManualZoneSpecifier): "));
  Serial.println(sizeof(ManualZoneSpecifier));

  Serial.print(F("sizeof(BasicZoneSpecifier): "));
  Serial.println(sizeof(BasicZoneSpecifier));

  Serial.print(F("sizeof(ExtendedZoneSpecifier): "));
  Serial.println(sizeof(ExtendedZoneSpecifier));

  Serial.print(F("sizeof(TimeZone): "));
  Serial.println(sizeof(TimeZone));

  Serial.print(F("sizeof(ZonedDateTime): "));
  Serial.println(sizeof(ZonedDateTime));

  Serial.print(F("sizeof(TimePeriod): "));
  Serial.println(sizeof(TimePeriod));

  // ace_time::provider classes

  Serial.print(F("sizeof(SystemTimeKeeper): "));
  Serial.println(sizeof(SystemTimeKeeper));

#ifdef ARDUINO
  Serial.print(F("sizeof(DS3231TimeKeeper): "));
  Serial.println(sizeof(DS3231TimeKeeper));

#if defined(ESP8266) || defined(ESP32)
  Serial.print(F("sizeof(NtpTimeProvider): "));
  Serial.println(sizeof(NtpTimeProvider));
#endif

  Serial.print(F("sizeof(SystemTimeSyncLoop): "));
  Serial.println(sizeof(SystemTimeSyncLoop));

  Serial.print(F("sizeof(SystemTimeHeartbeatLoop): "));
  Serial.println(sizeof(SystemTimeHeartbeatLoop));

  Serial.print(F("sizeof(SystemTimeSyncCoroutine): "));
  Serial.println(sizeof(SystemTimeSyncCoroutine));

  Serial.print(F("sizeof(SystemTimeHeartbeatCoroutine): "));
  Serial.println(sizeof(SystemTimeHeartbeatCoroutine));
#endif

  runBenchmarks();
#ifndef ARDUINO
  exit(0);
#endif
}

void loop() {
}
