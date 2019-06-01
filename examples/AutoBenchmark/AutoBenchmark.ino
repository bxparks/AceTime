/*
 * A program to determine how long it takes to execute some of the more complex
 * methods of ZonedDateTime and LocalDate.
 *
 * This should compile on all microcontrollers supported by the Arduino IDE.
 */

#include <AceRoutine.h> // activate SystemTime coroutines
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

  Serial.print(F("sizeof(TimeOffset): "));
  Serial.println(sizeof(TimeOffset));

  Serial.print(F("sizeof(OffsetDateTime): "));
  Serial.println(sizeof(OffsetDateTime));

  Serial.print(F("sizeof(basic::ZoneEra): "));
  Serial.println(sizeof(basic::ZoneEra));

  Serial.print(F("sizeof(basic::ZoneInfo): "));
  Serial.println(sizeof(basic::ZoneInfo));

  Serial.print(F("sizeof(basic::ZoneRule): "));
  Serial.println(sizeof(basic::ZoneRule));

  Serial.print(F("sizeof(basic::ZonePolicy): "));
  Serial.println(sizeof(basic::ZonePolicy));

  Serial.print(F("sizeof(basic::Transition): "));
  Serial.println(sizeof(basic::Transition));

  Serial.print(F("sizeof(extended::Transition): "));
  Serial.println(sizeof(extended::Transition));

  Serial.print(F("sizeof(extended::ZoneMatch): "));
  Serial.println(sizeof(extended::ZoneMatch));

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
