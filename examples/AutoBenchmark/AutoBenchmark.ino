/*
 * A program to determine how long it takes to execute some of the more complex
 * methods of ZonedDateTime and LocalDate.
 *
 * This should compile on all microcontrollers supported by the Arduino IDE.
 */

#include <Arduino.h>
#include <AceRoutine.h> // activate SystemClock coroutines
#include <AceTime.h>
#include "Benchmark.h"

using namespace ace_time;

void setup() {
#if defined(ARDUINO)
  delay(1000);
#endif

  SERIAL_PORT_MONITOR.begin(115200); // ESP8266 default of 74880 not supported on Linux
  while (!SERIAL_PORT_MONITOR); // Wait until SERIAL_PORT_MONITOR is ready - Leonardo/Micro

  // ace_time primitives

  SERIAL_PORT_MONITOR.print(F("sizeof(LocalDate): "));
  SERIAL_PORT_MONITOR.println(sizeof(LocalDate));

  SERIAL_PORT_MONITOR.print(F("sizeof(LocalTime): "));
  SERIAL_PORT_MONITOR.println(sizeof(LocalTime));

  SERIAL_PORT_MONITOR.print(F("sizeof(LocalDateTime): "));
  SERIAL_PORT_MONITOR.println(sizeof(LocalDateTime));

  SERIAL_PORT_MONITOR.print(F("sizeof(TimeOffset): "));
  SERIAL_PORT_MONITOR.println(sizeof(TimeOffset));

  SERIAL_PORT_MONITOR.print(F("sizeof(OffsetDateTime): "));
  SERIAL_PORT_MONITOR.println(sizeof(OffsetDateTime));

  SERIAL_PORT_MONITOR.print(F("sizeof(BasicZoneProcessor): "));
  SERIAL_PORT_MONITOR.println(sizeof(BasicZoneProcessor));

  SERIAL_PORT_MONITOR.print(F("sizeof(ExtendedZoneProcessor): "));
  SERIAL_PORT_MONITOR.println(sizeof(ExtendedZoneProcessor));

  SERIAL_PORT_MONITOR.print(F("sizeof(BasicZoneRegistrar): "));
  SERIAL_PORT_MONITOR.println(sizeof(BasicZoneRegistrar));

  SERIAL_PORT_MONITOR.print(F("sizeof(ExtendedZoneRegistrar): "));
  SERIAL_PORT_MONITOR.println(sizeof(ExtendedZoneRegistrar));

  SERIAL_PORT_MONITOR.print(F("sizeof(BasicZoneManager<1>): "));
  SERIAL_PORT_MONITOR.println(sizeof(BasicZoneManager<1>));

  SERIAL_PORT_MONITOR.print(F("sizeof(ExtendedZoneManager<1>): "));
  SERIAL_PORT_MONITOR.println(sizeof(ExtendedZoneManager<1>));

  SERIAL_PORT_MONITOR.print(F("sizeof(TimeZoneData): "));
  SERIAL_PORT_MONITOR.println(sizeof(TimeZoneData));

  SERIAL_PORT_MONITOR.print(F("sizeof(TimeZone): "));
  SERIAL_PORT_MONITOR.println(sizeof(TimeZone));

  SERIAL_PORT_MONITOR.print(F("sizeof(ZonedDateTime): "));
  SERIAL_PORT_MONITOR.println(sizeof(ZonedDateTime));

  SERIAL_PORT_MONITOR.print(F("sizeof(TimePeriod): "));
  SERIAL_PORT_MONITOR.println(sizeof(TimePeriod));

  // ace_time::clock classes

  SERIAL_PORT_MONITOR.print(F("sizeof(clock::SystemClock): "));
  SERIAL_PORT_MONITOR.println(sizeof(clock::SystemClock));

#ifdef ARDUINO
  SERIAL_PORT_MONITOR.print(F("sizeof(clock::DS3231TimeKeeper): "));
  SERIAL_PORT_MONITOR.println(sizeof(clock::DS3231TimeKeeper));

#if defined(ESP8266) || defined(ESP32)
  SERIAL_PORT_MONITOR.print(F("sizeof(clock::NtpTimeProvider): "));
  SERIAL_PORT_MONITOR.println(sizeof(clock::NtpTimeProvider));
#endif
#endif

  SERIAL_PORT_MONITOR.print(F("sizeof(clock::SystemClockSyncLoop): "));
  SERIAL_PORT_MONITOR.println(sizeof(clock::SystemClockSyncLoop));

  SERIAL_PORT_MONITOR.print(F("sizeof(clock::SystemClockSyncCoroutine): "));
  SERIAL_PORT_MONITOR.println(sizeof(clock::SystemClockSyncCoroutine));

  // ace_time::basic and ace_time::extended classes

  SERIAL_PORT_MONITOR.print(F("sizeof(basic::ZoneContext): "));
  SERIAL_PORT_MONITOR.println(sizeof(basic::ZoneContext));

  SERIAL_PORT_MONITOR.print(F("sizeof(basic::ZoneEra): "));
  SERIAL_PORT_MONITOR.println(sizeof(basic::ZoneEra));

  SERIAL_PORT_MONITOR.print(F("sizeof(basic::ZoneInfo): "));
  SERIAL_PORT_MONITOR.println(sizeof(basic::ZoneInfo));

  SERIAL_PORT_MONITOR.print(F("sizeof(basic::ZoneRule): "));
  SERIAL_PORT_MONITOR.println(sizeof(basic::ZoneRule));

  SERIAL_PORT_MONITOR.print(F("sizeof(basic::ZonePolicy): "));
  SERIAL_PORT_MONITOR.println(sizeof(basic::ZonePolicy));

  SERIAL_PORT_MONITOR.print(F("sizeof(basic::Transition): "));
  SERIAL_PORT_MONITOR.println(sizeof(basic::Transition));

  SERIAL_PORT_MONITOR.print(F("sizeof(extended::Transition): "));
  SERIAL_PORT_MONITOR.println(sizeof(extended::Transition));

  SERIAL_PORT_MONITOR.print(F("sizeof(extended::ZoneMatch): "));
  SERIAL_PORT_MONITOR.println(sizeof(extended::ZoneMatch));

  runBenchmarks();
#ifndef ARDUINO
  exit(0);
#endif
}

void loop() {
}
