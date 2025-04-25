/*
 * A program to determine how long it takes to execute some of the more complex
 * features of the AceTime library.
 *
 * This should compile on all microcontrollers supported by the Arduino IDE.
 */

#include <Arduino.h>
#include <AceTime.h>
#include "Benchmark.h"

using namespace ace_time;

void setup() {
#if ! defined(EPOXY_DUINO)
  delay(8000); // STM32, ESP8266, ESP32 now require > 5000ms instead of 1000ms
#endif

  SERIAL_PORT_MONITOR.begin(115200);
  while (!SERIAL_PORT_MONITOR); // Wait until ready - Leonardo/Micro
#if defined(EPOXY_DUINO)
  SERIAL_PORT_MONITOR.setLineModeUnix();
#endif

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

  SERIAL_PORT_MONITOR.print(F("sizeof(TimeZone): "));
  SERIAL_PORT_MONITOR.println(sizeof(TimeZone));

  SERIAL_PORT_MONITOR.print(F("sizeof(TimeZoneData): "));
  SERIAL_PORT_MONITOR.println(sizeof(TimeZoneData));

  SERIAL_PORT_MONITOR.print(F("sizeof(ZonedDateTime): "));
  SERIAL_PORT_MONITOR.println(sizeof(ZonedDateTime));

  SERIAL_PORT_MONITOR.print(F("sizeof(ZonedExtra): "));
  SERIAL_PORT_MONITOR.println(sizeof(ZonedExtra));

  SERIAL_PORT_MONITOR.print(F("sizeof(TimePeriod): "));
  SERIAL_PORT_MONITOR.println(sizeof(TimePeriod));

  // Basic

  SERIAL_PORT_MONITOR.println(F("Basic:"));

  SERIAL_PORT_MONITOR.print(F("  sizeof(basic::Info::ZoneContext): "));
  SERIAL_PORT_MONITOR.println(sizeof(basic::Info::ZoneContext));

  SERIAL_PORT_MONITOR.print(F("  sizeof(basic::Info::ZoneEra): "));
  SERIAL_PORT_MONITOR.println(sizeof(basic::Info::ZoneEra));

  SERIAL_PORT_MONITOR.print(F("  sizeof(basic::Info::ZoneInfo): "));
  SERIAL_PORT_MONITOR.println(sizeof(basic::Info::ZoneInfo));

  SERIAL_PORT_MONITOR.print(F("  sizeof(basic::Info::ZoneRule): "));
  SERIAL_PORT_MONITOR.println(sizeof(basic::Info::ZoneRule));

  SERIAL_PORT_MONITOR.print(F("  sizeof(basic::Info::ZonePolicy): "));
  SERIAL_PORT_MONITOR.println(sizeof(basic::Info::ZonePolicy));

  SERIAL_PORT_MONITOR.print(F("  sizeof(basic::ZoneRegistrar): "));
  SERIAL_PORT_MONITOR.println(sizeof(basic::ZoneRegistrar));

  SERIAL_PORT_MONITOR.print(F("  sizeof(BasicZoneProcessor): "));
  SERIAL_PORT_MONITOR.println(sizeof(BasicZoneProcessor));

  SERIAL_PORT_MONITOR.print(F("  sizeof(BasicZoneProcessorCache<1>): "));
  SERIAL_PORT_MONITOR.println(sizeof(BasicZoneProcessorCache<1>));

  SERIAL_PORT_MONITOR.print(F("  sizeof(BasicZoneManager): "));
  SERIAL_PORT_MONITOR.println(sizeof(BasicZoneManager));

  SERIAL_PORT_MONITOR.print(F("  sizeof(BasicZoneProcessor::Transition): "));
  SERIAL_PORT_MONITOR.println(sizeof(BasicZoneProcessor::Transition));

  // Extended

  SERIAL_PORT_MONITOR.println(F("Extended:"));

  SERIAL_PORT_MONITOR.print(F("  sizeof(extended::Info::ZoneContext): "));
  SERIAL_PORT_MONITOR.println(sizeof(extended::Info::ZoneContext));

  SERIAL_PORT_MONITOR.print(F("  sizeof(extended::Info::ZoneEra): "));
  SERIAL_PORT_MONITOR.println(sizeof(extended::Info::ZoneEra));

  SERIAL_PORT_MONITOR.print(F("  sizeof(extended::Info::ZoneInfo): "));
  SERIAL_PORT_MONITOR.println(sizeof(extended::Info::ZoneInfo));

  SERIAL_PORT_MONITOR.print(F("  sizeof(extended::Info::ZoneRule): "));
  SERIAL_PORT_MONITOR.println(sizeof(extended::Info::ZoneRule));

  SERIAL_PORT_MONITOR.print(F("  sizeof(extended::Info::ZonePolicy): "));
  SERIAL_PORT_MONITOR.println(sizeof(extended::Info::ZonePolicy));

  SERIAL_PORT_MONITOR.print(F("  sizeof(extended::ZoneRegistrar): "));
  SERIAL_PORT_MONITOR.println(sizeof(extended::ZoneRegistrar));

  SERIAL_PORT_MONITOR.print(F("  sizeof(ExtendedZoneProcessor): "));
  SERIAL_PORT_MONITOR.println(sizeof(ExtendedZoneProcessor));

  SERIAL_PORT_MONITOR.print(F("  sizeof(ExtendedZoneProcessorCache<1>): "));
  SERIAL_PORT_MONITOR.println(sizeof(ExtendedZoneProcessorCache<1>));

  SERIAL_PORT_MONITOR.print(F("  sizeof(ExtendedZoneManager): "));
  SERIAL_PORT_MONITOR.println(sizeof(ExtendedZoneManager));

  SERIAL_PORT_MONITOR.print(F("  sizeof(ExtendedZoneProcessor::Transition): "));
  SERIAL_PORT_MONITOR.println(sizeof(ExtendedZoneProcessor::Transition));

  SERIAL_PORT_MONITOR.print(
      F("  sizeof(ExtendedZoneProcessor::TransitionStorage): "));
  SERIAL_PORT_MONITOR.println(sizeof(ExtendedZoneProcessor::TransitionStorage));

  SERIAL_PORT_MONITOR.print(
      F("  sizeof(ExtendedZoneProcessor::MatchingEra): "));
  SERIAL_PORT_MONITOR.println(sizeof(ExtendedZoneProcessor::MatchingEra));

  // Complete

  SERIAL_PORT_MONITOR.println(F("Complete:"));

  SERIAL_PORT_MONITOR.print(F("  sizeof(complete::Info::ZoneContext): "));
  SERIAL_PORT_MONITOR.println(sizeof(complete::Info::ZoneContext));

  SERIAL_PORT_MONITOR.print(F("  sizeof(complete::Info::ZoneEra): "));
  SERIAL_PORT_MONITOR.println(sizeof(complete::Info::ZoneEra));

  SERIAL_PORT_MONITOR.print(F("  sizeof(complete::Info::ZoneInfo): "));
  SERIAL_PORT_MONITOR.println(sizeof(complete::Info::ZoneInfo));

  SERIAL_PORT_MONITOR.print(F("  sizeof(complete::Info::ZoneRule): "));
  SERIAL_PORT_MONITOR.println(sizeof(complete::Info::ZoneRule));

  SERIAL_PORT_MONITOR.print(F("  sizeof(complete::Info::ZonePolicy): "));
  SERIAL_PORT_MONITOR.println(sizeof(complete::Info::ZonePolicy));

  SERIAL_PORT_MONITOR.print(F("  sizeof(complete::ZoneRegistrar): "));
  SERIAL_PORT_MONITOR.println(sizeof(complete::ZoneRegistrar));

  SERIAL_PORT_MONITOR.print(F("  sizeof(CompleteZoneProcessor): "));
  SERIAL_PORT_MONITOR.println(sizeof(CompleteZoneProcessor));

  SERIAL_PORT_MONITOR.print(F("  sizeof(CompleteZoneProcessorCache<1>): "));
  SERIAL_PORT_MONITOR.println(sizeof(CompleteZoneProcessorCache<1>));

  SERIAL_PORT_MONITOR.print(F("  sizeof(CompleteZoneManager): "));
  SERIAL_PORT_MONITOR.println(sizeof(CompleteZoneManager));

  SERIAL_PORT_MONITOR.print(F("  sizeof(CompleteZoneProcessor::Transition): "));
  SERIAL_PORT_MONITOR.println(sizeof(CompleteZoneProcessor::Transition));

  SERIAL_PORT_MONITOR.print(
      F("  sizeof(CompleteZoneProcessor::TransitionStorage): "));
  SERIAL_PORT_MONITOR.println(sizeof(CompleteZoneProcessor::TransitionStorage));

  SERIAL_PORT_MONITOR.print(
      F("  sizeof(CompleteZoneProcessor::MatchingEra): "));
  SERIAL_PORT_MONITOR.println(sizeof(CompleteZoneProcessor::MatchingEra));

  // ace_time::basic and ace_time::extended classes

  SERIAL_PORT_MONITOR.println(F("BENCHMARKS"));
  runBenchmarks();
  SERIAL_PORT_MONITOR.println(F("END"));

#if defined(EPOXY_DUINO)
  exit(0);
#endif
}

void loop() {
}
