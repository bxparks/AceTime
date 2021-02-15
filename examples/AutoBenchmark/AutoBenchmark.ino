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
#if ! defined(EPOXY_DUINO)
  delay(1000);
#endif

  SERIAL_PORT_MONITOR.begin(115200);
  while (!SERIAL_PORT_MONITOR); // Wait until ready - Leonardo/Micro

  SERIAL_PORT_MONITOR.println(F("SIZEOF"));

  // print sizeof() various ace_time primitives

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

#if ! defined(EPOXY_DUINO)
  SERIAL_PORT_MONITOR.print(F("sizeof(clock::DS3231Clock): "));
  SERIAL_PORT_MONITOR.println(sizeof(clock::DS3231Clock));

  #if defined(ESP8266) || defined(ESP32)
    SERIAL_PORT_MONITOR.print(F("sizeof(clock::NtpClock): "));
    SERIAL_PORT_MONITOR.println(sizeof(clock::NtpClock));
  #endif
#endif

  SERIAL_PORT_MONITOR.print(F("sizeof(clock::SystemClock): "));
  SERIAL_PORT_MONITOR.println(sizeof(clock::SystemClock));

  SERIAL_PORT_MONITOR.print(F("sizeof(clock::SystemClockLoop): "));
  SERIAL_PORT_MONITOR.println(sizeof(clock::SystemClockLoop));

  SERIAL_PORT_MONITOR.print(F("sizeof(clock::SystemClockCoroutine): "));
  SERIAL_PORT_MONITOR.println(sizeof(clock::SystemClockCoroutine));

  // ace_time::basic and ace_time::extended classes

  SERIAL_PORT_MONITOR.print(F("sizeof(internal::ZoneContext): "));
  SERIAL_PORT_MONITOR.println(sizeof(internal::ZoneContext));

  SERIAL_PORT_MONITOR.print(F("sizeof(basic::ZoneEra): "));
  SERIAL_PORT_MONITOR.println(sizeof(basic::ZoneEra));

  SERIAL_PORT_MONITOR.print(F("sizeof(basic::ZoneInfo): "));
  SERIAL_PORT_MONITOR.println(sizeof(basic::ZoneInfo));

  SERIAL_PORT_MONITOR.print(F("sizeof(basic::ZoneRule): "));
  SERIAL_PORT_MONITOR.println(sizeof(basic::ZoneRule));

  SERIAL_PORT_MONITOR.print(F("sizeof(basic::ZonePolicy): "));
  SERIAL_PORT_MONITOR.println(sizeof(basic::ZonePolicy));

  SERIAL_PORT_MONITOR.print(F("sizeof(basic::ZoneRegistrar): "));
  SERIAL_PORT_MONITOR.println(sizeof(basic::ZoneRegistrar));

  SERIAL_PORT_MONITOR.print(F("sizeof(basic::LinkRegistrar): "));
  SERIAL_PORT_MONITOR.println(sizeof(basic::LinkRegistrar));

  SERIAL_PORT_MONITOR.print(F("sizeof(BasicZoneProcessor::Transition): "));
  SERIAL_PORT_MONITOR.println(sizeof(BasicZoneProcessor::Transition));

  SERIAL_PORT_MONITOR.print(F("sizeof(ExtendedZoneProcessor::Transition): "));
  SERIAL_PORT_MONITOR.println(sizeof(ExtendedZoneProcessor::Transition));

  SERIAL_PORT_MONITOR.print(
      F("sizeof(ExtendedZoneProcessor::TransitionStorage): "));
  SERIAL_PORT_MONITOR.println(sizeof(ExtendedZoneProcessor::TransitionStorage));

  SERIAL_PORT_MONITOR.print(F("sizeof(ExtendedZoneProcessor::ZoneMatch): "));
  SERIAL_PORT_MONITOR.println(sizeof(ExtendedZoneProcessor::ZoneMatch));

  SERIAL_PORT_MONITOR.println(F("BENCHMARKS"));
  runBenchmarks();
  SERIAL_PORT_MONITOR.println(F("END"));

#if defined(EPOXY_DUINO)
  exit(0);
#endif
}

void loop() {
}
